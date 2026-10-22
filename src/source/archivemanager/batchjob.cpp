// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "batchjob.h"
#include "uitools.h"
#include "singlejob.h"
#include "common/ddlog.h"

#include <QDir>
#include <QDebug>

BatchJob::BatchJob(QObject *parent)
    : ArchiveJob(parent)
{
    qCDebug(appLog) << "BatchJob instance created";
}

BatchJob::~BatchJob()
{
    qCDebug(appLog) << "BatchJob instance destroyed";
    clearSubjobs();
}

bool BatchJob::addSubjob(ArchiveJob *job)
{
    qCDebug(appLog) << "Adding subjob to BatchJob";
    if (nullptr == job || m_listSubjobs.contains(job)) {
        qCWarning(appLog) << "Cannot add subjob - null job or already exists";
        return false;
    }

    job->setParent(this);
    m_listSubjobs.append(job);
    qCDebug(appLog) << "Subjob added successfully, total subjobs:" << m_listSubjobs.count();
    return true;
}

bool BatchJob::removeSubjob(ArchiveJob *job)
{
    qCDebug(appLog) << "Removing subjob from BatchJob";
    if (m_listSubjobs.removeAll(job) > 0) {
        job->setParent(nullptr);
        delete job;
        qCDebug(appLog) << "Subjob removed successfully, remaining subjobs:" << m_listSubjobs.count();
        return true;
    }

    qCWarning(appLog) << "Failed to remove subjob - not found";
    return false;
}

bool BatchJob::hasSubjobs() const
{
    qCDebug(appLog) << "Checking if BatchJob has subjobs, count:" << m_listSubjobs.count();
    return !m_listSubjobs.isEmpty();
}

const QList<ArchiveJob *> &BatchJob::subjobs() const
{
    // qCDebug(appLog) << "Getting subjobs list, count:" << m_listSubjobs.count();
    return m_listSubjobs;
}

void BatchJob::clearSubjobs()
{
    qCDebug(appLog) << "Clearing all subjobs, count:" << m_listSubjobs.count();
    Q_FOREACH (ArchiveJob *job, m_listSubjobs) {
        job->setParent(nullptr);
        delete job;
    }

    m_listSubjobs.clear();
    qCDebug(appLog) << "All subjobs cleared";
}

void BatchJob::doPause()
{
    qCDebug(appLog) << "Pausing BatchJob";
    // 调用子job暂停接口
    if (m_pCurJob) {
        qCDebug(appLog) << "Pausing current subjob";
        m_pCurJob->doPause();
    } else {
        qCWarning(appLog) << "No current subjob to pause";
    }
}

void BatchJob::doContinue()
{
    qCDebug(appLog) << "Continuing BatchJob";
    // 调用子job继续接口
    if (m_pCurJob) {
        qCDebug(appLog) << "Continuing current subjob";
        m_pCurJob->doContinue();
    } else {
        qCWarning(appLog) << "No current subjob to continue";
    }
}

bool BatchJob::status()
{
    qCDebug(appLog) << "Getting BatchJob status";
    // 调用子job继续接口
    if (m_pCurJob) {
        return m_pCurJob->status();
    }

    qCWarning(appLog) << "No current subjob to get status";
    return false;
}

BatchExtractJob::BatchExtractJob(QObject *parent)
    : BatchJob(parent)
{
    qCDebug(appLog) << "BatchExtractJob instance created";
    m_eJobType = JT_BatchExtract;
}

BatchExtractJob::~BatchExtractJob()
{
    // qCDebug(appLog) << "BatchExtractJob instance destroyed";
}

void BatchExtractJob::start()
{
    qCDebug(appLog) << "Starting batch extraction with" << subjobs().count() << "jobs";
    if (subjobs().count() == 0) {
        qCWarning(appLog) << "No subjobs to start";
        return;
    }

    m_iCurArchiveIndex = 0;
    m_pCurJob = subjobs().at(0);
    qCDebug(appLog) << "Starting first subjob";
    m_pCurJob->start();
}

