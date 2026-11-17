// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_backend_adapter.h"

#include "archiveinterface.h"
#include "commonstruct.h"
#include "queries.h"
#include "datamanager.h"

#include <QDebug>

namespace archive {

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

LegacyBackendAdapter::LegacyBackendAdapter(ReadOnlyArchiveInterface *iface)
    : m_readOnlyIface(iface)
    , m_isReadWrite(false)
{
    m_capabilities.list = true;
    m_capabilities.test = true;
    m_capabilities.extract = true;
    m_capabilities.password = true;
    m_capabilities.progress = true;
    m_capabilities.cancel = true;
    m_capabilities.pause = true;
    setupSignalForwarding();
}

LegacyBackendAdapter::LegacyBackendAdapter(ReadOnlyArchiveInterface *iface,
                                           const BackendCapabilities &caps)
    : m_readOnlyIface(iface)
    , m_isReadWrite(false)
    , m_capabilities(caps)
{
    setupSignalForwarding();
}

LegacyBackendAdapter::LegacyBackendAdapter(ReadWriteArchiveInterface *iface)
    : m_readOnlyIface(iface)
    , m_readWriteIface(iface)
    , m_isReadWrite(true)
{
    m_capabilities.list = true;
    m_capabilities.test = true;
    m_capabilities.extract = true;
    m_capabilities.create = true;
    m_capabilities.append = true;
    m_capabilities.move = true;
    m_capabilities.copy = true;
    m_capabilities.remove = true;
    m_capabilities.rename = true;
    m_capabilities.comment = true;
    m_capabilities.update = true;
    m_capabilities.password = true;
    m_capabilities.headerPassword = true;
    m_capabilities.multiVolume = true;
    m_capabilities.progress = true;
    m_capabilities.cancel = true;
    m_capabilities.pause = true;
    setupSignalForwarding();
}

LegacyBackendAdapter::LegacyBackendAdapter(ReadWriteArchiveInterface *iface,
                                           const BackendCapabilities &caps)
    : m_readOnlyIface(iface)
    , m_readWriteIface(iface)
    , m_isReadWrite(true)
    , m_capabilities(caps)
{
    setupSignalForwarding();
}

LegacyBackendAdapter::~LegacyBackendAdapter()
{
    disconnectForwardingConnections();
    if (m_readOnlyIface) {
        delete m_readOnlyIface;
        m_readOnlyIface = nullptr;
        m_readWriteIface = nullptr;
    }
}

ReadOnlyArchiveInterface *LegacyBackendAdapter::releaseInterface()
{
    disconnectForwardingConnections();
    ReadOnlyArchiveInterface *iface = m_readOnlyIface;
    m_readOnlyIface = nullptr;
    m_readWriteIface = nullptr;
    return iface;
}

// ---------------------------------------------------------------------------
// Signal forwarding
// ---------------------------------------------------------------------------

void LegacyBackendAdapter::disconnectForwardingConnections()
{
    for (const QMetaObject::Connection &conn : m_forwardingConnections) {
        QObject::disconnect(conn);
    }
    m_forwardingConnections.clear();
}

void LegacyBackendAdapter::setupSignalForwarding()
{
    if (!m_readOnlyIface) {
        return;
    }

    m_forwardingConnections.append(
        QObject::connect(m_readOnlyIface, &ReadOnlyArchiveInterface::signalprogress,
                     [this](double percentage) {
                         if (m_observer) {
                             ProgressInfo info;
                             info.percentage = percentage;
                             m_observer->onProgress(info);
                         }
                     }));

    m_forwardingConnections.append(
        QObject::connect(m_readOnlyIface, &ReadOnlyArchiveInterface::signalCurFileName,
                     [this](const QString &filename) {
                         if (m_observer) {
                             m_observer->onCurrentFile(filename);
                         }
                     }));

    m_forwardingConnections.append(
        QObject::connect(m_readOnlyIface, &ReadOnlyArchiveInterface::error,
                     [this](const QString &message, const QString &details) {
                         if (m_observer) {
                             WarningInfo warning;
                             warning.message = message;
                             warning.details = details;
                             m_observer->onWarning(warning);
                         }
                     }));

    m_forwardingConnections.append(
        QObject::connect(m_readOnlyIface, &ReadOnlyArchiveInterface::signalQuery,
                     [this](Query *query) {
                         if (!query) {
                             return;
                         }

                         OverwriteQuery *overwriteQ = qobject_cast<OverwriteQuery *>(query);
                         PasswordNeededQuery *passwordQ = qobject_cast<PasswordNeededQuery *>(query);

                         if (passwordQ) {
                             if (!m_observer) {
                                 passwordQ->setResponse(QVariant(static_cast<int>(Result_Cancel)));
                                 return;
                             }
                             PasswordRequest req;
                             req.archivePath = m_readOnlyIface->property("archivePath").toString();
                             req.headerPassword = m_listEncrypted;
                             m_observer->onPasswordRequired(req);
                             if (req.cancelled) {
                                 passwordQ->setResponse(QVariant(static_cast<int>(Result_Cancel)));
                             } else {
                                 passwordQ->setResponse(QVariant(req.password));
                             }
                         } else if (overwriteQ) {
                             // Auto-respond when overwriteAll/skipAll is set.
                             if (m_overwriteAll) {
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_OverwriteAll)));
                                 return;
                             }
                             if (m_skipAll) {
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_SkipAll)));
                                 return;
                             }
                             if (!m_observer) {
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_Cancel)));
                                 return;
                             }
                             FileConflictRequest req;
                             m_observer->onFileConflict(req);
                             switch (req.action) {
                             case FileConflictRequest::Skip:
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_Skip)));
                                 break;
                             case FileConflictRequest::SkipAll:
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_SkipAll)));
                                 break;
                             case FileConflictRequest::Overwrite:
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_Overwrite)));
                                 break;
                             case FileConflictRequest::OverwriteAll:
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_OverwriteAll)));
                                 break;
                             case FileConflictRequest::Readonly:
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_Readonly)));
                                 break;
                             case FileConflictRequest::Cancel:
                             default:
                                 overwriteQ->setResponse(QVariant(static_cast<int>(Result_Cancel)));
                                 break;
                             }
                         }
                     }));

    m_forwardingConnections.append(
        QObject::connect(m_readOnlyIface, &ReadOnlyArchiveInterface::signalFinished,
                     [this](PluginFinishType eType) {
                         if (!m_observer) {
                             return;
                         }
                         FinishInfo finish;
                         finish.success = (eType == PFT_Nomral);
                         if (eType == PFT_Cancel) {
                             finish.code = ErrorCode::Cancelled;
                         } else if (eType == PFT_Error) {
                             finish.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
                         }
                         m_observer->onFinished(finish);
                         m_observer = nullptr;
                     }));
}

