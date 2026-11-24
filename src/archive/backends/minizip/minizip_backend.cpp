// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "minizip_backend.h"

#include "encoding_utils.h"
#include "extraction_output.h"
#include "path_guard.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include <minizip/unzip.h>

#include <cstring>

namespace dcarchive {
namespace {

constexpr int kChunkSize = 64 * 1024;

QDateTime dosDateTime(const unz_file_info &info)
{
    const QDate date(1980 + info.tmu_date.tm_year,
                     info.tmu_date.tm_mon + 1,
                     info.tmu_date.tm_mday);
    const QTime time(info.tmu_date.tm_hour,
                     info.tmu_date.tm_min,
                     info.tmu_date.tm_sec);
    return QDateTime(date, time);
}

} // namespace

MinizipBackend::MinizipBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
}

MinizipBackend::~MinizipBackend() = default;

BackendCapabilities MinizipBackend::capabilities() const
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

VoidResult MinizipBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_paused = false;
    m_cancelled = false;
    return {};
}

EntryResult MinizipBackend::list()
{
    EntryResult result;
    unzFile archive = unzOpen(QFile::encodeName(m_path).constData());
    if (!archive) {
        result.result.error = ArchiveError::CorruptArchive;
        result.result.message = m_path;
        return result;
    }

    int index = 0;
    int ret = unzGoToFirstFile(archive);
    while (ret == UNZ_OK) {
        unz_file_info info;
        char name[4096] = {0};
        if (unzGetCurrentFileInfo(archive, &info, name, sizeof(name) - 1,
                                  nullptr, 0, nullptr, 0) == UNZ_OK) {
            ArchiveEntry entry;
            entry.path = decodeArchiveName(name, int(strlen(name)));
            entry.name = QFileInfo(entry.path).fileName();
            if (entry.name.isEmpty() && entry.path.endsWith(QLatin1Char('/')))
                entry.name = QFileInfo(entry.path.chopped(1)).fileName();
            entry.size = qint64(info.uncompressed_size);
            entry.compressedSize = qint64(info.compressed_size);
            entry.modified = dosDateTime(info);
            entry.type = entry.path.endsWith(QLatin1Char('/'))
                    ? EntryType::Directory : EntryType::File;
            entry.index = index++;
            result.info.entries.append(entry);
            result.info.totalSize += entry.size;
            result.info.compressedSize += entry.compressedSize;
        }
        ret = unzGoToNextFile(archive);
    }

    unzClose(archive);
    return result;
}

VoidResult MinizipBackend::testArchive()
{
    ArchiveResult result;
    unzFile archive = unzOpen(QFile::encodeName(m_path).constData());
    if (!archive) {
        result.error = ArchiveError::CorruptArchive;
        result.message = m_path;
        return {result};
    }
    unzClose(archive);
    return {};
}

QString MinizipBackend::safeOutputPath(const QString &targetDir, const QString &entryPath) const
{
    QString clean = QDir::cleanPath(entryPath);
    if (clean.startsWith(QLatin1Char('/')) || clean == QLatin1String("..")
            || clean.startsWith(QLatin1String("../")))
        return {};
    return QDir(targetDir).filePath(clean);
}