void BatchExtractJob::setExtractPath(const QString &strPath/*, bool bAutoCreatDir*/)
{
    qCDebug(appLog) << "Setting extract path to:" << strPath;
    m_strExtractPath = strPath;
//    m_bAutoCreatDir = bAutoCreatDir;
}

bool BatchExtractJob::setArchiveFiles(const QStringList &listFile)
{
    qCDebug(appLog) << "Setting archive files list, count:" << listFile.count();
    m_listFiles = listFile;
    m_qBatchTotalSize = 0;
    m_iArchiveCount = m_listFiles.count();

    bool bResult = false;
    // 创建解压元素
    foreach (QString strFileName, listFile) {
        QFileInfo fileInfo(strFileName);
        qCDebug(appLog) << "Processing archive file:" << fileInfo.fileName();
        if (addExtractItem(fileInfo)) {
            bResult = true;
            // 计算压缩包总大小
            m_qBatchTotalSize += fileInfo.size();
            qCDebug(appLog) << "Added extract item for:" << fileInfo.fileName();
        } else {
            qCWarning(appLog) << "Failed to add extract item for:" << fileInfo.fileName();
        }
    }

    qCDebug(appLog) << "Total batch size:" << m_qBatchTotalSize << "bytes";
    return bResult;
}

bool BatchExtractJob::addExtractItem(const QFileInfo &fileInfo)
{
    qCDebug(appLog) << "Adding extract item for file:" << fileInfo.fileName();
    QString strName = fileInfo.filePath();
    UnCompressParameter::SplitType eType = UnCompressParameter::ST_No;
    UiTools::transSplitFileName(strName, eType);
    UiTools::AssignPluginType ePluginType = (UnCompressParameter::ST_Zip == eType) ?
                                            (UiTools::AssignPluginType::APT_Cli7z) : (UiTools::AssignPluginType::APT_Auto);
    qCDebug(appLog) << "Creating interface for extract item";
    ReadOnlyArchiveInterface *pIface = UiTools::createInterface(fileInfo.filePath(), false, ePluginType);

    if (pIface) {
        qCDebug(appLog) << "Interface created successfully, setting up extraction options";
        // 创建解压参数
        ExtractionOptions stOptions;
        stOptions.strTargetPath = m_strExtractPath;

        // 自动创建文件夹的名称（去除中间带.的文件夹名称）
        QString strpath = "";
        strpath = UiTools::handleFileName(fileInfo.filePath());
//        if (fileInfo.filePath().contains(".tar.")) {
//            strpath = strpath.remove(".tar"); // 类似tar.gz压缩文件，创建文件夹的时候移除.tar
//        } else if (fileInfo.filePath().contains(".7z.")) {
//            strpath = strpath.remove(".7z"); // 7z分卷文件，创建文件夹的时候移除.7z
//        } else if (fileInfo.filePath().contains(".part01.rar")) {
//            strpath = strpath.remove(".part01"); // tar分卷文件，创建文件夹的时候移除part01
//        } else if (fileInfo.filePath().contains(".part1.rar")) {
//            strpath = strpath.remove(".part1"); // rar分卷文件，创建文件夹的时候移除.part1
//        } else if (fileInfo.filePath().contains(".zip.")) {
//            strpath = strpath.remove(".zip"); // zip分卷文件，创建文件夹的时候移除.zip
//        }


        stOptions.strTargetPath += QDir::separator() + strpath; // 批量解压自动创建文件夹
        stOptions.qComressSize = fileInfo.size();
        stOptions.bExistList = false;
        stOptions.bAllExtract = true;
        stOptions.bBatchExtract = true;
        // tar.7z特殊处理，右键解压缩到当前文件夹使用cli7zplugin
        if (strName.endsWith(QLatin1String(".tar.7z"))
                && determineMimeType(strName).name() == QLatin1String("application/x-7z-compressed")) {
            qCDebug(appLog) << "Detected tar.7z archive, using special handling";
            stOptions.bTar_7z = true;
            ePluginType = UiTools::AssignPluginType::APT_Cli7z;
        }

        pIface->setParent(this);    // 跟随BatchExtractJob释放
        if (!stOptions.bTar_7z) {
            qCDebug(appLog) << "Creating ExtractJob for regular archive";
            ExtractJob *pExtractJob = new ExtractJob(QList<FileEntry>(), pIface, stOptions);
            connect(pExtractJob, &ExtractJob::signalprogress, this, &BatchExtractJob::slotHandleSingleJobProgress);
            connect(pExtractJob, &ExtractJob::signalCurFileName, this, &BatchExtractJob::slotHandleSingleJobCurFileName);
            connect(pExtractJob, &ExtractJob::signalQuery, this, &BatchExtractJob::signalQuery);
            connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &BatchExtractJob::slotHandleSingleJobFinished);
            addSubjob(pExtractJob);
        } else {
            qCDebug(appLog) << "Creating StepExtractJob for tar.7z archive";
            StepExtractJob *pStepExtractJob = new StepExtractJob(fileInfo.absoluteFilePath(), stOptions);
            connect(pStepExtractJob, &StepExtractJob::signalprogress, this, &BatchExtractJob::slotHandleSingleJobProgress);
            connect(pStepExtractJob, &StepExtractJob::signalCurFileName, this, &BatchExtractJob::slotHandleSingleJobCurFileName);
            connect(pStepExtractJob, &StepExtractJob::signalQuery, this, &BatchExtractJob::signalQuery);
            connect(pStepExtractJob, &StepExtractJob::signalJobFinshed, this, &BatchExtractJob::slotHandleSingleJobFinished);
            addSubjob(pStepExtractJob);
        }
        qCDebug(appLog) << "Extract item added successfully";
        return true;
    }

    qCWarning(appLog) << "Failed to create interface for extract item";
    return false;
}

