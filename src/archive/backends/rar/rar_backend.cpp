// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rar_backend.h"

#include "cli_long_extraction.h"
#include "path_guard.h"
#include "argument_chunks.h"
#include "staging_merge.h"
#include "write_progress.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QVector>

#include <algorithm>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <memory>

namespace dcarchive {
namespace {

ArchiveInfo parseTechnicalListing(const QString &output)
{
    ArchiveInfo info;
    QHash<QString, int> splitEntries;
    ArchiveEntry current;
    bool hasCurrent = false;
    bool inComment = false;
    QString ratio;
    QStringList comment;

    auto flush = [&]() {
        if (hasCurrent) {
            const int previousIndex = splitEntries.value(current.path, -1);
            const bool continued = ratio.startsWith(QLatin1String("<-"));
            if (continued && previousIndex >= 0) {
                info.entries[previousIndex].compressedSize += current.compressedSize;
            } else {
                current.index = info.entries.size();
                info.totalSize += current.size;
                info.entries.append(current);
            }
            if (ratio.endsWith(QLatin1String("->")))
                splitEntries.insert(current.path, continued && previousIndex >= 0 ? previousIndex : current.index);
            else
                splitEntries.remove(current.path);
            info.compressedSize += current.compressedSize;
        }
        current = ArchiveEntry();
        hasCurrent = false;
        ratio.clear();
    };

    const QStringList lines = output.split(QLatin1Char('\n'));
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line == QLatin1String("Archive comment:") && !hasCurrent) {
            inComment = true;
            continue;
        }
        if (inComment && line.startsWith(QLatin1String("Archive:"))) {
            inComment = false;
            if (info.comment.isEmpty())
                info.comment = comment.join(QLatin1Char('\n')).trimmed();
            comment.clear();
        } else if (inComment) {
            comment.append(rawLine);
            continue;
        }
        const int separator = line.indexOf(QLatin1Char(':'));
        if (separator <= 0)
            continue;
        const QString key = line.left(separator).trimmed();
        const QString value = line.mid(separator + 1).trimmed();

        if (key == QLatin1String("Details")) {
            flush();
            info.encrypted = info.encrypted || value.contains(QLatin1String("encrypted headers"));
        } else if (key == QLatin1String("Name")) {
            flush();
            current.name = QFileInfo(value).fileName();
            current.path = value;
            hasCurrent = true;
        } else if (!hasCurrent) {
            continue;
        } else if (key == QLatin1String("Type")) {
            current.type = value.contains(QLatin1String("symbolic"), Qt::CaseInsensitive)
                    ? EntryType::Symlink
                    : (value == QLatin1String("Directory") ? EntryType::Directory : EntryType::File);
        } else if (key.compare(QLatin1String("Symbolic link"), Qt::CaseInsensitive) == 0
                   || key.compare(QLatin1String("Target"), Qt::CaseInsensitive) == 0) {
            current.type = EntryType::Symlink;
            current.linkTarget = value;
        } else if (key == QLatin1String("Size")) {
            current.size = value.toLongLong();
        } else if (key == QLatin1String("Packed size")) {
            current.compressedSize = value.toLongLong();
        } else if (key == QLatin1String("Ratio")) {
            ratio = value;
        } else if (key == QLatin1String("mtime") && value.size() >= 19) {
            current.modified = QDateTime::fromString(value.left(19),
                                                     QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        }
    }
    flush();
    return info;
}

} // namespace

RarBackend::RarBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
}

RarBackend::~RarBackend() = default;

BackendCapabilities RarBackend::capabilities() const
{
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    const bool canWrite = !QStandardPaths::findExecutable(QStringLiteral("rar")).isEmpty();
    caps.create = canWrite;
    caps.append = canWrite;
    caps.remove = canWrite;
    caps.rename = canWrite;
    caps.comment = canWrite;
    caps.update = true;
    caps.password = true;
    caps.headerPassword = true;
    caps.multiVolume = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;
    return caps;
}

