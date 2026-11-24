// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_pigz_adapter.h"
#include "legacy_add_completion.h"

#include "archiveinterface.h"
#include "datamanager.h"
#include "kpluginmetadata.h"
#include "archivequeries.h"

#include "libpigzplugin.h"
#include "libpigzplugin.h"
#include "libpigzplugin.h"

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

} // namespace

LegacyPigzAdapter::LegacyPigzAdapter(const QString &path,
                                                 const QString &mimeType,
                                                 QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_mimeType(mimeType)
{
}

LegacyPigzAdapter::~LegacyPigzAdapter() = default;

bool LegacyPigzAdapter::ensureLegacy()
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

    m_legacy.reset(new LibPigzPlugin(nullptr, args));

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

BackendCapabilities LegacyPigzAdapter::capabilities() const
{
    BackendCapabilities caps;
    caps.create = true;
    caps.progress = true;
    caps.cancel = true;
    return caps;
}

VoidResult LegacyPigzAdapter::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;

    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("unsupported pigz format")};
    return {};
}

EntryResult LegacyPigzAdapter::list()
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

VoidResult LegacyPigzAdapter::testArchive()
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("unsupported pigz format")};
    m_cancelled = false;
    return {mapFinish(m_legacy->testArchive())};
}

VoidResult LegacyPigzAdapter::extractFiles(const ExtractRequest &request,
                                                 BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("unsupported pigz format")};

    m_observer = &observer;
    m_cancelled = false;

    ExtractionOptions options;
    options.strTargetPath = request.targetPath;
    options.bAllExtract = request.allEntries;
    options.bExistList = true;
    for (const ArchiveEntry &entry : request.entries)
        options.qSize += entry.size;

    QList<FileEntry> files;
    for (const ArchiveEntry &entry : request.entries)
        files.append(toFileEntry(entry));

    const ArchiveResult result = mapFinish(m_legacy->extractFiles(files, options));
    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyPigzAdapter::addFiles(const AddRequest &request, BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy pigz backend")};

    m_observer = &observer;
    m_cancelled = false;

    CompressOptions options;
    options.iCompressionLevel = request.compressionLevel;
    options.iCPUTheadNum = request.cpuThreads;
    for (const ArchiveEntry &entry : request.entries)
        options.qTotalSize += entry.size;

    QList<FileEntry> files;
    for (const ArchiveEntry &entry : request.entries) {
        FileEntry file;
        file.strFullPath = entry.path;
        file.strFileName = entry.name;
        file.qSize = entry.size;
        files.append(file);
    }

    const ArchiveResult result = mapFinish(awaitLegacyAdd(*m_legacy, files, options));
    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyPigzAdapter::moveFiles(const MoveRequest &, BackendObserver &)
{
    return notSupported();
}

VoidResult LegacyPigzAdapter::copyFiles(const CopyRequest &, BackendObserver &)
{
    return notSupported();
}

VoidResult LegacyPigzAdapter::deleteFiles(const DeleteRequest &)
{
    return notSupported();
}

VoidResult LegacyPigzAdapter::renameFiles(const RenameRequest &)
{
    return notSupported();
}

VoidResult LegacyPigzAdapter::addComment(const QString &)
{
    return notSupported();
}

VoidResult LegacyPigzAdapter::updateArchiveData(const UpdateRequest &)
{
    return notSupported();
}

void LegacyPigzAdapter::pause()
{
    if (m_legacy)
        m_legacy->pauseOperation();
}

void LegacyPigzAdapter::resume()
{
    if (m_legacy)
        m_legacy->continueOperation();
}

void LegacyPigzAdapter::cancel()
{
    m_cancelled = true;
    if (m_legacy)
        m_legacy->doKill();
}

void LegacyPigzAdapter::handleQuery(Query *query)
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
        // 旧接口没有公开的密码注入点；当前只表达取消/继续。
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

ArchiveResult LegacyPigzAdapter::mapFinish(PluginFinishType type) const
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

ArchiveError LegacyPigzAdapter::mapError(ErrorType type) const
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

void LegacyPigzAdapter::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

VoidResult LegacyPigzAdapter::notSupported() const
{
    return {ArchiveError::NotSupported, QStringLiteral("operation is not supported by single-file backend")};
}

} // namespace dcarchive
