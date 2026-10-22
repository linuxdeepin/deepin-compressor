// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlejob.h"
#include "processopenthread.h"
#include "openwithdialog.h"
#include "datamanager.h"
#include "uitools.h"
#include "common/ddlog.h"

#include <linux/limits.h>
#include <QUuid>
#include <QThread>
#include <QDebug>
#include <QDir>

// 工作线程
void SingleJobThread::run()
{
    // qCDebug(appLog) << "SingleJobThread started";
    q->doWork();    // 在线程中执行操作
}

// 单个操作
SingleJob::SingleJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : ArchiveJob(parent)
    , m_pInterface(pInterface)
    , d(new SingleJobThread(this))
{
    qCDebug(appLog) << "SingleJob instance created, interface:" << pInterface;
}

SingleJob::~SingleJob()
{
    qCDebug(appLog) << "SingleJob instance destroyed";
    if (d->isRunning()) {
        qCDebug(appLog) << "Stopping worker thread";
        d->quit();      // 线程安全退出，不能使用terminate强行退出
        d->wait();
    }

    delete d;
}

void SingleJob::start()
{
    qCDebug(appLog) << "Starting job, type:" << m_eJobType;
    jobTimer.start();

    // 若插件指针为空，立即异常退出
    if (nullptr == m_pInterface) {
        qCWarning(appLog) << "Interface is null, aborting job";
        slotFinished(PFT_Error);
        return;
    }

    // 判断是否通过线程的方式调用
    if (m_pInterface->waitForFinished()) {
        qCDebug(appLog) << "Executing work directly";
        doWork();   // 直接执行操作
    } else {
        qCDebug(appLog) << "Starting worker thread";
        d->start(); // 开启线程，执行操作
    }
}

void SingleJob::doPause()
{
    qCDebug(appLog) << "Pausing job";
    // 调用插件暂停接口
    if (m_pInterface) {
        qCDebug(appLog) << "Calling interface pause operation";
        m_pInterface->pauseOperation();
    } else {
        qCWarning(appLog) << "Interface is null, cannot pause";
    }
}

void SingleJob::doContinue()
{
    qCDebug(appLog) << "Resuming job";
    // 调用插件继续接口
    if (m_pInterface) {
        qCDebug(appLog) << "Calling interface continue operation";
        m_pInterface->continueOperation();
    } else {
        qCWarning(appLog) << "Interface is null, cannot continue";
    }
}

bool SingleJob::status()
{
    qCDebug(appLog) << "Checking job status";
    // 调用插件继续接口
    if (m_pInterface) {
        bool status = m_pInterface->status();
        qCDebug(appLog) << "Job status:" << status;
        return status;
    }

    qCWarning(appLog) << "Interface is null, returning false status";
    return false;
}

SingleJobThread *SingleJob::getdptr()
{
    qCDebug(appLog) << "Getting worker thread pointer";
    return d;
}

bool SingleJob::doKill()
{
    qCDebug(appLog) << "Killing job, type:" << m_eJobType;
    if (nullptr == m_pInterface) {
        qCWarning(appLog) << "Interface is null, cannot kill job";
        return false;
    }

    const bool killed = m_pInterface->doKill();
    if (killed) {
        qCDebug(appLog) << "Job killed successfully";
        return true;
    }

    if (d->isRunning()) { //Returns true if the thread is running
        qCInfo(appLog) << "Requesting graceful thread interruption, will abort in one second otherwise.";
        d->requestInterruption(); //请求中断线程(建议性)
        d->wait(1000); //阻塞1s或阻塞到线程结束(取小)
    }

    qCDebug(appLog) << "Job kill completed";
    return true;
}

void SingleJob::initConnections()
{
    qCDebug(appLog) << "Initializing signal connections";
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &SingleJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalprogress, this, &SingleJob::signalprogress, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalCurFileName, this, &SingleJob::signalCurFileName, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFileWriteErrorName, this, &SingleJob::signalFileWriteErrorName, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalQuery, this, &SingleJob::signalQuery, Qt::ConnectionType::AutoConnection);
    qCDebug(appLog) << "Signal connections initialized";
}

