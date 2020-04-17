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
#include "jobs.h"
#include "archiveentry.h"


#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>


class Job::Private : public QThread
{
    Q_OBJECT

public:
    Private(Job *job, QObject *parent = nullptr)
        : QThread(parent)
        , q(job)
    {
    }

    void run() override;

private:
    Job *q;
};

void Job::Private::run()
{
    q->doWork();
}

Job::Job(Archive *archive, ReadOnlyArchiveInterface *interface)
    : KJob()
    , m_archive(archive)
    , m_archiveInterface(interface)
    , d(new Private(this))
{
    setCapabilities(KJob::Killable);
}

Job::Job(Archive *archive)
    : Job(archive, nullptr)
{}

Job::Job(ReadOnlyArchiveInterface *interface)
    : Job(nullptr, interface)
{}

Job::~Job()
{
    if (d->isRunning()) {
        d->wait();
    }

    delete d;
}

ReadOnlyArchiveInterface *Job::archiveInterface()
{
    // Use the archive interface.
    if (archive()) {
        return archive()->interface();
    }

    // Use the interface passed to this job (e.g. JSONArchiveInterface in jobstest.cpp).
    return m_archiveInterface;
}

Archive *Job::archive() const
{
    return m_archive;
}

QString Job::errorString() const
{
    if (!errorText().isEmpty()) {
        return errorText();
    }

    if (archive()) {
        if (archive()->error() == NoPlugin) {
            //return tr("No suitable plugin found.");
            return ("No suitable plugin found.");
        }

        if (archive()->error() == FailedPlugin) {
            //return tr("Failed to load a suitable plugin.");
            return ("Failed to load a suitable plugin.");
        }
    }

    return QString();
}

void Job::start()
{
    jobTimer.start();

    // We have an archive but it's not valid, nothing to do.
    if (archive() && !archive()->isValid()) {
        QTimer::singleShot(0, this, [ = ]() {
            onFinished(false);
        });
        return;
    }

    if (archiveInterface()->waitForFinishedSignal()) {
        // CLI-based interfaces run a QProcess, no need to use threads.
        QTimer::singleShot(0, this, &Job::doWork);
    } else {
        // Run the job in another thread.
        d->start();
    }
}

void Job::connectToArchiveInterfaceSignals()
{
    connect(archiveInterface(), &ReadOnlyArchiveInterface::cancelled, this, &Job::onCancelled, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::error, this, &Job::onError, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::entry, this, &Job::onEntry, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::progress, this, &Job::onProgress, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::info, this, &Job::onInfo, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::finished, this, &Job::onFinished, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::userQuery, this, &Job::onUserQuery, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::progress_filename, this, &Job::onProgressFilename, Qt::ConnectionType::UniqueConnection);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::updateDestFileSignal, this, &Job::onUpdateDestFile, Qt::ConnectionType::UniqueConnection);

    auto readWriteInterface = qobject_cast<ReadWriteArchiveInterface *>(archiveInterface());
    if (readWriteInterface) {
        connect(readWriteInterface, &ReadWriteArchiveInterface::entryRemoved, this, &Job::onEntryRemoved, Qt::ConnectionType::UniqueConnection);
    }
}

void Job::onCancelled()
{
    qDebug() << "Cancelled emitted";
    setError(KJob::KilledJobError);
    emit sigCancelled();
}

void Job::onError(const QString &message, const QString &details)
{
    Q_UNUSED(details)

    qDebug() << "Error emitted:" << message;
    if (message.contains(QLatin1String("wrong password"))) {
        emit sigWrongPassword();
    } else if (message.contains("Listing the archive failed")) {
        setError(KJob::LoadError);
        setErrorText(message);
        emitResult();
        return;
    }
    setError(KJob::UserDefinedError);
    setErrorText(message);
}

void Job::onEntry(Archive::Entry *entry)
{
    emit newEntry(entry);
}

void Job::onProgress(double value)
{
    setPercent(static_cast<unsigned long>(100.0 * value));
}

