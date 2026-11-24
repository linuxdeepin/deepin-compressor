// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sevenzip_backend.h"

#include "path_guard.h"
#include "argument_chunks.h"
#include "staging_merge.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QVector>

#include <csignal>
#include <cerrno>
#include <cstring>

namespace dcarchive {
namespace {

QVector<ArchiveEntry> parseSltOutput(const QString &output, bool includeTimes)
{
    QVector<ArchiveEntry> entries;
    bool inEntries = false;
    QMap<QString, QString> fields;
    const QTimeZone localZone = includeTimes ? QTimeZone::systemTimeZone() : QTimeZone();

    auto flush = [&]() {
        if (fields.isEmpty())
            return;
        if (fields.contains(QStringLiteral("Path"))) {
            ArchiveEntry entry;
            entry.path = fields.value(QStringLiteral("Path"));
            entry.name = QFileInfo(entry.path).fileName();
            entry.size = fields.value(QStringLiteral("Size"), QStringLiteral("0")).toLongLong();
            entry.compressedSize = fields.value(QStringLiteral("Packed Size"),
                                                QStringLiteral("0")).toLongLong();
            const QString modified = fields.value(QStringLiteral("Modified"));
            if (includeTimes && modified.size() >= 19)
                entry.modified = QDateTime(QDate::fromString(modified.left(10), Qt::ISODate),
                                           QTime::fromString(modified.mid(11, 8), Qt::ISODate), localZone);
            const QString attributes = fields.value(QStringLiteral("Attributes"));
            const QString linkTarget = fields.value(QStringLiteral("Symbolic Link"));
            if (!linkTarget.isEmpty()) {
                entry.type = EntryType::Symlink;
                entry.linkTarget = linkTarget;
            } else {
                entry.type = (entry.path.endsWith(QLatin1Char('/'))
                              || attributes.startsWith(QLatin1Char('D')))
                        ? EntryType::Directory : EntryType::File;
            }
            entry.index = entries.size();
            entries.append(entry);
        }
        fields.clear();
    };

    const QStringList lines = output.split(QLatin1Char('\n'));
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line == QLatin1String("----------")) {
            inEntries = true;
            continue;
        }
        if (!inEntries)
            continue;
        if (line.isEmpty()) {
            flush();
            continue;
        }
        const int separator = line.indexOf(QStringLiteral(" = "));
        if (separator > 0)
            fields.insert(line.left(separator).trimmed(), line.mid(separator + 3).trimmed());
    }
    flush();
    return entries;
}

} // namespace

SevenZipBackend::SevenZipBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
}

SevenZipBackend::~SevenZipBackend() = default;

BackendCapabilities SevenZipBackend::capabilities() const
{
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.create = true;
    caps.remove = true;
    caps.rename = true;
    caps.update = true;
    caps.password = true;
    caps.headerPassword = true;
    caps.multiVolume = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;
    return caps;
}

VoidResult SevenZipBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_password = request.password;
    m_cancelled = false;
    return {};
}

SevenZipBackend::RunResult SevenZipBackend::run7z(const QStringList &arguments,
                                                  const OutputCallback &onOutput,
                                                  const QString &workingDirectory)
{
    RunResult result;
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("7z"));
    if (executable.isEmpty()) {
        result.output = QStringLiteral("7z executable not found");
        return result;
    }

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
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
    while (process.state() != QProcess::NotRunning) {
        if (m_cancelled)
            process.kill();
        process.waitForFinished(50);
        const QString stdoutChunk = QString::fromUtf8(process.readAllStandardOutput());
        const QString stderrChunk = QString::fromUtf8(process.readAllStandardError());
        result.output += stdoutChunk;
        result.errors += stderrChunk;
        if (onOutput && (!stdoutChunk.isEmpty() || !stderrChunk.isEmpty()))
            onOutput(stdoutChunk + stderrChunk);
    }
    m_processId = 0;
    if (process.exitStatus() == QProcess::NormalExit)
        result.exitCode = process.exitCode();
    result.output += QString::fromUtf8(process.readAllStandardOutput());
    result.errors += QString::fromUtf8(process.readAllStandardError());
    return result;
}

SevenZipBackend::RunResult SevenZipBackend::runArchiveCommand(QStringList switches,
                                                            const QStringList &entries,
                                                            const OutputCallback &onOutput,
                                                            const QString &workingDirectory,
                                                            const QString &archivePath)
{
    if (!m_password.isEmpty())
        switches << QStringLiteral("-p") + m_password;
    switches << QStringLiteral("-sccUTF-8") << QStringLiteral("--")
             << (archivePath.isEmpty() ? m_path : archivePath);
    switches += entries;
    return run7z(switches, onOutput, workingDirectory);
}

