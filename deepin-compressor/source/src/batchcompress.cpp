/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
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

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QPointer>
#include <QTimer>
#include <QUrl>

BatchCompress::BatchCompress(QObject *parent)
{
    setCapabilities(KJob::Killable);

//    connect(this, &KJob::result, this, &BatchExtract::showFailedFiles);
}

BatchCompress::~BatchCompress()
{
}

void BatchCompress::setCompressArgs(const QMap<QString, QString> &Args)
{
    m_Args = Args;
}

void BatchCompress::addCompress(const QStringList &files)
{
    const QStringList filesToAdd = files;
    const QString fixedMimeType = m_Args[QStringLiteral("fixedMimeType")];
    const QString password = m_Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = m_Args[QStringLiteral("encryptHeader")];
    QString filename = m_Args[QStringLiteral("localFilePath")] + QDir::separator() + m_Args[QStringLiteral("filename")];

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
    options.setCompressionLevel(m_Args[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod(m_Args[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize(m_Args[QStringLiteral("volumeSize")].toULongLong());


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

    auto job = Archive::create(filename, fixedMimeType, all_entries, options, this);
    addSubjob(job);

    if (!password.isEmpty()) {
        job->enableEncryption(password, enableHeaderEncryption.compare("true") ? false : true);
    }

    connect(job, SIGNAL(percent(KJob *, ulong)),
            this, SLOT(forwardProgress(KJob *, ulong)));
    connect(job, SIGNAL(percentfilename(KJob *, const QString &)),
            this, SLOT(SlotProgressFile(KJob *, const QString &)));




}

void BatchCompress::SlotProgressFile(KJob *job, const QString &name)
{
    emit batchFilenameProgress(job, name);
}

bool BatchCompress::doKill()
{
    if (subjobs().isEmpty()) {
        return false;
    }

    return subjobs().first()->kill();
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

        QString curfile = m_fileNames[subjobs().at(0)].first;
        qDebug() << "Fail curfilename"<<curfile;
        QFileInfo file(curfile);
//        while (hasSubjobs()) {
//            removeSubjob(job);
//        }
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
        QString curfile = m_fileNames[subjobs().at(0)].first;
        qDebug() << "Send curfilename"<<curfile;
        QFileInfo file(curfile);
        emit sendCurFile(file.fileName());
    }
}

void BatchCompress::forwardProgress(KJob *job, unsigned long percent)
{
    Q_UNUSED(job)
    qDebug() << percent;
    auto jobPart = static_cast<ulong>(100 / m_initialJobCount);
    auto remainingJobs = static_cast<ulong>(m_initialJobCount - subjobs().size());

    emit batchProgress(job, jobPart * remainingJobs + percent / static_cast<ulong>(m_initialJobCount));
}

void BatchCompress::addInput(const QStringList &files)
{
    qDebug() << "Adding archive" << files;

    m_inputs.append(files);
}



