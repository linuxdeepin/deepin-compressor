// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlefile_backend.h"

#include "path_guard.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include <linux/limits.h>
#include <unistd.h>

namespace dcarchive {
namespace {

constexpr int kChunkSize = 16 * 1024;

} // namespace

SingleFileBackend::SingleFileBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
    configureCodec();
}

SingleFileBackend::~SingleFileBackend() = default;

void SingleFileBackend::configureCodec()
{
    if (m_mimeType == QLatin1String("application/gzip")
            || m_mimeType == QLatin1String("application/x-gzip")) {
        m_codec = SingleFileCodec::Gzip;
        m_suffixes = {QStringLiteral(".gz"), QStringLiteral(".svgz")};
    } else if (m_mimeType == QLatin1String("application/x-bzip")
               || m_mimeType == QLatin1String("application/x-bzip2")) {
        m_codec = SingleFileCodec::Bzip2;
        m_suffixes = {QStringLiteral(".bz2")};
    } else if (m_mimeType == QLatin1String("application/x-xz")) {
        m_codec = SingleFileCodec::Xz;
        m_suffixes = {QStringLiteral(".xz")};
    } else if (m_mimeType == QLatin1String("application/x-lzma")) {
        m_codec = SingleFileCodec::Lzma;
        m_suffixes = {QStringLiteral(".lzma")};
    } else {
        m_codec = SingleFileCodec::Gzip;
        m_suffixes = {QStringLiteral(".gz")};
    }
}

QString SingleFileBackend::uncompressedFileName() const
{
    QString name = QFileInfo(m_path).fileName();

    if (name.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)) {
        name.chop(1);
        return name;
    }

    for (const QString &suffix : m_suffixes) {
        if (name.endsWith(suffix, Qt::CaseInsensitive)) {
            name.chop(suffix.size());
            return name;
        }
    }

    return name + QStringLiteral(".uncompressed");
}

BackendCapabilities SingleFileBackend::capabilities() const
{
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;
    return caps;
}

VoidResult SingleFileBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_cancelled = false;
    m_paused = false;
    configureCodec();
    return {};
}

EntryResult SingleFileBackend::list()
{
    EntryResult result;
    const QFileInfo info(m_path);
    if (!info.exists() || !info.isFile()) {
        result.result.error = ArchiveError::NotFound;
        result.result.message = QStringLiteral("archive does not exist");
        return result;
    }

    ArchiveEntry entry;
    entry.name = uncompressedFileName();
    entry.path = entry.name;
    entry.size = info.size();
    entry.compressedSize = info.size();
    entry.modified = info.lastModified();
    entry.type = EntryType::File;

    result.info.entries.append(entry);
    result.info.totalSize = info.size();
    result.info.compressedSize = info.size();
    return result;
}

VoidResult SingleFileBackend::testArchive()
{
    // 与迁移前行为保持一致：单文件后端只置状态，不做完整流校验。
    return {};
}

VoidResult SingleFileBackend::extractFiles(const ExtractRequest &request,
                                           BackendObserver &observer)
{
    const QString fileName = uncompressedFileName();

    QString targetDir = request.targetPath;
    if (!targetDir.endsWith(QLatin1Char('/')))
        targetDir += QLatin1Char('/');
    if (!QDir().exists(targetDir))
        QDir().mkpath(targetDir);
    const QString outputPath = targetDir + fileName;

    if (QFile::exists(outputPath)) {
        FileConflictRequest conflict;
        conflict.path = outputPath;
        observer.onFileConflict(conflict);

        switch (conflict.response) {
        case FileConflictRequest::Response::Cancel:
        case FileConflictRequest::Response::Skip:
        case FileConflictRequest::Response::SkipAll: {
            ArchiveResult cancelled;
            cancelled.error = ArchiveError::Cancelled;
            cancelled.cancelled = true;
            notifyFinished(observer, cancelled);
            return {cancelled};
        }
        case FileConflictRequest::Response::Overwrite:
        case FileConflictRequest::Response::OverwriteAll:
            break;
        }
    }

    if (!ensureParentWithinRoot(request.targetPath, outputPath)) {
        ArchiveResult denied;
        denied.error = ArchiveError::PermissionDenied;
        denied.message = outputPath;
        notifyFinished(observer, denied);
        return {denied};
    }

    QFile output(outputPath);
    if (!output.open(QIODevice::WriteOnly)) {
        ArchiveResult failed;
        failed.error = ArchiveError::PermissionDenied;
        failed.message = outputPath;
        notifyFinished(observer, failed);
        return {failed};
    }

    StreamDecoder decoder;
    if (!decoder.open(m_path, m_codec)) {
        ArchiveResult failed;
        failed.error = ArchiveError::CorruptArchive;
        failed.message = m_path;
        notifyFinished(observer, failed);
        return {failed};
    }

    observer.onCurrentFile(fileName);
    ProgressInfo progress;
    observer.onProgress(progress);

    qint64 written = 0;
    QByteArray chunk(kChunkSize, '\0');
    ArchiveResult result;
    const auto waitForResume = [&] {
        while (m_paused && !m_cancelled)
            QThread::msleep(20);
        if (!m_cancelled)
            return true;
        result = {ArchiveError::Cancelled, {}, true};
        return false;
    };

    while (waitForResume()) {
        const qint64 bytesRead = decoder.read(chunk.data(), chunk.size());
        if (!waitForResume())
            break;
        if (bytesRead < 0) {
            result.error = ArchiveError::CorruptArchive;
            break;
        }
        if (bytesRead == 0)
            break;

        if (output.write(chunk.constData(), bytesRead) != bytesRead) {
            result.error = ArchiveError::DiskFull;
            break;
        }
        written += bytesRead;
        progress.processedBytes = written;
        observer.onProgress(progress);
    }

    output.close();
    decoder.close();

    if (result.ok()) {
        progress.percentage = 100.0;
        progress.processedBytes = written;
        observer.onProgress(progress);
    }

    notifyFinished(observer, result);
    return {result};
}

VoidResult SingleFileBackend::addFiles(const AddRequest &, BackendObserver &)
{
    return unsupported();
}

VoidResult SingleFileBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    return unsupported();
}

VoidResult SingleFileBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    return unsupported();
}

VoidResult SingleFileBackend::deleteFiles(const DeleteRequest &)
{
    return unsupported();
}

VoidResult SingleFileBackend::renameFiles(const RenameRequest &)
{
    return unsupported();
}

VoidResult SingleFileBackend::addComment(const QString &)
{
    return unsupported();
}

VoidResult SingleFileBackend::updateArchiveData(const UpdateRequest &)
{
    return unsupported();
}

void SingleFileBackend::pause()
{
    m_paused = true;
}

void SingleFileBackend::resume()
{
    m_paused = false;
}

void SingleFileBackend::cancel()
{
    m_cancelled = true;
}

void SingleFileBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

VoidResult SingleFileBackend::unsupported() const
{
    return {ArchiveError::NotSupported, QStringLiteral("operation is not supported by single-file backend")};
}

} // namespace dcarchive
