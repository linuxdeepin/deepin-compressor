// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libarchive_backend.h"

#include "extraction_names.h"
#include "path_guard.h"
#include "extraction_output.h"
#include "write_progress.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QTemporaryFile>
#include <QThread>

#include <archive.h>
#include <archive_entry.h>

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <algorithm>

namespace dcarchive {
namespace {

constexpr int kBlockSize = 10240;
constexpr int kCopyBlock = 64 * 1024;

ArchiveEntry entryFromHeader(::archive_entry *entry, int index)
{
    ArchiveEntry result;
    const char *pathname = archive_entry_pathname(entry);
    result.path = QString::fromUtf8(pathname ? pathname : "");
    result.name = QFileInfo(result.path).fileName();
    if (result.name.isEmpty() && result.path.endsWith(QLatin1Char('/')))
        result.name = QFileInfo(result.path.chopped(1)).fileName();
    result.size = archive_entry_size(entry);
    result.compressedSize = result.size;
    result.modified = QDateTime::fromSecsSinceEpoch(archive_entry_mtime(entry));
    result.index = index;

    const mode_t type = archive_entry_filetype(entry);
    if (type == AE_IFDIR) {
        result.type = EntryType::Directory;
    } else if (type == AE_IFLNK) {
        result.type = EntryType::Symlink;
        const char *link = archive_entry_symlink(entry);
        result.linkTarget = QString::fromUtf8(link ? link : "");
    } else {
        result.type = EntryType::File;
    }
    return result;
}

bool applyWriterFormat(::archive *writer, const QString &path, const QString &mimeType)
{
    const QString lower = path.toLower();
    const QString mime = mimeType.toLower();
    if (mime == QLatin1String("application/zip")
            || mime == QLatin1String("application/x-zip-compressed")
            || mime == QLatin1String("application/java-archive")
            || lower.endsWith(QLatin1String(".zip"))
            || lower.endsWith(QLatin1String(".jar"))) {
        return archive_write_set_format_zip(writer) == ARCHIVE_OK;
    }

    if (archive_write_set_format_pax_restricted(writer) != ARCHIVE_OK)
        return false;

    int ret = ARCHIVE_OK;
    bool externalFilter = false;
    if (lower.endsWith(QLatin1String(".tar.gz")) || lower.endsWith(QLatin1String(".tgz")))
        ret = archive_write_add_filter_gzip(writer);
    else if (lower.endsWith(QLatin1String(".tar.bz2")) || lower.endsWith(QLatin1String(".tbz2")))
        ret = archive_write_add_filter_bzip2(writer);
    else if (lower.endsWith(QLatin1String(".tar.xz")) || lower.endsWith(QLatin1String(".txz")))
        ret = archive_write_add_filter_xz(writer);
    else if (lower.endsWith(QLatin1String(".tar.lzma")) || lower.endsWith(QLatin1String(".tlz")))
        ret = archive_write_add_filter_lzma(writer);
    else if (lower.endsWith(QLatin1String(".tar.lz")))
        ret = archive_write_add_filter_lzip(writer);
    else if (lower.endsWith(QLatin1String(".tar.lzo")) || lower.endsWith(QLatin1String(".tzo"))) {
        ret = archive_write_add_filter_lzop(writer);
        externalFilter = true;
    } else if (lower.endsWith(QLatin1String(".tar.lrz")) || lower.endsWith(QLatin1String(".tar.lrzip"))) {
        ret = archive_write_add_filter_lrzip(writer);
        externalFilter = true;
    } else if (lower.endsWith(QLatin1String(".tar.lz4")))
        ret = archive_write_add_filter_lz4(writer);
    else if (lower.endsWith(QLatin1String(".tar.zst"))
             || lower.endsWith(QLatin1String(".tar.zstd"))
             || lower.endsWith(QLatin1String(".tzst")))
        ret = archive_write_add_filter_zstd(writer);
    else if (lower.endsWith(QLatin1String(".tar.z")) || lower.endsWith(QLatin1String(".taz")))
        ret = archive_write_add_filter_compress(writer);
    else
        ret = archive_write_add_filter_none(writer);
    // Libarchive registers these external filters and returns a warning by design.
    return ret == ARCHIVE_OK || (externalFilter && ret == ARCHIVE_WARN);
}


bool copyWriteResult(::archive *writer, const QByteArray &buffer, la_ssize_t size,
                     ArchiveResult &result)
{
    if (archive_write_data(writer, buffer.constData(), size_t(size)) != size) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromUtf8(archive_error_string(writer));
        return false;
    }
    return true;
}

bool copyExistingEntries(::archive *reader, ::archive *writer, ArchiveResult &result,
                         const std::atomic_bool &paused, const std::atomic_bool &cancelled,
                         WriteProgress &progress)
{
    ::archive_entry *entry = nullptr;
    while (true) {
        const int next = archive_read_next_header(reader, &entry);
        if (next == ARCHIVE_EOF)
            return true;
        if (next != ARCHIVE_OK) {
            result.error = ArchiveError::CorruptArchive;
            result.message = QString::fromUtf8(archive_error_string(reader));
            return false;
        }
        if (!waitForWriteResume(paused, cancelled)) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            return false;
        }

        ::archive_entry *copy = archive_entry_clone(entry);
        if (!copy || archive_write_header(writer, copy) != ARCHIVE_OK) {
            result.error = ArchiveError::Unknown;
            result.message = QString::fromUtf8(archive_error_string(writer));
            archive_entry_free(copy);
            return false;
        }

        QByteArray buffer(kCopyBlock, '\0');
        while (true) {
            const la_ssize_t read = archive_read_data(reader, buffer.data(), buffer.size());
            if (read < 0) {
                result.error = ArchiveError::CorruptArchive;
                result.message = QString::fromUtf8(archive_error_string(reader));
                archive_entry_free(copy);
                return false;
            }
            if (read == 0)
                break;
            if (!waitForWriteResume(paused, cancelled)) {
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                archive_entry_free(copy);
                return false;
            }
            if (!copyWriteResult(writer, buffer, read, result)) {
                archive_entry_free(copy);
                return false;
            }
            progress.addBytes(read);
        }
        archive_entry_free(copy);
    }
}

