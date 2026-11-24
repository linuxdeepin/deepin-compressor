// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libzip_backend.h"
#include "extraction_names.h"
#include "write_progress.h"

#include "encoding_utils.h"
#include "extraction_output.h"
#include "path_guard.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QTemporaryFile>
#include <QThread>

#include <archive.h>
#include <archive_entry.h>
#include <zip.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <memory>

namespace dcarchive {
namespace {

constexpr int kChunkSize = 64 * 1024;

bool needsAesCompatibilityReader(const zip_stat_t &stat)
{
    const auto method = stat.encryption_method;
    return (stat.valid & ZIP_STAT_ENCRYPTION_METHOD)
            && (method == ZIP_EM_AES_128 || method == ZIP_EM_AES_192 || method == ZIP_EM_AES_256)
            && (!(stat.valid & ZIP_STAT_CRC) || stat.crc == 0);
}

// libzip 1.7 treats AE-2's omitted CRC as a real CRC for larger entries.
// Use libarchive's authenticated ZIP stream for those entries, not a short read
// that would stop before integrity checks. Output paths remain under libzip's control.
class AesCompatibilityReader
{
public:
    explicit AesCompatibilityReader(const QString &path) : m_path(QFile::encodeName(path)) {}

    bool seek(const QByteArray &name, qint64 size, const QByteArray &password,
              const std::atomic_bool &cancelled)
    {
        if (m_reader && m_password == password && find(name, size, cancelled))
            return true;
        if (cancelled)
            return false;
        m_reader.reset(archive_read_new());
        m_password = password;
        return m_reader && archive_read_support_format_zip_seekable(m_reader.get()) == ARCHIVE_OK
                && archive_read_add_passphrase(m_reader.get(), password.constData()) == ARCHIVE_OK
                && archive_read_open_filename(m_reader.get(), m_path.constData(), kChunkSize) == ARCHIVE_OK
                && find(name, size, cancelled);
    }

    qint64 read(char *buffer, size_t size)
    {
        const auto count = archive_read_data(m_reader.get(), buffer, size);
        if (count < 0)
            m_reader.reset();
        return count;
    }

private:
    bool find(const QByteArray &name, qint64 size, const std::atomic_bool &cancelled)
    {
        // A seekable ZIP reader visits entries by local file offset. Reuse it
        // for entries in that order; a central directory may list them differently.
        ::archive_entry *entry = nullptr;
        while (!cancelled && archive_read_next_header(m_reader.get(), &entry) == ARCHIVE_OK) {
            const char *path = archive_entry_pathname(entry);
            if (path && name == path)
                return archive_entry_is_data_encrypted(entry) && archive_entry_size_is_set(entry)
                        && archive_entry_size(entry) == size;
        }
        return false;
    }

    const QByteArray m_path;
    QByteArray m_password;
    std::unique_ptr<::archive, decltype(&archive_read_free)> m_reader{nullptr, &archive_read_free};
};

ArchiveEntry entryFromStat(const zip_stat_t &stat, int index)
{
    ArchiveEntry entry;
    entry.path = stat.name ? decodeArchiveName(stat.name, int(strlen(stat.name))) : QString();
    entry.name = QFileInfo(entry.path).fileName();
    if (entry.name.isEmpty() && entry.path.endsWith(QLatin1Char('/')))
        entry.name = QFileInfo(entry.path.chopped(1)).fileName();
    entry.size = stat.valid & ZIP_STAT_SIZE ? qint64(stat.size) : 0;
    entry.compressedSize = stat.valid & ZIP_STAT_COMP_SIZE ? qint64(stat.comp_size) : 0;
    if (stat.valid & ZIP_STAT_MTIME)
        entry.modified = QDateTime::fromSecsSinceEpoch(stat.mtime);
    entry.type = entry.path.endsWith(QLatin1Char('/')) ? EntryType::Directory : EntryType::File;
    entry.index = index;
    return entry;
}

QString joinArchiveName(const QString &prefix, const QString &name)
{
    const QString normalizedName = normalizedArchivePath(name);
    const QString normalizedPrefix = normalizedArchivePath(prefix);
    if (normalizedPrefix.isEmpty())
        return normalizedName;
    return (normalizedPrefix.endsWith(QLatin1Char('/'))
            ? normalizedPrefix : normalizedPrefix + QLatin1Char('/')) + normalizedName;
}

bool addZipDirectory(zip_t *archive, const QString &name, ArchiveResult &result)
{
    QByteArray path = name.toUtf8();
    if (!path.endsWith('/'))
        path.append('/');
    if (zip_dir_add(archive, path.constData(), 0) < 0
            && zip_error_code_zip(zip_get_error(archive)) != ZIP_ER_EXISTS) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromUtf8(path);
        return false;
    }
    return true;
}

bool addZipFile(zip_t *archive, const QString &sourcePath, const QString &archiveName,
                const QByteArray &password, ArchiveResult &result)
{
    zip_source_t *source = zip_source_file(
            archive, QFile::encodeName(sourcePath).constData(), 0, -1);
    if (!source) {
        result.error = ArchiveError::NotFound;
        result.message = sourcePath;
        return false;
    }
    const QByteArray name = archiveName.toUtf8();
    const zip_int64_t index = zip_file_add(archive, name.constData(), source, ZIP_FL_OVERWRITE);
    if (index < 0) {
        zip_source_free(source);
        result.error = ArchiveError::Unknown;
        result.message = archiveName;
        return false;
    }
    if (!password.isEmpty() && zip_file_set_encryption(archive, zip_uint64_t(index),
                                                      ZIP_EM_AES_256, password.constData()) != 0) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromLocal8Bit(zip_strerror(archive));
        return false;
    }
    return true;
}