void SingleJob::slotFinished(PluginFinishType eType)
{
    qCInfo(appLog) << "Job finished, type:" << m_eJobType
           << ", result:" << eType
           << ", time:" << jobTimer.elapsed() << "ms";
    m_eFinishedType = eType;

    if (m_pInterface) {
        m_eErrorType = m_pInterface->errorType();
        qCDebug(appLog) << "Error type:" << m_eErrorType;
    }

    emit signalJobFinshed();
}

// 加载操作
LoadJob::LoadJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
{
    qCDebug(appLog) << "LoadJob instance created";
    m_eJobType = JT_Load;
    initConnections();
}

LoadJob::~LoadJob()
{
    qCDebug(appLog) << "LoadJob instance destroyed";
}

void LoadJob::doWork()
{
    qCDebug(appLog) << "LoadJob starting work";
    if (m_pInterface) {
        PluginFinishType eType = m_pInterface->list();
        qCDebug(appLog) << "List operation completed, result:" << eType;

        if (!(m_pInterface->waitForFinished())) {
            qCDebug(appLog) << "Emitting finished signal";
            slotFinished(eType);
        }
    } else {
        qCWarning(appLog) << "Interface is null in LoadJob";
    }
}

// 压缩操作
AddJob::AddJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
    , m_stCompressOptions(options)
{
    initConnections();
    m_eJobType = JT_Add;
}

AddJob::~AddJob()
{

}

void AddJob::doWork()
{
    qCDebug(appLog) << "AddJob starting work, files count:" << m_vecFiles.size();
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (nullptr == pWriteInterface) {
        qCWarning(appLog) << "Failed to cast to ReadWriteArchiveInterface";
        return;
    }

    PluginFinishType eType = pWriteInterface->addFiles(m_vecFiles, m_stCompressOptions);
    qCDebug(appLog) << "Add files operation completed, result:" << eType;

    if (!(pWriteInterface->waitForFinished())) {
        qCDebug(appLog) << "Emitting finished signal";
        slotFinished(eType);
    }
}

// 创建压缩包操作
CreateJob::CreateJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
    , m_stCompressOptions(options)
{
    initConnections();
    m_eJobType = JT_Create;
}

CreateJob::~CreateJob()
{

}

void CreateJob::doWork()
{
    qCDebug(appLog) << "CreateJob starting work, files count:" << m_vecFiles.size();
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface == nullptr) {
        qCWarning(appLog) << "Failed to cast to ReadWriteArchiveInterface";
        return;
    }

    // 调用压缩接口
    PluginFinishType eType = pWriteInterface->addFiles(m_vecFiles, m_stCompressOptions);
    qCDebug(appLog) << "Create archive operation completed, result:" << eType;

    if (!(pWriteInterface->waitForFinished())) {
        qCDebug(appLog) << "Emitting finished signal";
        slotFinished(eType);
    }
}

bool CreateJob::doKill()
{
    qCDebug(appLog) << "CreateJob doKill";
    if (nullptr == m_pInterface) {
        qCDebug(appLog) << "CreateJob doKill, interface is null";
        return false;
    }

    const bool killed = m_pInterface->doKill();
    if (killed) {
        qCDebug(appLog) << "CreateJob doKill, interface killed";
        cleanCompressFileCancel();
        return true;
    }

    if (d->isRunning()) { //Returns true if the thread is running
        qCInfo(appLog) << "Requesting graceful thread interruption, will abort in one second otherwise.";
        d->requestInterruption(); //请求中断线程(建议性)
        d->wait(1000); //阻塞1s或阻塞到线程结束(取小)
    }
    cleanCompressFileCancel();
    return true;
}

void CreateJob::cleanCompressFileCancel()
{
    qCDebug(appLog) << "CreateJob cleanCompressFileCancel";
    if (m_stCompressOptions.bSplit) {
        qCDebug(appLog) << "CreateJob cleanCompressFileCancel, split compress";
        // 判断 7z分卷压缩的 文件名
        QFileInfo file(dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface)->getArchiveName());
        QStringList nameFilters;
        nameFilters << file.fileName() + ".0*";
        QDir dir(file.path());
        QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

        foreach (QFileInfo fi, files) {
            QFile fiRemove(fi.filePath());
            if (fiRemove.exists()) {
                qCInfo(appLog) << "取消时删除:" << fiRemove.fileName();
                fiRemove.remove();
            }
        }
    } else {
        qCDebug(appLog) << "CreateJob cleanCompressFileCancel, not split compress";
        QFile fiRemove(dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface)->getArchiveName());  // 没有判断 7z分卷压缩的 文件名
        if (fiRemove.exists()) {
            qCInfo(appLog) << "取消时删除:" << fiRemove.fileName();
            fiRemove.remove();
        }
    }
}