bool writeLocalFile(::archive *writer, const QString &sourcePath,
                    const QString &archiveName, ArchiveResult &result,
                    const std::atomic_bool &paused, const std::atomic_bool &cancelled,
                    WriteProgress &progress)
{
    QFile source(sourcePath);
    if (!source.open(QIODevice::ReadOnly)) {
        result.error = ArchiveError::NotFound;
        result.message = sourcePath;
        return false;
    }

    ::archive_entry *item = archive_entry_new();
    archive_entry_set_pathname(item, archiveName.toUtf8().constData());
    archive_entry_set_filetype(item, AE_IFREG);
    archive_entry_set_perm(item, 0644);
    archive_entry_set_size(item, source.size());
    if (archive_write_header(writer, item) != ARCHIVE_OK) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromUtf8(archive_error_string(writer));
        archive_entry_free(item);
        return false;
    }

    QByteArray buffer(kCopyBlock, '\0');
    while (true) {
        const qint64 read = source.read(buffer.data(), buffer.size());
        if (read < 0) {
            result.error = ArchiveError::Unknown;
            result.message = sourcePath;
            archive_entry_free(item);
            return false;
        }
        if (read == 0)
            break;
        if (!waitForWriteResume(paused, cancelled)) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            archive_entry_free(item);
            return false;
        }
        if (!copyWriteResult(writer, buffer, la_ssize_t(read), result)) {
            archive_entry_free(item);
            return false;
        }
        progress.addBytes(read);
    }
    archive_entry_free(item);
    return true;
}