bool addZipDirectoryTree(zip_t *archive, const QString &sourceDir, const QString &archiveRoot,
                         const QByteArray &password, ArchiveResult &result,
                         const QStringList &excludedPaths)
{
    if (!addZipDirectory(archive, archiveRoot, result))
        return false;
    const QDir root(sourceDir);
    QDirIterator it(sourceDir,
                    QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString sourcePath = it.next();
        if (std::any_of(excludedPaths.cbegin(), excludedPaths.cend(), [&](const QString &path) {
                return archivePathWithin(it.fileInfo().absoluteFilePath(), path);
            }))
            continue;
        const QString name = joinArchiveName(archiveRoot, root.relativeFilePath(sourcePath));
        if (it.fileInfo().isDir()) {
            if (!addZipDirectory(archive, name, result))
                return false;
        } else if (!addZipFile(archive, sourcePath, name, password, result)) {
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

bool closeZipForWrite(zip_t *archive, const std::atomic_bool &paused,
                      const std::atomic_bool &cancelled, ArchiveResult &result)
{
    struct WriteState {
        const std::atomic_bool &paused;
        const std::atomic_bool &cancelled;
    } state{paused, cancelled};
    if (result.ok() && !waitForWriteResume(paused, cancelled))
        result = {ArchiveError::Cancelled, {}, true};
    if (result.ok()) {
        const auto check = [](zip_t *, void *context) -> int {
            const auto &write = *static_cast<const WriteState *>(context);
            return !waitForWriteResume(write.paused, write.cancelled);
        };
        if (zip_register_cancel_callback_with_state(archive, check, nullptr, &state) == 0
                && zip_close(archive) == 0)
            return true;
        if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_CANCELLED)
            result = {ArchiveError::Cancelled, {}, true};
        else
            result = {ArchiveError::Unknown, QString::fromLocal8Bit(zip_strerror(archive))};
    }
    zip_discard(archive);
    return false;
}

} // namespace

LibzipBackend::LibzipBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
}

LibzipBackend::~LibzipBackend() = default;

BackendCapabilities LibzipBackend::capabilities() const
{
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.create = true;
    caps.append = true;
    caps.move = true;
    caps.copy = true;
    caps.remove = true;
    caps.rename = true;
    caps.comment = true;
    caps.update = true;
    caps.password = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;
    return caps;
}

VoidResult LibzipBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_password = request.password.toUtf8();
    m_cancelled = false;
    m_paused = false;
    return {};
}

zip_t *LibzipBackend::openArchive(int flags, ArchiveResult &result) const
{
    return openArchiveAt(m_path, flags, result);
}

zip_t *LibzipBackend::openArchiveAt(const QString &path, int flags, ArchiveResult &result) const
{
    int errorCode = 0;
    zip_t *archive = zip_open(QFile::encodeName(path).constData(), flags, &errorCode);
    if (!archive) {
        result.error = ArchiveError::CorruptArchive;
        zip_error_t error;
        zip_error_init_with_code(&error, errorCode);
        result.message = QString::fromLocal8Bit(zip_error_strerror(&error));
        zip_error_fini(&error);
        return nullptr;
    }
    return archive;
}

