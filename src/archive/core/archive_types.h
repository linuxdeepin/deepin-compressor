// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_TYPES_H
#define ARCHIVE_TYPES_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QDateTime>

namespace archive {

// ---------------------------------------------------------------------------
// Operation types — used by ArchiveRegistry to route to backends that
// actually support the requested operation, not merely "read" vs "write".
// ---------------------------------------------------------------------------
enum class ArchiveOperation {
    Open,     // open an archive for listing / extraction
    List,     // list archive contents
    Test,     // test archive integrity
    Extract,  // extract files from archive
    Add,      // add files to archive
    Move,     // move files within archive
    Copy,     // copy files within archive
    Delete,   // delete files from archive
    Rename,   // rename files within archive
    Comment,  // add / update archive comment
    Update    // update archive metadata
};

// ---------------------------------------------------------------------------
// Result types
// ---------------------------------------------------------------------------

enum class ErrorCode {
    NoError = 0,
    NotSupported,
    Cancelled,
    WrongPassword,
    NeedPassword,
    CorruptArchive,
    DiskFull,
    FileOpenError,
    FileReadError,
    FileWriteError,
    FileDeleteError,
    FileRenameError,
    MissingVolume,
    InsufficientDiskSpace,
    LongNameError,
    ToolMissing,
    PluginError,
    Unknown
};

struct VoidResult {
    bool success = false;
    ErrorCode code = ErrorCode::NoError;
    QString message;

    static VoidResult ok()
    {
        return {true, ErrorCode::NoError, {}};
    }

    static VoidResult fail(ErrorCode c, const QString &msg = {})
    {
        return {false, c, msg};
    }

    static VoidResult notSupported()
    {
        return {false, ErrorCode::NotSupported,
                QStringLiteral("operation not supported")};
    }
};

struct ArchiveEntry {
    QString fullPath;
    QString fileName;
    QString alias;
    bool isDirectory = false;
    qint64 size = 0;
    QDateTime lastModified;
    int index = -1;
};

struct EntryResult {
    bool success = false;
    ErrorCode code = ErrorCode::NoError;
    QString message;
    QList<ArchiveEntry> entries;

    static EntryResult ok(const QList<ArchiveEntry> &e = {})
    {
        return {true, ErrorCode::NoError, {}, e};
    }

    static EntryResult fail(ErrorCode c, const QString &msg = {})
    {
        return {false, c, msg, {}};
    }
};

// ---------------------------------------------------------------------------
// Request types
// ---------------------------------------------------------------------------

struct OpenRequest {
    QString archivePath;
    QString password;
    bool headerEncrypted = false;
};

struct ExtractRequest {
    QStringList entries;       // empty = extract all
    QString targetPath;
    QString password;
    bool preservePaths = true;
    bool overwriteAll = false;
    bool skipAll = false;
};

struct AddRequest {
    QStringList files;
    QString password;
    bool encrypt = false;
    bool headerEncrypt = false;
    int compressionLevel = -1;
    QString compressionMethod;
    QString encryptionMethod;
    int volumeSize = 0;        // 0 = no split
    int cpuThreads = 1;
};

struct MoveRequest {
    QStringList entries;
    QString targetDir;
};

struct CopyRequest {
    QStringList entries;
    QString targetDir;
};

struct DeleteRequest {
    QStringList entries;
};

struct RenameRequest {
    QStringList entries;
    QStringList newNames;
};

struct UpdateRequest {
    enum Type { Delete, Rename, Add };
    Type type = Delete;
    QString parentPath;
    QStringList entries;
    qint64 size = 0;
};

// ---------------------------------------------------------------------------
// Observer event payloads
// ---------------------------------------------------------------------------

struct ProgressInfo {
    double percentage = 0.0;
    qint64 processedSize = 0;
    qint64 totalSize = 0;
};

struct PasswordRequest {
    QString archivePath;
    QString password;
    bool headerPassword = false;
    bool cancelled = false;
};

struct FileConflictRequest {
    QString path;
    enum Action {
        Cancel = 0,
        Skip,
        SkipAll,
        Overwrite,
        OverwriteAll,
        Readonly
    };
    Action action = Cancel;
};

struct WarningInfo {
    QString message;
    QString details;
};

struct FinishInfo {
    bool success = false;
    ErrorCode code = ErrorCode::NoError;
    QString message;
};

} // namespace archive

#endif // ARCHIVE_TYPES_H
