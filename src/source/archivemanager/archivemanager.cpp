// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archivemanager.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "singlejob.h"
#include "batchjob.h"
#include "eventlogutils.h"

#include <QMimeDatabase>
#include <QFileInfo>
#include <QDebug>
#include "dfmstandardpaths.h"

//静态成员变量初始化。
QMutex ArchiveManager::m_mutex;//一个线程可以多次锁同一个互斥量
QAtomicPointer<ArchiveManager> ArchiveManager::m_instance = nullptr;//原子指针，默认初始化是0

ArchiveManager::ArchiveManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "ArchiveManager instance created";
}

ArchiveManager::~ArchiveManager()
{
    qDebug() << "ArchiveManager instance destroyed";
    SAFE_DELETE_ELE(m_pArchiveJob);
    SAFE_DELETE_ELE(m_pInterface);
    SAFE_DELETE_ELE(m_pTempInterface);
}

ArchiveManager *ArchiveManager::get_instance()
{
    // qDebug() << "get_instance called";
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<ArchiveManager>::isTestAndSetNative()) //运行时检测
        qInfo() << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (m_instance.testAndSetOrdered(nullptr, nullptr)) { //第一次检测
        qDebug() << "Creating new ArchiveManager instance";
        QMutexLocker locker(&m_mutex);//加互斥锁。

        m_instance.testAndSetOrdered(nullptr, new ArchiveManager);//第二次检测。
    }

    return m_instance;
}

void ArchiveManager::destory_instance()
{
    qDebug() << "destroy_instance called";
    SAFE_DELETE_ELE(m_instance)
}

bool ArchiveManager::createArchive(const QList<FileEntry> &files, const QString &strDestination, const CompressOptions &stOptions, UiTools::AssignPluginType eType)
{
    qDebug() << "Starting createArchive operation for destination:" << strDestination;
    // 重新创建压缩包首先释放之前的interface
    if (m_pInterface != nullptr) {
        qDebug() << "Clearing previous archive interface";
        delete m_pInterface;
        m_pInterface = nullptr;
    }
    if(DFMStandardPaths::pathControl(strDestination)) {
        qWarning() << "Path control check failed for destination:" << strDestination;
        return false;
    }
    qDebug() << "Creating interface for destination";
    m_pTempInterface = UiTools::createInterface(strDestination, true, eType);

    if (m_pTempInterface) {
        qDebug() << "Creating CreateJob";
        CreateJob *pCreateJob = new CreateJob(files, m_pTempInterface, stOptions, this);

        // 连接槽函数
        connect(pCreateJob, &CreateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pCreateJob, &CreateJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pCreateJob, &CreateJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);

        m_pArchiveJob = pCreateJob;
        pCreateJob->start();

        qInfo() << "CreateArchive operation started successfully";
        return true;
    }

    qWarning() << "Failed to create archive interface";
    return false;
}