EntryResult LibzipBackend::list()
{
    EntryResult result;
    ArchiveResult openResult;
    zip_t *archive = openArchive(ZIP_RDONLY, openResult);
    if (!archive) {
        result.result = openResult;
        return result;
    }

    int commentLength = 0;
    const char *comment = zip_get_archive_comment(archive, &commentLength, ZIP_FL_ENC_RAW);
    if (comment && commentLength > 0)
        result.info.comment = decodeArchiveName(comment, commentLength);

    const zip_int64_t count = zip_get_num_entries(archive, 0);
    for (zip_int64_t index = 0; index < count; ++index) {
        zip_stat_t stat;
        zip_stat_init(&stat);
        if (zip_stat_index(archive, zip_uint64_t(index), ZIP_FL_ENC_RAW, &stat) != 0)
            continue;

        ArchiveEntry entry = entryFromStat(stat, int(index));
        result.info.entries.append(entry);
        result.info.totalSize += entry.size;
        result.info.compressedSize += entry.compressedSize;
    }

    zip_close(archive);
    return result;
}

VoidResult LibzipBackend::testArchive()
{
    // 与旧实现保持一致：只做打开/关闭，不做全量 CRC 校验。
    ArchiveResult result;
    zip_t *archive = openArchive(ZIP_RDONLY, result);
    if (!archive)
        return {result};
    zip_close(archive);
    return {};
}

QString LibzipBackend::safeOutputPath(const QString &targetDir, const QString &entryPath) const
{
    QString clean = QDir::cleanPath(entryPath);
    if (clean.startsWith(QLatin1Char('/')) || clean == QLatin1String("..")
            || clean.startsWith(QLatin1String("../")))
        return {};
    return QDir(targetDir).filePath(clean);
}