void Job::onProgressFilename(const QString &filename)
{
    setPercentFilename(filename);
}

void Job::onInfo(const QString &info)
{
    emit infoMessage(this, info);
}

void Job::onEntryRemoved(const QString &path)
{
    emit entryRemoved(path);
}

void Job::onFinished(bool result)
{
    qDebug() << "Job finished, result:" << result << ", time:" << jobTimer.elapsed() << "ms";
    if (m_archiveInterface && m_archiveInterface->isUserCancel()) {
        setError(KJob::KilledJobError);
    } else if ((archive() && !archive()->isValid())  || false == result) {
        setError(KJob::UserDefinedError);
    } else if (m_archiveInterface && m_archiveInterface->isAnyFileExtracted() == false) {
        setError(KJob::UserSkiped);
    } else {
        setError(KJob::NoError);
    }

    if (!d->isInterruptionRequested()) {
        emitResult();
    }
}

void Job::onUserQuery(Query *query)
{
    if (archiveInterface()->waitForFinishedSignal()) {
        qDebug() << "Plugins run from the main thread should call directly query->execute()";
    }

    emit userQuery(query);
}

void Job::onUpdateDestFile(QString dstFile)
{
    emit updateDestFile(dstFile);
}

bool Job::doKill()
{
    const bool killed = archiveInterface()->doKill();
    if (killed) {
        return true;
    }

    if (d->isRunning()) {
        qDebug() << "Requesting graceful thread interruption, will abort in one second otherwise.";
        d->requestInterruption();
        d->wait(1000);
    }

    return true;
}

LoadJob::LoadJob(Archive *archive, ReadOnlyArchiveInterface *interface)
    : Job(archive, interface)
    , m_isSingleFolderArchive(true)
    , m_isPasswordProtected(false)
    , m_extractedFilesSize(0)
    , m_dirCount(0)
    , m_filesCount(0)
{
    qDebug() << "LoadJob job instance";
    connect(this, &LoadJob::newEntry, this, &LoadJob::onNewEntry);
}

LoadJob::LoadJob(Archive *archive, bool isbatch)
    : LoadJob(archive, nullptr)
{
    m_isbatch = isbatch;
}

LoadJob::LoadJob(ReadOnlyArchiveInterface *interface, bool isbatch)
    : LoadJob(nullptr, interface)
{
    m_isbatch = isbatch;
}

void LoadJob::doWork()
{
    //emit description(this, tr("Loading archive"), qMakePair(tr("Archive"), archiveInterface()->filename()));
    emit description(this, ("Loading archive"), qMakePair(QString("Archive"), archiveInterface()->filename()));
    connectToArchiveInterfaceSignals();

    bool ret = false;

    if (archiveInterface()) {
        connect(archiveInterface(), &ReadOnlyArchiveInterface::sigExtractNeedPassword, this, &LoadJob::sigLodJobPassword);

        ret = archiveInterface()->list(m_isbatch);
    }

    if (!archiveInterface()->waitForFinishedSignal()) {
        // onFinished() needs to be called after onNewEntry(), because the former reads members set in the latter.
        // So we need to put it in the event queue, just like the single-thread case does by emitting finished().
        QTimer::singleShot(0, this, [ = ]() {
            onFinished(ret);
        });
    }
}

void LoadJob::onFinished(bool result)
{
    if (archive() && result) {
        archive()->setProperty("unpackedSize", extractedFilesSize());
        archive()->setProperty("isSingleFolder", isSingleFolderArchive());
        const auto name = subfolderName().isEmpty() ? archive()->completeBaseName() : subfolderName();
        archive()->setProperty("subfolderName", name);
        if (isPasswordProtected()) {
            archive()->setProperty("encryptionType",  archive()->password().isEmpty() ? Archive::Encrypted : Archive::HeaderEncrypted);
        }
    }

    Job::onFinished(result);
}

qlonglong LoadJob::extractedFilesSize() const
{
    return m_extractedFilesSize;
}

bool LoadJob::isPasswordProtected() const
{
    return m_isPasswordProtected;
}

