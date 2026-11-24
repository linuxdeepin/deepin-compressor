// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_TYPES_H
#define ARCHIVE_TYPES_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

namespace dcarchive {

enum class ArchiveError {
    None = 0,
    NotFound,
    PermissionDenied,
    CorruptArchive,
    WrongPassword,
    PasswordRequired,
    LongName,
    DiskFull,
    ToolMissing,
    NotSupported,
    Cancelled,
    Unknown,
    MissingVolume,
};

struct ArchiveResult {
    ArchiveError error = ArchiveError::None;
    QString message;
    bool cancelled = false;
    ArchiveError warning = ArchiveError::None;
    // Populated only when a caller asks to open the extracted file.
    QString outputPath;

    bool ok() const { return error == ArchiveError::None && !cancelled; }
};

enum class OpenMode {
    ReadOnly,
    ReadWrite,
};

enum class EntryType {
    File,
    Directory,
    Symlink,
};

struct ArchiveEntry {
    QString path;
    QString name;
    qint64 size = 0;
    qint64 compressedSize = 0;
    QDateTime modified;
    EntryType type = EntryType::File;
    QString linkTarget;
    int index = -1;
};

struct ArchiveInfo {
    QString comment;
    bool encrypted = false;
    qint64 totalSize = 0;
    qint64 compressedSize = 0;
    QVector<ArchiveEntry> entries;
};

struct OpenRequest {
    QString path;
    OpenMode mode = OpenMode::ReadOnly;
    QString password;
    QString mimeType;
};

enum class OverwritePolicy {
    Ask,
    Overwrite,
    Skip,
    OverwriteAll,
    SkipAll,
};

struct ExtractRequest {
    QString targetPath;
    QString destinationPrefix;
    QVector<ArchiveEntry> entries;
    OverwritePolicy overwrite = OverwritePolicy::Ask;
    bool preserveTimes = true;
    bool allEntries = true;
    bool reportOutputPath = false;
    bool allowPartial = false;
    bool keepIncompleteOnCancel = false;
};

struct AddRequest {
    QString archivePath;
    QString destinationPath;
    QStringList excludedSourcePaths;
    QVector<ArchiveEntry> entries;
    QString format;
    int compressionLevel = 6;
    int cpuThreads = 0;
    QString password;
    bool encryptList = false;
    qint64 splitSize = 0;
    QString comment;
    bool tar7z = false;
    qint64 sourceBytes = 0;
    qint64 existingBytes = 0;
};

struct MoveRequest {
    QVector<ArchiveEntry> entries;
    QString targetPath;
};

struct CopyRequest {
    QVector<ArchiveEntry> entries;
    QString targetPath;
};

struct DeleteRequest {
    QVector<ArchiveEntry> entries;
};

struct RenameRequest {
    ArchiveEntry entry;
    QString newName;
};

struct UpdateRequest {
    ArchiveInfo info;
};

struct ProgressInfo {
    double percentage = 0.0;
    qint64 processedBytes = 0;
    qint64 totalBytes = 0;
};

struct WarningInfo {
    ArchiveError error = ArchiveError::None;
    QString message;
};

struct FinishInfo {
    ArchiveResult result;
};

struct PasswordRequest {
    enum class Response {
        Cancel,
        Retry,
        Provide,
    };

    Response response = Response::Cancel;
    QString password;
};

struct FileConflictRequest {
    enum class Response {
        Cancel,
        Overwrite,
        Skip,
        OverwriteAll,
        SkipAll,
    };

    Response response = Response::Cancel;
    QString path;
};

struct VoidResult {
    ArchiveResult result;
};

struct EntryResult {
    ArchiveResult result;
    ArchiveInfo info;
};

} // namespace dcarchive

#endif // ARCHIVE_TYPES_H
