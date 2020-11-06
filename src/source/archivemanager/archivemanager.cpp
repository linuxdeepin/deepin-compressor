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

void ArchiveManager::getLoadArchiveData(ArchiveData &stArchiveData)
{
    if (m_pInterface != nullptr) {
        m_pInterface->getArchiveData(stArchiveData);
    }
}

void ArchiveManager::extractFiles(const QString &strArchiveName, const QList<FileEntry> &files, const ExtractionOptions &stOptions)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveName);
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

void ArchiveManager::extractFiles2Path(const QString &strArchiveName, const QList<FileEntry> &listCurEntry, const QList<FileEntry> &listAllEntry, const ExtractionOptions &stOptions)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveName);
    }

    ExtractJob *pExtractJob = nullptr;

    if (m_pInterface->getHandleCurEntry()) {
        // 部分插件需要传入第一层文件
        pExtractJob = new ExtractJob(listCurEntry, m_pInterface, stOptions);
    } else {
        // 部分插件需要传入所有文件
        pExtractJob = new ExtractJob(listAllEntry, m_pInterface, stOptions);
    }

    // 连接槽函数
    connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);


    m_pArchiveJob = pExtractJob;
    pExtractJob->start();
}

void ArchiveManager::deleteFiles(const QString &strArchiveName, const QList<FileEntry> &listCurEntry, const QList<FileEntry> &listAllEntry)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveName);
    }

    DeleteJob *pDeleteJob = nullptr;

    if (m_pInterface->getHandleCurEntry()) {
        // 部分插件需要传入第一层文件
        pDeleteJob = new DeleteJob(listCurEntry, m_pInterface);
    } else {
        // 部分插件需要传入所有文件
        pDeleteJob = new DeleteJob(listAllEntry, m_pInterface);
    }

    // 连接槽函数
    connect(pDeleteJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pDeleteJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pDeleteJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pDeleteJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);

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

void ArchiveManager::openFile(const QString &strArchiveName, const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram)
{
    if (m_pInterface == nullptr) {
        m_pInterface = UiTools::createInterface(strArchiveName);
    }

    OpenJob *pOpenJob = nullptr;

    pOpenJob = new OpenJob(stEntry, strTempExtractPath, strProgram, m_pInterface);

    // 连接槽函数
    connect(pOpenJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pOpenJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);


    m_pArchiveJob = pOpenJob;
    pOpenJob->start();
}

void ArchiveManager::slotJobFinished()
{
    emit signalJobFinished();

    if (m_pArchiveJob != nullptr) {
        m_pArchiveJob->deleteLater();
        m_pArchiveJob = nullptr;
    }
}
