/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "singlejob.h"
#include "kprocess.h"
#include "openwithdialog.h"
#include "datamanager.h"
#include "uitools.h"

#include <QThread>
#include <QDebug>
#include <QDir>

// 工作线程
void SingleJobThread::run()
{
    q->doWork();    // 在线程中执行操作
}

// 单个操作
SingleJob::SingleJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : ArchiveJob(parent)
    , m_pInterface(pInterface)
    , d(new SingleJobThread(this))
{

}

SingleJob::~SingleJob()
{

}

void SingleJob::start()
{
    jobTimer.start();

    // 若插件指针为空，立即异常退出
    if (m_pInterface == nullptr) {
        slotFinished(PFT_Error);
        return;
    }

    // 判断是否通过线程的方式调用
    if (m_pInterface->waitForFinished()) {
        doWork();   // 直接执行操作
    } else {
        d->start(); // 开启线程，执行操作
    }
}

void SingleJob::doPause()
{
    // 调用插件暂停接口
    if (m_pInterface) {
        m_pInterface->pauseOperation();
    }
}

void SingleJob::doContinue()
{
    // 调用插件继续接口
    if (m_pInterface) {
        m_pInterface->continueOperation();
    }
}

bool SingleJob::doKill()
{
    const bool killed = m_pInterface->doKill();
    if (killed) {
        return true;
    }

    if (d->isRunning()) { //Returns true if the thread is running
        qDebug() << "Requesting graceful thread interruption, will abort in one second otherwise.";
        d->requestInterruption(); //请求中断线程(建议性)
        d->wait(1000); //阻塞1s或阻塞到线程结束(取小)
    }

    return true;
}

void SingleJob::initConnections()
{
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &SingleJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalprogress, this, &SingleJob::signalprogress, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalCurFileName, this, &SingleJob::signalCurFileName, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalQuery, this, &SingleJob::signalQuery, Qt::ConnectionType::AutoConnection);
}

void SingleJob::slotFinished(PluginFinishType eType)
{
    qDebug() << "Job finished, result:" << eType << ", time:" << jobTimer.elapsed() << "ms";
    m_eFinishedType = eType;
    m_eErrorType = m_pInterface->errorType();

    emit signalJobFinshed();

}

// 加载操作
LoadJob::LoadJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
{
    m_eJobType = JT_Load;
    initConnections();
}

LoadJob::~LoadJob()
{

}

void LoadJob::doWork()
{
    PluginFinishType eType = m_pInterface->list();

    if (!(m_pInterface->waitForFinished())) {
        slotFinished(eType);
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
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface == nullptr) {
        return;
    }

    PluginFinishType eType = pWriteInterface->addFiles(m_vecFiles, m_stCompressOptions);

    if (!(pWriteInterface->waitForFinished())) {
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
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface == nullptr) {
        return;
    }

    // 调用压缩接口
    PluginFinishType eType = pWriteInterface->addFiles(m_vecFiles, m_stCompressOptions);

    if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
    }

}

bool CreateJob::doKill()
{
    const bool killed = m_pInterface->doKill();
    if (killed) {
        cleanCompressFileCancel();
        return true;
    }

    if (d->isRunning()) { //Returns true if the thread is running
        qDebug() << "Requesting graceful thread interruption, will abort in one second otherwise.";
        d->requestInterruption(); //请求中断线程(建议性)
        d->wait(1000); //阻塞1s或阻塞到线程结束(取小)
    }
    cleanCompressFileCancel();
    return true;
}

void CreateJob::cleanCompressFileCancel()
{
    if (m_stCompressOptions.bSplit) {
        // 判断 7z分卷压缩的 文件名
        QFileInfo file(dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface)->getArchiveName());
        QStringList nameFilters;
        nameFilters << file.fileName() + ".0*";
        QDir dir(file.path());
        QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

        foreach (QFileInfo fi, files) {
            QFile fiRemove(fi.filePath());
            if (fiRemove.exists()) {
                qDebug() << "取消时删除:" << fiRemove.fileName();
                fiRemove.remove();
            }
        }
    } else {
        QFile fiRemove(dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface)->getArchiveName());  // 没有判断 7z分卷压缩的 文件名
        if (fiRemove.exists()) {
            qDebug() << "取消时删除:" << fiRemove.fileName();
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
    initConnections();
    m_eJobType = JT_Extract;
}

ExtractJob::~ExtractJob()
{

}

void ExtractJob::doWork()
{
    PluginFinishType eType = m_pInterface->extractFiles(m_vecFiles, m_stExtractionOptions);

    if (!(m_pInterface->waitForFinished())) {
        slotFinished(eType);
    }
}

// 删除操作
DeleteJob::DeleteJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
{
    m_eJobType = JT_Delete;
    initConnections();
    m_eJobType = JT_Delete;
}

DeleteJob::~DeleteJob()
{

}

void DeleteJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface == nullptr) {
        return;
    }

    PluginFinishType eType = pWriteInterface->deleteFiles(m_vecFiles);

    if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
    }
}

