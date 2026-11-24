// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipzip_backend.h"

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
    caps.create = true;
    caps.password = true;
    caps.cancel = true;
    return caps;
}

VoidResult ClipzipBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_cancelled = false;
    return {};
}

QString ClipzipBackend::resolvePzip() const
{
#ifdef DC_BUILT_PZIP
    return QStringLiteral(DC_BUILT_PZIP);
#else
    const QString installed = QStringLiteral("/usr/lib/deepin-compressor/pzip");
    if (QFileInfo::exists(installed))
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

    QStringList arguments;
    if (request.cpuThreads > 1)
        arguments << QStringLiteral("-c") << QString::number(request.cpuThreads);
    arguments << QStringLiteral("-l")
              << QString::number(request.compressionLevel >= 0 ? request.compressionLevel : 1);
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
    while (process.state() != QProcess::NotRunning) {
        if (m_cancelled)
            process.kill();
        process.waitForFinished(100);
    }

    if (m_cancelled) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    } else if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromLocal8Bit(process.readAll());
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
}

void ClipzipBackend::resume()
{
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
