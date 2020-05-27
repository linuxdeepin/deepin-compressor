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
#ifndef JOBS_H
#define JOBS_H


#include "archiveinterface.h"
#include "archive_manager.h"
#include "archiveentry.h"
#include "queries.h"

#include "archivejob.h"

#include <QElapsedTimer>
#include <QTemporaryDir>

class  Job : public KJob
{
    Q_OBJECT

public:

    Archive *archive() const;
    QString errorString() const override;
    void start() override;
    ReadOnlyArchiveInterface *archiveInterface();

protected:
    Job(Archive *archive, ReadOnlyArchiveInterface *interface);
    Job(Archive *archive);
    Job(ReadOnlyArchiveInterface *interface);
    ~Job() override;
    bool doKill() override;



    void connectToArchiveInterfaceSignals();

public Q_SLOTS:
    virtual void doWork() = 0;

protected Q_SLOTS:
    virtual void onCancelled();
    virtual void onError(const QString &message, const QString &details);
    virtual void onInfo(const QString &info);
    virtual void onEntry(Archive::Entry *entry);
    virtual void onProgress(double progress);
    virtual void onProgressFilename(const QString &filename);
    virtual void onEntryRemoved(const QString &path);
    virtual void onFinished(bool result);
    virtual void onUserQuery(Query *query);
    void onUpdateDestFile(QString dstFile);

Q_SIGNALS:
    void entryRemoved(const QString &entry);
    void newEntry(Archive::Entry *);
    void userQuery(Query *);
    void sigWrongPassword();
    void sigCancelled();
    void updateDestFile(QString dstFile);
    void sigExtractSpinnerFinished();
private:
    Archive *m_archive;
    ReadOnlyArchiveInterface *m_archiveInterface;
    QElapsedTimer jobTimer;

    class Private;
    Private *const d;
};


class LoadJob : public Job
{
    Q_OBJECT

public:
    explicit LoadJob(Archive *archive, bool isbatch = false);
    explicit LoadJob(ReadOnlyArchiveInterface *interface, bool isbatch = false);

    qlonglong extractedFilesSize() const;
    bool isPasswordProtected() const;
    bool isSingleFolderArchive() const;
    QString subfolderName() const;

public Q_SLOTS:
    void doWork() override;

protected Q_SLOTS:
    void onFinished(bool result) override;

private:
    explicit LoadJob(Archive *archive, ReadOnlyArchiveInterface *interface);

    bool m_isSingleFolderArchive;
    bool m_isPasswordProtected;
    QString m_subfolderName;
    QString m_basePath;
    qlonglong m_extractedFilesSize;
    qlonglong m_dirCount;
    qlonglong m_filesCount;
    bool m_isbatch;

private Q_SLOTS:
    void onNewEntry(const Archive::Entry *);

signals:
    void sigLodJobPassword();
};


class BatchExtractJob : public Job
{
    Q_OBJECT

public:
    explicit BatchExtractJob(LoadJob *loadJob, const QString &destination, bool autoSubfolder, bool preservePaths);

public Q_SLOTS:
    void doWork() override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void slotLoadingProgress(double progress);
    void slotExtractProgress(double progress);
    void slotExtractFilenameProgress(const QString &filename);
    void slotLoadingFinished(KJob *job);

signals:
    void signeedpassword();

private:

    enum Step {Loading, Extracting};

    void setupDestination();

    Step m_step = Loading;
    ExtractJob *m_extractJob = nullptr;
    LoadJob *m_loadJob;
    QString m_destination;
    bool m_autoSubfolder;
    bool m_preservePaths;
    unsigned long m_lastPercentage = 0;
};


class CreateJob : public Job
{
    Q_OBJECT

public:
    explicit CreateJob(Archive *archive, const QVector<Archive::Entry *> &entries, const CompressionOptions &options);

    void enableEncryption(const QString &password, bool encryptHeader);

    void setMultiVolume(bool isMultiVolume);

public Q_SLOTS:
    void doWork() override;

protected:
    bool doKill() override;

private:
    AddJob *m_addJob = nullptr;
    QVector<Archive::Entry *> m_entries;
    CompressionOptions m_options;
};