// 解压操作
ExtractJob::ExtractJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const ExtractionOptions &options, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
    , m_stExtractionOptions(options)
{
    qCDebug(appLog) << "ExtractJob constructor";
    initConnections();
    m_eJobType = JT_Extract;
}

ExtractJob::~ExtractJob()
{
    qCDebug(appLog) << "ExtractJob instance destroyed";
}

void ExtractJob::doWork()
{
    qCDebug(appLog) << "ExtractJob starting work, files count:" << m_vecFiles.size();
    if (m_pInterface) {
        PluginFinishType eType = m_pInterface->extractFiles(m_vecFiles, m_stExtractionOptions);
        qCDebug(appLog) << "Extract operation completed, result:" << eType;

        if (!(m_pInterface->waitForFinished())) {
            qCDebug(appLog) << "Emitting finished signal";
            slotFinished(eType);
        } else {
            if (PFT_Error == eType) {
                qCWarning(appLog) << "Extract operation failed";
                errorcode = false;
            }
        }
    } else {
        qCWarning(appLog) << "Interface is null in ExtractJob";
    }
}

// 删除操作
DeleteJob::DeleteJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
{
    qCDebug(appLog) << "DeleteJob constructor";
    m_eJobType = JT_Delete;
    initConnections();
    m_eJobType = JT_Delete;
}

DeleteJob::~DeleteJob()
{
    qCDebug(appLog) << "DeleteJob instance destroyed";
}

void DeleteJob::doWork()
{
    qCDebug(appLog) << "DeleteJob starting work, files count:" << m_vecFiles.size();
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (nullptr == pWriteInterface) {
        qCWarning(appLog) << "Failed to cast to ReadWriteArchiveInterface in DeleteJob";
        return;
    }

    PluginFinishType eType = pWriteInterface->deleteFiles(m_vecFiles);

    if (!(pWriteInterface->waitForFinished())) {
        qCDebug(appLog) << "Emitting finished signal";
        slotFinished(eType);
    }
}

// 重命名操作
RenameJob::RenameJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
{
    qCDebug(appLog) << "RenameJob constructor";
    initConnections();
    m_eJobType = JT_Rename;
}

RenameJob::~RenameJob()
{
    qCDebug(appLog) << "RenameJob instance destroyed";
}

void RenameJob::doWork()
{
    qCDebug(appLog) << "RenameJob starting work, files count:" << m_vecFiles.size();
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (nullptr == pWriteInterface) {
        qCWarning(appLog) << "Failed to cast to ReadWriteArchiveInterface in RenameJob";
        return;
    }

    PluginFinishType eType = pWriteInterface->renameFiles(m_vecFiles);
    qCDebug(appLog) << "Rename operation completed, result:" << eType;

    if (!(pWriteInterface->waitForFinished())) {
        qCDebug(appLog) << "Emitting finished signal";
        slotFinished(eType);
    } else {
        qCDebug(appLog) << "Rename operation completed synchronously";
    }
}

OpenJob::OpenJob(const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_stEntry(stEntry)
    , m_strTempExtractPath(strTempExtractPath)
    , m_strProgram(strProgram)
{
    qCDebug(appLog) << "OpenJob instance created for file:" << stEntry.strFileName;
    m_eJobType = JT_Open;
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &OpenJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalQuery, this, &SingleJob::signalQuery, Qt::ConnectionType::AutoConnection);
}

OpenJob::~OpenJob()
{
    qCDebug(appLog) << "OpenJob instance destroyed";
}

