// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_cli7z_adapter.h"

#include "archiveinterface.h"
#include "cli7zplugin.h"
#include "datamanager.h"
#include "kpluginmetadata.h"
#include "archivequeries.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QThread>

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

LegacyCli7zAdapter::LegacyCli7zAdapter(const QString &path,
                                       const QString &mimeType,
                                       const QString &metaDataFile,
                                       QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_mimeType(mimeType)
    , m_metaDataFile(metaDataFile)
{
}

LegacyCli7zAdapter::~LegacyCli7zAdapter() = default;

bool LegacyCli7zAdapter::ensureLegacy()
{
    if (m_legacy)
        return true;
    if (m_path.isEmpty())
        return false;

    KPluginMetaData metaData = m_metaDataFile.isEmpty()
            ? KPluginMetaData() : KPluginMetaData(m_metaDataFile);
    CustomMimeType customMime;
    customMime.m_bUnKnown = true;
    customMime.m_strTypeName = m_mimeType;

    QVariantList args;
    args << QVariant(m_path)
         << QVariant::fromValue(metaData)
         << QVariant::fromValue(customMime);

    m_legacy.reset(new Cli7zPlugin(nullptr, args));

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
    connect(m_legacy.get(), &ReadOnlyArchiveInterface::signalFinished,
            this, [this](PluginFinishType type) {
        m_finishType = type;
        m_finished = true;
    });

    return true;
}

bool LegacyCli7zAdapter::waitForLegacy(int timeoutMs)
{
    if (!m_legacy || !m_legacy->waitForFinished())
        return true;

    QElapsedTimer timer;
    timer.start();
    while (!m_finished && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(10);
    }
    return m_finished;
}

BackendCapabilities LegacyCli7zAdapter::capabilities() const
{
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.create = true;
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

VoidResult LegacyCli7zAdapter::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};
    return {};
}

EntryResult LegacyCli7zAdapter::list()
{
    EntryResult result;
    if (!ensureLegacy()) {
        result.result.error = ArchiveError::NotSupported;
        return result;
    }
    m_finished = false;
    const PluginFinishType type = m_legacy->list();
    if (type != PFT_Nomral) {
        result.result = mapFinish(type);
        return result;
    }
    if (!waitForLegacy()) {
        result.result.error = ArchiveError::Unknown;
        result.result.message = QStringLiteral("7z list timeout");
        return result;
    }
    result.result = mapFinish(m_finishType);
    if (result.result.ok())
        result.info = toArchiveInfo(DataManager::get_instance().archiveData());
    return result;
}

VoidResult LegacyCli7zAdapter::testArchive()
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};
    m_finished = false;
    const PluginFinishType type = m_legacy->testArchive();
    if (type != PFT_Nomral)
        return {mapFinish(type)};
    if (!waitForLegacy())
        return {ArchiveError::Unknown, QStringLiteral("7z test timeout")};
    return {mapFinish(m_finishType)};
}

VoidResult LegacyCli7zAdapter::extractFiles(const ExtractRequest &request,
                                            BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};

    m_observer = &observer;
    m_cancelled = false;
    m_finished = false;

    ExtractionOptions options;
    options.strTargetPath = request.targetPath;
    options.bAllExtract = request.allEntries;
    options.bExistList = true;
    for (const ArchiveEntry &entry : request.entries)
        options.qSize += entry.size;

    QList<FileEntry> files;
    for (const ArchiveEntry &entry : request.entries)
        files.append(toFileEntry(entry));

    const PluginFinishType type = m_legacy->extractFiles(files, options);
    ArchiveResult result;
    if (type == PFT_Nomral)
        result = waitForLegacy() ? mapFinish(m_finishType)
                                 : ArchiveResult{ArchiveError::Unknown, QStringLiteral("7z extract timeout"), false};
    else
        result = mapFinish(type);

    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyCli7zAdapter::addFiles(const AddRequest &request, BackendObserver &observer)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};

    m_observer = &observer;
    m_cancelled = false;
    m_finished = false;

    CompressOptions options;
    options.strPassword = request.password;
    options.strEncryptionMethod = request.password.isEmpty() ? QString() : QStringLiteral("AES256");
    options.strCompressionMethod = QStringLiteral("LZMA2");
    options.iCompressionLevel = request.compressionLevel;
    options.bEncryption = !request.password.isEmpty();
    options.bHeaderEncryption = request.encryptList;
    options.bSplit = request.splitSize > 0;
    options.iVolumeSize = static_cast<int>(request.splitSize / 1024);
    options.iCPUTheadNum = request.cpuThreads;
    for (const ArchiveEntry &entry : request.entries)
        options.qTotalSize += entry.size;

    QList<FileEntry> files;
    for (const ArchiveEntry &entry : request.entries)
        files.append(toFileEntry(entry));

    const PluginFinishType type = m_legacy->addFiles(files, options);
    ArchiveResult result;
    if (type == PFT_Nomral)
        result = waitForLegacy() ? mapFinish(m_finishType)
                                 : ArchiveResult{ArchiveError::Unknown, QStringLiteral("7z add timeout"), false};
    else
        result = mapFinish(type);

    notifyFinished(observer, result);
    m_observer = nullptr;
    return {result};
}