void BatchExtractJob::slotHandleSingleJobProgress(double dPercentage)
{
    QFileInfo fileInfo(m_listFiles[m_iCurArchiveIndex]);
    qint64 qCurSize = fileInfo.size();      // 当前解压的压缩包大小
    double dProgress = double(qCurSize) * dPercentage /  m_qBatchTotalSize + m_dLastPercentage;
    qCDebug(appLog) << "Batch job progress:" << dProgress << "%, current file:" << fileInfo.fileName();
    emit signalprogress(dProgress);
}

void BatchExtractJob::slotHandleSingleJobCurFileName(const QString &strName)
{
    qCDebug(appLog) << "Current file being processed:" << strName;
    emit signalCurFileName(strName);
}

void BatchExtractJob::slotHandleSingleJobFinished()
{
    qCDebug(appLog) << "Single job finished handler called";
    if (m_pCurJob != nullptr) {
        qCDebug(appLog) << "Single job finished - Type:" << m_pCurJob->m_eJobType
                << "Result:" << m_pCurJob->m_eFinishedType;

        if (PFT_Error == m_pCurJob->m_eFinishedType || PFT_Cancel == m_pCurJob->m_eFinishedType) {
            // 获取结束结果
            m_eJobType = m_pCurJob->m_eJobType;
            m_eFinishedType = m_pCurJob->m_eFinishedType;
            m_eErrorType = m_pCurJob->m_eErrorType;

            qCWarning(appLog) << "Job failed or cancelled - ErrorType:" << m_eErrorType;
            // 子job错误或取消，发送结束信号
            emit signalJobFinshed();
            return;
        }

        // 移除当前job
        qCDebug(appLog) << "Removing completed subjob";
        removeSubjob(m_pCurJob);

        if (!hasSubjobs()) {
            qCDebug(appLog) << "All subjobs completed";
            // 若没有子job，发送结束信号
            emit signalJobFinshed();
        } else {
            ++m_iCurArchiveIndex;
            QFileInfo curFileInfo(m_listFiles[m_iCurArchiveIndex]);

            qCDebug(appLog) << "Starting next subjob for:" << curFileInfo.fileName();
            // 还存在子job，执行子job操作

            m_dLastPercentage += double(curFileInfo.size()) / m_qBatchTotalSize * 100;
            emit signalCurArchiveName(curFileInfo.fileName());
            m_pCurJob = subjobs().at(0);
            m_pCurJob->start();
        }
    } else {
        qCWarning(appLog) << "No current job to handle";
    }
}
