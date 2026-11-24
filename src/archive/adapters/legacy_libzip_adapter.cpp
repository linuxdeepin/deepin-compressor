// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_libzip_adapter.h"

#include "archiveinterface.h"
#include "datamanager.h"
#include "kpluginmetadata.h"
#include "libzipplugin.h"
#include "archivequeries.h"

#include <QDateTime>

namespace dcarchive {
namespace {

ArchiveEntry toArchiveEntry(const FileEntry &entry)
{
    ArchiveEntry result;
    result.path = entry.strFullPath;
    result.name = entry.strFileName;
    result.size = entry.qSize;
    result.type = entry.isDirectory ? EntryType::Directory : EntryType::File;
    if (entry.uLastModifiedTime > 0)
        result.modified = QDateTime::fromSecsSinceEpoch(entry.uLastModifiedTime);
    result.index = entry.iIndex;
    return result;
}

FileEntry toFileEntry(const ArchiveEntry &entry)
{
    FileEntry result;
    result.strFullPath = entry.path;
    result.strFileName = entry.name;
    result.isDirectory = entry.type == EntryType::Directory;
    result.qSize = entry.size;
    result.uLastModifiedTime = entry.modified.isValid()
            ? static_cast<uint>(entry.modified.toSecsSinceEpoch()) : 0;
    result.iIndex = entry.index;
    return result;
}

ArchiveInfo toArchiveInfo(const ArchiveData &data)
{
    ArchiveInfo info;
    info.comment = data.strComment;
    info.encrypted = data.isListEncrypted;
    info.totalSize = data.qSize;
    info.compressedSize = data.qComressSize;
    for (const FileEntry &entry : data.listRootEntry)
        info.entries.append(toArchiveEntry(entry));
    return info;
}

CompressOptions toCompressOptions(const AddRequest &request)
{
    CompressOptions options;
    options.strPassword = request.password;
    options.strEncryptionMethod = request.password.isEmpty() ? QString() : QStringLiteral("AES256");
    options.strCompressionMethod = request.compressionLevel == 0
            ? QStringLiteral("Store") : QStringLiteral("Deflate");
    options.iCompressionLevel = request.compressionLevel;
    options.bEncryption = !request.password.isEmpty();
    options.bHeaderEncryption = request.encryptList;
    options.bSplit = request.splitSize > 0;
    options.iVolumeSize = static_cast<int>(request.splitSize / 1024);
    options.iCPUTheadNum = request.cpuThreads;
    options.qTotalSize = 0;
    for (const ArchiveEntry &entry : request.entries)
        options.qTotalSize += entry.size;
    return options;
}

QList<FileEntry> toFileEntries(const QVector<ArchiveEntry> &entries)
{
    QList<FileEntry> result;
    for (const ArchiveEntry &entry : entries)
        result.append(toFileEntry(entry));
    return result;
}

} // namespace

LegacyLibzipAdapter::LegacyLibzipAdapter(const QString &path,
                                         const QString &mimeType,
                                         QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_mimeType(mimeType)
{
}

LegacyLibzipAdapter::~LegacyLibzipAdapter() = default;

bool LegacyLibzipAdapter::ensureLegacy()
{
    if (m_legacy)
        return true;
    if (m_path.isEmpty())
        return false;

    KPluginMetaData metaData;
    CustomMimeType customMime;
    customMime.m_bUnKnown = true;
    customMime.m_strTypeName = m_mimeType;

    QVariantList args;
    args << QVariant(m_path)
         << QVariant::fromValue(metaData)
         << QVariant::fromValue(customMime);

    m_legacy.reset(new LibzipPlugin(nullptr, args));

    connect(m_legacy.get(), &ReadOnlyArchiveInterface::signalprogress,
            this, [this](double percentage) {
        if (m_observer) {
            ProgressInfo info;
            info.percentage = percentage;
            m_observer->onProgress(info);
        }
    });
    connect(m_legacy.get(), &ReadOnlyArchiveInterface::signalCurFileName,
            this, [this](const QString &name) {
        if (m_observer)
            m_observer->onCurrentFile(name);
    });
    connect(m_legacy.get(), &ReadOnlyArchiveInterface::signalQuery,
            this, [this](Query *query) { handleQuery(query); });
    connect(m_legacy.get(), &ReadOnlyArchiveInterface::signalCancel,
            this, [this]() { m_cancelled = true; });
    connect(m_legacy.get(), &ReadOnlyArchiveInterface::signalFileWriteErrorName,
            this, [this](const QString &name) {
        if (m_observer) {
            WarningInfo warning;
            warning.error = ArchiveError::Unknown;
            warning.message = name;
            m_observer->onWarning(warning);
        }
    });

    return true;
}

BackendCapabilities LegacyLibzipAdapter::capabilities() const
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

VoidResult LegacyLibzipAdapter::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};
    return {};
}

EntryResult LegacyLibzipAdapter::list()
{
    EntryResult result;
    if (!ensureLegacy()) {
        result.result.error = ArchiveError::NotSupported;
        return result;
    }

    m_cancelled = false;
    result.result = mapFinish(m_legacy->list());
    if (!result.result.ok())
        return result;

    result.info = toArchiveInfo(DataManager::get_instance().archiveData());
    return result;
}

VoidResult LegacyLibzipAdapter::testArchive()
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};
    m_cancelled = false;
    return {mapFinish(m_legacy->testArchive())};
}