VoidResult RarBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_password = request.password;
    m_cancelled = false;
    return {};
}

RarBackend::RunResult RarBackend::runTool(const QString &tool, const QStringList &arguments,
                                          const OutputCallback &onOutput,
                                          const QString &workingDirectory)
{
    RunResult result;
    const QString executable = QStandardPaths::findExecutable(tool);
    if (executable.isEmpty()) {
        result.errors = QStringLiteral("%1 executable not found").arg(tool);
        return result;
    }

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C.UTF-8"));
    process.setProcessEnvironment(environment);
    if (!workingDirectory.isEmpty())
        process.setWorkingDirectory(workingDirectory);
    process.start(executable, arguments);
    if (!process.waitForStarted()) {
        result.errors = process.errorString();
        return result;
    }
    process.closeWriteChannel();
    m_processId = process.processId();
    result.started = true;
    QByteArray output;
    QByteArray errors;
    const auto readOutput = [&]() {
        const auto stdoutChunk = process.readAllStandardOutput();
        const auto stderrChunk = process.readAllStandardError();
        output += stdoutChunk;
        errors += stderrChunk;
        if (onOutput && (!stdoutChunk.isEmpty() || !stderrChunk.isEmpty()))
            onOutput(QString::fromUtf8(stdoutChunk + stderrChunk));
    };
    while (process.state() != QProcess::NotRunning) {
        if (m_cancelled)
            process.kill();
        process.waitForFinished(50);
        readOutput();
    }
    m_processId = 0;
    if (process.exitStatus() == QProcess::NormalExit)
        result.exitCode = process.exitCode();
    readOutput();
    result.output = QString::fromUtf8(output);
    result.errors = QString::fromUtf8(errors);
    return result;
}

RarBackend::RunResult RarBackend::runReadCommand(QStringList options, const QStringList &entries,
                                                const OutputCallback &onOutput,
                                                const QString &archivePath)
{
    options << QStringLiteral("-cfg-") << QStringLiteral("-@-")
            << (m_password.isEmpty() ? QStringLiteral("-p-") : QStringLiteral("-p") + m_password)
            << QStringLiteral("--") << (archivePath.isEmpty() ? m_path : archivePath);
    options += entries;
    return runTool(QStringLiteral("unrar"), options, onOutput);
}

RarBackend::RunResult RarBackend::runWriteCommand(QStringList options, const QStringList &entries,
                                                 bool encryptHeaders,
                                                 const QString &workingDirectory,
                                                 const QString &archivePath,
                                                 const OutputCallback &onOutput)
{
    options << QStringLiteral("-cfg-") << QStringLiteral("-@-");
    if (!m_password.isEmpty())
        options << (encryptHeaders ? QStringLiteral("-hp") : QStringLiteral("-p")) + m_password;
    options << QStringLiteral("--") << (archivePath.isEmpty() ? m_path : archivePath);
    options += entries;
    return runTool(QStringLiteral("rar"), options, onOutput, workingDirectory);
}

ArchiveResult RarBackend::resultForRun(const RunResult &run, ArchiveError fallback) const
{
    ArchiveResult result;
    if (m_cancelled) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    } else if (!run.started) {
        result.error = ArchiveError::ToolMissing;
    } else if (run.exitCode != 0) {
        const QString errors = run.errors.toLower();
        if (run.exitCode == 11 || errors.contains(QLatin1String("incorrect password"))
                || errors.contains(QLatin1String("password is incorrect"))
                || errors.contains(QLatin1String("password incorrect"))) {
            result.error = m_password.isEmpty() ? ArchiveError::PasswordRequired : ArchiveError::WrongPassword;
        } else {
            result.error = fallback;
        }
    }
    if (!result.ok())
        result.message = run.errors.isEmpty() ? run.output : run.errors;
    return result;
}

EntryResult RarBackend::list()
{
    const RunResult run = runReadCommand({QStringLiteral("lt"), QStringLiteral("-v")});
    EntryResult result;
    result.result = resultForRun(run, ArchiveError::CorruptArchive);
    if (result.result.ok())
        result.info = parseTechnicalListing(run.output);
    return result;
}