void OpenJob::doWork()
{
    qCDebug(appLog) << "OpenJob starting work for file:" << m_stEntry.strFileName;
    if (m_pInterface) {
        // 构建解压参数
        ExtractionOptions options;
        options.bOpen = true;
        options.strTargetPath = m_strTempExtractPath;
        // 当作提取，去除父目录
        if (m_stEntry.strFullPath.contains(QDir::separator())) {
            qCDebug(appLog) << "Setting destination for nested file";
            int iIndex = m_stEntry.strFullPath.lastIndexOf(QDir::separator());
            if (iIndex > 0)
                options.strDestination = m_stEntry.strFullPath.left(iIndex + 1); // 当前路径截掉最后一级目录(保留'/')
        }
        options.qSize = m_stEntry.qSize;

        PluginFinishType eType = m_pInterface->extractFiles(QList<FileEntry>() << m_stEntry, options);
        qCDebug(appLog) << "Open extraction operation completed, result:" << eType;

        if (!(m_pInterface->waitForFinished())) {
            qCDebug(appLog) << "Emitting finished signal";
            slotFinished(eType);
        } else {
            qCDebug(appLog) << "Open extraction completed synchronously";
        }
    } else {
        qCWarning(appLog) << "Interface is null in OpenJob";
    }
}

void OpenJob::slotFinished(PluginFinishType eType)
{
    qCDebug(appLog) << "OpenJob finished with type:" << eType;
    if (PFT_Nomral == eType) {
        qCDebug(appLog) << "Processing file for opening";
        QString name = m_stEntry.strFileName;

        //对于超长文件打开失败问题处理
        QString strTempFileName = m_stEntry.strFileName;
        if (NAME_MAX < QString(strTempFileName).toLocal8Bit().length() && !strTempFileName.endsWith(QDir::separator())) {
            qCDebug(appLog) << "Handling long filename";
            QString strTemp = strTempFileName.left(60);
            name = strTemp + QString("(%1)").arg(1, 3, 10, QChar('0'))+"." + QFileInfo(strTempFileName).completeSuffix();
        }

        if (name.contains("%")) { // 文件名含有%的时候无法直接双击打开, 创建一个该文件的链接，文件名不含有%，通过打开链接打开源文件
            qCDebug(appLog) << "Creating link for file with % in name";
            name = m_strTempExtractPath + QDir::separator() + name.replace("%", "1"); // 将文件名中的%替换为1;
            if (!QFile::link(m_stEntry.strFileName, name)) { // 创建链接
                qCWarning(appLog) << "Failed to create link for file";
                return;
            }
        } else {
            name = m_strTempExtractPath + QDir::separator() + name;
        }

        // 在线程中执行外部应用打开的命令
        qCDebug(appLog) << "Starting external process to open file:" << name;
        ProcessOpenThread *p = new ProcessOpenThread;
        p->setProgramPath(OpenWithDialog::getProgramPathByExec(m_strProgram));
        p->setArguments(QStringList() << name);
        p->start();
    } else {
        qCWarning(appLog) << "Failed to open file, type:" << eType;
    }

    SingleJob::slotFinished(eType); // 在结束上述操作之后发送job结束信号
}

UpdateJob::UpdateJob(const UpdateOptions &options, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_stOptions(options)
{
    qCDebug(appLog) << "UpdateJob instance created";
    m_eJobType = JT_Update;
    //connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &UpdateJob::slotFinished, Qt::ConnectionType::UniqueConnection);
}

UpdateJob::~UpdateJob()
{
    qCDebug(appLog) << "UpdateJob instance destroyed";
}

void UpdateJob::start()
{
    qCDebug(appLog) << "Starting UpdateJob";
    jobTimer.start();

    // 若插件指针为空，立即异常退出
    if (nullptr == m_pInterface) {
        qCWarning(appLog) << "Interface is null in UpdateJob";
        slotFinished(PFT_Error);
        return;
    }

    d->start(); // 开启线程，执行操作
}

void UpdateJob::doWork()
{
    qCDebug(appLog) << "UpdateJob starting work";
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface) {
        qCDebug(appLog) << "Calling updateArchiveData";
        // 调用更新函数
        PluginFinishType eType = pWriteInterface->updateArchiveData(m_stOptions);
        qCDebug(appLog) << "Update operation completed, result:" << eType;

        //if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
        //}
    } else {
        qCWarning(appLog) << "Failed to cast to ReadWriteArchiveInterface in UpdateJob";
    }
}