bool ArchiveManager::loadArchive(const QString &strArchiveFullPath, UiTools::AssignPluginType eType)
{
    qDebug() << "Starting loadArchive operation for file:" << strArchiveFullPath;
    QJsonObject obj{
        {"tid", EventLogUtils::LoadCompressFile},
        {"operate", "LoadCompressFile"},
        {"describe", QString("Load Compress File : ") + strArchiveFullPath}
    };
    EventLogUtils::get().writeLogs(obj);
    // 重新加载首先释放之前的interface
    if (m_pInterface != nullptr) {
        qDebug() << "Clearing previous archive interface";
        delete m_pInterface;
        m_pInterface = nullptr;
    }

    qDebug() << "Creating interface for loading";
    m_pInterface = UiTools::createInterface(strArchiveFullPath, false, eType);

    if (m_pInterface) {
        qDebug() << "Creating LoadJob";
        LoadJob *pLoadJob = new LoadJob(m_pInterface);

        // 连接槽函数
        connect(pLoadJob, &LoadJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pLoadJob, &LoadJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pLoadJob;
        pLoadJob->start();

        qInfo() << "LoadArchive operation started successfully";
        return true;
    }

    qWarning() << "Failed to load archive interface";
    return false;
}

bool ArchiveManager::addFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listAddEntry, const CompressOptions &stOptions)
{
    qDebug() << "Starting addFiles operation for archive:" << strArchiveFullPath;
    QJsonObject obj{
        {"tid", EventLogUtils::AddCompressFile},
        {"operate", "AddCompressFile"},
        {"describe", QString("Add File to package: ") + strArchiveFullPath}
    };
    EventLogUtils::get().writeLogs(obj);
    // workaround:
    // pzip 仅支持新建压缩，目前先将zip 追加时显式指定使用 libzip 插件
    UiTools::AssignPluginType eType = UiTools::APT_Auto;
    CustomMimeType mimeType = determineMimeType(strArchiveFullPath);
    if (mimeType.name() == QLatin1String("application/zip")) {
        eType = UiTools::APT_Libzip;
    }

    qDebug() << "Creating interface for adding files, plugin type:" << eType;
    m_pTempInterface = UiTools::createInterface(strArchiveFullPath, true, eType);

    if (m_pTempInterface) {
        qDebug() << "Creating AddJob";
        AddJob *pAddJob = new AddJob(listAddEntry, m_pTempInterface, stOptions);

        // 连接槽函数
        connect(pAddJob, &AddJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pAddJob, &AddJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pAddJob, &AddJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pAddJob, &AddJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pAddJob;
        pAddJob->start();

        qDebug() << "AddJob started successfully";
        return true;
    }

    qWarning() << "Failed to create interface for adding files";
    return false;
}

bool ArchiveManager::extractFiles(const QString &strArchiveFullPath, const QList<FileEntry> &files, const ExtractionOptions &stOptions, UiTools::AssignPluginType eType)
{
    qDebug() << "Starting extractFiles operation for archive:" << strArchiveFullPath;
    QJsonObject obj{
        {"tid", EventLogUtils::ExtractCompressFile},
        {"operate", "ExtractCompressFile"},
        {"describe", QString("Decompress the package : ") + strArchiveFullPath}
    };
    EventLogUtils::get().writeLogs(obj);
    if (nullptr == m_pInterface) {
        qDebug() << "Creating interface for extraction";
        m_pInterface = UiTools::createInterface(strArchiveFullPath, false, eType);
    }

    if (m_pInterface) {
        if (!stOptions.bTar_7z) {
            qDebug() << "Creating ExtractJob for regular extraction";
            ExtractJob *pExtractJob = new ExtractJob(files, m_pInterface, stOptions);

            // 连接槽函数
            connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
            connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
            connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
            connect(pExtractJob, &ExtractJob::signalFileWriteErrorName, this, &ArchiveManager::signalFileWriteErrorName);
            connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);

            m_pArchiveJob = pExtractJob;
            pExtractJob->start();

            return pExtractJob->errorcode;
        } else {
            qDebug() << "Creating StepExtractJob for tar.7z archive";
            // tar.7z包使用分步解压流程
            StepExtractJob *pStepExtractJob = new StepExtractJob(strArchiveFullPath, stOptions/*, strTargetFullPath, strNewArchiveFullPath*/);
            m_pArchiveJob = pStepExtractJob;

            // 连接槽函数
            connect(pStepExtractJob, &StepExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
            connect(pStepExtractJob, &StepExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
            connect(pStepExtractJob, &StepExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
            connect(pStepExtractJob, &StepExtractJob::signalQuery, this, &ArchiveManager::signalQuery);

            pStepExtractJob->start();
            qDebug() << "StepExtractJob started successfully";
            return true;
        }
    }

    qWarning() << "Failed to create interface for extraction";
    // 发送结束信号
    emit signalJobFinished(ArchiveJob::JT_Extract, PFT_Error, ET_PluginError);
    return false;
}

bool ArchiveManager::extractFiles2Path(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions)
{
    qDebug() << "Starting extractFiles2Path operation for archive:" << strArchiveFullPath;
    QJsonObject obj{
        {"tid", EventLogUtils::ExtractSingleFile},
        {"operate", "ExtractSingleFile"},
        {"describe", QString("Extract file from the compressed package : ") + strArchiveFullPath}
    };
    EventLogUtils::get().writeLogs(obj);
    if (nullptr == m_pInterface) {
        qDebug() << "Creating interface for extraction to path";
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        qDebug() << "Creating ExtractJob for extraction to path";
        ExtractJob *pExtractJob = new ExtractJob(listSelEntry, m_pInterface, stOptions);

        // 连接槽函数
        connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pExtractJob;
        pExtractJob->start();

        qDebug() << "ExtractJob for extraction to path started successfully";
        return true;
    }

    qWarning() << "Failed to create interface for extraction to path";
    return false;
}

bool ArchiveManager::deleteFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry)
{
    qDebug() << "Starting deleteFiles operation for archive:" << strArchiveFullPath;
    QJsonObject obj{
        {"tid", EventLogUtils::DelCompressFile},
        {"operate", "DelCompressFile"},
        {"describe", QString("Delete file from package : ") + strArchiveFullPath}
    };
    EventLogUtils::get().writeLogs(obj);
    if (nullptr == m_pInterface) {
        qDebug() << "Creating interface for deletion";
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        qDebug() << "Creating DeleteJob";
        DeleteJob *pDeleteJob = new DeleteJob(listSelEntry, m_pInterface);

        // 连接槽函数
        connect(pDeleteJob, &DeleteJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pDeleteJob, &DeleteJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pDeleteJob, &DeleteJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pDeleteJob, &DeleteJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pDeleteJob;
        pDeleteJob->start();

        qDebug() << "DeleteJob started successfully";
        return true;
    }

    qWarning() << "Failed to create interface for deletion";
    return false;
}

bool ArchiveManager::renameFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry)
{
    qDebug() << "Starting renameFiles operation for archive:" << strArchiveFullPath;
    QJsonObject obj{
        {"tid", EventLogUtils::RenameCompressFile},
        {"operate", "RenameCompressFile"},
        {"describe", QString("Rename file from package : ") + strArchiveFullPath}
    };
    EventLogUtils::get().writeLogs(obj);
    if (nullptr == m_pInterface) {
        qDebug() << "Creating interface for renaming";
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        qDebug() << "Creating RenameJob";
        RenameJob *pRenameJob = new RenameJob(listSelEntry, m_pInterface);

        // 连接槽函数
        connect(pRenameJob, &RenameJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pRenameJob, &RenameJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pRenameJob, &RenameJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pRenameJob, &RenameJob::signalQuery, this, &ArchiveManager::signalQuery);

        m_pArchiveJob = pRenameJob;
        pRenameJob->start();

        qDebug() << "RenameJob started successfully";
        return true;
    }

    qWarning() << "Failed to create interface for renaming";
    return false;
}

bool ArchiveManager::batchExtractFiles(const QStringList &listFiles, const QString &strTargetPath/*, bool bAutoCreatDir*/)
{
    qDebug() << "Starting batchExtractFiles operation for" << listFiles.size() << "files";
    BatchExtractJob *pBatchExtractJob = new BatchExtractJob();
    pBatchExtractJob->setExtractPath(strTargetPath/*, bAutoCreatDir*/);

    if (pBatchExtractJob->setArchiveFiles(listFiles)) {
        qDebug() << "Archive files set successfully for batch extraction";
        // 连接槽函数
        connect(pBatchExtractJob, &BatchExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pBatchExtractJob, &BatchExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pBatchExtractJob, &BatchExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
        connect(pBatchExtractJob, &BatchExtractJob::signalQuery, this, &ArchiveManager::signalQuery);
        connect(pBatchExtractJob, &BatchExtractJob::signalCurArchiveName, this, &ArchiveManager::signalCurArchiveName);

        m_pArchiveJob = pBatchExtractJob;
        pBatchExtractJob->start();

        qDebug() << "BatchExtractJob started successfully";
        return true;
    }

    qWarning() << "Failed to set archive files for batch extraction";
    SAFE_DELETE_ELE(pBatchExtractJob);
    return false;
}

bool ArchiveManager::openFile(const QString &strArchiveFullPath, const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram)
{
    qDebug() << "Starting openFile operation for archive:" << strArchiveFullPath;
    QJsonObject obj{
        {"tid", EventLogUtils::OpenCompressFile},
        {"operate", "OpenCompressFile"},
        {"describe", QString("Open file from package : ") + strArchiveFullPath}
    };
    EventLogUtils::get().writeLogs(obj);
    if (nullptr == m_pInterface) {
        qDebug() << "Creating interface for opening file";
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    if (m_pInterface) {
        qDebug() << "Creating OpenJob";
        OpenJob *pOpenJob = new OpenJob(stEntry, strTempExtractPath, strProgram, m_pInterface);

        // 连接槽函数
        connect(pOpenJob, &OpenJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pOpenJob, &OpenJob::signalQuery, this, &ArchiveManager::signalQuery);


        m_pArchiveJob = pOpenJob;
        pOpenJob->start();

        qDebug() << "OpenJob started successfully";
        return true;
    }

    qWarning() << "Failed to create interface for opening file";
    return false;
}

bool ArchiveManager::updateArchiveCacheData(const UpdateOptions &stOptions)
{
    qDebug() << "Starting updateArchiveCacheData operation";
    if (m_pInterface) {
        qDebug() << "Creating UpdateJob";
        UpdateJob *pUpdateJob = new UpdateJob(stOptions, m_pInterface);

        // 连接槽函数
        connect(pUpdateJob, &UpdateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);

        m_pArchiveJob = pUpdateJob;
        pUpdateJob->start();

        qDebug() << "UpdateJob started successfully";
        return true;
    }

    qWarning() << "No interface available for updating archive cache data";
    return false;
}

bool ArchiveManager::updateArchiveComment(const QString &strArchiveFullPath, const QString &strComment)
{
    qDebug() << "Starting updateArchiveComment operation for archive:" << strArchiveFullPath;
    ReadOnlyArchiveInterface *pInterface = UiTools::createInterface(strArchiveFullPath, true, UiTools::APT_Libzip); // zip添加注释使用libzipplugin

    if (pInterface) {
        qDebug() << "Creating CommentJob";
        CommentJob *pCommentJob = new CommentJob(strComment, pInterface);

        // 连接槽函数
        connect(pCommentJob, &CommentJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pCommentJob, &CommentJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);

        m_pArchiveJob = pCommentJob;
        pCommentJob->start();

        qDebug() << "CommentJob started successfully";
        return true;
    }

    qWarning() << "Failed to create interface for updating archive comment";
    return false;
}

bool ArchiveManager::convertArchive(const QString &strOriginalArchiveFullPath, const QString &strTargetFullPath, const QString &strNewArchiveFullPath)
{
    qDebug() << "Starting convertArchive operation from:" << strOriginalArchiveFullPath << "to:" << strNewArchiveFullPath;
    ConvertJob *pConvertJob = new ConvertJob(strOriginalArchiveFullPath, strTargetFullPath, strNewArchiveFullPath);
    m_pArchiveJob = pConvertJob;

    // 连接槽函数
    connect(pConvertJob, &ConvertJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pConvertJob, &ConvertJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pConvertJob, &ConvertJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pConvertJob, &ConvertJob::signalQuery, this, &ArchiveManager::signalQuery);

    pConvertJob->start();
    qDebug() << "ConvertJob started successfully";
    return true;
}

bool ArchiveManager::pauseOperation()
{
    qDebug() << "Attempting to pause current operation";
    // 调用job暂停接口
    if (m_pArchiveJob) {
        qDebug() << "Pausing archive job";
        m_pArchiveJob->doPause();

        return true;
    }

    qWarning() << "No archive job available to pause";
    return false;
}

bool ArchiveManager::continueOperation()
{
    qDebug() << "Attempting to continue current operation";
    // 调用job继续接口
    if (m_pArchiveJob) {
        qDebug() << "Continuing archive job";
        m_pArchiveJob->doContinue();

        return true;
    }

    qWarning() << "No archive job available to continue";
    return false;
}

bool ArchiveManager::cancelOperation()
{
    qDebug() << "Attempting to cancel current operation";
    // 调用job取消接口
    if (m_pArchiveJob) {
        qDebug() << "Canceling archive job";
        m_pArchiveJob->kill();
        m_pArchiveJob->deleteLater();
        m_pArchiveJob = nullptr;

        return true;
    }

    qWarning() << "No archive job available to cancel";
    return false;
}

QString ArchiveManager::getCurFilePassword()
{
    qDebug() << "Getting current file password";
    if (m_pInterface) {
        return m_pInterface->getPassword();
    }

    qWarning() << "No interface available to get password";
    return "";
}

bool ArchiveManager::currentStatus()
{
    qDebug() << "Getting current operation status";
    // 调用job状态接口
    if (m_pArchiveJob) {
        return m_pArchiveJob->status();
    }

    qWarning() << "No archive job available to get status";
    return false;
}

void ArchiveManager::slotJobFinished()
{
    qDebug() << "slotJobFinished called";
    if (m_pArchiveJob) {
        // 获取结束结果
        ArchiveJob::JobType eJobType = m_pArchiveJob->m_eJobType;
        PluginFinishType eFinishType = m_pArchiveJob->m_eFinishedType;
        ErrorType eErrorType = m_pArchiveJob->m_eErrorType;

        qInfo() << "Job finished - Type:" << eJobType
               << "FinishType:" << eFinishType
               << "ErrorType:" << eErrorType;

        // 释放job
        m_pArchiveJob->deleteLater();
        m_pArchiveJob = nullptr;

        // 发送结束信号
        emit signalJobFinished(eJobType, eFinishType, eErrorType);
    }

    // 释放临时记录的interface
    SAFE_DELETE_ELE(m_pTempInterface);
    qDebug() << "Temporary interface cleared";
}