OpenJob::OpenJob(const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_stEntry(stEntry)
    , m_strTempExtractPath(strTempExtractPath)
    , m_strProgram(strProgram)
{
    m_eJobType = JT_Open;
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &OpenJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalQuery, this, &SingleJob::signalQuery, Qt::ConnectionType::AutoConnection);
}

OpenJob::~OpenJob()
{

}

void OpenJob::doWork()
{
    // 构建解压参数
    ExtractionOptions options;
    options.strTargetPath = m_strTempExtractPath;
    // 当作提取，去除父目录
    if (m_stEntry.strFullPath.contains(QDir::separator())) {
        int iIndex = m_stEntry.strFullPath.lastIndexOf(QDir::separator());
        options.strDestination = m_stEntry.strFullPath.left(iIndex); // 当前路径截掉最后一级目录(不保留'/')
    }
    options.qSize = m_stEntry.qSize;

    PluginFinishType eType = m_pInterface->extractFiles(QList<FileEntry>() << m_stEntry, options);

    if (!(m_pInterface->waitForFinished())) {
        slotFinished(eType);
    }
}

void OpenJob::slotFinished(PluginFinishType eType)
{
    SingleJob::slotFinished(eType);

    if (eType == PFT_Nomral) {
        KProcess *cmdprocess = new KProcess;
        QStringList arguments;
        arguments << m_strTempExtractPath + QDir::separator() + m_stEntry.strFileName;
        QString programPath = OpenWithDialog::getProgramPathByExec(m_strProgram);
        cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
        cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
        cmdprocess->setProgram(programPath, arguments);
        cmdprocess->start();
        cmdprocess->waitForFinished();
        delete  cmdprocess;
    }

}

UpdateJob::UpdateJob(const UpdateOptions &options, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_stOptions(options)
{
    m_eJobType = JT_Update;
    //connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &UpdateJob::slotFinished, Qt::ConnectionType::UniqueConnection);
}

UpdateJob::~UpdateJob()
{

}

void UpdateJob::start()
{
    jobTimer.start();

    // 若插件指针为空，立即异常退出
    if (m_pInterface == nullptr) {
        slotFinished(PFT_Error);
        return;
    }

    d->start(); // 开启线程，执行操作
}

void UpdateJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface) {
        // 调用更新函数
        PluginFinishType eType = pWriteInterface->updateArchiveData(m_stOptions);

        //if (!(pWriteInterface->waitForFinished())) {
        slotFinished(eType);
        //}
    }
}

CommentJob::CommentJob(const QString &strComment, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_strComment(strComment)
{
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &CommentJob::slotFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalprogress, this, &CommentJob::signalprogress, Qt::ConnectionType::UniqueConnection);
}

CommentJob::~CommentJob()
{
    if (m_pInterface) {
        delete  m_pInterface;
        m_pInterface = nullptr;
    }
}

void CommentJob::doWork()
{
    qInfo() << "Adding comment";

    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface) {
        PluginFinishType eType = pWriteInterface->addComment(m_strComment);
        if (!(pWriteInterface->waitForFinished())) {
            slotFinished(eType);
        }
    }
}

ConvertJob::ConvertJob(const QString strOriginalArchiveFullPath, const QString strTargetFullPath, QObject *parent)
    : ArchiveJob(parent)
    , m_strOriginalArchiveFullPath(strOriginalArchiveFullPath)
    , m_strTargetFullPath(strTargetFullPath)
{

}

ConvertJob::~ConvertJob()
{

}

void ConvertJob::start()
{
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(m_strOriginalArchiveFullPath);

    if (pIface) {
        // 创建解压参数
        QFileInfo file(m_strOriginalArchiveFullPath);
        ExtractionOptions stOptions;
        stOptions.strTargetPath = file.filePath();
        stOptions.qComressSize = file.size();
        stOptions.bAllExtract = true;
        // 创建解压操作
        m_pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions);
        connect(m_pExtractJob, &ExtractJob::signalprogress, this, &ConvertJob::slotHandleSingleJobProgress);
        connect(m_pExtractJob, &ExtractJob::signalCurFileName, this, &ConvertJob::slotHandleSingleJobCurFileName);
        connect(m_pExtractJob, &ExtractJob::signalQuery, this, &ConvertJob::signalQuery);
        connect(m_pExtractJob, &ExtractJob::signalJobFinshed, this, &ConvertJob::slotHandleExtractFinished);

        m_pExtractJob->doWork();
    }

}

void ConvertJob::slotHandleSingleJobProgress(double dPercentage)
{

}

void ConvertJob::slotHandleSingleJobCurFileName(const QString &strName)
{

}

void ConvertJob::slotHandleExtractFinished()
{
    // 解压结束
    if (m_pExtractJob) {
        m_eFinishedType = m_pExtractJob->m_eFinishedType;
        m_eErrorType = m_pExtractJob->m_eErrorType;

        switch (m_eFinishedType) {
        // 正常结束之后，进行压缩操作
        case PFT_Nomral: {
        }
        break;
        // 用户取消之后，不进行压缩
        case PFT_Cancel: {
        }
        break;
        // 出现错误的情况，提示用户
        case PFT_Error: {
        }
        break;
        }
    }
}