ArchiveResult SevenZipBackend::resultForRun(const RunResult &run, ArchiveError fallback) const
{
    ArchiveResult result;
    if (m_cancelled) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    } else if (!run.started) {
        result.error = ArchiveError::ToolMissing;
    } else if (run.exitCode != 0) {
        const QString prompt = QStringLiteral("\nEnter password (will not be echoed):");
        if (run.output.contains(prompt) || run.errors.contains(prompt)
                || run.errors.contains(QLatin1String("Wrong password"))) {
            result.error = m_password.isEmpty() ? ArchiveError::PasswordRequired : ArchiveError::WrongPassword;
        } else {
            result.error = fallback;
        }
    }
    if (!result.ok())
        result.message = run.errors.isEmpty() ? run.output : run.errors;
    return result;
}

EntryResult SevenZipBackend::list()
{
    return readListing(true);
}

EntryResult SevenZipBackend::readListing(bool includeTimes)
{
    EntryResult result;
    const RunResult run = runArchiveCommand({QStringLiteral("l"), QStringLiteral("-slt")});
    result.result = resultForRun(run, ArchiveError::CorruptArchive);
    if (!result.result.ok())
        return result;
    result.info.entries = parseSltOutput(run.output, includeTimes);
    for (const ArchiveEntry &entry : result.info.entries) {
        result.info.totalSize += entry.size;
        result.info.compressedSize += entry.compressedSize;
    }
    return result;
}

VoidResult SevenZipBackend::testArchive()
{
    return {resultForRun(runArchiveCommand({QStringLiteral("t")}), ArchiveError::CorruptArchive)};
}