CommentJob::CommentJob(const QString &strComment, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_strComment(strComment)
{
    qCDebug(appLog) << "CommentJob instance created with comment length:" << strComment.length();
    m_eJobType = JT_Comment;

    // 进度和结束处理
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &CommentJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalprogress, this, &CommentJob::signalprogress, Qt::ConnectionType::UniqueConnection);
}

CommentJob::~CommentJob()
{
    qCDebug(appLog) << "CommentJob instance destroyed";
    if (m_pInterface) {
        delete  m_pInterface;
        m_pInterface = nullptr;
    }
}

void CommentJob::doWork()
{
    qCDebug(appLog) << "CommentJob starting work";
    qCInfo(appLog) << "Adding comment";

    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface) {
        PluginFinishType eType = pWriteInterface->addComment(m_strComment);
        qCDebug(appLog) << "Add comment operation completed, result:" << eType;
        if (!(pWriteInterface->waitForFinished())) {
            qCDebug(appLog) << "Emitting finished signal";
            slotFinished(eType);
        } else {
            qCDebug(appLog) << "Comment operation completed synchronously";
        }
    } else {
        qCWarning(appLog) << "Failed to cast to ReadWriteArchiveInterface in CommentJob";
    }
}

ComplexJob::ComplexJob(const QString strOriginalArchiveFullPath, QObject *parent)
    : ArchiveJob(parent)
    , m_strOriginalArchiveFullPath(strOriginalArchiveFullPath)
{
    qCDebug(appLog) << "ComplexJob instance created for:" << strOriginalArchiveFullPath;
}

ComplexJob::~ComplexJob()
{
    qCDebug(appLog) << "ComplexJob instance destroyed";
    if (m_pIface) {
        delete  m_pIface;
        m_pIface = nullptr;
    }
}

void ComplexJob::doPause()
{
    qCDebug(appLog) << "Pausing ComplexJob";
    // 调用插件暂停接口
    if (m_pIface) {
        qCDebug(appLog) << "Calling interface pause operation";
        m_pIface->pauseOperation();
    } else {
        qCWarning(appLog) << "Interface is null, cannot pause";
    }
}

void ComplexJob::doContinue()
{
    qCDebug(appLog) << "Continuing ComplexJob";
    // 调用插件继续接口
    if (m_pIface) {
        qCDebug(appLog) << "Calling interface continue operation";
        m_pIface->continueOperation();
    } else {
        qCWarning(appLog) << "Interface is null, cannot continue";
    }
}

bool ComplexJob::doKill()
{
    qCDebug(appLog) << "Killing ComplexJob";
    if (m_pIface) {
        return m_pIface->doKill();
    }
    qCWarning(appLog) << "Interface is null, cannot kill";
    return false;
}

void ComplexJob::slotHandleSingleJobProgress(double dPercentage)
{
    if (0 == m_iStepNo) { // 解压进度
        qCDebug(appLog) << "Extract step progress:" << dPercentage;
        emit signalprogress(dPercentage * 0.3);
    } else { // 压缩进度
        qCDebug(appLog) << "Compress step progress:" << dPercentage;
        emit signalprogress(30 + dPercentage * 0.7);
    }
}

void ComplexJob::slotHandleSingleJobCurFileName(const QString &strName)
{
    qCDebug(appLog) << "Current file in complex job:" << strName;
    emit signalCurFileName(strName);
}

ConvertJob::ConvertJob(const QString strOriginalArchiveFullPath, const QString strTargetFullPath, const QString strNewArchiveFullPath, QObject *parent)
    : ComplexJob(strOriginalArchiveFullPath, parent)
    , m_strTargetFullPath(strTargetFullPath)
    , m_strNewArchiveFullPath(strNewArchiveFullPath)
{
    qCDebug(appLog) << "ConvertJob instance created from:" << strOriginalArchiveFullPath << "to:" << strNewArchiveFullPath;
    m_eJobType = JT_Convert;
}

ConvertJob::~ConvertJob()
{
    qCDebug(appLog) << "ConvertJob instance destroyed";
    SAFE_DELETE_ELE(m_pCreateJob)
    SAFE_DELETE_ELE(m_pExtractJob);
}