VoidResult LegacyLibzipAdapter::extractFiles(const ExtractRequest &request,
                                             BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};

    m_observer = &observer;
    m_cancelled = false;

    ExtractionOptions options;
    options.strTargetPath = request.targetPath;
    options.bAllExtract = request.allEntries;
    options.bExistList = true;
    options.password = m_legacy->getPassword();
    for (const ArchiveEntry &entry : request.entries)
        options.qSize += entry.size;

    const QList<FileEntry> files = toFileEntries(request.entries);
    const ArchiveResult result = mapFinish(m_legacy->extractFiles(files, options));
    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyLibzipAdapter::addFiles(const AddRequest &request, BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};

    m_observer = &observer;
    m_cancelled = false;
    const ArchiveResult result = mapFinish(
            m_legacy->addFiles(toFileEntries(request.entries), toCompressOptions(request)));
    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyLibzipAdapter::moveFiles(const MoveRequest &request, BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};

    m_observer = &observer;
    CompressOptions options;
    options.strDestination = request.targetPath;
    const ArchiveResult result = mapFinish(m_legacy->moveFiles(toFileEntries(request.entries), options));
    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyLibzipAdapter::copyFiles(const CopyRequest &request, BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};

    m_observer = &observer;
    CompressOptions options;
    options.strDestination = request.targetPath;
    const ArchiveResult result = mapFinish(m_legacy->copyFiles(toFileEntries(request.entries), options));
    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyLibzipAdapter::deleteFiles(const DeleteRequest &request)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};
    m_cancelled = false;
    return {mapFinish(m_legacy->deleteFiles(toFileEntries(request.entries)))};
}

VoidResult LegacyLibzipAdapter::renameFiles(const RenameRequest &request)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};

    FileEntry entry = toFileEntry(request.entry);
    entry.strAlias = request.newName;
    QList<FileEntry> files;
    files.append(entry);
    m_cancelled = false;
    return {mapFinish(m_legacy->renameFiles(files))};
}

VoidResult LegacyLibzipAdapter::addComment(const QString &comment)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};
    m_cancelled = false;
    return {mapFinish(m_legacy->addComment(comment))};
}

VoidResult LegacyLibzipAdapter::updateArchiveData(const UpdateRequest &)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy libzip backend")};
    UpdateOptions options;
    m_cancelled = false;
    return {mapFinish(m_legacy->updateArchiveData(options))};
}

void LegacyLibzipAdapter::pause()
{
    if (m_legacy)
        m_legacy->pauseOperation();
}

void LegacyLibzipAdapter::resume()
{
    if (m_legacy)
        m_legacy->continueOperation();
}

void LegacyLibzipAdapter::cancel()
{
    m_cancelled = true;
    if (m_legacy)
        m_legacy->doKill();
}

void LegacyLibzipAdapter::handleQuery(Query *query)
{
    if (auto *overwrite = qobject_cast<OverwriteQuery *>(query)) {
        FileConflictRequest request;
        if (m_observer)
            m_observer->onFileConflict(request);

        OverwriteQuery_Result response = Result_Cancel;
        switch (request.response) {
        case FileConflictRequest::Response::Overwrite:
            response = Result_Overwrite;
            break;
        case FileConflictRequest::Response::Skip:
            response = Result_Skip;
            break;
        case FileConflictRequest::Response::OverwriteAll:
            response = Result_OverwriteAll;
            break;
        case FileConflictRequest::Response::SkipAll:
            response = Result_SkipAll;
            break;
        case FileConflictRequest::Response::Cancel:
            response = Result_Cancel;
            break;
        }
        overwrite->setResponse(response);
        return;
    }

    if (auto *password = qobject_cast<PasswordNeededQuery *>(query)) {
        PasswordRequest request;
        if (m_observer)
            m_observer->onPasswordRequired(request);
        password->setResponse(request.response == PasswordRequest::Response::Provide);
        return;
    }

    if (auto *corrupt = qobject_cast<LoadCorruptQuery *>(query)) {
        corrupt->setResponse(true);
        return;
    }

    if (auto *extractTo = qobject_cast<ExtractToQuery *>(query)) {
        extractTo->setResponse(true);
        return;
    }

    query->setResponse(Result_Cancel);
}

ArchiveResult LegacyLibzipAdapter::mapFinish(PluginFinishType type) const
{
    ArchiveResult result;
    if (type == PFT_Nomral) {
        result.error = ArchiveError::None;
    } else if (type == PFT_Cancel) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    } else {
        result.error = mapError(m_legacy ? m_legacy->errorType() : ET_PluginError);
    }
    return result;
}

ArchiveError LegacyLibzipAdapter::mapError(ErrorType type) const
{
    switch (type) {
    case ET_NoError:
        return ArchiveError::None;
    case ET_WrongPassword:
        return ArchiveError::WrongPassword;
    case ET_NeedPassword:
        return ArchiveError::PasswordRequired;
    case ET_LongNameError:
        return ArchiveError::LongName;
    case ET_ArchiveDamaged:
    case ET_MissingVolume:
        return ArchiveError::CorruptArchive;
    case ET_InsufficientDiskSpace:
        return ArchiveError::DiskFull;
    case ET_UserCancelOpertion:
        return ArchiveError::Cancelled;
    case ET_FileOpenError:
    case ET_FileReadError:
    case ET_FileWriteError:
        return ArchiveError::PermissionDenied;
    default:
        return ArchiveError::Unknown;
    }
}

void LegacyLibzipAdapter::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
