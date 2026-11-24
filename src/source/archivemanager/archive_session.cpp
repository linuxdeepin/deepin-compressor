// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archive_session.h"

#include "archive_registry.h"
#include "archiveui_types.h"
#include "archiveviewstate.h"
#include "archivequeries.h"
#include "path_guard.h"

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QHash>
#include <QMetaMethod>
#include <QSet>
#include <QTemporaryDir>

#include <cerrno>
#include <cstdio>
#include <cstring>

namespace dcarchive {
namespace {

class SessionObserver : public BackendObserver
{
public:
    explicit SessionObserver(ArchiveSession *session, QString archivePath, qint64 extractBytes = 0)
        : m_session(session)
        , m_archivePath(std::move(archivePath))
        , m_extractBytes(extractBytes)
    {
    }

    void onProgress(const ProgressInfo &progress) override
    {
        double percentage = progress.percentage;
        if (percentage == 0.0 && progress.processedBytes > 0) {
            const qint64 total = progress.totalBytes > 0 ? progress.totalBytes : m_extractBytes;
            if (total > 0) {
                percentage = qMin(99.0, 100.0 * progress.processedBytes / total);
                if (percentage <= m_lastByteProgress
                        || (m_byteProgressTimer.isValid() && m_byteProgressTimer.elapsed() < 100))
                    return;
                m_lastByteProgress = percentage;
                m_byteProgressTimer.start();
            }
        }
        emit m_session->signalprogress(percentage);
    }

    void onCurrentFile(const QString &path) override
    {
        emit m_session->signalCurFileName(path);
    }

    void onFileConflict(FileConflictRequest &request) override
    {
        OverwriteQuery query(request.path);
        emit m_session->signalQuery(&query);
        query.waitForResponse();

        if (query.responseCancelled())
            request.response = FileConflictRequest::Response::Cancel;
        else if (query.responseSkip())
            request.response = FileConflictRequest::Response::Skip;
        else if (query.responseSkipAll())
            request.response = FileConflictRequest::Response::SkipAll;
        else if (query.responseOverwriteAll())
            request.response = FileConflictRequest::Response::OverwriteAll;
        else if (query.responseOverwrite())
            request.response = FileConflictRequest::Response::Overwrite;
        else
            request.response = FileConflictRequest::Response::Cancel;
    }

    void onPasswordRequired(PasswordRequest &request) override
    {
        PasswordNeededQuery query(m_archivePath);
        emit m_session->signalQuery(&query);
        query.waitForResponse();
        if (query.responseCancelled()) {
            request.response = PasswordRequest::Response::Cancel;
        } else {
            request.password = query.password();
            request.response = PasswordRequest::Response::Provide;
            m_session->setPassword(request.password);
        }
    }

