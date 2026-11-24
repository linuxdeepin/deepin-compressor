// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipzip_backend.h"
#include "process_pause.h"
#include "write_progress.h"
#include "../../../pzip/include/pzip/cli_status.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTemporaryFile>

namespace dcarchive {

ClipzipBackend::ClipzipBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
}

ClipzipBackend::~ClipzipBackend() = default;

BackendCapabilities ClipzipBackend::capabilities() const
{
    BackendCapabilities caps;
    caps.progress = true;
    caps.create = true;
    caps.password = true;
    caps.cancel = true;
    caps.pause = true;
    return caps;
}

VoidResult ClipzipBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_cancelled = false;
    m_paused = false;
    return {};
}

QString ClipzipBackend::resolvePzip() const
{
#ifdef DC_BUILT_PZIP
    return QStringLiteral(DC_BUILT_PZIP);
#else
    const QString bundled = QCoreApplication::applicationDirPath()
            + QStringLiteral("/../lib/deepin-compressor/pzip");
    if (QFileInfo(bundled).isExecutable())
        return bundled;

    const QString installed = QStringLiteral("/usr/lib/deepin-compressor/pzip");
    if (QFileInfo(installed).isExecutable())
        return installed;

    const QString fromPath = QStandardPaths::findExecutable(QStringLiteral("pzip"));
    if (!fromPath.isEmpty())
        return fromPath;

    const QString buildTree = QCoreApplication::applicationDirPath()
            + QStringLiteral("/../src/pzip/pzip");
    if (QFileInfo::exists(buildTree))
        return buildTree;

    return {};
#endif
}

EntryResult ClipzipBackend::list()
{
    EntryResult result;
    result.result.error = ArchiveError::NotSupported;
    result.result.message = QStringLiteral("pzip does not support listing");
    return result;
}

VoidResult ClipzipBackend::testArchive()
{
    return {ArchiveError::NotSupported, QStringLiteral("pzip does not support testing")};
}

VoidResult ClipzipBackend::extractFiles(const ExtractRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("punzip is not part of the current backend")};
}

VoidResult ClipzipBackend::addFiles(const AddRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (request.splitSize > 0 || request.encryptList) {
        result.error = ArchiveError::NotSupported;
        notifyFinished(observer, result);
        return {result};
    }
    const QString pzip = resolvePzip();
    if (pzip.isEmpty()) {
        result.error = ArchiveError::ToolMissing;
        result.message = QStringLiteral("pzip executable not found");
        notifyFinished(observer, result);
        return {result};
    }
    if (request.entries.isEmpty()) {
        result.error = ArchiveError::NotFound;
        notifyFinished(observer, result);
        return {result};
    }

    QStringList arguments{QStringLiteral("--ui-events"), QStringLiteral("-q")};
    if (request.cpuThreads > 1)
        arguments << QStringLiteral("-c") << QString::number(request.cpuThreads);
    arguments << QStringLiteral("-l")
              << QString::number(request.compressionLevel >= 0 ? request.compressionLevel : 1);
    for (const QString &path : request.excludedSourcePaths) {
        for (const ArchiveEntry &entry : request.entries) {
            const QFileInfo source(entry.path);
            if (source.isDir() && path.startsWith(source.absoluteFilePath() + QLatin1Char('/'))) {
                arguments << QStringLiteral("--exclude-path") << QDir::cleanPath(path);
                break;
            }
        }
    }
    QTemporaryFile passwordFile;
    if (!request.password.isEmpty()) {
        const QByteArray password = request.password.toUtf8();
        if (!passwordFile.open() || passwordFile.write(password) != password.size()
                || !passwordFile.flush()) {
            result.error = ArchiveError::PermissionDenied;
            result.message = passwordFile.errorString();
            notifyFinished(observer, result);
            return {result};
        }
        passwordFile.close();
        arguments << QStringLiteral("--password-file") << passwordFile.fileName();
    }
    arguments << m_path;
    for (const ArchiveEntry &entry : request.entries)
        arguments << entry.path;

    WriteProgress progress(observer, compressionSourceBytes(request, request.excludedSourcePaths, m_cancelled));
    QByteArray pending;
    QByteArray diagnostics;
    const auto consumeOutput = [&](const QByteArray &chunk) {
        pending += chunk;
        qsizetype start = 0;
        for (qsizetype end = pending.indexOf('\n'); end >= 0; end = pending.indexOf('\n', start)) {
            const QByteArray line = pending.mid(start, end - start);
            start = end + 1;
            if (line.startsWith("[PZIP_UI] entry ")) {
                observer.onCurrentFile(QString::fromUtf8(line.mid(16)));
            } else if (line.startsWith("[PZIP_UI] inbytes ")) {
                bool valid = false;
                const qint64 bytes = line.mid(18).toLongLong(&valid);
                if (valid && bytes >= 0)
                    progress.setBytes(bytes);
            } else if (diagnostics.size() < 8192) {
                diagnostics += line + '\n';
            }
        }
        pending.remove(0, start);
    };

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    process.setProcessEnvironment(environment);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(pzip, arguments);
    if (!process.waitForStarted()) {
        result.error = ArchiveError::ToolMissing;
        result.message = QStringLiteral("failed to start pzip");
        notifyFinished(observer, result);
        return {result};
    }
    bool paused = false;
    while (process.state() != QProcess::NotRunning) {
        if (m_cancelled)
            process.kill();
        else
            syncProcessPause(process, m_paused, paused);
        process.waitForFinished(100);
        consumeOutput(process.readAll());
    }
    consumeOutput(process.readAll() + '\n');

    if (m_cancelled) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    } else if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        result.error = process.exitStatus() == QProcess::NormalExit
                && process.exitCode() == pzip::diskFullExitStatus ? ArchiveError::DiskFull : ArchiveError::Unknown;
        result.message = QString::fromLocal8Bit(diagnostics).trimmed();
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult ClipzipBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("move is not supported by clipzip backend")};
}

VoidResult ClipzipBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("copy is not supported by clipzip backend")};
}

VoidResult ClipzipBackend::deleteFiles(const DeleteRequest &)
{
    return {ArchiveError::NotSupported, QStringLiteral("delete is not supported by clipzip backend")};
}

VoidResult ClipzipBackend::renameFiles(const RenameRequest &)
{
    return {ArchiveError::NotSupported, QStringLiteral("rename is not supported by clipzip backend")};
}

VoidResult ClipzipBackend::addComment(const QString &)
{
    return {ArchiveError::NotSupported, QStringLiteral("comment is not supported by clipzip backend")};
}

VoidResult ClipzipBackend::updateArchiveData(const UpdateRequest &)
{
    return {ArchiveError::NotSupported, QStringLiteral("update is not supported by clipzip backend")};
}

void ClipzipBackend::pause()
{
    m_paused = true;
}

void ClipzipBackend::resume()
{
    m_paused = false;
}

void ClipzipBackend::cancel()
{
    m_cancelled = true;
}

void ClipzipBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