VoidResult RarBackend::testArchive()
{
    return {resultForRun(runReadCommand({QStringLiteral("t"), QStringLiteral("-c-")}),
                         ArchiveError::CorruptArchive)};
}

VoidResult RarBackend::extractFiles(const ExtractRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (!QDir().mkpath(request.targetPath)) {
        result.error = ArchiveError::PermissionDenied;
        notifyFinished(observer, result);
        return {result};
    }

    EntryResult listed;
    for (;;) {
        listed = list();
        if (listed.result.error != ArchiveError::PasswordRequired
                && listed.result.error != ArchiveError::WrongPassword)
            break;
        PasswordRequest password;
        observer.onPasswordRequired(password);
        if (m_cancelled || password.response == PasswordRequest::Response::Cancel) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            notifyFinished(observer, result);
            return {result};
        }
        if (password.response != PasswordRequest::Response::Provide || password.password.isEmpty())
            break;
        m_password = password.password;
    }
    if (!listed.result.ok()) {
        notifyFinished(observer, listed.result);
        return {listed.result};
    }

    QSet<int> selected;
    QVector<ArchiveEntry> selectedDirectories;
    for (const ArchiveEntry &entry : request.entries)
        selected.insert(entry.index);
    for (const ArchiveEntry &entry : request.entries) {
        if (entry.type == EntryType::Directory)
            selectedDirectories.append(entry);
    }
    QVector<ArchiveEntry> includedEntries;
    for (const ArchiveEntry &entry : listed.info.entries) {
        if (request.allEntries || selected.contains(entry.index)) {
            includedEntries.append(entry);
            continue;
        }
        for (const ArchiveEntry &directory : selectedDirectories) {
            if (archivePathWithin(entry.path, directory.path)) {
                includedEntries.append(entry);
                break;
            }
        }
    }

    const bool hasLongPaths = std::any_of(includedEntries.cbegin(), includedEntries.cend(),
                                         [](const ArchiveEntry &entry) { return hasOversizedComponent(entry.path); });
    std::unique_ptr<CliLongExtraction> longPaths;
    if (hasLongPaths && !QStandardPaths::findExecutable(QStringLiteral("rar")).isEmpty()) {
        longPaths.reset(new CliLongExtraction(request.targetPath, listed.info.entries));
        if (request.allEntries)
            std::stable_sort(includedEntries.begin(), includedEntries.end(),
                             [](const auto &a, const auto &b) { return a.path < b.path; });
    }

    bool overwriteAll = request.overwrite == OverwritePolicy::Overwrite
            || request.overwrite == OverwritePolicy::OverwriteAll;
    bool skipAll = request.overwrite == OverwritePolicy::Skip
            || request.overwrite == OverwritePolicy::SkipAll;
    QVector<ArchiveEntry> acceptedEntries;
    QStringList entries;
    for (const ArchiveEntry &entry : includedEntries) {
        const QString relativePath = archivePathRelativeTo(entry.path, request.destinationPrefix);
        QString outputPath = safeOutputPath(request.targetPath, relativePath);
        if (longPaths && !outputPath.isEmpty()) {
            const QString diskName = longPaths->diskName(relativePath, entry.type == EntryType::Directory);
            if (diskName != relativePath) {
                if (diskName.isEmpty()) {
                    observer.onCurrentFile(entry.path);
                    result = {ArchiveError::LongName, entry.path};
                    notifyFinished(observer, result);
                    return {result};
                }
                outputPath = safeOutputPath(request.targetPath, diskName);
            }
        }
        if (outputPath.isEmpty()) {
            result.error = ArchiveError::PermissionDenied;
            result.message = entry.path;
            notifyFinished(observer, result);
            return {result};
        }
        if (entry.type == EntryType::Symlink
                && !symlinkTargetWithinRoot(request.targetPath, outputPath, entry.linkTarget)) {
            result.error = ArchiveError::PermissionDenied;
            result.message = entry.path;
            notifyFinished(observer, result);
            return {result};
        }
        if (entry.type == EntryType::Directory) {
            if (longPaths)
                longPaths->addDirectory(outputPath);
            continue;
        }
        const bool exists = QFileInfo::exists(outputPath)
                || (longPaths && longPaths->hasFileDestination(outputPath));
        if (exists && skipAll)
            continue;
        if (exists && !overwriteAll) {
            FileConflictRequest conflict;
            conflict.path = outputPath;
            observer.onFileConflict(conflict);
            switch (conflict.response) {
            case FileConflictRequest::Response::Cancel:
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                notifyFinished(observer, result);
                return {result};
            case FileConflictRequest::Response::Skip:
                continue;
            case FileConflictRequest::Response::SkipAll:
                skipAll = true;
                continue;
            case FileConflictRequest::Response::OverwriteAll:
                overwriteAll = true;
                break;
            case FileConflictRequest::Response::Overwrite:
                break;
            }
        }
        if (longPaths) {
            entries << longPaths->addFile(entry, outputPath);
        } else {
            acceptedEntries.append(entry);
            entries << entry.path;
        }
    }
    if (!longPaths) {
        for (const ArchiveEntry &entry : includedEntries) {
            if (entry.type != EntryType::Directory)
                continue;
            bool hasDescendant = false;
            for (const ArchiveEntry &candidate : listed.info.entries) {
                if (candidate.index != entry.index && archivePathWithin(candidate.path, entry.path)) {
                    hasDescendant = true;
                    break;
                }
            }
            if (!hasDescendant) {
                acceptedEntries.append(entry);
                entries << entry.path;
            }
        }
    }
    if (entries.isEmpty() && !(longPaths && longPaths->hasOutput())) {
        if (longPaths && longPaths->shortened())
            result.warning = ArchiveError::LongName;
        notifyFinished(observer, result);
        return {result};
    }

    std::unique_ptr<QTemporaryDir> staging;
    QString extractionTarget = request.targetPath;
    QString workingArchive;
    if (longPaths) {
        result = longPaths->prepare(m_path, m_cancelled, [&](const QString &archive, const QStringList &pairs) {
            return resultForRun(runWriteCommand({QStringLiteral("rn"), QStringLiteral("-y")},
                                                pairs, false, {}, archive), ArchiveError::LongName);
        });
        if (!result.ok()) {
            if (result.error == ArchiveError::LongName)
                observer.onCurrentFile(result.message);
            notifyFinished(observer, result);
            return {result};
        }
        extractionTarget = longPaths->targetPath();
        workingArchive = longPaths->archivePath();
    } else if (!request.destinationPrefix.isEmpty()) {
        staging.reset(new QTemporaryDir);
        if (!staging->isValid()) {
            result.error = ArchiveError::PermissionDenied;
            notifyFinished(observer, result);
            return {result};
        }
        extractionTarget = staging->path();
    }
    QStringList switches{QStringLiteral("x"), QStringLiteral("-idp")};
    switches << QStringLiteral("-o+");
    QVector<QStringList> chunks = splitArgumentChunks(entries);
    QTemporaryFile selectionFile;
    if (chunks.size() > 1 && writeArgumentList(selectionFile, entries, true)) {
        switches << QStringLiteral("-n@") + selectionFile.fileName();
        chunks = {QStringList{}};
    }
    for (int chunkIndex = 0; chunkIndex < chunks.size(); ++chunkIndex) {
        int lastPercentage = -1;
        const auto reportProgress = [&](const QString &output) {
            static const QRegularExpression percentage(QStringLiteral("(\\d{1,3})%"));
            QRegularExpressionMatchIterator it = percentage.globalMatch(output);
            while (it.hasNext()) {
                const int value = it.next().captured(1).toInt();
                if (value < 0 || value > 100 || value <= lastPercentage)
                    continue;
                lastPercentage = value;
                ProgressInfo progress;
                progress.percentage = (chunkIndex * 100.0 + value) / chunks.size();
                observer.onProgress(progress);
            }
        };
        QStringList chunk = chunks.at(chunkIndex);
        chunk << QDir(extractionTarget).absolutePath() + QLatin1Char('/');
        for (;;) {
            const RunResult run = runReadCommand(switches, chunk, reportProgress, workingArchive);
            result = resultForRun(run, ArchiveError::Unknown);
            if (result.error == ArchiveError::Unknown) {
                const QStringList lines = (run.errors + QLatin1Char('\n') + run.output).split(QLatin1Char('\n'));
                for (qsizetype i = 0; i < lines.size(); ++i) {
                    const auto prefix = QLatin1String("Cannot create ");
                    const QString line = lines.at(i).trimmed();
                    if (!line.startsWith(prefix))
                        continue;
                    const QString path = line.mid(prefix.size());
                    if (!hasOversizedComponent(path) && (i + 1 == lines.size()
                            || lines.at(i + 1).trimmed() != QLatin1String("File name too long")))
                        continue;
                    result.error = ArchiveError::LongName;
                    result.message = archivePathRelativeTo(path, QDir(extractionTarget).absolutePath());
                    observer.onCurrentFile(result.message);
                    break;
                }
            }
            if (result.error != ArchiveError::PasswordRequired
                    && result.error != ArchiveError::WrongPassword)
                break;
            PasswordRequest password;
            observer.onPasswordRequired(password);
            if (m_cancelled || password.response == PasswordRequest::Response::Cancel) {
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                break;
            }
            if (password.response != PasswordRequest::Response::Provide || password.password.isEmpty())
                break;
            m_password = password.password;
        }
        if (!result.ok())
            break;
    }
    if (result.ok() && longPaths) {
        result = longPaths->publish(request.reportOutputPath, m_cancelled);
    } else if (result.ok() && staging) {
        mergeStagedEntries(staging->path(), request.targetPath,
                           acceptedEntries, request.destinationPrefix, result);
    }
    if (result.ok()) {
        if (longPaths && longPaths->shortened())
            result.warning = ArchiveError::LongName;
        ProgressInfo progress;
        progress.percentage = 100.0;
        observer.onProgress(progress);
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult RarBackend::addFiles(const AddRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (request.entries.isEmpty()) {
        result.error = ArchiveError::NotFound;
    } else if (request.encryptList && request.password.isEmpty() && m_password.isEmpty()) {
        result.error = ArchiveError::PasswordRequired;
    } else {
        QStringList switches{QStringLiteral("a"), QStringLiteral("-ep1")};
        if (request.compressionLevel >= 0 && request.compressionLevel <= 5)
            switches << QStringLiteral("-m%1").arg(request.compressionLevel);
        if (request.cpuThreads > 1)
            switches << QStringLiteral("-mt%1").arg(request.cpuThreads);
        if (request.splitSize > 0)
            switches << QStringLiteral("-v%1b").arg(request.splitSize);
        if (!request.password.isEmpty())
            m_password = request.password;
        QTemporaryDir staging;
        QStringList inputs;
        QString workingDirectory;
        if (!request.destinationPath.isEmpty()) {
            if (!staging.isValid()) {
                result.error = ArchiveError::PermissionDenied;
                notifyFinished(observer, result);
                return {result};
            }
            const QString dataRoot = QDir(staging.path()).filePath(QStringLiteral("data"));
            QStringList stagedNames;
            if (!stageLocalEntries(request.entries, request.destinationPath, dataRoot,
                                   &stagedNames, result)) {
                notifyFinished(observer, result);
                return {result};
            }
            QString destinationRoot = normalizedArchivePath(request.destinationPath);
            while (destinationRoot.endsWith(QLatin1Char('/')))
                destinationRoot.chop(1);
            for (const QString &name : stagedNames)
                inputs << destinationRoot + QLatin1Char('/') + name;
            workingDirectory = dataRoot;
        } else {
            for (const ArchiveEntry &entry : request.entries)
                inputs << QFileInfo(entry.path).absoluteFilePath();
        }
        const QFileInfo destination(m_path);
        QTemporaryDir writeDirectory(QDir(destination.absolutePath()).filePath(QStringLiteral(".dc-rar-XXXXXX")));
        QString writePath = m_path;
        if (request.splitSize <= 0) {
            writeDirectory.setAutoRemove(true);
            if (!writeDirectory.isValid()) {
                result.error = ArchiveError::PermissionDenied;
                notifyFinished(observer, result);
                return {result};
            }
            writePath = QDir(writeDirectory.path()).filePath(QFileInfo(m_path).fileName());
            if (QFileInfo::exists(m_path) && !QFile::copy(m_path, writePath)) {
                result.error = ArchiveError::PermissionDenied;
                notifyFinished(observer, result);
                return {result};
            }
        }
        if (workingDirectory.isEmpty()) {
            QStringList excluded = request.excludedSourcePaths;
            if (writeDirectory.isValid())
                excluded << writeDirectory.path();
            excluded << m_path;
            for (const QString &path : relativeSourceExclusions(inputs, excluded))
                switches << QStringLiteral("-x") + path;
        }
        WriteProgress progress(observer);
        const auto reportProgress = [&](const QString &output) { progress.toolOutput(output); };
        result = resultForRun(runWriteCommand(switches, inputs, request.encryptList,
                                              workingDirectory, writePath, reportProgress),
                              ArchiveError::Unknown);
        if (request.splitSize <= 0) {
            if (!result.ok()) {
                QFile::remove(writePath);
            } else if (::rename(QFile::encodeName(writePath).constData(),
                                QFile::encodeName(m_path).constData()) != 0) {
                result.error = ArchiveError::PermissionDenied;
                result.message = QString::fromLocal8Bit(std::strerror(errno));
                QFile::remove(writePath);
            }
        }
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult RarBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("move is not supported by RAR backend")};
}

VoidResult RarBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("copy is not supported by RAR backend")};
}

