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
#include "archivemanager.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "singlejob.h"
#include "batchjob.h"
#include "uitools.h"

#include <QMimeDatabase>
#include <QFileInfo>
#include <QDebug>

//静态成员变量初始化。
QMutex ArchiveManager::m_mutex;//一个线程可以多次锁同一个互斥量
QAtomicPointer<ArchiveManager> ArchiveManager::m_instance = nullptr;//原子指针，默认初始化是0

ArchiveManager::ArchiveManager(QObject *parent)
    : QObject(parent)
{

}

ArchiveManager::~ArchiveManager()
{
    if (m_pArchiveJob != nullptr) {
        delete m_pArchiveJob;
        m_pArchiveJob = nullptr;
    }

    if (m_pInterface != nullptr) {
        delete m_pInterface;
        m_pInterface = nullptr;
    }

}

ArchiveManager *ArchiveManager::get_instance()
{
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<ArchiveManager>::isTestAndSetNative()) //运行时检测
        qDebug() << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (m_instance.testAndSetOrdered(nullptr, nullptr)) { //第一次检测
        QMutexLocker locker(&m_mutex);//加互斥锁。

        m_instance.testAndSetOrdered(nullptr, new ArchiveManager);//第二次检测。
    }

    return m_instance;
}

ArchiveJob *ArchiveManager::archiveJob()
{
    return m_pArchiveJob;
}

void ArchiveManager::createArchive(const QList<FileEntry> &files, const QString &strDestination, const CompressOptions &stOptions, bool useLibArchive/*, bool bBatch*/)
{
    ReadOnlyArchiveInterface *pInterface = UiTools::createInterface(strDestination, true, useLibArchive);

//    if (bBatch) {       // 批量压缩（多路径）
//        CreateJob *pCreateJob = new CreateJob(files, pInterface, options, this);

//        // 连接槽函数
//        connect(pCreateJob, &CreateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
//        connect(pCreateJob, &CreateJob::signalprogress, this, &ArchiveManager::signalprogress);
//        connect(pCreateJob, &CreateJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);


//        m_pArchiveJob = pCreateJob;
//        pCreateJob->start();
//    } else {            // 单路径压缩
    CreateJob *pCreateJob = new CreateJob(files, pInterface, stOptions, this);

    // 连接槽函数
    connect(pCreateJob, &CreateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pCreateJob, &CreateJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pCreateJob, &CreateJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);

    m_pArchiveJob = pCreateJob;
    pCreateJob->start();
    //}
}

void ArchiveManager::loadArchive(const QString &strArchiveName)
{
    m_pInterface = UiTools::createInterface(strArchiveName);

    LoadJob *pLoadJob = new LoadJob(m_pInterface);

    // 连接槽函数
    connect(pLoadJob, &LoadJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pLoadJob, &LoadJob::signalQuery, this, &ArchiveManager::signalQuery);

    m_pArchiveJob = pLoadJob;
    pLoadJob->start();
}

void ArchiveManager::extractFiles(const QString &strArchiveFullPath, const QList<FileEntry> &files, const ExtractionOptions &stOptions)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    ExtractJob *pExtractJob = new ExtractJob(files, m_pInterface, stOptions);

    // 连接槽函数
    connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);


    m_pArchiveJob = pExtractJob;
    pExtractJob->start();
}

void ArchiveManager::extractFiles2Path(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    ExtractJob *pExtractJob = new ExtractJob(listSelEntry, m_pInterface, stOptions);

    // 连接槽函数
    connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);


    m_pArchiveJob = pExtractJob;
    pExtractJob->start();
}

void ArchiveManager::deleteFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    DeleteJob *pDeleteJob = new DeleteJob(listSelEntry, m_pInterface);

    // 连接槽函数
    connect(pDeleteJob, &DeleteJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pDeleteJob, &DeleteJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pDeleteJob, &DeleteJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pDeleteJob, &DeleteJob::signalQuery, this, &ArchiveManager::signalQuery);

    m_pArchiveJob = pDeleteJob;
    pDeleteJob->start();
}

void ArchiveManager::batchExtractFiles(const QStringList &listFiles, const QString &strTargetPath, bool bAutoCreatDir)
{
    BatchExtractJob *pBatchExtractJob = new BatchExtractJob();
    pBatchExtractJob->setExtractPath(strTargetPath, bAutoCreatDir);
    pBatchExtractJob->setArchiveFiles(listFiles);

    // 连接槽函数
    connect(pBatchExtractJob, &BatchExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pBatchExtractJob, &BatchExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pBatchExtractJob, &BatchExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pBatchExtractJob, &BatchExtractJob::signalQuery, this, &ArchiveManager::signalQuery);
    connect(pBatchExtractJob, &BatchExtractJob::signalCurArchiveName, this, &ArchiveManager::signalCurArchiveName);

    m_pArchiveJob = pBatchExtractJob;
    pBatchExtractJob->start();
}

void ArchiveManager::openFile(const QString &strArchiveFullPath, const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveFullPath);
    }

    OpenJob *pOpenJob = nullptr;

    pOpenJob = new OpenJob(stEntry, strTempExtractPath, strProgram, m_pInterface);

    // 连接槽函数
    connect(pOpenJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pOpenJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);


    m_pArchiveJob = pOpenJob;
    pOpenJob->start();
}

void ArchiveManager::pauseOperation()
{
    // 调用job暂停接口
    if (m_pArchiveJob) {
        m_pArchiveJob->doPause();
    }
}

void ArchiveManager::continueOperation()
{
    // 调用job继续接口
    if (m_pArchiveJob) {
        m_pArchiveJob->doContinue();
    }
}

void ArchiveManager::cancelOperation()
{
    // 调用job取消接口
    if (m_pArchiveJob) {
        m_pArchiveJob->kill();
        m_pArchiveJob = nullptr;
    }
}

void ArchiveManager::slotJobFinished()
{
    if (m_pArchiveJob != nullptr && m_pInterface != nullptr) {
        // 如果是追加压缩或者删除压缩包数据，需要再次刷新数据
        if (m_pArchiveJob->m_eJobType == ArchiveJob::JT_Add || m_pArchiveJob->m_eJobType == ArchiveJob::JT_Delete) {
            m_pInterface->updateArchiveData();
        }
    }

    // 发送结束信号
    emit signalJobFinished();

    // 释放job
    if (m_pArchiveJob != nullptr) {
        m_pArchiveJob->deleteLater();
        m_pArchiveJob = nullptr;
    }
}