bool LoadJob::isSingleFolderArchive() const
{
    if (m_filesCount == 1 && m_dirCount == 0) {
        return false;
    }

    return m_isSingleFolderArchive;
}

void LoadJob::onNewEntry(const Archive::Entry *entry)
{
    m_extractedFilesSize += entry->property("size").toLongLong();
    m_isPasswordProtected |= entry->property("isPasswordProtected").toBool();

    if (entry->isDir()) {
        m_dirCount++;
    } else {
        m_filesCount++;
    }

    if (m_isSingleFolderArchive) {
        // RPM filenames have the ./ prefix, and "." would be detected as the subfolder name, so we remove it.
        const QString fullPath = entry->fullPath().replace(QRegularExpression(QStringLiteral("^\\./")), QString());
        const QString basePath = fullPath.split(QLatin1Char('/')).at(0);

        if (m_basePath.isEmpty()) {
            m_basePath = basePath;
            m_subfolderName = basePath;
        } else {
            if (m_basePath != basePath) {
                m_isSingleFolderArchive = false;
                m_subfolderName.clear();
            }
        }
    }
}

QString LoadJob::subfolderName() const
{
    if (!isSingleFolderArchive()) {
        return QString();
    }

    return m_subfolderName;
}

BatchExtractJob::BatchExtractJob(LoadJob *loadJob, const QString &destination, bool autoSubfolder, bool preservePaths)
    : Job(loadJob->archive())
    , m_loadJob(loadJob)
    , m_destination(destination)
    , m_autoSubfolder(autoSubfolder)
    , m_preservePaths(preservePaths)
{
    qDebug() << "BatchExtractJob job instance";
}

void BatchExtractJob::doWork()
{
    connect(m_loadJob, &KJob::result, this, &BatchExtractJob::slotLoadingFinished);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::cancelled, this, &BatchExtractJob::onCancelled);

//    if (archiveInterface()->hasBatchExtractionProgress()) {
    // progress() will be actually emitted by the LoadJob, but the archiveInterface() is the same.
    connect(archiveInterface(), &ReadOnlyArchiveInterface::progress, this, &BatchExtractJob::slotLoadingProgress);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::progress_filename, this, &BatchExtractJob::slotExtractFilenameProgress);
//    }

    // Forward LoadJob's signals.
    connect(m_loadJob, &Job::newEntry, this, &BatchExtractJob::newEntry);
    connect(m_loadJob, &Job::userQuery, this, &BatchExtractJob::userQuery);
    m_loadJob->start();
}

bool BatchExtractJob::doKill()
{
    if (m_step == Loading) {
        return m_loadJob->kill();
    }

    return m_extractJob->kill();
}

void BatchExtractJob::slotLoadingProgress(double progress)
{
    // Progress from LoadJob counts only for 50% of the BatchExtractJob's duration.

    m_lastPercentage = static_cast<unsigned long>(100.0 * progress);
    qDebug() << m_lastPercentage;
    setPercent(m_lastPercentage);
}

void BatchExtractJob::slotExtractFilenameProgress(const QString &filename)
{
    setPercentFilename(filename);
}

void BatchExtractJob::slotExtractProgress(double progress)
{
    // The 2nd 50% of the BatchExtractJob's duration comes from the ExtractJob.
    qDebug() << m_lastPercentage + static_cast<unsigned long>(progress);
    setPercent(m_lastPercentage + static_cast<unsigned long>(progress));
}