void ConvertJob::start()
{
    qCDebug(appLog) << "Starting ConvertJob";
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strOriginalArchiveFullPath);

    if (pIface) {
        qCInfo(appLog) << "格式转换开始解压";
        m_pIface = pIface;
        m_iStepNo = 0;

        // 创建解压参数
        QFileInfo file(m_strOriginalArchiveFullPath);
        ExtractionOptions stOptions;
        stOptions.strTargetPath = m_strTargetFullPath;
        stOptions.qComressSize = file.size();
        stOptions.bAllExtract = true;

        // 创建解压操作
        qCDebug(appLog) << "Creating ExtractJob for conversion";
        m_pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions);
        connect(m_pExtractJob, &ExtractJob::signalprogress, this, &ConvertJob::slotHandleSingleJobProgress);
        connect(m_pExtractJob, &ExtractJob::signalCurFileName, this, &ConvertJob::slotHandleSingleJobCurFileName);
        connect(m_pExtractJob, &ExtractJob::signalQuery, this, &ConvertJob::signalQuery);
        connect(m_pExtractJob, &ExtractJob::signalJobFinshed, this, &ConvertJob::slotHandleExtractFinished);

        m_pExtractJob->doWork();
    } else {
        qCWarning(appLog) << "Failed to create interface for conversion";
    }
}

void ConvertJob::slotHandleExtractFinished()
{
    qCDebug(appLog) << "Extract step finished in ConvertJob";
    // 解压结束
    if (m_pExtractJob) {
        m_eFinishedType = m_pExtractJob->m_eFinishedType;
        m_eErrorType = m_pExtractJob->m_eErrorType;
        qCDebug(appLog) << "Extract finished with type:" << m_eFinishedType;

        switch (m_eFinishedType) {
        // 正常结束之后，进行压缩操作
        case PFT_Nomral: {
            qCInfo(appLog) << "格式转换开始压缩";
            m_iStepNo = 1;

            ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strNewArchiveFullPath, true);

            if (pIface) {
                qCDebug(appLog) << "Creating interface for compression step";
                SAFE_DELETE_ELE(m_pIface);
                m_pIface = pIface;
                QList<FileEntry> listEntry;
                // 在临时路径里面获取待压缩文件
                QDir dir(m_strTargetFullPath);
                QFileInfoList fileList = dir.entryInfoList(QDir::AllEntries | QDir::System
                                                           | QDir::NoDotAndDotDot | QDir::NoSymLinks
                                                           | QDir::Hidden);

                qCDebug(appLog) << "Found" << fileList.size() << "files to compress";
                foreach (QFileInfo strFile, fileList) {
                    FileEntry stFileEntry;
                    stFileEntry.strFullPath = strFile.filePath();
                    listEntry.push_back(stFileEntry);
                }

                // 构建压缩参数
                CompressOptions options;
                options.iCompressionLevel = 3; // 默认压缩等级为3，其余参数均为默认选项
                options.qTotalSize = DataManager::get_instance().archiveData().qSize; // list压缩包时存储的压缩包内文件实际总大小

                // 创建压缩操作
                qCDebug(appLog) << "Creating CreateJob for compression";
                m_pCreateJob = new CreateJob(listEntry, pIface, options);
                connect(m_pCreateJob, &CreateJob::signalprogress, this, &ConvertJob::slotHandleSingleJobProgress);
                connect(m_pCreateJob, &CreateJob::signalCurFileName, this, &ConvertJob::slotHandleSingleJobCurFileName);
                connect(m_pCreateJob, &CreateJob::signalJobFinshed, this, &ConvertJob::signalJobFinshed);

                m_pCreateJob->doWork();
            } else {
                qCWarning(appLog) << "Failed to create interface for compression step";
            }
        }
        break;
        // 用户取消之后，不进行压缩
        case PFT_Cancel: {
            qCInfo(appLog) << "取消格式转换";
            emit signalJobFinshed();
        }
        break;
        // 出现错误的情况，提示用户
        case PFT_Error: {
            qCInfo(appLog) << "格式转换错误";
            emit signalJobFinshed();
        }
        break;
        }
    } else {
        qCWarning(appLog) << "No extract job to handle";
    }
}