class ExtractJob : public Job
{
    Q_OBJECT

public:
    ExtractJob(const QVector<Archive::Entry *> &entries, const QString &destinationDir, const ExtractionOptions &options, ReadOnlyArchiveInterface *interface);

    QString destinationDirectory() const;
    ExtractionOptions extractionOptions() const;
    bool Killjob();


    void resetTimeOut();

public Q_SLOTS:
    void doWork() override;
    void onFinished(bool result)override;
    void slotWorkTimeOut(bool isWorkProcess);
    void slotExtractJobPwdCheckDown();
    void onProgress(double progress)override;
    void onProgressFilename(const QString &filename)override;
signals:
    void sigExtractJobPassword();
    void sigExtractJobFinished();
    void sigExtractJobPwdCheckDown();

private:
    void cleanIfCanceled();
private:
    QVector<Archive::Entry *> m_entries;
    QString m_destinationDir;
    ExtractionOptions m_options;
    bool m_bTimeout = true;//if work time out,if greater than 700ms,emit the progress info.
};


class TempExtractJob : public Job
{
    Q_OBJECT

public:
    TempExtractJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface);
    QString validatedFilePath() const;

    ExtractionOptions extractionOptions() const;
    QTemporaryDir *tempDir() const;

public Q_SLOTS:
    void doWork() override;

private:
    QString extractionDir() const;

    Archive::Entry *m_entry;
    QTemporaryDir *m_tmpExtractDir;
    bool m_passwordProtectedHint;
};

class PreviewJob : public TempExtractJob
{
    Q_OBJECT

public:
    PreviewJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface);
};


class OpenJob : public TempExtractJob
{
    Q_OBJECT

public:
    OpenJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface);
};

class OpenWithJob : public OpenJob
{
    Q_OBJECT

public:
    OpenWithJob(Archive::Entry *entry, bool passwordProtectedHint, ReadOnlyArchiveInterface *interface);
};

class AddJob : public Job
{
    Q_OBJECT

public:
    AddJob(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, ReadWriteArchiveInterface *interface);

public Q_SLOTS:
    void doWork() override;

protected Q_SLOTS:
    void onFinished(bool result) override;

private:
    QString m_oldWorkingDir;
    const QVector<Archive::Entry *> m_entries;
    const Archive::Entry *m_destination;
    CompressionOptions m_options;
};

class MoveJob : public Job
{
    Q_OBJECT

public:
    MoveJob(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options, ReadWriteArchiveInterface *interface);

public Q_SLOTS:
    void doWork() override;

protected Q_SLOTS:
    void onFinished(bool result) override;

private:
    int m_finishedSignalsCount;
    const QVector<Archive::Entry *> m_entries;
    Archive::Entry *m_destination;
    CompressionOptions m_options;
};


class CopyJob : public Job
{
    Q_OBJECT

public:
    CopyJob(const QVector<Archive::Entry *> &entries, Archive::Entry *destination, const CompressionOptions &options, ReadWriteArchiveInterface *interface);

public Q_SLOTS:
    void doWork() override;

protected Q_SLOTS:
    void onFinished(bool result) override;

private:
    int m_finishedSignalsCount;
    const QVector<Archive::Entry *> m_entries;
    Archive::Entry *m_destination;
    CompressionOptions m_options;
};

class DeleteJob : public Job
{
    Q_OBJECT

public:
    DeleteJob(const QVector<Archive::Entry *> &files, ReadWriteArchiveInterface *interface);

public Q_SLOTS:
    void doWork() override;

private:
    QVector<Archive::Entry *> m_entries;
};

class CommentJob : public Job
{
    Q_OBJECT

public:
    CommentJob(const QString &comment, ReadWriteArchiveInterface *interface);

public Q_SLOTS:
    void doWork() override;

private:
    QString m_comment;
};

class TestJob : public Job
{
    Q_OBJECT

public:
    explicit TestJob(ReadOnlyArchiveInterface *interface);
    bool testSucceeded();

public Q_SLOTS:
    void doWork() override;

private Q_SLOTS:
    virtual void onTestSuccess();

private:
    bool m_testSuccess;
};


#endif // JOBS_H