void BatchExtractJob::slotLoadingFinished(KJob *job)
{
    if (job->error()) {
        // Forward errors as well.
        onError(job->errorString(), QString());
        onFinished(false);
        return;
    }

    // Now we can start extraction.
    setupDestination();

    ExtractionOptions options;
    options.setPreservePaths(m_preservePaths);
    options.setBatchExtract(true);

    m_extractJob = archive()->extractFiles({}, m_destination, options);
    if (m_extractJob) {
        connect(m_extractJob, &KJob::result, this, &BatchExtractJob::emitResult);
        connect(m_extractJob, &Job::userQuery, this, &BatchExtractJob::userQuery);
//        if (archiveInterface()->hasBatchExtractionProgress()) {
        // The LoadJob is done, change slot and start setting the percentage from m_lastPercentage on.
        disconnect(archiveInterface(), &ReadOnlyArchiveInterface::progress, this, &BatchExtractJob::slotLoadingProgress);
        connect(archiveInterface(), &ReadOnlyArchiveInterface::progress, this, &BatchExtractJob::slotExtractProgress);
//        }
        m_step = Extracting;
        m_extractJob->start();
    } else {
        emitResult();
    }
}

void BatchExtractJob::setupDestination()
{
    const bool isSingleFolderRPM = (archive()->isSingleFolder() &&
                                    (archive()->mimeType().name() == QLatin1String("application/x-rpm")));

    if (m_autoSubfolder && (!archive()->isSingleFolder() || isSingleFolderRPM)) {
        const QDir d(m_destination);
        QString subfolderName = archive()->subfolderName();

        // Special case for single folder RPM archives.
        // We don't want the autodetected folder to have a meaningless "usr" name.
        if (isSingleFolderRPM && subfolderName == QStringLiteral("usr")) {
            qDebug() << "Detected single folder RPM archive. Using archive basename as subfolder name";
            subfolderName = QFileInfo(archive()->fileName()).completeBaseName();
        }

        if (d.exists(subfolderName)) {
//TODO_DS            subfolderName = KIO::suggestName(QUrl::fromUserInput(m_destination, QString(), QUrl::AssumeLocalFile), subfolderName);
        }

        d.mkdir(subfolderName);

        m_destination += QLatin1Char('/') + subfolderName;
    }
}

CreateJob::CreateJob(Archive *archive, const QVector<Archive::Entry *> &entries, const CompressionOptions &options)
    : Job(archive)
    , m_entries(entries)
    , m_options(options)
{
    qDebug() << "Created job instance";
}

void CreateJob::enableEncryption(const QString &password, bool encryptHeader)
{
    archive()->encrypt(password, encryptHeader);
}

void CreateJob::setMultiVolume(bool isMultiVolume)
{
    archive()->setMultiVolume(isMultiVolume);
}

void CreateJob::doWork()
{
    connect(archiveInterface(), &ReadOnlyArchiveInterface::progress, this, &CreateJob::onProgress);
    connect(archiveInterface(), &ReadOnlyArchiveInterface::progress_filename, this, &CreateJob::onProgressFilename);

    m_addJob = archive()->addFiles(m_entries, nullptr, m_options);

    if (m_addJob) {
        connect(m_addJob, &KJob::result, this, &CreateJob::emitResult);
        connect(m_addJob, &KJob::result, this, &CreateJob::result);
        // Forward description signal from AddJob, we need to change the first argument ('this' needs to be a CreateJob).
        connect(m_addJob, &KJob::description, this, [ = ](KJob *, const QString & title, const QPair<QString, QString> &field1, const QPair<QString, QString> &) {
            emit description(this, title, field1);
        });

        m_addJob->start();
    } else {
        emitResult();
    }
}

bool CreateJob::doKill()
{
    return m_addJob && m_addJob->kill();
}

ExtractJob::ExtractJob(const QVector<Archive::Entry *> &entries, const QString &destinationDir, const ExtractionOptions &options, ReadOnlyArchiveInterface *interface)
    : Job(interface)
    , m_entries(entries)
    , m_destinationDir(destinationDir)
    , m_options(options)
{
    qDebug() << "ExtractJob job instance";
    connect(interface, &ReadOnlyArchiveInterface::sigExtractNeedPassword, this, &ExtractJob::sigExtractJobPassword, Qt::QueuedConnection);
}