// ---------------------------------------------------------------------------
// Capabilities
// ---------------------------------------------------------------------------

BackendCapabilities LegacyBackendAdapter::capabilities() const
{
    return m_capabilities;
}

// ---------------------------------------------------------------------------
// Type conversion helpers
// ---------------------------------------------------------------------------

ErrorCode LegacyBackendAdapter::toErrorCode(int errorType)
{
    switch (static_cast<ErrorType>(errorType)) {
    case ET_NoError:
        return ErrorCode::NoError;
    case ET_WrongPassword:
        return ErrorCode::WrongPassword;
    case ET_NeedPassword:
        return ErrorCode::NeedPassword;
    case ET_LongNameError:
        return ErrorCode::LongNameError;
    case ET_ArchiveDamaged:
        return ErrorCode::CorruptArchive;
    case ET_FileOpenError:
        return ErrorCode::FileOpenError;
    case ET_FileReadError:
        return ErrorCode::FileReadError;
    case ET_FileWriteError:
        return ErrorCode::FileWriteError;
    case ET_DeleteError:
        return ErrorCode::FileDeleteError;
    case ET_RenameError:
        return ErrorCode::FileRenameError;
    case ET_MissingVolume:
        return ErrorCode::MissingVolume;
    case ET_InsufficientDiskSpace:
        return ErrorCode::InsufficientDiskSpace;
    case ET_PluginError:
        return ErrorCode::PluginError;
    case ET_UserCancelOpertion:
        return ErrorCode::Cancelled;
    case ET_ExistVolume:
        return ErrorCode::Unknown;
    case ET_FileSeekError:
        return ErrorCode::FileReadError;
    default:
        return ErrorCode::Unknown;
    }
}