VoidResult LegacyCli7zAdapter::moveFiles(const MoveRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("move is not supported")};
}

VoidResult LegacyCli7zAdapter::copyFiles(const CopyRequest &, BackendObserver &)
{
    return {ArchiveError::NotSupported, QStringLiteral("copy is not supported")};
}

VoidResult LegacyCli7zAdapter::deleteFiles(const DeleteRequest &request)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};
    m_finished = false;
    QList<FileEntry> files;
    for (const ArchiveEntry &entry : request.entries)
        files.append(toFileEntry(entry));
    const PluginFinishType type = m_legacy->deleteFiles(files);
    if (type != PFT_Nomral)
        return {mapFinish(type)};
    return {waitForLegacy() ? mapFinish(m_finishType)
                            : ArchiveResult{ArchiveError::Unknown, QStringLiteral("7z delete timeout"), false}};
}

VoidResult LegacyCli7zAdapter::renameFiles(const RenameRequest &request)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};
    m_finished = false;
    FileEntry entry = toFileEntry(request.entry);
    entry.strAlias = request.newName;
    QList<FileEntry> files;
    files.append(entry);
    const PluginFinishType type = m_legacy->renameFiles(files);
    if (type != PFT_Nomral)
        return {mapFinish(type)};
    return {waitForLegacy() ? mapFinish(m_finishType)
                            : ArchiveResult{ArchiveError::Unknown, QStringLiteral("7z rename timeout"), false}};
}

VoidResult LegacyCli7zAdapter::addComment(const QString &comment)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};
    m_finished = false;
    const PluginFinishType type = m_legacy->addComment(comment);
    if (type != PFT_Nomral)
        return {mapFinish(type)};
    return {waitForLegacy() ? mapFinish(m_finishType)
                            : ArchiveResult{ArchiveError::Unknown, QStringLiteral("7z comment timeout"), false}};
}

VoidResult LegacyCli7zAdapter::updateArchiveData(const UpdateRequest &)
{
    if (!ensureLegacy())
        return {ArchiveError::NotSupported, QStringLiteral("failed to create legacy 7z backend")};
    UpdateOptions options;
    return {mapFinish(m_legacy->updateArchiveData(options))};
}

void LegacyCli7zAdapter::pause()
{
    if (m_legacy)
        m_legacy->pauseOperation();
}

void LegacyCli7zAdapter::resume()
{
    if (m_legacy)
        m_legacy->continueOperation();
}

void LegacyCli7zAdapter::cancel()
{
    m_cancelled = true;
    if (m_legacy)
        m_legacy->doKill();
}

void LegacyCli7zAdapter::handleQuery(Query *query)
{
    if (auto *overwrite = qobject_cast<OverwriteQuery *>(query)) {
        FileConflictRequest request;
        if (m_observer)
            m_observer->onFileConflict(request);
        OverwriteQuery_Result response = Result_Cancel;
        switch (request.response) {
        case FileConflictRequest::Response::Overwrite: response = Result_Overwrite; break;
        case FileConflictRequest::Response::Skip: response = Result_Skip; break;
        case FileConflictRequest::Response::OverwriteAll: response = Result_OverwriteAll; break;
        case FileConflictRequest::Response::SkipAll: response = Result_SkipAll; break;
        case FileConflictRequest::Response::Cancel: response = Result_Cancel; break;
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

ArchiveResult LegacyCli7zAdapter::mapFinish(PluginFinishType type) const
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

ArchiveError LegacyCli7zAdapter::mapError(ErrorType type) const
{
    switch (type) {
    case ET_NoError: return ArchiveError::None;
    case ET_WrongPassword: return ArchiveError::WrongPassword;
    case ET_NeedPassword: return ArchiveError::PasswordRequired;
    case ET_LongNameError: return ArchiveError::LongName;
    case ET_ArchiveDamaged:
    case ET_MissingVolume: return ArchiveError::CorruptArchive;
    case ET_InsufficientDiskSpace: return ArchiveError::DiskFull;
    case ET_UserCancelOpertion: return ArchiveError::Cancelled;
    case ET_FileOpenError:
    case ET_FileReadError:
    case ET_FileWriteError: return ArchiveError::PermissionDenied;
    default: return ArchiveError::Unknown;
    }
}

void LegacyCli7zAdapter::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
