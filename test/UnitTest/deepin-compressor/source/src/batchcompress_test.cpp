#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "batchcompress.h"
#include "options.h"
#include "archive_manager.h"
#include "batchjobs.h"
#include "jobs.h"
#include <QDir>

TEST(BatchCompress_BatchCompress_UT, BatchCompress_BatchCompress_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
//    batchCompress->m_Args = new QMap<QString, QString>();
//    batchCompress->m_Args->insert("fixedMimeType", "application/zip");
    ASSERT_NE(batchCompress, nullptr);
    delete batchCompress;
}

TEST(BatchCompress_setCompressArgs_UT, BatchCompress_setCompressArgs_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
//    batchCompress->m_Args->insert("fixedMimeType", "application/zip");
    QMap<QString, QString> Args;
    Args.insert("fixedMimeType", "application/zip");
    batchCompress->setCompressArgs(Args);
}

TEST(BatchCompress_addCompress_UT, BatchCompress_addCompress_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    batchCompress->m_Args->insert("localFilePath", "");
    batchCompress->m_Args->insert("filename", "");
    batchCompress->addCompress(files);
}

TEST(BatchCompress_addCompress_UT, BatchCompress_addCompress_UT002)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    batchCompress->m_Args->insert("fixedMimeType", "application/zip");
    batchCompress->m_Args->insert("encryptionPassword", "1");
    QStringList files;
    files.append("/home/chenglu/Desktop/file/");
    files.append("/home/chenglu/Desktop/so.sh");
    batchCompress->addCompress(files);
}

TEST(BatchCompress_SlotProgressFile_UT, BatchCompress_SlotProgressFile_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries;
    CreateJob *job = Archive::create(filename, fixedMimeType, all_entries, options, nullptr, false, false);
    batchCompress->SlotProgressFile(job, "/home/chenglu/Desktop/error.log");
}

TEST(BatchCompress_clearSubjobs_UT, BatchCompress_clearSubjobs_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries;
    CreateJob *job = Archive::create(filename, fixedMimeType, all_entries, options, nullptr, false, false);

    QStringList files1;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd1 = files;
    const QString fixedMimeType1 = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password1 = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption1 = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename1 = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options1;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries1;
    CreateJob *job1 = Archive::create(filename1, fixedMimeType1, all_entries1, options1, nullptr, false, false);
    batchCompress->addSubjob(job);
    batchCompress->addSubjob(job1);
    batchCompress->clearSubjobs();
}

TEST(BatchCompress_doKill_UT, BatchCompress_doKill_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    batchCompress->doKill();
}

TEST(BatchCompress_doKill_UT, BatchCompress_doKill_UT002)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries;
    CreateJob *job = Archive::create(filename, fixedMimeType, all_entries, options, nullptr, false, false);
    batchCompress->addSubjob(job);
    batchCompress->doKill();
}

TEST(BatchCompress_start_UT, BatchCompress_start_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    batchCompress->start();
}

TEST(BatchCompress_slotStartJob_UT, BatchCompress_slotStartJob_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    batchCompress->slotStartJob();
}

TEST(BatchCompress_slotStartJob_UT, BatchCompress_slotStartJob_UT002)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    batchCompress->m_inputs.append(files);
    batchCompress->slotStartJob();
}

TEST(BatchCompress_slotResult_UT, BatchCompress_slotResult_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries;
    CreateJob *job = Archive::create(filename, fixedMimeType, all_entries, options, nullptr, false, false);

    QStringList files1;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd1 = files;
    const QString fixedMimeType1 = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password1 = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption1 = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename1 = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options1;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries1;
    CreateJob *job1 = Archive::create(filename1, fixedMimeType1, all_entries1, options1, nullptr, false, false);
    batchCompress->addSubjob(job);
    batchCompress->addSubjob(job1);
    batchCompress->slotResult(job);
}

TEST(BatchCompress_slotResult_UT, BatchCompress_slotResult_UT002)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries;
    CreateJob *job = Archive::create(filename, fixedMimeType, all_entries, options, nullptr, false, false);

    batchCompress->addSubjob(job);
    job->setError(8);
    batchCompress->slotResult(job);
}

TEST(BatchCompress_forwardProgress_UT, BatchCompress_forwardProgress_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries;
    CreateJob *job = Archive::create(filename, fixedMimeType, all_entries, options, nullptr, false, false);
    batchCompress->addSubjob(job);
    batchCompress->forwardProgress(job, 100);
}

TEST(BatchCompress_addInput_UT, BatchCompress_addInput_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files.append("/home/chenglu/Desktop/error.log");
    batchCompress->addInput(files);
}

TEST(BatchCompress_SlotCreateJobFinished_UT, BatchCompress_SlotCreateJobFinished_UT001)
{
    BatchCompress *batchCompress = new BatchCompress(nullptr);
    QStringList files;
    files << "/home/chenglu/Desktop/error.log";
    const QStringList filesToAdd = files;
    const QString fixedMimeType = (*(batchCompress->m_Args))[QStringLiteral("fixedMimeType")];
    const QString password = (*(batchCompress->m_Args))[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = (*(batchCompress->m_Args))[QStringLiteral("encryptHeader")];
    QString filename = (*(batchCompress->m_Args))[QStringLiteral("localFilePath")] + QDir::separator() + (*(batchCompress->m_Args))[QStringLiteral("filename")];

    CompressionOptions options;
    options.setCompressionLevel((*(batchCompress->m_Args))[QStringLiteral("compressionLevel")].toInt());
    options.setEncryptionMethod((*(batchCompress->m_Args))[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize((*(batchCompress->m_Args))[QStringLiteral("volumeSize")].toULongLong());

    QVector<Archive::Entry *> all_entries;
    CreateJob *job = Archive::create(filename, fixedMimeType, all_entries, options, nullptr, false, false);
    batchCompress->addSubjob(job);
    job->setError(8);
    batchCompress->SlotCreateJobFinished(job);
}