bool writeDirectoryEntry(::archive *writer, const QString &archiveName, ArchiveResult &result)
{
    ::archive_entry *item = archive_entry_new();
    archive_entry_set_pathname(item, (archiveName + QLatin1Char('/')).toUtf8().constData());
    archive_entry_set_filetype(item, AE_IFDIR);
    archive_entry_set_perm(item, 0755);
    archive_entry_set_size(item, 0);
    if (archive_write_header(writer, item) != ARCHIVE_OK) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromUtf8(archive_error_string(writer));
        archive_entry_free(item);
        return false;
    }
    archive_entry_free(item);
    return true;
}

bool writeSymlinkEntry(::archive *writer, const QString &sourcePath,
                       const QString &archiveName, ArchiveResult &result)
{
    ::archive_entry *item = archive_entry_new();
    archive_entry_set_pathname(item, archiveName.toUtf8().constData());
    archive_entry_set_filetype(item, AE_IFLNK);
    archive_entry_set_perm(item, 0777);
    const QString target = QFile::symLinkTarget(sourcePath);
    archive_entry_set_symlink(item, target.toUtf8().constData());
    if (archive_write_header(writer, item) != ARCHIVE_OK) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromUtf8(archive_error_string(writer));
        archive_entry_free(item);
        return false;
    }
    archive_entry_free(item);
    return true;
}

bool writeLocalDirectory(::archive *writer, const QString &sourceDir,
                         const QString &archiveName, ArchiveResult &result,
                         const std::atomic_bool &paused, const std::atomic_bool &cancelled, const QStringList &excludedPaths,
                         WriteProgress &progress)
{
    if (!writeDirectoryEntry(writer, archiveName, result))
        return false;

    const QDir root(sourceDir);
    QDirIterator it(sourceDir,
                    QDir::AllEntries | QDir::Readable | QDir::Hidden | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (!waitForWriteResume(paused, cancelled)) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            return false;
        }
        it.next();
        const QString sourcePath = it.fileInfo().absoluteFilePath();
        if (std::any_of(excludedPaths.cbegin(), excludedPaths.cend(), [&](const QString &path) {
                return archivePathWithin(sourcePath, path);
            }))
            continue;
        const QString relative = root.relativeFilePath(it.filePath());
        if (relative.isEmpty())
            continue;
        const QString targetName = archiveName + QLatin1Char('/') + relative;
        if (it.fileInfo().isDir()) {
            if (!writeDirectoryEntry(writer, targetName, result))
                return false;
        } else if (it.fileInfo().isSymLink()) {
            if (!writeSymlinkEntry(writer, it.filePath(), targetName, result))
                return false;
        } else if (!writeLocalFile(writer, it.filePath(), targetName, result, paused, cancelled, progress)) {
            return false;
        }
    }
    return true;
}

QString joinArchivePath(const QString &prefix, const QString &name)
{
    QString path = normalizedArchivePath(name);
    const QString normalizedPrefix = normalizedArchivePath(prefix);
    if (normalizedPrefix.isEmpty())
        return path;
    path.prepend(normalizedPrefix.endsWith(QLatin1Char('/'))
                         ? normalizedPrefix : normalizedPrefix + QLatin1Char('/'));
    return path;
}

bool writeNewEntries(::archive *writer, const QVector<ArchiveEntry> &entries,
                     const QString &destinationPath, ArchiveResult &result,
                     const std::atomic_bool &paused, const std::atomic_bool &cancelled, const QStringList &excludedPaths,
                     BackendObserver &observer, WriteProgress &progress)
{
    for (const ArchiveEntry &entry : entries) {
        observer.onCurrentFile(entry.path);
        if (!waitForWriteResume(paused, cancelled)) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            return false;
        }
        const QString sourceName = entry.name.isEmpty()
                ? QFileInfo(entry.path).fileName() : entry.name;
        if (sourceName.isEmpty()) {
            result.error = ArchiveError::NotFound;
            result.message = entry.path;
            return false;
        }
        const QString name = joinArchivePath(destinationPath, sourceName);
        if (entry.type == EntryType::Directory) {
            if (!writeLocalDirectory(writer, entry.path, name, result, paused, cancelled, excludedPaths, progress))
                return false;
        } else if (QFileInfo(entry.path).isSymLink()) {
            if (!writeSymlinkEntry(writer, entry.path, name, result))
                return false;
        } else if (!writeLocalFile(writer, entry.path, name, result, paused, cancelled, progress)) {
            return false;
        }
    }
    return true;
}