StepExtractJob::StepExtractJob(const QString strOriginalArchiveFullPath, const ExtractionOptions &stOptions, QObject *parent)
    : ComplexJob(strOriginalArchiveFullPath, parent)
    , m_stExtractionOptions(stOptions)
{
    qCDebug(appLog) << "StepExtractJob instance created for:" << strOriginalArchiveFullPath;
    m_eJobType = JT_StepExtract;
}

StepExtractJob::~StepExtractJob()
{
    qCDebug(appLog) << "StepExtractJob instance destroyed";
}

void StepExtractJob::start()
{
    qCDebug(appLog) << "Starting StepExtractJob";
    // tar.7z 指定使用cli7zplugin先解压出tar包
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strOriginalArchiveFullPath, false);

    if (nullptr != pIface) {
        qCInfo(appLog) << "StepExtractJob: 开始解压tar.7z成tar";
        m_pIface = pIface;
        m_iStepNo = 0;

        // 设置解压临时路径
        QFileInfo file(m_strOriginalArchiveFullPath);
        QString strProcessID = QString::number(QCoreApplication::applicationPid());   // 获取应用进程号
        m_strTempFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                            + QDir::separator() + strProcessID + QDir::separator()
                            + QUuid::createUuid().toString(QUuid::Id128);
        qCDebug(appLog) << "Temp extraction path:" << m_strTempFilePath;
        // 创建解压参数
        ExtractionOptions stOptions;
        stOptions.strTargetPath = m_strTempFilePath;
        stOptions.qComressSize = file.size();
        stOptions.bAllExtract = true;

        // 创建解压操作
        qCDebug(appLog) << "Creating first ExtractJob for tar.7z";
        m_pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions, this);
        connect(m_pExtractJob, &ExtractJob::signalprogress, this, &StepExtractJob::slotHandleSingleJobProgress);
        connect(m_pExtractJob, &ExtractJob::signalCurFileName, this, &StepExtractJob::slotHandleSingleJobCurFileName);
        connect(m_pExtractJob, &ExtractJob::signalQuery, this, &StepExtractJob::signalQuery);
        connect(m_pExtractJob, &ExtractJob::signalJobFinshed, this, &StepExtractJob::slotHandleExtractFinished);

        m_pExtractJob->start();
    } else {
        qCWarning(appLog) << "Failed to create interface for StepExtractJob";
    }
}