VoidResult MinizipBackend::extractFiles(const ExtractRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (!QDir().mkpath(request.targetPath)) {
        result.error = ArchiveError::PermissionDenied;
        notifyFinished(observer, result);
        return {result};
    }
    unzFile archive = unzOpen(QFile::encodeName(m_path).constData());
    if (!archive) {
        result.error = ArchiveError::CorruptArchive;
        result.message = m_path;
        notifyFinished(observer, result);
        return {result};
    }

    QSet<int> selected;
    QVector<ArchiveEntry> selectedDirectories;
    for (const ArchiveEntry &entry : request.entries) {
        selected.insert(entry.index);
        if (entry.type == EntryType::Directory)
            selectedDirectories.append(entry);
    }
    const bool all = request.allEntries || request.entries.isEmpty();

    bool overwriteAll = false;
    bool skipAll = false;
    qint64 done = 0;
    int index = 0;
    int ret = unzGoToFirstFile(archive);
    const auto waitForResume = [&] {
        while (m_paused && !m_cancelled)
            QThread::msleep(20);
        if (!m_cancelled)
            return true;
        result = {ArchiveError::Cancelled, {}, true};
        return false;
    };

    while (ret == UNZ_OK && waitForResume()) {
        const int current = index++;
        unz_file_info info;
        char name[4096] = {0};
        unzGetCurrentFileInfo(archive, &info, name, sizeof(name) - 1,
                              nullptr, 0, nullptr, 0);
        const QString entryPath = decodeArchiveName(name, int(strlen(name)));
        const bool isDir = entryPath.endsWith(QLatin1Char('/'));

        bool selectedByDirectory = false;
        for (const ArchiveEntry &directory : selectedDirectories) {
            if (archivePathWithin(entryPath, directory.path)) {
                selectedByDirectory = true;
                break;
            }
        }
        if (!all && !selected.contains(current) && !selectedByDirectory) {
            ret = unzGoToNextFile(archive);
            continue;
        }
        const QString relativePath = archivePathRelativeTo(entryPath, request.destinationPrefix);
        const QString outputPath = safeOutputPath(request.targetPath, relativePath);
        if (outputPath.isEmpty()) {
            result.error = ArchiveError::PermissionDenied;
            result.message = entryPath;
            break;
        }

        if (isDir) {
            if (!ensureDirectoryWithinRoot(request.targetPath, outputPath)) {
                result.error = ArchiveError::PermissionDenied;
                result.message = entryPath;
                break;
            }
            ret = unzGoToNextFile(archive);
            continue;
        }

        if (QFile::exists(outputPath) && !overwriteAll && !skipAll) {
            FileConflictRequest conflict;
            conflict.path = outputPath;
            observer.onFileConflict(conflict);
            switch (conflict.response) {
            case FileConflictRequest::Response::Cancel:
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                break;
            case FileConflictRequest::Response::Skip:
                ret = unzGoToNextFile(archive);
                continue;
            case FileConflictRequest::Response::SkipAll:
                skipAll = true;
                ret = unzGoToNextFile(archive);
                continue;
            case FileConflictRequest::Response::OverwriteAll:
                overwriteAll = true;
                break;
            case FileConflictRequest::Response::Overwrite:
                break;
            }
            if (result.cancelled)
                break;
        }
        if (skipAll) {
            ret = unzGoToNextFile(archive);
            continue;
        }

        if (!ensureParentWithinRoot(request.targetPath, outputPath)) {
            result.error = ArchiveError::PermissionDenied;
            result.message = entryPath;
            break;
        }
        ExtractionOutput output;
        if (!output.open(outputPath)) {
            result.error = output.error();
            result.message = outputPath;
            break;
        }

        if (unzOpenCurrentFile(archive) != UNZ_OK) {
            result.error = ArchiveError::CorruptArchive;
            result.message = entryPath;
            break;
        }

        observer.onCurrentFile(entryPath);
        QByteArray buffer(kChunkSize, '\0');
        while (waitForResume()) {
            const int read = unzReadCurrentFile(archive, buffer.data(), buffer.size());
            if (read < 0) {
                result.error = ArchiveError::CorruptArchive;
                result.message = entryPath;
                break;
            }
            if (read == 0)
                break;
            if (!waitForResume())
                break;
            if (!output.write(buffer.constData(), read)) {
                result.error = output.error();
                result.message = outputPath;
                break;
            }
            done += read;
            if (!waitForResume())
                break;
            ProgressInfo progress;
            progress.processedBytes = done;
            observer.onProgress(progress);
        }
        const int closeResult = unzCloseCurrentFile(archive);
        if (!result.ok() || !waitForResume())
            break;
        if (closeResult != UNZ_OK) {
            result.error = ArchiveError::CorruptArchive;
            result.message = entryPath;
            break;
        }
        if (!output.commit()) {
            result.error = output.error();
            result.message = outputPath;
            break;
        }
        ret = unzGoToNextFile(archive);
    }

    unzClose(archive);
    if (result.ok()) {
        ProgressInfo progress;
        progress.percentage = 100.0;
        progress.processedBytes = done;
        observer.onProgress(progress);
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult MinizipBackend::addFiles(const AddRequest &, BackendObserver &)
{
    return unsupported();
}

VoidResult MinizipBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    return unsupported();
}

VoidResult MinizipBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    return unsupported();
}

VoidResult MinizipBackend::deleteFiles(const DeleteRequest &)
{
    return unsupported();
}

VoidResult MinizipBackend::renameFiles(const RenameRequest &)
{
    return unsupported();
}

VoidResult MinizipBackend::addComment(const QString &)
{
    return unsupported();
}

VoidResult MinizipBackend::updateArchiveData(const UpdateRequest &)
{
    const EntryResult listed = list();
    return {listed.result};
}

void MinizipBackend::pause()
{
    m_paused = true;
}

void MinizipBackend::resume()
{
    m_paused = false;
}

void MinizipBackend::cancel()
{
    m_cancelled = true;
}

VoidResult MinizipBackend::unsupported() const
{
    return {ArchiveError::NotSupported, QStringLiteral("operation is not supported by minizip backend")};
}

void MinizipBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