void ExtractJob::doWork()
{
    //percent( this, 0);

    QString desc;
    if (m_entries.count() == 0) {
        //desc = tr("Extracting all files");
        desc = ("Extracting all files");
    } else {
        //desc = tr("Extracting one file", "Extracting %1 files", m_entries.count());
        desc = QString("Extracting %1 files").arg(m_entries.count());
    }

    //emit description(this, desc, qMakePair(tr("Archive"), archiveInterface()->filename()), qMakePair(tr("extraction folder", "Destination"), m_destinationDir));
    emit description(this, desc, qMakePair(QString("Archive"), archiveInterface()->filename()), qMakePair(QString("extraction folder Destination"), m_destinationDir));

    QFileInfo destDirInfo(m_destinationDir);
    if (destDirInfo.isDir() && (!destDirInfo.isWritable() || !destDirInfo.isExecutable())) {
//        onError(tr("Could not write to destination <filename>%1</filename>.<nl/>Check whether you have sufficient permissions.", m_destinationDir), QString());
        onFinished(false);
        return;
    }

    connectToArchiveInterfaceSignals();

//    qDebug() << "Starting extraction with" << m_entries.count() << "selected files."
//             << m_entries
//             << "Destination dir:" << m_destinationDir
//             << "Options:" << m_options;

    bool ret = archiveInterface()->extractFiles(m_entries, m_destinationDir, m_options);

    if (!archiveInterface()->waitForFinishedSignal() /*&& archiveInterface()->isUserCancel() == false*/) {
        onFinished(ret);
        emit sigExtractJobFinished();
    }
}

bool ExtractJob::Killjob()
{
    return kill();
}

QString ExtractJob::destinationDirectory() const
{
    return m_destinationDir;
}

ExtractionOptions ExtractJob::extractionOptions() const
{
    return m_options;
}

TempExtractJob::TempExtractJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface)
    : Job(interface)
    , m_entry(entry)
    , m_passwordProtectedHint(passwordProtectedHint)
{
    m_tmpExtractDir = new QTemporaryDir();
}

QString TempExtractJob::validatedFilePath() const
{
    QString path = extractionDir() + QLatin1Char('/') + m_entry->fullPath();

    // Make sure a maliciously crafted archive with parent folders named ".." do
    // not cause the previewed file path to be located outside the temporary
    // directory, resulting in a directory traversal issue.
    path.remove(QStringLiteral("../"));

    return path;
}

ExtractionOptions TempExtractJob::extractionOptions() const
{
    ExtractionOptions options;

    if (m_passwordProtectedHint) {
        options.setEncryptedArchiveHint(true);
    }

    return options;
}

QTemporaryDir *TempExtractJob::tempDir() const
{
    return m_tmpExtractDir;
}

void TempExtractJob::doWork()
{
    // pass 1 to i18np on purpose so this translation may properly be reused.
    //emit description(this, tr("Extracting one file", "Extracting %1 files", 1));
    emit description(this, "Extracting one file");

    connectToArchiveInterfaceSignals();

    qDebug() << "Extracting:" << m_entry;

    bool ret = archiveInterface()->extractFiles({m_entry}, extractionDir(), extractionOptions());

    if (!archiveInterface()->waitForFinishedSignal()) {
        onFinished(ret);
    }
}

QString TempExtractJob::extractionDir() const
{
    return m_tmpExtractDir->path();
}

PreviewJob::PreviewJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface)
    : TempExtractJob(entry, passwordProtectedHint, interface)
{
    qDebug() << "PreviewJob job instance";
}

OpenJob::OpenJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface)
    : TempExtractJob(entry, passwordProtectedHint, interface)
{
    qDebug() << "OpenJob job instance";
}

OpenWithJob::OpenWithJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface)
    : OpenJob(entry, passwordProtectedHint, interface)
{
    qDebug() << "OpenWithJob job instance";
}

AddJob::AddJob(const QVector<Archive::Entry *> &entries, const Archive::Entry *destination, const CompressionOptions &options, ReadWriteArchiveInterface *interface)
    : Job(interface)
    , m_entries(entries)
    , m_destination(destination)
    , m_options(options)
{
    qDebug() << "AddJob job instance";
}