QString temporaryPathFor(const QString &targetPath)
{
    QTemporaryFile file(QDir(QFileInfo(targetPath).absolutePath())
                        .filePath(QStringLiteral(".dc-archive-XXXXXX")));
    file.setAutoRemove(false);
    if (!file.open())
        return {};
    const QString path = file.fileName();
    file.close();
    return path;
}

bool replaceFileAtomically(const QString &tempPath, const QString &targetPath)
{
    return ::rename(QFile::encodeName(tempPath).constData(),
                    QFile::encodeName(targetPath).constData()) == 0;
}

} // namespace

LibarchiveBackend::LibarchiveBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
}

LibarchiveBackend::~LibarchiveBackend() = default;

BackendCapabilities LibarchiveBackend::capabilities() const
{
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.create = true;
    caps.remove = true;
    caps.rename = true;
    caps.update = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;
    return caps;
}

VoidResult LibarchiveBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_cancelled = false;
    m_paused = false;
    return {};
}

EntryResult LibarchiveBackend::list()
{
    EntryResult result;
    ::archive *reader = archive_read_new();
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    if (archive_read_open_filename(reader, QFile::encodeName(m_path).constData(), kBlockSize) != ARCHIVE_OK) {
        result.result.error = ArchiveError::CorruptArchive;
        result.result.message = QString::fromUtf8(archive_error_string(reader));
        archive_read_free(reader);
        return result;
    }

    ::archive_entry *entry = nullptr;
    int index = 0;
    int status = ARCHIVE_OK;
    while ((status = archive_read_next_header(reader, &entry)) == ARCHIVE_OK
           || status == ARCHIVE_WARN) {
        ArchiveEntry item = entryFromHeader(entry, index++);
        result.info.entries.append(item);
        result.info.totalSize += item.size;
        result.info.compressedSize += item.compressedSize;
        if (archive_read_data_skip(reader) < ARCHIVE_WARN) {
            result.result.error = ArchiveError::CorruptArchive;
            result.result.message = QString::fromUtf8(archive_error_string(reader));
            break;
        }
    }
    if (status != ARCHIVE_EOF && result.result.ok()) {
        result.result.error = ArchiveError::CorruptArchive;
        result.result.message = QString::fromUtf8(archive_error_string(reader));
    }
    archive_read_free(reader);
    return result;
}

VoidResult LibarchiveBackend::testArchive()
{
    ArchiveResult result;
    ::archive *reader = archive_read_new();
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);
    if (archive_read_open_filename(reader, QFile::encodeName(m_path).constData(), kBlockSize) != ARCHIVE_OK) {
        result.error = ArchiveError::CorruptArchive;
        result.message = QString::fromUtf8(archive_error_string(reader));
    } else {
        ::archive_entry *entry = nullptr;
        int status = ARCHIVE_OK;
        while ((status = archive_read_next_header(reader, &entry)) == ARCHIVE_OK
               || status == ARCHIVE_WARN) {
            if (archive_read_data_skip(reader) < ARCHIVE_WARN) {
                result.error = ArchiveError::CorruptArchive;
                result.message = QString::fromUtf8(archive_error_string(reader));
                break;
            }
        }
        if (status != ARCHIVE_EOF && result.ok()) {
            result.error = ArchiveError::CorruptArchive;
            result.message = QString::fromUtf8(archive_error_string(reader));
        }
    }
    archive_read_free(reader);
    return {result};
}

QString LibarchiveBackend::safeOutputPath(const QString &targetDir, const QString &entryPath) const
{
    QString clean = QDir::cleanPath(entryPath);
    if (clean.startsWith(QLatin1Char('/')) || clean == QLatin1String("..")
            || clean.startsWith(QLatin1String("../")))
        return {};
    return QDir(targetDir).filePath(clean);
}