VoidResult LibzipBackend::extractFiles(const ExtractRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (!QDir().mkpath(request.targetPath)) {
        result.error = ArchiveError::PermissionDenied;
        notifyFinished(observer, result);
        return {result};
    }
    zip_t *archive = openArchive(ZIP_RDONLY, result);
    if (!archive) {
        notifyFinished(observer, result);
        return {result};
    }

    QVector<ArchiveEntry> archiveEntries;
    QSet<int> aesEntries;
    const zip_int64_t count = zip_get_num_entries(archive, 0);
    for (zip_int64_t index = 0; index < count; ++index) {
        zip_stat_t stat;
        zip_stat_init(&stat);
        if (zip_stat_index(archive, zip_uint64_t(index), ZIP_FL_ENC_RAW, &stat) == 0) {
            archiveEntries.append(entryFromStat(stat, int(index)));
            if (needsAesCompatibilityReader(stat))
                aesEntries.insert(int(index));
        }
    }

    QHash<QByteArray, int> rawNameCounts;
    if (!aesEntries.isEmpty()) {
        for (const auto &entry : archiveEntries) {
            if (const char *rawName = zip_get_name(archive, entry.index, ZIP_FL_ENC_RAW))
                ++rawNameCounts[QByteArray(rawName)];
        }
    }

    QSet<int> selected;
    QVector<ArchiveEntry> selectedDirectories;
    for (const ArchiveEntry &entry : request.entries) {
        selected.insert(entry.index);
        if (entry.type == EntryType::Directory)
            selectedDirectories.append(entry);
    }
    QVector<ArchiveEntry> entries;
    for (const ArchiveEntry &entry : archiveEntries) {
        bool selectedByDirectory = false;
        for (const ArchiveEntry &directory : selectedDirectories) {
            if (archivePathWithin(entry.path, directory.path)) {
                selectedByDirectory = true;
                break;
            }
        }
        if (request.allEntries || selected.contains(entry.index) || selectedByDirectory)
            entries.append(entry);
    }

    bool overwriteAll = request.overwrite == OverwritePolicy::Overwrite
            || request.overwrite == OverwritePolicy::OverwriteAll;
    bool skipAll = request.overwrite == OverwritePolicy::Skip
            || request.overwrite == OverwritePolicy::SkipAll;
    qint64 done = 0;
    std::unique_ptr<AesCompatibilityReader> aesReader;
    ExtractionNames names(ExtractionNames::Suffix::Zip);
    QByteArray buffer;

    for (int i = 0; i < entries.size(); ++i) {
        if (m_cancelled) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            break;
        }
        while (m_paused && !m_cancelled)
            QThread::msleep(20);
        if (m_cancelled) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            break;
        }

        const ArchiveEntry &entry = entries.at(i);
        const QString relativePath = archivePathRelativeTo(entry.path, request.destinationPrefix);
        QString outputPath = safeOutputPath(request.targetPath, relativePath);
        if (outputPath.isEmpty()) {
            result = {ArchiveError::PermissionDenied, entry.path};
            break;
        }
        const QString diskName = names.outputName(relativePath, entry.type == EntryType::Directory);
        if (diskName != relativePath) {
            if (diskName.isEmpty()) {
                observer.onCurrentFile(entry.path);
                result = {ArchiveError::LongName, entry.path};
                break;
            }
            outputPath = safeOutputPath(request.targetPath, diskName);
            observer.onCurrentFile(entry.path);
        }
        if (entry.type == EntryType::Directory) {
            if (!ensureDirectoryWithinRoot(request.targetPath, outputPath)) {
                result.error = ArchiveError::PermissionDenied;
                result.message = entry.path;
                break;
            }
            continue;
        }

        const bool hasExistingOutput = !overwriteAll && QFile::exists(outputPath);
        if (hasExistingOutput && skipAll)
            continue;
        if (hasExistingOutput) {
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
            result.message = entry.path;
            break;
        }
        zip_file_t *zipFile = nullptr;
        while (!m_cancelled) {
            zipFile = zip_fopen_index_encrypted(archive, zip_uint64_t(entry.index), 0,
                                              m_password.isEmpty() ? nullptr : m_password.constData());
            if (zipFile)
                break;
            const int errorCode = zip_error_code_zip(zip_get_error(archive));
            if (errorCode != ZIP_ER_NOPASSWD && errorCode != ZIP_ER_WRONGPASSWD) {
                result.error = ArchiveError::CorruptArchive;
                result.message = entry.path;
                break;
            }
            PasswordRequest password;
            observer.onPasswordRequired(password);
            if (password.response == PasswordRequest::Response::Cancel) {
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                break;
            }
            if (password.response != PasswordRequest::Response::Provide || password.password.isEmpty()) {
                result.error = errorCode == ZIP_ER_WRONGPASSWD
                        ? ArchiveError::WrongPassword : ArchiveError::PasswordRequired;
                result.message = entry.path;
                break;
            }
            m_password = password.password.toUtf8();
        }
        if (!zipFile) {
            if (m_cancelled) {
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
            }
            break;
        }

        const bool useAesReader = aesEntries.contains(entry.index);
        if (useAesReader) {
            // libzip has checked the password verifier. libarchive checks the
            // full authentication tag as we stream this same entry to disk.
            zip_fclose(zipFile);
            zipFile = nullptr;
            const QByteArray rawName(zip_get_name(archive, entry.index, ZIP_FL_ENC_RAW));
            if (!aesReader)
                aesReader.reset(new AesCompatibilityReader(m_path));
            if (rawName.isEmpty() || rawNameCounts.value(rawName) != 1
                    || !aesReader->seek(rawName, entry.size, m_password, m_cancelled)) {
                result = m_cancelled ? ArchiveResult{ArchiveError::Cancelled, {}, true}
                                     : ArchiveResult{ArchiveError::CorruptArchive, entry.path};
                break;
            }
        }

        ExtractionOutput output;
        if (!output.open(outputPath)) {
            if (zipFile)
                zip_fclose(zipFile);
            result.error = output.error();
            result.message = outputPath;
            break;
        }

        observer.onCurrentFile(entry.path);
        if (buffer.isEmpty())
            buffer.resize(kChunkSize);
        qint64 entryBytes = 0;
        while (true) {
            while (m_paused && !m_cancelled)
                QThread::msleep(20);
            if (m_cancelled) {
                result.error = ArchiveError::Cancelled;
                result.cancelled = true;
                break;
            }
            const qint64 read = useAesReader ? aesReader->read(buffer.data(), buffer.size())
                                            : zip_fread(zipFile, buffer.data(), buffer.size());
            if (read < 0) {
                result.error = ArchiveError::CorruptArchive;
                result.message = entry.path;
                break;
            }
            if (read == 0)
                break;
            if (!output.write(buffer.constData(), read)) {
                result.error = output.error();
                result.message = outputPath;
                break;
            }
            entryBytes += read;
            done += read;
            ProgressInfo progress;
            progress.processedBytes = done;
            observer.onProgress(progress);
        }
        const bool closeFailed = zipFile && zip_fclose(zipFile) != 0;
        if (result.error == ArchiveError::Cancelled && request.keepIncompleteOnCancel
                && request.allEntries && !request.reportOutputPath
                && entryBytes > 0 && entryBytes < entry.size) {
            zip_stat_t stat;
            zip_stat_init(&stat);
            if (zip_stat_index(archive, zip_uint64_t(entry.index), ZIP_FL_ENC_RAW, &stat) == 0
                    && (stat.valid & ZIP_STAT_ENCRYPTION_METHOD) && stat.encryption_method == ZIP_EM_NONE)
                output.keepIncompleteFile();
        }
        if (result.ok() && (closeFailed || (useAesReader && entryBytes != entry.size))) {
            result.error = ArchiveError::CorruptArchive;
            result.message = entry.path;
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

    zip_close(archive);
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

VoidResult LibzipBackend::addFiles(const AddRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    const QString tempPath = temporaryPathFor(m_path);
    if (tempPath.isEmpty()) {
        result.error = ArchiveError::PermissionDenied;
        notifyFinished(observer, result);
        return {result};
    }
    const bool hadArchive = QFileInfo::exists(m_path);
    if (hadArchive) {
        QFile::remove(tempPath);
        if (!QFile::copy(m_path, tempPath)) {
            result.error = ArchiveError::PermissionDenied;
            notifyFinished(observer, result);
            return {result};
        }
    } else {
        QFile::remove(tempPath);
    }
    zip_t *archive = openArchiveAt(tempPath, ZIP_CREATE, result);
    if (!archive) {
        QFile::remove(tempPath);
        notifyFinished(observer, result);
        return {result};
    }

    const QByteArray password = request.password.toUtf8();
    QStringList excludedPaths = request.excludedSourcePaths;
    excludedPaths << QFileInfo(m_path).absoluteFilePath() << tempPath;
    for (const ArchiveEntry &entry : request.entries) {
        observer.onCurrentFile(entry.path);
        if (!waitForWriteResume(m_paused, m_cancelled)) {
            result.error = ArchiveError::Cancelled;
            result.cancelled = true;
            break;
        }
        const QString sourceName = entry.name.isEmpty()
                ? QFileInfo(entry.path).fileName() : entry.name;
        if (sourceName.isEmpty()) {
            result.error = ArchiveError::NotFound;
            result.message = entry.path;
            break;
        }
        const QString archiveName = joinArchiveName(request.destinationPath, sourceName);
        if (entry.type == EntryType::Directory) {
            if (!addZipDirectoryTree(archive, entry.path, archiveName, password, result, excludedPaths))
                break;
        } else if (!addZipFile(archive, entry.path, archiveName, password, result)) {
            break;
        }
    }

    if (result.ok() && m_cancelled) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    }
    WriteProgress progress(observer);
    if (result.ok()) {
        zip_register_progress_callback_with_state(archive, 0.001, [](zip_t *, double fraction, void *state) {
            static_cast<WriteProgress *>(state)->report(fraction * 100.0);
        }, nullptr, &progress);
    }
    closeZipForWrite(archive, m_paused, m_cancelled, result);
    if (result.ok() && m_cancelled) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    }
    if (!result.ok()) {
        QFile::remove(tempPath);
    } else {
        if (hadArchive)
            QFile::setPermissions(tempPath, QFileInfo(m_path).permissions());
        if (::rename(QFile::encodeName(tempPath).constData(),
                     QFile::encodeName(m_path).constData()) != 0) {
            result.error = ArchiveError::PermissionDenied;
            result.message = QString::fromLocal8Bit(std::strerror(errno));
            QFile::remove(tempPath);
        }
    }
    notifyFinished(observer, result);
    return {result};
}

