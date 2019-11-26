/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2008 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2009-2010 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "batchextract.h"
#include "jobs.h"
#include "queries.h"

#include <QDir>
#include <QFileInfo>
#include <QPointer>
#include <QTimer>
#include <QUrl>

BatchExtract::BatchExtract(QObject *parent)
    : m_autoSubfolder(false),
      m_preservePaths(true),
      m_openDestinationAfterExtraction(false)
{
    setCapabilities(KJob::Killable);

    connect(this, &KJob::result, this, &BatchExtract::showFailedFiles);
}

BatchExtract::~BatchExtract()
{
}

void BatchExtract::addExtraction(const QUrl &url)
{

    QString destination = destinationFolder();

    auto job = Archive::batchExtract(url.toLocalFile(), destination, autoSubfolder(), preservePaths());

    qDebug() << QString(QStringLiteral("Registering job from archive %1, to %2, preservePaths %3")).arg(url.toLocalFile(), destination, QString::number(preservePaths()));

    addSubjob(job);

    m_fileNames[job] = qMakePair(url.toLocalFile(), destination);

    connect(job, SIGNAL(percent(KJob *, ulong)),
            this, SLOT(forwardProgress(KJob *, ulong)));
    connect(job, &BatchExtractJob::userQuery,
            this, &BatchExtract::slotUserQuery);
    connect(job, SIGNAL(percentfilename(KJob *, const QString &)),
            this, SLOT(SlotProgressFile(KJob *, const QString &)));
    connect(job, &BatchExtractJob::signeedpassword,
            this, [=]
    {
        qDebug()<<"need password";
    });
}

void BatchExtract::SlotProgressFile(KJob *job, const QString &name)
{
    emit batchFilenameProgress(job, name);
}

bool BatchExtract::doKill()
{
    if (subjobs().isEmpty()) {
        return false;
    }

    return subjobs().first()->kill();
}

void BatchExtract::slotUserQuery(Query *query)
{
    query->execute();
}

bool BatchExtract::autoSubfolder() const
{
    return m_autoSubfolder;
}

void BatchExtract::setAutoSubfolder(bool value)
{
    m_autoSubfolder = value;
}

void BatchExtract::start()
{
    QTimer::singleShot(0, this, &BatchExtract::slotStartJob);
}

void BatchExtract::slotStartJob()
{
    if (m_inputs.isEmpty()) {
        emitResult();
        return;
    }

    for (const auto &url : qAsConst(m_inputs)) {
        addExtraction(url);
    }


    m_initialJobCount = subjobs().size();

    qDebug() << "Starting first job";

    subjobs().at(0)->start();
}

void BatchExtract::showFailedFiles()
{
    if (!m_failedFiles.isEmpty()) {
//        KMessageBox::informationList(nullptr, i18n("The following files could not be extracted:"), m_failedFiles);
    }
}

void BatchExtract::slotResult(KJob *job)
{
    if (job->error()) {
        qDebug() << "There was en error:" << job->error() << ", errorText:" << job->errorString();

        QString curfile = m_fileNames[subjobs().at(0)].first;
        qDebug() << "Fail curfilename"<<curfile;
        QFileInfo file(curfile);
//        while (hasSubjobs()) {
//            removeSubjob(job);
//        }
        removeSubjob(job);
        emit sendFailFile(file.fileName());

        return;
    }

    removeSubjob(job);

    if (!hasSubjobs()) {
        qDebug() << "Finished, emitting the result";
        emitResult();
    } else {
        qDebug() << "Starting the next job";
        subjobs().at(0)->start();
        QString curfile = m_fileNames[subjobs().at(0)].first;
        qDebug() << "Send curfilename"<<curfile;
        QFileInfo file(curfile);
        emit sendCurFile(file.fileName());
    }
}

void BatchExtract::forwardProgress(KJob *job, unsigned long percent)
{
    Q_UNUSED(job)
    qDebug() << percent;
    auto jobPart = static_cast<ulong>(100 / m_initialJobCount);
    auto remainingJobs = static_cast<ulong>(m_initialJobCount - subjobs().size());

    emit batchProgress(job, jobPart * remainingJobs + percent / static_cast<ulong>(m_initialJobCount));
}

void BatchExtract::addInput(const QUrl &url)
{
    qDebug() << "Adding archive" << url.toLocalFile();

    if (!QFileInfo::exists(url.toLocalFile())) {
        m_failedFiles.append(url.fileName());
        return;
    }

    m_inputs.append(url);
}

bool BatchExtract::openDestinationAfterExtraction() const
{
    return m_openDestinationAfterExtraction;
}

bool BatchExtract::preservePaths() const
{
    return m_preservePaths;
}

QString BatchExtract::destinationFolder() const
{
    qDebug()<<m_destinationFolder;
    if (m_destinationFolder.isEmpty()) {
        return QDir::currentPath();
    } else {
        return m_destinationFolder;
    }
}

void BatchExtract::setDestinationFolder(const QString &folder)
{
    if (QFileInfo(folder).isDir()) {
        m_destinationFolder = folder;
    }
}

void BatchExtract::setOpenDestinationAfterExtraction(bool value)
{
    m_openDestinationAfterExtraction = value;
}

void BatchExtract::setPreservePaths(bool value)
{
    m_preservePaths = value;
}

bool BatchExtract::showExtractDialog()
{

    return true;
}