VoidResult LibarchiveBackend::extractFiles(const ExtractRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (!QDir().mkpath(request.targetPath)) {
        result.error = ArchiveError::PermissionDenied;
        notifyFinished(observer, result);
        return {result};
    }
    ::archive *reader = archive_read_new();
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);
    if (archive_read_open_filename(reader, QFile::encodeName(m_path).constData(), kBlockSize) != ARCHIVE_OK) {
        result.error = ArchiveError::CorruptArchive;
        result.message = QString::fromUtf8(archive_error_string(reader));
        archive_read_free(reader);
        notifyFinished(observer, result);
        return {result};
    }

    QSet<int> selected;
    QVector<ArchiveEntry> selectedDirectories;
    for (const ArchiveEntry &entry : request.entries)
        selected.insert(entry.index);
    for (const ArchiveEntry &entry : request.entries) {
        if (entry.type == EntryType::Directory)
            selectedDirectories.append(entry);
    }
    const bool all = request.allEntries;

    bool overwriteAll = request.overwrite == OverwritePolicy::Overwrite
            || request.overwrite == OverwritePolicy::OverwriteAll;
    bool skipAll = request.overwrite == OverwritePolicy::Skip
            || request.overwrite == OverwritePolicy::SkipAll;
    qint64 done = 0;
    ::archive_entry *entry = nullptr;
    int index = 0;
    ExtractionNames names;

    int headerStatus;
    while ((headerStatus = archive_read_next_header(reader, &entry)) == ARCHIVE_OK
           || headerStatus == ARCHIVE_WARN) {
        const int currentIndex = index++;
        while (m_paused && !m_cancelled)
            QThread::msleep(20);
        if (m_cancelled) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            break;
        }
        const ArchiveEntry item = entryFromHeader(entry, currentIndex);
        bool selectedByDirectory = false;
        for (const ArchiveEntry &directory : selectedDirectories) {
            if (archivePathWithin(item.path, directory.path)) {
                selectedByDirectory = true;
                break;
            }
        }
        if (!all && !selected.contains(currentIndex) && !selectedByDirectory)
            continue;

        const QString relativePath = archivePathRelativeTo(item.path, request.destinationPrefix);
        QString outputPath = safeOutputPath(request.targetPath, relativePath);
        if (outputPath.isEmpty()) {
            result.error = ArchiveError::PermissionDenied;
            result.message = item.path;
            break;
        }
        const QString diskName = names.outputName(relativePath, item.type == EntryType::Directory);
        if (diskName != relativePath) {
            if (diskName.isEmpty()) {
                observer.onCurrentFile(item.path);
                result = {ArchiveError::LongName, item.path};
                break;
            }
            outputPath = safeOutputPath(request.targetPath, diskName);
        }

        if (item.type == EntryType::Directory) {
            if (!ensureDirectoryWithinRoot(request.targetPath, outputPath)) {
                result.error = ArchiveError::PermissionDenied;
                result.message = item.path;
                break;
            }
            continue;
        }

        if (QFile::exists(outputPath) && skipAll)
            continue;
        if (QFile::exists(outputPath) && !overwriteAll) {
            FileConflictRequest conflict;
            conflict.path = outputPath;
            observer.onFileConflict(conflict);
            switch (conflict.response) {
            case FileConflictRequest::Response::Cancel:
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                break;
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
            if (result.cancelled)
                break;
        }
        if (!ensureParentWithinRoot(request.targetPath, outputPath)) {
            result.error = ArchiveError::PermissionDenied;
            result.message = item.path;
            break;
        }
        if (item.type == EntryType::Symlink) {
            if (!symlinkTargetWithinRoot(request.targetPath, outputPath, item.linkTarget)
                    || !ensureParentWithinRoot(request.targetPath, outputPath)) {
                result.error = ArchiveError::PermissionDenied;
                result.message = item.path;
                break;
            }
            QFile::remove(outputPath);
            if (::symlink(QFile::encodeName(item.linkTarget).constData(),
                          QFile::encodeName(outputPath).constData()) != 0) {
                result.error = ArchiveError::PermissionDenied;
                result.message = outputPath;
                break;
            }
            continue;
        }

        ExtractionOutput output;
        if (!output.open(outputPath)) {
            result.error = output.error();
            result.message = outputPath;
            break;
        }

        observer.onCurrentFile(item.path);
        QByteArray buffer(kCopyBlock, '\0');
        while (true) {
            while (m_paused && !m_cancelled)
                QThread::msleep(20);
            if (m_cancelled) {
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                break;
            }
            const la_ssize_t read = archive_read_data(reader, buffer.data(), buffer.size());
            if (read < 0) {
                result.error = ArchiveError::CorruptArchive;
                result.message = QString::fromUtf8(archive_error_string(reader));
                break;
            }
            if (read == 0)
                break;
            if (!output.write(buffer.constData(), read)) {
                result.error = output.error();
                result.message = outputPath;
                break;
            }
            done += read;
            ProgressInfo progress;
            progress.processedBytes = done;
            observer.onProgress(progress);
        }
        if (!result.ok())
            break;
        if (m_cancelled) {
            result = {ArchiveError::Cancelled, {}, true};
            break;
        }
        if (!output.commit()) {
            result.error = output.error();
            result.message = outputPath;
            break;
        }
        if (request.reportOutputPath)
            result.outputPath = outputPath;
    }

    if (result.ok() && headerStatus != ARCHIVE_EOF) {
        result.error = ArchiveError::CorruptArchive;
        result.message = QString::fromUtf8(archive_error_string(reader));
    }
    archive_read_free(reader);
    if (result.ok()) {
        if (names.shortened())
            result.warning = ArchiveError::LongName;
        ProgressInfo progress;
        progress.percentage = 100.0;
        progress.processedBytes = done;
        observer.onProgress(progress);
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult LibarchiveBackend::addFiles(const AddRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    const QString tempPath = temporaryPathFor(m_path);
    if (tempPath.isEmpty()) {
        result.error = ArchiveError::PermissionDenied;
        notifyFinished(observer, result);
        return {result};
    }

    ::archive *writer = archive_write_new();
    if (!applyWriterFormat(writer, m_path, m_mimeType)
            || archive_write_open_filename(writer, QFile::encodeName(tempPath).constData()) != ARCHIVE_OK) {
        result.error = ArchiveError::PermissionDenied;
        result.message = QString::fromUtf8(archive_error_string(writer));
        archive_write_free(writer);
        QFile::remove(tempPath);
        notifyFinished(observer, result);
        return {result};
    }

    const QFileInfo destination(m_path);
    QStringList excludedPaths = request.excludedSourcePaths;
    excludedPaths << destination.absoluteFilePath() << tempPath;
    const qint64 previousSize = destination.exists()
            ? (request.existingBytes > 0 ? request.existingBytes : destination.size()) : 0;
    WriteProgress progress(observer, previousSize + compressionSourceBytes(request, excludedPaths, m_cancelled));
    if (destination.exists()) {
        ::archive *reader = archive_read_new();
        archive_read_support_filter_all(reader);
        archive_read_support_format_all(reader);
        if (archive_read_open_filename(reader, QFile::encodeName(m_path).constData(), kBlockSize) != ARCHIVE_OK) {
            result.error = ArchiveError::CorruptArchive;
            result.message = QString::fromUtf8(archive_error_string(reader));
            archive_read_free(reader);
            archive_write_fail(writer);
            archive_write_free(writer);
            QFile::remove(tempPath);
            notifyFinished(observer, result);
            return {result};
        }
        const bool copied = copyExistingEntries(reader, writer, result, m_paused, m_cancelled, progress);
        archive_read_free(reader);
        if (!copied) {
            archive_write_fail(writer);
            archive_write_free(writer);
            QFile::remove(tempPath);
            notifyFinished(observer, result);
            return {result};
        }
    }

    if (!writeNewEntries(writer, request.entries, request.destinationPath, result,
                         m_paused, m_cancelled, excludedPaths, observer, progress)) {
        archive_write_fail(writer);
        archive_write_free(writer);
        QFile::remove(tempPath);
        notifyFinished(observer, result);
        return {result};
    }

    if (archive_write_close(writer) != ARCHIVE_OK) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromUtf8(archive_error_string(writer));
        archive_write_free(writer);
        QFile::remove(tempPath);
        notifyFinished(observer, result);
        return {result};
    }
    archive_write_free(writer);

    if (!waitForWriteResume(m_paused, m_cancelled)) {
        QFile::remove(tempPath);
        result = {ArchiveError::Cancelled, {}, true};
        notifyFinished(observer, result);
        return {result};
    }

    if (result.ok() && QFileInfo::exists(m_path))
        QFile::setPermissions(tempPath, QFileInfo(m_path).permissions());
    if (!replaceFileAtomically(tempPath, m_path)) {
        result.error = ArchiveError::PermissionDenied;
        result.message = QString::fromLocal8Bit(std::strerror(errno));
        QFile::remove(tempPath);
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult LibarchiveBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    return {};
}

VoidResult LibarchiveBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    return {};
}

