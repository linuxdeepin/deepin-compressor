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
#include "batchjob.h"
#include "uitools.h"
#include "singlejob.h"

#include <QDir>
#include <QDebug>

BatchJob::BatchJob(QObject *parent)
    : ArchiveJob(parent)
{

}

BatchJob::~BatchJob()
{
    clearSubjobs();
}

bool BatchJob::addSubjob(ArchiveJob *job)
{
    if (job == nullptr || m_listSubjobs.contains(job)) {
        return false;
    }

    job->setParent(this);
    m_listSubjobs.append(job);
    return true;
}

bool BatchJob::removeSubjob(ArchiveJob *job)
{
    if (m_listSubjobs.removeAll(job) > 0) {
        job->setParent(nullptr);
        return true;
    }

    return false;
}

bool BatchJob::hasSubjobs() const
{
    return !m_listSubjobs.isEmpty();
}

const QList<ArchiveJob *> &BatchJob::subjobs() const
{
    return m_listSubjobs;
}

void BatchJob::clearSubjobs()
{
    Q_FOREACH (ArchiveJob *job, m_listSubjobs) {
        job->setParent(nullptr);
    }

    m_listSubjobs.clear();
}



BatchExtractJob::BatchExtractJob(QObject *parent)
    : BatchJob(parent)
{
    m_eJobType = JT_BatchExtract;
}

BatchExtractJob::~BatchExtractJob()
{

}

void BatchExtractJob::start()
{
    if (subjobs().count() == 0) {
        return;
    }

    m_iCurArchiveIndex = 0;
    m_pCurJob = subjobs().at(0);
    m_pCurJob->start();
}

void BatchExtractJob::setExtractPath(const QString &strPath, bool bAutoCreatDir)
{
    m_strExtractPath = strPath;
    m_bAutoCreatDir = bAutoCreatDir;
}

void BatchExtractJob::setArchiveFiles(const QStringList &listFile)
{
    m_listFiles = listFile;
    m_qBatchTotalSize = 0;
    m_iArchiveCount = m_listFiles.count();

    // 创建解压元素
    foreach (QString strFileName, listFile) {
        QFileInfo fileInfo(strFileName);
        addExtractItem(fileInfo);
        // 计算压缩包总大小
        m_qBatchTotalSize += fileInfo.size();
    }
}

void BatchExtractJob::addExtractItem(const QFileInfo &fileInfo)
{
    // 创建解压参数
    ExtractionOptions stOptions;
    stOptions.strTargetPath = fileInfo.path();
    if (m_bAutoCreatDir)
        stOptions.strTargetPath += QDir::separator() + fileInfo.completeBaseName();
    stOptions.qComressSize = fileInfo.size();
    stOptions.bRightExtract = true;
    stOptions.bAllExtract = true;
    stOptions.bBatchExtract = true;

    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(fileInfo.filePath());
    ExtractJob *pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions);
    connect(pExtractJob, &ExtractJob::signalprogress, this, &BatchExtractJob::slotHandleSingleJobProgress);
    connect(pExtractJob, &ExtractJob::signalCurFileName, this, &BatchExtractJob::slotHandleSingleJobCurFileName);
    connect(pExtractJob, &ExtractJob::signalQuery, this, &BatchExtractJob::signalQuery);
    connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &BatchExtractJob::slotHandleSingleJobFinished);

    addSubjob(pExtractJob);

}

void BatchExtractJob::slotHandleSingleJobProgress(double dPercentage)
{
    QFileInfo fileInfo(m_listFiles[m_iCurArchiveIndex]);
    qint64 qCurSize = fileInfo.size();      // 当前解压的压缩包大小
    double dProgress = double(qCurSize) * dPercentage /  m_qBatchTotalSize + m_dLastPercentage;
    emit signalprogress(dProgress);
}

void BatchExtractJob::slotHandleSingleJobCurFileName(const QString &strName)
{
    emit signalCurFileName(strName);
}

void BatchExtractJob::slotHandleSingleJobFinished()
{
    if (m_pCurJob != nullptr) {
        // 移除当前job
        removeSubjob(m_pCurJob);

        if (!hasSubjobs()) {
            // 若没有子job，发送结束信号
            emit signalJobFinshed();
        } else {
            ++m_iCurArchiveIndex;
            QFileInfo curFileInfo(m_listFiles[m_iCurArchiveIndex]);

            if (m_pCurJob->m_eFinishedType == PFT_Nomral) {
                // 正常结束
            } else if (m_pCurJob->m_eFinishedType == PFT_Cancel) {
                // 取消
            } else {
                // 错误
            }

            // 还存在子job，执行子job操作

            m_dLastPercentage += double(curFileInfo.size()) / m_qBatchTotalSize * 100;
            emit signalCurArchiveName(curFileInfo.fileName());
            m_pCurJob = subjobs().at(0);
            m_pCurJob->start();
        }
    }
}