VoidResult LegacyBackendAdapter::toVoidResult(int finishType)
{
    switch (static_cast<PluginFinishType>(finishType)) {
    case PFT_Nomral:
        return VoidResult::ok();
    case PFT_Cancel:
        return VoidResult::fail(ErrorCode::Cancelled,
                                QStringLiteral("operation cancelled"));
    case PFT_Error:
        return VoidResult::fail(ErrorCode::Unknown,
                                QStringLiteral("operation failed"));
    default:
        return VoidResult::fail(ErrorCode::Unknown,
                                QStringLiteral("unknown finish type"));
    }
}

QList<ArchiveEntry> LegacyBackendAdapter::toArchiveEntries(const QList<FileEntry> &oldEntries)
{
    QList<ArchiveEntry> result;
    for (const FileEntry &old : oldEntries) {
        ArchiveEntry entry;
        entry.fullPath = old.strFullPath;
        entry.fileName = old.strFileName;
        entry.alias = old.strAlias;
        entry.isDirectory = old.isDirectory;
        entry.size = old.qSize;
        entry.lastModified = QDateTime::fromSecsSinceEpoch(old.uLastModifiedTime);
        entry.index = old.iIndex;
        result.append(entry);
    }
    return result;
}

bool LegacyBackendAdapter::toExtractionOptions(const ExtractRequest &request,
                                                ExtractionOptions &options,
                                                QList<FileEntry> &entries,
                                                ErrorCode &error)
{
    // The old KDE interface always preserves directory structure; it does not
    // support flatten/extract-without-paths.  When the caller requests
    // preservePaths=false we must reject explicitly rather than silently
    // discarding the option.
    if (!request.preservePaths) {
        error = ErrorCode::NotSupported;
        return false;
    }

    options.strTargetPath = request.targetPath;
    options.password = request.password;
    options.bAllExtract = request.entries.isEmpty();
    options.bExistList = true;
    options.bBatchExtract = request.entries.isEmpty();
    options.bOpen = false;
    options.bTar_7z = false;

    for (const QString &path : request.entries) {
        FileEntry entry;
        entry.strFullPath = path;
        entry.strFileName = QFileInfo(path).fileName();
        entries.append(entry);
    }

    error = ErrorCode::NoError;
    return true;
}

void LegacyBackendAdapter::toCompressOptions(const AddRequest &request,
                                              CompressOptions &options,
                                              QList<FileEntry> &entries)
{
    options.strPassword = request.password;
    options.bEncryption = request.encrypt;
    options.bHeaderEncryption = request.headerEncrypt;
    options.iCompressionLevel = request.compressionLevel;
    options.strCompressionMethod = request.compressionMethod;
    options.strEncryptionMethod = request.encryptionMethod;
    options.iVolumeSize = request.volumeSize;
    options.bSplit = request.volumeSize > 0;
    options.iCPUTheadNum = request.cpuThreads;

    for (const QString &filePath : request.files) {
        FileEntry entry;
        entry.strFullPath = filePath;
        entry.strFileName = QFileInfo(filePath).fileName();
        entries.append(entry);
    }
}

// ---------------------------------------------------------------------------
// ArchiveBackend operations
// ---------------------------------------------------------------------------

VoidResult LegacyBackendAdapter::open(const OpenRequest &request)
{
    if (!m_readOnlyIface) {
        return VoidResult::fail(ErrorCode::PluginError,
                                QStringLiteral("no backend interface"));
    }

    m_readOnlyIface->setPassword(request.password);
    m_readOnlyIface->setProperty("archivePath", request.archivePath);

    // Set m_listEncrypted BEFORE calling list() so that signalQuery
    // (which may fire during list() for header-encrypted archives) sees
    // the correct value and sets PasswordRequest.headerPassword correctly.
    if (request.headerEncrypted) {
        m_listEncrypted = true;
    } else {
        m_listEncrypted = false;
    }

    int finishType = static_cast<int>(m_readOnlyIface->list());
    if (finishType != PFT_Nomral) {
        ErrorCode code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
        return VoidResult::fail(code, QStringLiteral("open failed"));
    }

    // After list() completes, supplement with the plugin-reported encryption
    // status.  This catches archives that are list-encrypted but whose caller
    // did not set headerEncrypted explicitly.
    if (!m_listEncrypted) {
        m_listEncrypted = DataManager::get_instance().archiveData().isListEncrypted;
    }

    return VoidResult::ok();
}