VoidResult LibarchiveBackend::deleteFiles(const DeleteRequest &request)
{
    return request.entries.isEmpty() ? VoidResult{} : rebuild(request.entries, nullptr);
}

VoidResult LibarchiveBackend::renameFiles(const RenameRequest &request)
{
    return rebuild({}, &request);
}

VoidResult LibarchiveBackend::addComment(const QString &)
{
    // libarchive 不提供通用归档注释写接口；保持成功 no-op。
    return {};
}

VoidResult LibarchiveBackend::updateArchiveData(const UpdateRequest &)
{
    const EntryResult listed = list();
    return {listed.result};
}

VoidResult LibarchiveBackend::rebuild(const QVector<ArchiveEntry> &removed, const RenameRequest *rename)
{
    ArchiveResult result;
    const QString tempPath = temporaryPathFor(m_path);
    if (tempPath.isEmpty()) {
        result.error = ArchiveError::PermissionDenied;
        return {result};
    }

    ::archive *reader = archive_read_new();
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);
    if (archive_read_open_filename(reader, QFile::encodeName(m_path).constData(), kBlockSize) != ARCHIVE_OK) {
        result.error = ArchiveError::CorruptArchive;
        archive_read_free(reader);
        QFile::remove(tempPath);
        return {result};
    }

    ::archive *writer = archive_write_new();
    if (!applyWriterFormat(writer, m_path, m_mimeType)
            || archive_write_open_filename(writer, QFile::encodeName(tempPath).constData()) != ARCHIVE_OK) {
        result.error = ArchiveError::PermissionDenied;
        archive_read_free(reader);
        archive_write_free(writer);
        QFile::remove(tempPath);
        return {result};
    }

    QSet<int> removeSet;
    QStringList removeDirectories;
    for (const ArchiveEntry &item : removed) {
        if (item.type == EntryType::Directory && !item.path.isEmpty())
            removeDirectories.append(item.path);
        else if (item.index >= 0)
            removeSet.insert(item.index);
    }
    const bool renameTree = rename && rename->entry.type == EntryType::Directory;
    const auto renamed = [&](const QString &path, int entryIndex) {
        if (renameTree && archivePathWithin(path, rename->entry.path))
            return renamedArchivePath(path, rename->entry.path, rename->newName);
        return rename && !renameTree && entryIndex == rename->entry.index ? rename->newName : path;
    };
    QSet<QString> renamedPaths;
    QSet<QString> unchangedPaths;
    bool changed = false;

    ::archive_entry *entry = nullptr;
    int index = 0;
    int headerStatus = ARCHIVE_OK;
    while ((headerStatus = archive_read_next_header(reader, &entry)) == ARCHIVE_OK
           || headerStatus == ARCHIVE_WARN) {
        if (m_cancelled) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            break;
        }
        const int current = index++;
        const QString path = entryFromHeader(entry, current).path;
        if (removeSet.contains(current) || std::any_of(removeDirectories.cbegin(), removeDirectories.cend(),
                [&](const QString &directory) { return archivePathWithin(path, directory); })) {
            changed = true;
            if (archive_read_data_skip(reader) < ARCHIVE_WARN) {
                result.error = ArchiveError::CorruptArchive;
                result.message = QString::fromUtf8(archive_error_string(reader));
                break;
            }
            continue;
        }

        ::archive_entry *outEntry = archive_entry_clone(entry);
        if (!outEntry) {
            result.error = ArchiveError::Unknown;
            result.message = QStringLiteral("failed to clone archive entry");
            break;
        }
        const QString newPath = renamed(path, current);
        if (rename) {
            const bool moved = path != newPath;
            QString key = normalizedArchivePath(newPath);
            if (key.endsWith(QLatin1Char('/')))
                key.chop(1);
            if ((moved ? unchangedPaths : renamedPaths).contains(key)) {
                archive_entry_free(outEntry);
                result = {ArchiveError::PermissionDenied, newPath};
                break;
            }
            (moved ? renamedPaths : unchangedPaths).insert(key);
            if (moved) {
                archive_entry_set_pathname(outEntry, newPath.toUtf8().constData());
                changed = true;
            }
            if (const char *link = archive_entry_hardlink(outEntry)) {
                const QString linkPath = QString::fromUtf8(link);
                if (renameTree && archivePathWithin(linkPath, rename->entry.path))
                    archive_entry_set_hardlink(outEntry, renamedArchivePath(linkPath, rename->entry.path,
                                                                          rename->newName).toUtf8().constData());
                else if (!renameTree && normalizedArchivePath(linkPath) == normalizedArchivePath(rename->entry.path))
                    archive_entry_set_hardlink(outEntry, rename->newName.toUtf8().constData());
            }
        }

        if (archive_write_header(writer, outEntry) != ARCHIVE_OK) {
            result.error = ArchiveError::Unknown;
            result.message = QString::fromUtf8(archive_error_string(writer));
            archive_entry_free(outEntry);
            break;
        }

        QByteArray buffer(kCopyBlock, '\0');
        while (true) {
            const la_ssize_t read = archive_read_data(reader, buffer.data(), buffer.size());
            if (read < 0) {
                result.error = ArchiveError::CorruptArchive;
                result.message = QString::fromUtf8(archive_error_string(reader));
                break;
            }
            if (read == 0)
                break;
            if (m_cancelled) {
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                break;
            }
            if (archive_write_data(writer, buffer.constData(), size_t(read)) != read) {
                result.error = ArchiveError::Unknown;
                result.message = QString::fromUtf8(archive_error_string(writer));
                break;
            }
        }
        archive_entry_free(outEntry);
        if (!result.ok())
            break;
    }

    if (headerStatus != ARCHIVE_EOF && result.ok()) {
        result.error = ArchiveError::CorruptArchive;
        result.message = QString::fromUtf8(archive_error_string(reader));
    }
    if (result.ok() && !changed)
        result.error = ArchiveError::NotFound;
    if (result.ok() && archive_write_close(writer) != ARCHIVE_OK) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromUtf8(archive_error_string(writer));
    }
    if (!result.ok())
        archive_write_fail(writer);
    archive_write_free(writer);
    archive_read_free(reader);

    if (result.ok() && m_cancelled)
        result = {ArchiveError::Cancelled, {}, true};
    if (result.ok())
        QFile::setPermissions(tempPath, QFileInfo(m_path).permissions());
    if (!result.ok()) {
        QFile::remove(tempPath);
    } else if (!replaceFileAtomically(tempPath, m_path)) {
        result.error = ArchiveError::PermissionDenied;
        result.message = QString::fromLocal8Bit(std::strerror(errno));
        QFile::remove(tempPath);
    }
    return {result};
}

void LibarchiveBackend::pause()
{
    m_paused = true;
}

void LibarchiveBackend::resume()
{
    m_paused = false;
}

void LibarchiveBackend::cancel()
{
    m_cancelled = true;
}

void LibarchiveBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