VoidResult SevenZipBackend::extractFiles(const ExtractRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (!QDir().mkpath(request.targetPath)) {
        result.error = ArchiveError::PermissionDenied;
        notifyFinished(observer, result);
        return {result};
    }

    EntryResult listed;
    for (;;) {
        listed = readListing(false);
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

    bool overwriteAll = request.overwrite == OverwritePolicy::Overwrite
            || request.overwrite == OverwritePolicy::OverwriteAll;
    bool skipAll = request.overwrite == OverwritePolicy::Skip
            || request.overwrite == OverwritePolicy::SkipAll;
    QVector<ArchiveEntry> acceptedEntries;
    QStringList entries;
    for (const ArchiveEntry &entry : includedEntries) {
        const QString relativePath = archivePathRelativeTo(entry.path, request.destinationPrefix);
        const QString outputPath = safeOutputPath(request.targetPath, relativePath);
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
        if (entry.type == EntryType::Directory)
            continue;
        if (QFileInfo::exists(outputPath) && skipAll)
            continue;
        if (QFileInfo::exists(outputPath) && !overwriteAll) {
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
        acceptedEntries.append(entry);
        entries << entry.path;
    }
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
    if (entries.isEmpty()) {
        notifyFinished(observer, result);
        return {result};
    }

    QTemporaryDir staging;
    QString extractionTarget = request.targetPath;
    if (!request.destinationPrefix.isEmpty()) {
        staging.setAutoRemove(true);
        if (!staging.isValid() || !QDir().mkpath(staging.path())) {
            result.error = ArchiveError::PermissionDenied;
            notifyFinished(observer, result);
            return {result};
        }
        extractionTarget = staging.path();
    }
    QStringList switches = {QStringLiteral("x"), QStringLiteral("-y"), QStringLiteral("-spd"),
                                  QStringLiteral("-bsp1"),
                                  QStringLiteral("-o") + extractionTarget};

    QVector<QStringList> chunks = splitArgumentChunks(entries);
    QTemporaryFile selectionFile;
    if (chunks.size() > 1 && writeArgumentList(selectionFile, entries)) {
        switches << QStringLiteral("-scsUTF-8") << QStringLiteral("-i@") + selectionFile.fileName();
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
        for (;;) {
            result = resultForRun(runArchiveCommand(switches, chunks.at(chunkIndex), reportProgress),
                                  ArchiveError::Unknown);
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
    if (result.ok() && !request.destinationPrefix.isEmpty()) {
        mergeStagedEntries(staging.path(), request.targetPath,
                           acceptedEntries, request.destinationPrefix, result);
    }
    if (result.ok()) {
        ProgressInfo progress;
        progress.percentage = 100.0;
        observer.onProgress(progress);
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult SevenZipBackend::addFiles(const AddRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    const bool zip = m_mimeType == QLatin1String("application/zip")
            || m_mimeType == QLatin1String("application/x-zip-compressed")
            || m_path.endsWith(QLatin1String(".zip"), Qt::CaseInsensitive);
    if (request.entries.isEmpty()) {
        result.error = ArchiveError::NotFound;
        notifyFinished(observer, result);
        return {result};
    }
    if (request.encryptList && request.password.isEmpty()) {
        result.error = ArchiveError::PasswordRequired;
        notifyFinished(observer, result);
        return {result};
    }
    if (zip && request.encryptList) {
        result.error = ArchiveError::NotSupported;
        notifyFinished(observer, result);
        return {result};
    }
    QStringList switches = {QStringLiteral("a"), QStringLiteral("-y"), QStringLiteral("-spd")};
    if (zip) {
        switches << QStringLiteral("-tzip");
        if (!request.password.isEmpty())
            switches << QStringLiteral("-mem=AES256");
    }
    if (request.compressionLevel >= 0)
        switches << QStringLiteral("-mx=%1").arg(request.compressionLevel);
    if (request.cpuThreads > 1)
        switches << QStringLiteral("-mmt=%1").arg(request.cpuThreads);
    if (request.splitSize > 0)
        switches << QStringLiteral("-v%1b").arg(request.splitSize);
    if (request.encryptList)
        switches << QStringLiteral("-mhe=on");

    QTemporaryDir staging;
    QStringList entries;
    QString workingDirectory;
    if (request.tar7z) {
        if (!staging.isValid()) {
            ArchiveResult failed;
            failed.error = ArchiveError::PermissionDenied;
            notifyFinished(observer, failed);
            return {failed};
        }
        const QString dataRoot = QDir(staging.path()).filePath(QStringLiteral("data"));
        if (!stageLocalEntries(request.entries, request.destinationPath, dataRoot,
                               nullptr, result)) {
            notifyFinished(observer, result);
            return {result};
        }
        const QString tarPath = QDir(staging.path()).filePath(QStringLiteral("inner.tar"));
        QProcess tar;
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
        tar.setProcessEnvironment(environment);
        tar.start(QStringLiteral("tar"),
                  {QStringLiteral("-cf"), tarPath, QStringLiteral("-C"), dataRoot,
                   QStringLiteral(".")});
        if (!tar.waitForStarted()) {
            result.error = ArchiveError::ToolMissing;
            result.message = tar.errorString();
            notifyFinished(observer, result);
            return {result};
        }
        while (tar.state() != QProcess::NotRunning) {
            if (m_cancelled)
                tar.kill();
            tar.waitForFinished(50);
        }
        if (m_cancelled) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            notifyFinished(observer, result);
            return {result};
        }
        if (tar.exitStatus() != QProcess::NormalExit || tar.exitCode() != 0) {
            result.error = ArchiveError::Unknown;
            result.message = QString::fromLocal8Bit(tar.readAllStandardError());
            notifyFinished(observer, result);
            return {result};
        }
        entries << QFileInfo(tarPath).fileName();
        workingDirectory = staging.path();
    } else if (!request.destinationPath.isEmpty()) {
        if (!staging.isValid()) {
            ArchiveResult failed;
            failed.error = ArchiveError::PermissionDenied;
            notifyFinished(observer, failed);
            return {failed};
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
            entries << destinationRoot + QLatin1Char('/') + name;
        workingDirectory = dataRoot;
    } else {
        for (const ArchiveEntry &entry : request.entries)
            entries << entry.path;
    }

    m_password = request.password;
    const QFileInfo destination(m_path);
    QTemporaryDir writeDirectory(QDir(destination.absolutePath()).filePath(QStringLiteral(".dc-7z-XXXXXX")));
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
        for (const QString &path : relativeSourceExclusions(entries, excluded))
            switches << QStringLiteral("-x!") + path;
    }
    result = resultForRun(runArchiveCommand(switches, entries, {}, workingDirectory, writePath),
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
    notifyFinished(observer, result);
    return {result};
}

VoidResult SevenZipBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("move is not supported by 7z backend")};
}

VoidResult SevenZipBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("copy is not supported by 7z backend")};
}

VoidResult SevenZipBackend::deleteFiles(const DeleteRequest &request)
{
    QStringList entries;
    for (const ArchiveEntry &entry : request.entries)
        entries << entry.path;
    if (entries.isEmpty())
        return {};
    return {resultForRun(runArchiveCommand({QStringLiteral("d"), QStringLiteral("-y"), QStringLiteral("-spd")},
                                           entries), ArchiveError::Unknown)};
}

VoidResult SevenZipBackend::renameFiles(const RenameRequest &request)
{
    QString from = request.entry.path;
    QString to = request.newName;
    if (request.entry.type == EntryType::Directory) {
        if (from.endsWith(QLatin1Char('/')))
            from.chop(1);
        if (to.endsWith(QLatin1Char('/')))
            to.chop(1);
    }
    return {resultForRun(runArchiveCommand({QStringLiteral("rn"), QStringLiteral("-spd")},
                                           {from, to}), ArchiveError::Unknown)};
}

VoidResult SevenZipBackend::addComment(const QString &)
{
    return {};
}

VoidResult SevenZipBackend::updateArchiveData(const UpdateRequest &)
{
    const EntryResult listed = list();
    return {listed.result};
}

void SevenZipBackend::pause()
{
    m_paused = true;
    const qint64 processId = m_processId.load();
    if (processId > 0)
        ::kill(pid_t(processId), SIGSTOP);
}

void SevenZipBackend::resume()
{
    m_paused = false;
    const qint64 processId = m_processId.load();
    if (processId > 0)
        ::kill(pid_t(processId), SIGCONT);
}

void SevenZipBackend::cancel()
{
    m_cancelled = true;
    const qint64 processId = m_processId.load();
    if (processId > 0)
        ::kill(pid_t(processId), SIGKILL);
}

void SevenZipBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