VoidResult RarBackend::deleteFiles(const DeleteRequest &request)
{
    QStringList entries;
    for (const ArchiveEntry &entry : request.entries)
        entries << entry.path;
    if (entries.isEmpty())
        return {};
    return {resultForRun(runWriteCommand({QStringLiteral("d")}, entries), ArchiveError::Unknown)};
}

VoidResult RarBackend::renameFiles(const RenameRequest &request)
{
    if (request.entry.path.isEmpty() || request.newName.isEmpty())
        return {ArchiveError::NotSupported, QStringLiteral("RAR rename requires both entry names")};
    return {resultForRun(runWriteCommand({QStringLiteral("rn")}, {request.entry.path, request.newName}),
                         ArchiveError::Unknown)};
}

VoidResult RarBackend::addComment(const QString &comment)
{
    QTemporaryFile commentFile;
    const QByteArray bytes = comment.toUtf8();
    if (!commentFile.open() || commentFile.write(bytes) != bytes.size() || !commentFile.flush())
        return {ArchiveError::PermissionDenied, commentFile.errorString()};
    commentFile.close();
    return {resultForRun(runWriteCommand({QStringLiteral("c"), QStringLiteral("-z") + commentFile.fileName()}),
                         ArchiveError::Unknown)};
}

VoidResult RarBackend::updateArchiveData(const UpdateRequest &)
{
    const EntryResult listed = list();
    return {listed.result};
}

void RarBackend::pause()
{
    m_paused = true;
    const qint64 processId = m_processId.load();
    if (processId > 0)
        ::kill(pid_t(processId), SIGSTOP);
}

void RarBackend::resume()
{
    m_paused = false;
    const qint64 processId = m_processId.load();
    if (processId > 0)
        ::kill(pid_t(processId), SIGCONT);
}

void RarBackend::cancel()
{
    m_cancelled = true;
    const qint64 processId = m_processId.load();
    if (processId > 0)
        ::kill(pid_t(processId), SIGKILL);
}

void RarBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