void AddJob::doWork()
{
    // Set current dir.
    const QString globalWorkDir = m_options.globalWorkDir();
    const QDir workDir = globalWorkDir.isEmpty() ? QDir::current() : QDir(globalWorkDir);
    if (!globalWorkDir.isEmpty()) {
        qDebug() << "GlobalWorkDir is set, changing dir to " << globalWorkDir;
        m_oldWorkingDir = QDir::currentPath();
        QDir::setCurrent(globalWorkDir);
    }

    // Count total number of entries to be added.
    uint totalCount = 0;
    QElapsedTimer timer;
    timer.start();
    for (const Archive::Entry *entry : qAsConst(m_entries)) {
        totalCount++;
        if (QFileInfo(entry->fullPath()).isDir()) {
            QDirIterator it(entry->fullPath(), QDir::AllEntries | QDir::Readable | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                totalCount++;
            }
        }
    }

    qDebug() << "Going to add" << totalCount << "entries, counted in" << timer.elapsed() << "ms";

    //const QString desc = tr("Compressing a file", "Compressing %1 files", totalCount);
    const QString desc = QString("Compressing %1 files").arg(totalCount);
    //emit description(this, desc, qMakePair(tr("Archive"), archiveInterface()->filename()));
    emit description(this, desc, qMakePair(QString("Archive"), archiveInterface()->filename()));

    ReadWriteArchiveInterface *m_writeInterface = dynamic_cast<ReadWriteArchiveInterface *>(archiveInterface());

    Q_ASSERT(m_writeInterface);


    // The file paths must be relative to GlobalWorkDir.
    for (Archive::Entry *entry : qAsConst(m_entries)) {
        qDebug() << entry->fullPath();

        const QString &fullPath = entry->fullPath();
        QString relativePath = workDir.relativeFilePath(fullPath);

        if (fullPath.endsWith(QLatin1Char('/'))) {
            relativePath += QLatin1Char('/');
        }

        entry->setFullPath(relativePath);
    }

    connectToArchiveInterfaceSignals();
    bool ret = m_writeInterface->addFiles(m_entries, m_destination, m_options, totalCount);

    if (!archiveInterface()->waitForFinishedSignal()) {
        onFinished(ret);
    }
}

void AddJob::onFinished(bool result)
{
    if (!m_oldWorkingDir.isEmpty()) {
        QDir::setCurrent(m_oldWorkingDir);
    }

    Job::onFinished(result);
}

MoveJob::MoveJob(const QVector<Archive::Entry *> &entries, Archive::Entry *destination, const CompressionOptions &options, ReadWriteArchiveInterface *interface)
    : Job(interface)
    , m_finishedSignalsCount(0)
    , m_entries(entries)
    , m_destination(destination)
    , m_options(options)
{
    qDebug() << "MoveJob job instance";
}

void MoveJob::doWork()
{
    qDebug() << "Going to move" << m_entries.count() << "file(s)";

    //QString desc = tr("Moving a file", "Moving %1 files", m_entries.count());
    QString desc = QString("Moving %1 files").arg(m_entries.count());
    //emit description(this, desc, qMakePair(tr("Archive"), archiveInterface()->filename()));
    emit description(this, desc, qMakePair(QString("Archive"), archiveInterface()->filename()));

    ReadWriteArchiveInterface *m_writeInterface =
        qobject_cast<ReadWriteArchiveInterface *>(archiveInterface());

    Q_ASSERT(m_writeInterface);

    connectToArchiveInterfaceSignals();
    bool ret = m_writeInterface->moveFiles(m_entries, m_destination, m_options);

    if (!archiveInterface()->waitForFinishedSignal()) {
        onFinished(ret);
    }
}

void MoveJob::onFinished(bool result)
{
    m_finishedSignalsCount++;
    if (m_finishedSignalsCount == archiveInterface()->moveRequiredSignals()) {
        Job::onFinished(result);
    }
}