void StepExtractJob::slotHandleExtractFinished()
{
    qCDebug(appLog) << "Step extract finished handler called";
    // 解压结束
    if (nullptr != m_pExtractJob) {
        m_eFinishedType = m_pExtractJob->m_eFinishedType;
        m_eErrorType = m_pExtractJob->m_eErrorType;
        qCDebug(appLog) << "Extract finished with type:" << m_eFinishedType;

        switch (m_eFinishedType) {
        // 正常结束之后，进行压缩操作
        case PFT_Nomral: {
            qCDebug(appLog) << "Extract completed successfully, checking temp files";
            // 获取临时解压文件
            QDir dir(m_strTempFilePath);
            if (!dir.exists()) {
                qCWarning(appLog) << "Temp directory does not exist";
                return;
            }
            QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::System
                                                   | QDir::NoDotAndDotDot | QDir::Hidden);
            qCDebug(appLog) << "Found" << list.count() << "files in temp directory";

            /***tar.7z格式压缩流程特殊处理***
             * 1、tar.7z本质上就是一个tar包压缩成7z包，类型依然是x-7z-compressed
             * 2、只针对7z里只有一个tar包的解压才做特殊处理，即直接解压出tar包内的文件
             * 3、对于7z里有多个文件或唯一文件不是tar包的情况，解压不做特殊处理
             * 4、后缀不为tar.7z,解压不做特殊处理
             */
            if (1 == list.count()
                    && list.at(0).filePath().endsWith(".tar")
                    && determineMimeType(list.at(0).filePath()).name() == QLatin1String("application/x-tar")) {
                qCDebug(appLog) << "Found single tar file, proceeding with special handling";
                QFileInfo fileInfo = list.at(0);

                ReadOnlyArchiveInterface *pIface = UiTools::createInterface(fileInfo.absoluteFilePath(), false);

                if (nullptr != pIface) {
                    qCInfo(appLog) << "StepExtractJob: 开始解压tar";
                    SAFE_DELETE_ELE(m_pIface);
                    m_pIface = pIface;
                    m_iStepNo = 1;

                    // 创建解压参数
                    ExtractionOptions stOptions = m_stExtractionOptions;
                    stOptions.qComressSize = fileInfo.size();
                    stOptions.qSize = fileInfo.size();
                    stOptions.bAllExtract = true;
                    stOptions.bTar_7z = false;

                    // 创建解压操作
                    qCDebug(appLog) << "Creating second ExtractJob for tar file";
                    m_pExtractJob2 = new ExtractJob(QList<FileEntry>(), pIface, stOptions, this);
                    connect(m_pExtractJob2, &ExtractJob::signalprogress, this, &StepExtractJob::slotHandleSingleJobProgress);
                    connect(m_pExtractJob2, &ExtractJob::signalCurFileName, this, &StepExtractJob::slotHandleSingleJobCurFileName);
                    connect(m_pExtractJob2, &ExtractJob::signalQuery, this, &StepExtractJob::signalQuery);
                    connect(m_pExtractJob2, &ExtractJob::signalJobFinshed, this, &StepExtractJob::signalJobFinshed);

                    m_pExtractJob2->start();
                } else {
                    qCWarning(appLog) << "Failed to create interface for tar file";
                }
            } else {
                qCDebug(appLog) << "Using fallback extraction method";
                ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strOriginalArchiveFullPath, false);

                if (nullptr != pIface) {
                    qCInfo(appLog) << "StepExtractJob: 开始直接解压原文件";
                    SAFE_DELETE_ELE(m_pIface);
                    m_pIface = pIface;
                    m_iStepNo = 1;

                    // 创建解压参数
                    ExtractionOptions stOptions = m_stExtractionOptions;
                    stOptions.password = DataManager::get_instance().archiveData().strPassword; // 第二次解压使用第一次保存的密码
                    stOptions.bAllExtract = true;
                    stOptions.bTar_7z = false;

                    // 创建解压操作
                    qCDebug(appLog) << "Creating fallback ExtractJob";
                    m_pExtractJob2 = new ExtractJob(QList<FileEntry>(), pIface, stOptions, this);
                    connect(m_pExtractJob2, &ExtractJob::signalprogress, this, &StepExtractJob::slotHandleSingleJobProgress);
                    connect(m_pExtractJob2, &ExtractJob::signalCurFileName, this, &StepExtractJob::slotHandleSingleJobCurFileName);
                    connect(m_pExtractJob2, &ExtractJob::signalQuery, this, &StepExtractJob::signalQuery);
                    connect(m_pExtractJob2, &ExtractJob::signalJobFinshed, this, &StepExtractJob::signalJobFinshed);

                    m_pExtractJob2->start();
                } else {
                    qCWarning(appLog) << "Failed to create interface for fallback extraction";
                }
            }
        }
        break;
        // 用户取消之后，不进行压缩
        case PFT_Cancel: {
            qCInfo(appLog) << "Step extract cancelled";
            emit signalJobFinshed();
        }
        break;
        // 出现错误的情况，提示用户
        case PFT_Error: {
            qCInfo(appLog) << "Step extract error";
            emit signalJobFinshed();
        }
        break;
        }
    } else {
        qCWarning(appLog) << "No extract job to handle";
    }
}

bool StepExtractJob::doKill()
{
    qCDebug(appLog) << "Killing StepExtractJob";
    if (m_pIface) {
        const bool killed = m_pIface->doKill();
        if (killed) {
            qCDebug(appLog) << "StepExtractJob killed successfully";
            return true;
        }
    }
    if (m_pExtractJob2 && m_pExtractJob2->getdptr()->isRunning()) { //Returns true if the thread is running
        qCInfo(appLog) << "Requesting graceful thread interruption, will abort in one second otherwise.";
        m_pExtractJob2->getdptr()->requestInterruption(); //请求中断线程(建议性)
        m_pExtractJob2->getdptr()->wait(1000); //阻塞1s或阻塞到线程结束(取小)
    }

    return true;
}