EntryResult LegacyBackendAdapter::list()
{
    if (!m_readOnlyIface) {
        return EntryResult::fail(ErrorCode::PluginError,
                                 QStringLiteral("no backend interface"));
    }

    ArchiveData &data = DataManager::get_instance().archiveData();
    if (data.mapFileEntry.isEmpty() && data.listRootEntry.isEmpty()) {
        int finishType = static_cast<int>(m_readOnlyIface->list());
        if (finishType != PFT_Nomral) {
            ErrorCode code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
            return EntryResult::fail(code, QStringLiteral("list failed"));
        }
        data = DataManager::get_instance().archiveData();
    }

    // mapFileEntry contains ALL entries (including root entries).
    // listRootEntry is a subset containing only top-level entries.
    // Using both would duplicate root entries, so use mapFileEntry only.
    QList<FileEntry> allEntries;
    for (auto it = data.mapFileEntry.constBegin(); it != data.mapFileEntry.constEnd(); ++it) {
        allEntries.append(it.value());
    }
    return EntryResult::ok(toArchiveEntries(allEntries));
}

VoidResult LegacyBackendAdapter::testArchive()
{
    if (!m_readOnlyIface) {
        return VoidResult::fail(ErrorCode::PluginError,
                                QStringLiteral("no backend interface"));
    }

    int finishType = static_cast<int>(m_readOnlyIface->testArchive());
    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }
    return result;
}

VoidResult LegacyBackendAdapter::extractFiles(const ExtractRequest &request,
                                               BackendObserver &observer)
{
    if (!m_readOnlyIface) {
        return VoidResult::fail(ErrorCode::PluginError,
                                QStringLiteral("no backend interface"));
    }

    ExtractionOptions options;
    QList<FileEntry> entries;
    ErrorCode convError = ErrorCode::NoError;
    if (!toExtractionOptions(request, options, entries, convError)) {
        return VoidResult::fail(convError,
                                QStringLiteral("extraction options not supported"));
    }

    // Set overwrite/skip-all flags before calling the old interface so the
    // signalQuery handler can auto-respond.
    m_overwriteAll = request.overwriteAll;
    m_skipAll = request.skipAll;

    m_observer = &observer;

    int finishType = static_cast<int>(m_readOnlyIface->extractFiles(entries, options));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }

    // Reset flags after extraction.
    m_overwriteAll = false;
    m_skipAll = false;

    if (!m_readOnlyIface->waitForFinished()) {
        FinishInfo finish;
        finish.success = result.success;
        finish.code = result.code;
        finish.message = result.message;
        observer.onFinished(finish);
        m_observer = nullptr;
    }
    return result;
}

VoidResult LegacyBackendAdapter::addFiles(const AddRequest &request,
                                           BackendObserver &observer)
{
    if (!m_readWriteIface) {
        return VoidResult::notSupported();
    }

    m_observer = &observer;

    CompressOptions options;
    QList<FileEntry> entries;
    toCompressOptions(request, options, entries);

    int finishType = static_cast<int>(m_readWriteIface->addFiles(entries, options));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }

    if (!m_readOnlyIface->waitForFinished()) {
        FinishInfo finish;
        finish.success = result.success;
        finish.code = result.code;
        finish.message = result.message;
        observer.onFinished(finish);
        m_observer = nullptr;
    }
    return result;
}