CopyJob::CopyJob(const QVector<Archive::Entry *> &entries, Archive::Entry *destination, const CompressionOptions &options, ReadWriteArchiveInterface *interface)
    : Job(interface)
    , m_finishedSignalsCount(0)
    , m_entries(entries)
    , m_destination(destination)
    , m_options(options)
{
    qDebug() << "CopyJob job instance";
}

void CopyJob::doWork()
{
    qDebug() << "Going to copy" << m_entries.count() << "file(s)";

    //QString desc = tr("Copying a file", "Copying %1 files", m_entries.count());
    QString desc = QString("Copying %1 files").arg(m_entries.count());
    //emit description(this, desc, qMakePair(tr("Archive"), archiveInterface()->filename()));
    emit description(this, desc, qMakePair(QString("Archive"), archiveInterface()->filename()));

    ReadWriteArchiveInterface *m_writeInterface =
        qobject_cast<ReadWriteArchiveInterface *>(archiveInterface());

    Q_ASSERT(m_writeInterface);

    connectToArchiveInterfaceSignals();
    bool ret = m_writeInterface->copyFiles(m_entries, m_destination, m_options);

    if (!archiveInterface()->waitForFinishedSignal()) {
        onFinished(ret);
    }
}

void CopyJob::onFinished(bool result)
{
    m_finishedSignalsCount++;
    if (m_finishedSignalsCount == archiveInterface()->copyRequiredSignals()) {
        Job::onFinished(result);
    }
}

DeleteJob::DeleteJob(const QVector<Archive::Entry *> &entries, ReadWriteArchiveInterface *interface)
    : Job(interface)
    , m_entries(entries)
{
}

void DeleteJob::doWork()
{
    //QString desc = tr("Deleting a file from the archive", "Deleting %1 files", m_entries.count());
    QString desc = QString("Deleting %1 files").arg(m_entries.count());
    //emit description(this, desc, qMakePair(tr("Archive"), archiveInterface()->filename()));
    emit description(this, desc, qMakePair(QString("Archive"), archiveInterface()->filename()));

    ReadWriteArchiveInterface *m_writeInterface =
        qobject_cast<ReadWriteArchiveInterface *>(archiveInterface());

    Q_ASSERT(m_writeInterface);

    connectToArchiveInterfaceSignals();
    bool ret = m_writeInterface->deleteFiles(m_entries);

    if (!archiveInterface()->waitForFinishedSignal()) {
        onFinished(ret);
    }
}

CommentJob::CommentJob(const QString &comment, ReadWriteArchiveInterface *interface)
    : Job(interface)
    , m_comment(comment)
{
}

void CommentJob::doWork()
{
    //emit description(this, tr("Adding comment"));
    emit description(this, "Adding comment");

    ReadWriteArchiveInterface *m_writeInterface =
        qobject_cast<ReadWriteArchiveInterface *>(archiveInterface());

    Q_ASSERT(m_writeInterface);

    connectToArchiveInterfaceSignals();
    bool ret = m_writeInterface->addComment(m_comment);

    if (!archiveInterface()->waitForFinishedSignal()) {
        onFinished(ret);
    }
}

TestJob::TestJob(ReadOnlyArchiveInterface *interface)
    : Job(interface)
{
    m_testSuccess = false;
}

void TestJob::doWork()
{
    qDebug() << "Job started";

    //emit description(this, tr("Testing archive"), qMakePair(tr("Archive"), archiveInterface()->filename()));
    emit description(this, ("Testing archive"), qMakePair(QString("Archive"), archiveInterface()->filename()));

    connectToArchiveInterfaceSignals();
    connect(archiveInterface(), &ReadOnlyArchiveInterface::testSuccess, this, &TestJob::onTestSuccess);

    bool ret = archiveInterface()->testArchive();

    if (!archiveInterface()->waitForFinishedSignal()) {
        onFinished(ret);
    }
}

void TestJob::onTestSuccess()
{
    m_testSuccess = true;
}

bool TestJob::testSucceeded()
{
    return m_testSuccess;
}


#include "jobs.moc"