VoidResult LibzipBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    // 与旧实现保持一致：成功 no-op。
    return {};
}

VoidResult LibzipBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    // 与旧实现保持一致：成功 no-op。
    return {};
}

VoidResult LibzipBackend::deleteFiles(const DeleteRequest &request)
{
    if (request.entries.isEmpty())
        return {};
    ArchiveResult result;
    zip_t *archive = openArchive(0, result);
    if (!archive)
        return {result};

    QSet<int> indexes;
    QStringList directories;
    for (const ArchiveEntry &entry : request.entries) {
        if (entry.type == EntryType::Directory && !entry.path.isEmpty())
            directories.append(entry.path);
        else if (entry.index >= 0)
            indexes.insert(entry.index);
    }
    bool changed = false;
    const auto count = zip_get_num_entries(archive, 0);
    for (zip_int64_t index = 0; index < count && !m_cancelled; ++index) {
        bool selected = indexes.contains(int(index));
        if (!selected && !directories.isEmpty()) {
            const char *raw = zip_get_name(archive, index, ZIP_FL_ENC_RAW);
            const QString path = raw ? decodeArchiveName(raw, int(strlen(raw))) : QString();
            selected = std::any_of(directories.cbegin(), directories.cend(),
                                   [&](const QString &directory) { return archivePathWithin(path, directory); });
        }
        if (!selected)
            continue;
        if (zip_delete(archive, index) != 0) {
            result = {ArchiveError::Unknown, QString::fromLocal8Bit(zip_strerror(archive))};
            break;
        }
        changed = true;
    }
    if (result.ok() && !changed && !m_cancelled)
        result.error = ArchiveError::NotFound;
    closeZipForWrite(archive, m_paused, m_cancelled, result);
    return {result};
}