VoidResult LegacyBackendAdapter::moveFiles(const MoveRequest &request,
                                            BackendObserver &observer)
{
    if (!m_readWriteIface) {
        return VoidResult::notSupported();
    }

    m_observer = &observer;

    QList<FileEntry> entries;
    for (const QString &path : request.entries) {
        FileEntry entry;
        entry.strFullPath = path;
        entry.strFileName = QFileInfo(path).fileName();
        entries.append(entry);
    }

    CompressOptions options;
    options.strDestination = request.targetDir;

    int finishType = static_cast<int>(m_readWriteIface->moveFiles(entries, options));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }

    if (!m_readOnlyIface->waitForFinished()) {
        FinishInfo finish;
        finish.success = result.success;
        finish.code = result.code;
        finish.message = result.message;
        observer.onFinished(finish);
        m_observer = nullptr;
    }
    return result;
}

VoidResult LegacyBackendAdapter::copyFiles(const CopyRequest &request,
                                            BackendObserver &observer)
{
    if (!m_readWriteIface) {
        return VoidResult::notSupported();
    }

    m_observer = &observer;

    QList<FileEntry> entries;
    for (const QString &path : request.entries) {
        FileEntry entry;
        entry.strFullPath = path;
        entry.strFileName = QFileInfo(path).fileName();
        entries.append(entry);
    }

    CompressOptions options;
    options.strDestination = request.targetDir;

    int finishType = static_cast<int>(m_readWriteIface->copyFiles(entries, options));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }

    if (!m_readOnlyIface->waitForFinished()) {
        FinishInfo finish;
        finish.success = result.success;
        finish.code = result.code;
        finish.message = result.message;
        observer.onFinished(finish);
        m_observer = nullptr;
    }
    return result;
}

VoidResult LegacyBackendAdapter::deleteFiles(const DeleteRequest &request)
{
    if (!m_readWriteIface) {
        return VoidResult::notSupported();
    }

    QList<FileEntry> entries;
    for (const QString &path : request.entries) {
        FileEntry entry;
        entry.strFullPath = path;
        entry.strFileName = QFileInfo(path).fileName();
        entries.append(entry);
    }

    int finishType = static_cast<int>(m_readWriteIface->deleteFiles(entries));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }
    return result;
}

VoidResult LegacyBackendAdapter::renameFiles(const RenameRequest &request)
{
    if (!m_readWriteIface) {
        return VoidResult::notSupported();
    }

    QList<FileEntry> entries;
    for (int i = 0; i < request.entries.size() && i < request.newNames.size(); ++i) {
        FileEntry entry;
        entry.strFullPath = request.entries[i];
        entry.strAlias = request.newNames[i];
        entry.strFileName = QFileInfo(request.entries[i]).fileName();
        entries.append(entry);
    }

    int finishType = static_cast<int>(m_readWriteIface->renameFiles(entries));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }
    return result;
}

VoidResult LegacyBackendAdapter::addComment(const QString &comment)
{
    if (!m_readWriteIface) {
        return VoidResult::notSupported();
    }

    int finishType = static_cast<int>(m_readWriteIface->addComment(comment));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }
    return result;
}

VoidResult LegacyBackendAdapter::updateArchiveData(const UpdateRequest &request)
{
    if (!m_readWriteIface) {
        return VoidResult::notSupported();
    }

    UpdateOptions options;
    options.eType = static_cast<UpdateOptions::Type>(request.type);
    options.strParentPath = request.parentPath;
    options.qSize = request.size;
    for (const QString &path : request.entries) {
        FileEntry entry;
        entry.strFullPath = path;
        entry.strFileName = QFileInfo(path).fileName();
        options.listEntry.append(entry);
    }

    int finishType = static_cast<int>(m_readWriteIface->updateArchiveData(options));

    VoidResult result = toVoidResult(finishType);
    if (!result.success) {
        result.code = toErrorCode(static_cast<int>(m_readOnlyIface->errorType()));
    }
    return result;
}

void LegacyBackendAdapter::pause()
{
    if (m_readOnlyIface) {
        m_readOnlyIface->pauseOperation();
    }
}

void LegacyBackendAdapter::resume()
{
    if (m_readOnlyIface) {
        m_readOnlyIface->continueOperation();
    }
}

void LegacyBackendAdapter::cancel()
{
    if (m_readOnlyIface) {
        m_readOnlyIface->doKill();
    }
}

} // namespace archive