    void onWarning(const WarningInfo &warning) override
    {
        emit m_session->signalFileWriteErrorName(warning.message);
    }

private:
    ArchiveSession *m_session;
    QString m_archivePath;
    qint64 m_extractBytes;
    double m_lastByteProgress = 0.0;
    QElapsedTimer m_byteProgressTimer;
};

} // namespace

ArchiveSession::ArchiveSession(const QString &path, const QString &mimeType,
                               BackendPreference preference, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_mimeType(mimeType)
    , m_preference(preference)
{
}

ArchiveSession::~ArchiveSession() = default;

ArchiveSession *ArchiveSession::create(const QString &path,
                                                     const QString &mimeType,
                                                     bool forWrite,
                                                     BackendPreference preference,
                                                     QObject *parent)
{
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    std::unique_ptr<ArchiveBackend> backend = ArchiveRegistry::create(
            absolutePath, mimeType,
            forWrite ? ArchiveOperation::Create : ArchiveOperation::Read,
            preference);
    if (!backend)
        return nullptr;

    auto *session = new ArchiveSession(absolutePath, mimeType, preference, parent);
    session->m_backend = std::move(backend);
    session->m_operation = forWrite ? ArchiveOperation::Create : ArchiveOperation::Read;
    session->m_backendPath = absolutePath;
    return session;
}

ArchiveBackend *ArchiveSession::backendFor(ArchiveOperation operation, const QString &path)
{
    QMutexLocker lock(&m_backendMutex);
    if (m_partialRead && operation != ArchiveOperation::Read)
        return nullptr;
    const QString archivePath = path.isEmpty() ? m_path : path;
    std::unique_ptr<ArchiveBackend> replacement;
    ArchiveBackend *backend = m_backend.get();
    if (!backend || m_operation != operation || m_backendPath != archivePath) {
        replacement = ArchiveRegistry::create(archivePath, m_mimeType, operation, m_preference);
        if (!replacement)
            return nullptr;
        backend = replacement.get();
    } else if (!m_reopen && m_backendPassword == m_password) {
        return backend;
    }

    OpenRequest request;
    request.path = archivePath;
    request.mimeType = m_mimeType;
    request.mode = operation == ArchiveOperation::Read
            ? OpenMode::ReadOnly : OpenMode::ReadWrite;
    request.password = m_password;
    if (!backend->open(request).result.ok())
        return nullptr;

    if (replacement)
        m_backend = std::move(replacement);
    m_operation = operation;
    m_backendPassword = request.password;
    m_backendPath = archivePath;
    m_reopen = false;
    if (m_cancelRequested)
        backend->cancel();
    else if (m_paused)
        backend->pause();
    return backend;
}

PluginFinishType ArchiveSession::applyResult(const ArchiveResult &result)
{
    m_error = toErrorType(result.ok() ? result.warning : result.error);
    if (result.cancelled || result.error == ArchiveError::Cancelled) {
        prepareOperation();
        return PFT_Cancel;
    }
    if (result.error == ArchiveError::None)
        return PFT_Nomral;
    return PFT_Error;
}

ErrorType ArchiveSession::toErrorType(ArchiveError error)
{
    switch (error) {
    case ArchiveError::None: return ET_NoError;
    case ArchiveError::NotFound: return ET_FileOpenError;
    case ArchiveError::PermissionDenied: return ET_FileWriteError;
    case ArchiveError::CorruptArchive: return ET_ArchiveDamaged;
    case ArchiveError::WrongPassword: return ET_WrongPassword;
    case ArchiveError::PasswordRequired: return ET_NeedPassword;
    case ArchiveError::LongName: return ET_LongNameError;
    case ArchiveError::DiskFull: return ET_InsufficientDiskSpace;
    case ArchiveError::MissingVolume: return ET_MissingVolume;
    case ArchiveError::Cancelled: return ET_UserCancelOpertion;
    default: return ET_PluginError;
    }
}

ArchiveEntry ArchiveSession::toArchiveEntry(const FileEntry &entry)
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

FileEntry ArchiveSession::toFileEntry(const ArchiveEntry &entry)
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

void ArchiveSession::populateViewState(const ArchiveInfo &info)
{
    ArchiveData data;
    data.qSize = info.totalSize;
    data.qComressSize = info.compressedSize;
    data.strComment = info.comment;
    data.isListEncrypted = info.encrypted;
    data.strPassword = getPassword();

    QHash<QString, qsizetype> rootPositions;
    QSet<QString> knownParents;
    const auto setRootEntry = [&](const FileEntry &file) {
        auto existing = rootPositions.constFind(file.strFullPath);
        if (existing == rootPositions.cend()) {
            rootPositions.insert(file.strFullPath, data.listRootEntry.size());
            data.listRootEntry.append(file);
        } else {
            data.listRootEntry[*existing] = file;
        }
    };
    for (const ArchiveEntry &entry : info.entries) {
        const FileEntry file = toFileEntry(entry);
        const qsizetype firstSeparator = file.strFullPath.indexOf(QLatin1Char('/'));
        if (firstSeparator < 0 || firstSeparator == file.strFullPath.size() - 1)
            setRootEntry(file);

        const qsizetype lastSeparator = file.strFullPath.lastIndexOf(QLatin1Char('/'));
        const QString parent = file.strFullPath.left(lastSeparator + 1);
        if (!parent.isEmpty() && !knownParents.contains(parent)) {
            knownParents.insert(parent);
            qsizetype start = 0;
            for (qsizetype end = firstSeparator; end >= 0;
                 end = parent.indexOf(QLatin1Char('/'), start)) {
                FileEntry directory;
                directory.strFullPath = parent.left(end + 1);
                directory.strFileName = parent.mid(start, end - start);
                directory.isDirectory = true;
                if (!data.mapFileEntry.contains(directory.strFullPath)) {
                    data.mapFileEntry.insert(directory.strFullPath, directory);
                    if (start == 0)
                        setRootEntry(directory);
                }
                start = end + 1;
            }
        }
        data.mapFileEntry[file.strFullPath] = file;
    }
    ArchiveViewState::instance().archiveData() = std::move(data);
}

PluginFinishType ArchiveSession::list()
{
    ArchiveBackend *backend = backendFor(ArchiveOperation::Read);
    if (!backend) {
        m_error = ET_PluginError;
        return PFT_Error;
    }
    EntryResult result = backend->list();
    bool headerEncrypted = false;
    SessionObserver observer(this, m_path);
    while (result.result.error == ArchiveError::PasswordRequired
           || result.result.error == ArchiveError::WrongPassword) {
        if (!isSignalConnected(QMetaMethod::fromSignal(&ArchiveSession::signalQuery)))
            break;
        PasswordRequest password;
        observer.onPasswordRequired(password);
        if (password.response == PasswordRequest::Response::Cancel) {
            result.result.error = ArchiveError::Cancelled;
            result.result.cancelled = true;
            break;
        }
        if (password.password.isEmpty())
            break;
        backend = backendFor(ArchiveOperation::Read);
        if (!backend)
            return applyResult(VoidResult{ArchiveError::Unknown}.result);
        headerEncrypted = true;
        result = backend->list();
    }
    if (result.result.error == ArchiveError::MissingVolume && !result.info.entries.isEmpty()) {
        if (!m_partialRead && isSignalConnected(QMetaMethod::fromSignal(&ArchiveSession::signalQuery))) {
            LoadCorruptQuery query(m_path);
            emit signalQuery(&query);
            query.waitForResponse();
            if (!query.responseYes())
                return applyResult({ArchiveError::Cancelled, {}, true});
            QMutexLocker lock(&m_backendMutex);
            if (m_cancelRequested) {
                lock.unlock();
                return applyResult({ArchiveError::Cancelled, {}, true});
            }
            m_partialRead = true;
        }
        if (m_partialRead)
            result.result = {};
    }
    if (result.result.ok()) {
        result.info.encrypted = result.info.encrypted || headerEncrypted;
        populateViewState(result.info);
    }
    return applyResult(result.result);
}

PluginFinishType ArchiveSession::extractFiles(const QList<FileEntry> &files,
                                             const ExtractionOptions &options,
                                             QString *outputPath)
{
    if (!QDir().mkpath(options.strTargetPath)) {
        emit signalFileWriteErrorName(QFileInfo(options.strTargetPath).fileName());
        m_error = ET_FileWriteError;
        return PFT_Error;
    }
    if (!options.password.isEmpty())
        setPassword(options.password);
    if (!options.bExistList) {
        const PluginFinishType listed = list();
        if (listed != PFT_Nomral)
            return listed;
    }

    ArchiveBackend *backend = backendFor(ArchiveOperation::Read);
    if (!backend) {
        m_error = ET_PluginError;
        return PFT_Error;
    }

    ExtractRequest request;
    request.targetPath = options.strTargetPath;
    request.destinationPrefix = options.strDestination;
    request.allEntries = options.bAllExtract || files.isEmpty();
    request.reportOutputPath = outputPath != nullptr;
    request.allowPartial = m_partialRead;
    request.keepIncompleteOnCancel = options.keepIncompleteOnCancel;
    for (const FileEntry &file : files)
        request.entries.append(toArchiveEntry(file));

    const qint64 extractBytes = !options.bExistList && request.allEntries
            ? ArchiveViewState::instance().archiveData().qSize : options.qSize;
    SessionObserver observer(this, m_path, extractBytes);
    const ArchiveResult result = backend->extractFiles(request, observer).result;
    if (result.error == ArchiveError::PermissionDenied) {
        const QString path = result.message.isEmpty() ? options.strTargetPath : result.message;
        emit signalFileWriteErrorName(QFileInfo(QDir::cleanPath(path)).fileName());
    }
    if (outputPath && result.ok())
        *outputPath = result.outputPath;
    return applyResult(result);
}

PluginFinishType ArchiveSession::addFiles(const QList<FileEntry> &files,
                                                 const CompressOptions &options)
{
    if (options.bSplit && options.iVolumeSize <= 0) {
        m_error = ET_PluginError;
        return PFT_Error;
    }
    ArchiveOperation operation = ArchiveOperation::Create;
    if (QFileInfo::exists(m_path) && !options.bCreateNew) {
        if (options.bSplit) {
            m_error = ET_PluginError;
            return PFT_Error;
        }
        operation = ArchiveOperation::Append;
    } else if (options.bSplit) {
        operation = ArchiveOperation::CreateSplit;
    }
    const QFileInfo target(m_path);
    QDir targetDir(target.absolutePath());
    std::unique_ptr<QTemporaryDir> staging;
    QString writePath = m_path;
    if (options.bCreateNew || options.bSplit) {
        if (options.bSplit && target.exists())
            return applyResult({ArchiveError::NotSupported});
        staging.reset(new QTemporaryDir(targetDir.filePath(QStringLiteral(".dc-create-XXXXXX"))));
        if (!staging->isValid())
            return applyResult({ArchiveError::PermissionDenied, staging->errorString()});
        writePath = staging->filePath(target.fileName());
    }

    ArchiveBackend *backend = backendFor(operation, writePath);
    if (!backend) {
        m_error = ET_PluginError;
        return PFT_Error;
    }

    AddRequest request;
    request.archivePath = writePath;
    if (staging)
        request.excludedSourcePaths << staging->path() << m_path;
    request.destinationPath = options.strDestination;
    request.password = options.strPassword;
    request.encryptList = options.bHeaderEncryption;
    request.sourceBytes = operation == ArchiveOperation::Append ? 0 : options.qTotalSize;
    request.existingBytes = operation == ArchiveOperation::Append ? options.qTotalSize : 0;
    request.compressionLevel = options.iCompressionLevel;
    request.cpuThreads = options.iCPUTheadNum;
    request.splitSize = options.bSplit ? qint64(options.iVolumeSize) * 1024 : 0;
    request.tar7z = options.bTar_7z;
    for (const FileEntry &file : files) {
        ArchiveEntry entry = toArchiveEntry(file);
        if (QFileInfo(entry.path).isDir())
            entry.type = EntryType::Directory;
        request.entries.append(entry);
    }

    SessionObserver observer(this, m_path);
    ArchiveResult result = backend->addFiles(request, observer).result;
    if (staging && result.ok()) {
        QMutexLocker lock(&m_backendMutex);
        if (m_cancelRequested) {
            result = {ArchiveError::Cancelled, {}, true};
        } else if (!options.bSplit) {
            if (::rename(QFile::encodeName(writePath).constData(),
                         QFile::encodeName(m_path).constData()) != 0)
                result = {ArchiveError::PermissionDenied, QString::fromLocal8Bit(std::strerror(errno))};
        } else {
            const QDir staged(staging->path());
            const QStringList volumes = staged.entryList(QDir::Files);
            QStringList published;
            if (volumes.isEmpty())
                result = {ArchiveError::NotFound};
            for (const QString &volume : volumes) {
                const QString destination = targetDir.filePath(volume);
                if (!QFile::rename(staged.filePath(volume), destination)) {
                    result = {ArchiveError::PermissionDenied, destination};
                    for (const QString &path : published)
                        QFile::remove(path);
                    break;
                }
                published.append(destination);
            }
        }
    }
    if (result.ok())
        observer.onProgress({100.0, 0, 0});
    return applyResult(result);
}

PluginFinishType ArchiveSession::deleteFiles(const QList<FileEntry> &files)
{
    ArchiveBackend *backend = backendFor(ArchiveOperation::Modify);
    if (!backend) {
        m_error = ET_PluginError;
        return PFT_Error;
    }
    DeleteRequest request;
    for (const FileEntry &file : files)
        request.entries.append(toArchiveEntry(file));
    return applyResult(backend->deleteFiles(request).result);
}

PluginFinishType ArchiveSession::renameFiles(const QList<FileEntry> &files)
{
    ArchiveBackend *backend = backendFor(ArchiveOperation::Modify);
    if (!backend || files.isEmpty()) {
        m_error = ET_PluginError;
        return PFT_Error;
    }
    RenameRequest request;
    request.entry = toArchiveEntry(files.first());
    const QString alias = files.first().strAlias;
    if (alias.isEmpty() || alias == QLatin1String(".") || alias == QLatin1String("..")
            || alias.contains(QLatin1Char('/')))
        return applyResult({ArchiveError::NotSupported});
    QString path = normalizedArchivePath(request.entry.path);
    if (path.endsWith(QLatin1Char('/')))
        path.chop(1);
    request.newName = path.left(path.lastIndexOf(QLatin1Char('/')) + 1) + alias;
    if (request.entry.type == EntryType::Directory)
        request.newName += QLatin1Char('/');
    return applyResult(backend->renameFiles(request).result);
}

PluginFinishType ArchiveSession::addComment(const QString &comment)
{
    ArchiveBackend *backend = backendFor(ArchiveOperation::Modify);
    if (!backend) {
        m_error = ET_PluginError;
        return PFT_Error;
    }
    SessionObserver observer(this, m_path);
    return applyResult(backend->addComment(comment, observer).result);
}

PluginFinishType ArchiveSession::updateArchiveData(const UpdateOptions &)
{
    ArchiveBackend *backend = backendFor(ArchiveOperation::Read);
    if (!backend) {
        m_error = ET_PluginError;
        return PFT_Error;
    }
    const PluginFinishType result = applyResult(backend->updateArchiveData(UpdateRequest()).result);
    return result == PFT_Nomral ? list() : result;
}

void ArchiveSession::pauseOperation()
{
    QMutexLocker lock(&m_backendMutex);
    m_paused = true;
    if (m_backend)
        m_backend->pause();
}

void ArchiveSession::continueOperation()
{
    QMutexLocker lock(&m_backendMutex);
    m_paused = false;
    if (m_backend)
        m_backend->resume();
}

void ArchiveSession::cancel()
{
    QMutexLocker lock(&m_backendMutex);
    m_cancelRequested = true;
    if (m_backend)
        m_backend->cancel();
}

void ArchiveSession::prepareOperation()
{
    QMutexLocker lock(&m_backendMutex);
    m_reopen = true;
    m_cancelRequested = false;
    m_paused = false;
}

bool ArchiveSession::status()
{
    QMutexLocker lock(&m_backendMutex);
    return m_paused;
}

void ArchiveSession::setPassword(const QString &password)
{
    QMutexLocker lock(&m_backendMutex);
    m_password = password;
}

QString ArchiveSession::getPassword()
{
    QMutexLocker lock(&m_backendMutex);
    return m_password;
}

} // namespace dcarchive
