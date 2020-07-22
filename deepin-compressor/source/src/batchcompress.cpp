/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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

#include "batchcompress.h"
#include "jobs.h"
#include "queries.h"
#include "utils.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QTimer>

BatchCompress::BatchCompress(QObject *parent): BatchJobs(parent)
{
    mType = Job::ENUM_JOBTYPE::BATCHCOMPRESSJOB;
    setCapabilities(KJob::Killable);
    if (m_Args != nullptr) {
        delete m_Args;
        m_Args = nullptr;
    }
    m_Args = new QMap<QString, QString>();
//    connect(this, &KJob::result, this, &BatchExtract::showFailedFiles);
}

void BatchCompress::setCompressArgs(QMap<QString, QString> Args)
{
//    if(m_Args!= nullptr){
//        m_Args->clear();
//        delete m_Args;
//        m_Args = nullptr;
//    }
    m_Args = new QMap<QString, QString>();

    if (m_Args->count() > 0) {
        m_Args->clear();
    }

    QMap<QString, QString>::iterator it = Args.begin();
    while (it != Args.end()) {
        m_Args->insert(it.key(), it.value());
        it++;
    }

    m_bZipPasswordIsChinese = Utils::zipPasswordIsChinese(*m_Args);
}

void BatchCompress::addCompress(const QStringList &files)
{
    Q_ASSERT(m_Args);
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*m_Args)[QStringLiteral("fixedMimeType")];
    const QString password = (*m_Args)[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*m_Args)[QStringLiteral("encryptHeader")];
    QString filename = (*m_Args)[QStringLiteral("localFilePath")] + QDir::separator() + (*m_Args)[QStringLiteral("filename")];

    if (filename.isEmpty()) {
        qDebug() << "filename.isEmpty()";
        return;
    }

    int num = 2;
    while (QFileInfo::exists(filename)) {
        filename = filename.remove("." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix()) + "(" + "0" + QString::number(num) + ")" + "." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();
        num++;
    }
    qDebug() << filename;


    CompressionOptions options;
    options.setCompressionLevel((*m_Args)[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*m_Args)[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*m_Args)[QStringLiteral("volumeSize")].toULongLong());


    QVector<Archive::Entry *> all_entries;

    foreach (QString file, filesToAdd) {
        Archive::Entry *entry = new Archive::Entry();
        entry->setFullPath(file);
        all_entries.append(entry);
    }


    if (all_entries.isEmpty()) {
        qDebug() << "all_entries.isEmpty()";
        return;
    }

    QString globalWorkDir = filesToAdd.first();
    if (globalWorkDir.right(1) == QLatin1String("/")) {
        globalWorkDir.chop(1);
    }
    globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
    options.setGlobalWorkDir(globalWorkDir);

    auto job = Archive::create(filename, fixedMimeType, all_entries, options, this, false, !m_bZipPasswordIsChinese);
    addSubjob(job);

    if (!password.isEmpty()) {
        job->enableEncryption(password, enableHeaderEncryption.compare("true") ? false : true);
    }

    connect(job, SIGNAL(percent(KJob *, ulong)),
            this, SLOT(forwardProgress(KJob *, ulong)));
    connect(job, SIGNAL(percentfilename(KJob *, const QString &)),
            this, SLOT(SlotProgressFile(KJob *, const QString &)));

//    connect(job, &KJob::result, this, &BatchCompress::SlotCreateJobFinished);
//    connect(job, &KJob::result, this, &BatchCompress::SlotCreateJobFinished, Qt::ConnectionType::UniqueConnection);

}

void BatchCompress::SlotProgressFile(KJob *job, const QString &name)
{
    emit batchFilenameProgress(job, name);
}

void BatchCompress::clearSubjobs()
{
    Q_FOREACH (KJob *job, subjobs()) {
        job->setParent(nullptr);
        disconnect(job, &KJob::result, this, &BatchJobs::slotResult);
        disconnect(job, &KJob::infoMessage, this, &BatchJobs::slotInfoMessage);
        disconnect(job, SIGNAL(percent(KJob *, ulong)), this, SLOT(forwardProgress(KJob *, ulong)));
        disconnect(job, SIGNAL(percentfilename(KJob *, const QString &)), this, SLOT(SlotProgressFile(KJob *, const QString &)));
    }
    BatchJobs::clearSubjobs();
}

bool BatchCompress::doKill()
{
    if (subjobs().isEmpty()) {
        return false;
    }
    KJob *pCurJob = subjobs().first();
    this->clearSubjobs();
    if (pCurJob) {
        return pCurJob->kill();
    } else {
        return false;
    }
}

void BatchCompress::start()
{
    QTimer::singleShot(0, this, &BatchCompress::slotStartJob);
}

void BatchCompress::slotStartJob()
{
    if (m_inputs.isEmpty()) {
        emitResult();
        return;
    }

    //Firstly,clear subjobs
    this->clearSubjobs();
    for (const auto &url : qAsConst(m_inputs)) {
        addCompress(url);
    }


    m_initialJobCount = subjobs().size();

    qDebug() << "Starting first job";

    subjobs().at(0)->start();
}

void BatchCompress::slotResult(KJob *job)
{
    if (job->error()) {
        qDebug() << "There was en error:" << job->error() << ", errorText:" << job->errorString();

//        QString curfile = m_fileNames[subjobs().at(0)].first;
//        qDebug() << "Fail curfilename"<<curfile;
//        QFileInfo file(curfile);
        while (hasSubjobs()) {
            removeSubjob(job);
        }
        removeSubjob(job);

        return;
    }

    removeSubjob(job);

    if (!hasSubjobs()) {
        qDebug() << "Finished, emitting the result";
        emitResult();
    } else {
        qDebug() << "Starting the next job";
        subjobs().at(0)->start();
//        QString curfile = m_fileNames[subjobs().at(0)].first;
//        qDebug() << "Send curfilename"<<curfile;
//        QFileInfo file(curfile);
//        emit sendCurFile(file.fileName());
    }
}

void BatchCompress::forwardProgress(KJob *job, unsigned long percent)
{
    Q_UNUSED(job)
    //qDebug() << percent;
    auto jobPart = static_cast<ulong>(100 / m_initialJobCount);
    auto remainingJobs = static_cast<ulong>(m_initialJobCount - subjobs().size());
    if (percent >= 100) {
        percent = 100;
    }
    emit batchProgress(job, jobPart * remainingJobs + percent / static_cast<ulong>(m_initialJobCount));
}

void BatchCompress::addInput(const QStringList &files)
{
    qDebug() << "Adding archive" << files;

    m_inputs.append(files);
}

void BatchCompress::SlotCreateJobFinished(KJob *job)
{
    qDebug() << "job finished" << job->error();

    if (job->error() && (job->error() != KJob::KilledJobError)) {
//        if (m_pathstore.left(6) == "/media") {
//            if (getMediaFreeSpace() <= 50) {
//                m_CompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
//            } else {
//                m_CompressFail->setFailStrDetail(tr("Damaged file"));
//            }
//        } else {
//            if (getDiskFreeSpace() <= 50) {
//                m_CompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
//            } else {
//                m_CompressFail->setFailStrDetail(tr("Damaged file"));
//            }
//        }
//        m_pageid = PAGE_ZIP_FAIL;
//        refreshPage();
//        return;
    }

//    createCompressFile_.clear();
//    m_pageid = PAGE_ZIP_SUCCESS;
//    refreshPage();

    if (job) {
        job->deleteLater();
        job = nullptr;
    }
}