VoidResult LibzipBackend::renameFiles(const RenameRequest &request)
{
    ArchiveResult result;
    zip_t *archive = openArchive(0, result);
    if (!archive)
        return {result};

    bool changed = false;
    const auto count = zip_get_num_entries(archive, 0);
    for (zip_int64_t index = 0; index < count && !m_cancelled; ++index) {
        QString newPath;
        if (request.entry.type == EntryType::Directory && !request.entry.path.isEmpty()) {
            const char *raw = zip_get_name(archive, index, ZIP_FL_ENC_RAW);
            const QString path = raw ? decodeArchiveName(raw, int(strlen(raw))) : QString();
            if (archivePathWithin(path, request.entry.path))
                newPath = renamedArchivePath(path, request.entry.path, request.newName);
        } else if (index == request.entry.index) {
            newPath = request.newName;
        }
        if (newPath.isEmpty())
            continue;
        if (zip_file_rename(archive, index, newPath.toUtf8().constData(), ZIP_FL_ENC_UTF_8) != 0) {
            result = {ArchiveError::Unknown, QString::fromLocal8Bit(zip_strerror(archive))};
            break;
        }
        changed = true;
    }
    if (result.ok() && !changed && !m_cancelled)
        result.error = ArchiveError::NotFound;
    closeZipForWrite(archive, m_paused, m_cancelled, result);
    return {result};
}

VoidResult LibzipBackend::addComment(const QString &comment)
{
    BackendObserver observer;
    return addComment(comment, observer);
}

VoidResult LibzipBackend::addComment(const QString &comment, BackendObserver &observer)
{
    ArchiveResult result;
    zip_t *archive = openArchive(ZIP_CREATE, result);
    if (!archive)
        return {result};

    const QByteArray bytes = comment.toUtf8();
    if (zip_set_archive_comment(archive, bytes.constData(), zip_uint16_t(bytes.size())) != 0) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromLocal8Bit(zip_strerror(archive));
        zip_discard(archive);
        return {result};
    }

    WriteProgress progress(observer);
    zip_register_progress_callback_with_state(archive, 0.001, [](zip_t *, double fraction, void *state) {
        static_cast<WriteProgress *>(state)->report(fraction * 100.0);
    }, nullptr, &progress);
    observer.onProgress({});
    if (zip_close(archive) != 0) {
        result.error = ArchiveError::Unknown;
        result.message = QString::fromLocal8Bit(zip_strerror(archive));
        zip_discard(archive);
    }
    return {result};
}

VoidResult LibzipBackend::updateArchiveData(const UpdateRequest &)
{
    const EntryResult listed = list();
    return {listed.result};
}

void LibzipBackend::pause()
{
    m_paused = true;
}

void LibzipBackend::resume()
{
    m_paused = false;
}

void LibzipBackend::cancel()
{
    m_cancelled = true;
}

void LibzipBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
