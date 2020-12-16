#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "batchextract.h"
#include "mimetypes.h"
#include "jobs.h"
#include "kpluginloader.h"
#include "pluginmanager.h"

TEST(BatchExtract_BatchExtract_UT, BatchExtract_BatchExtract_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    EXPECT_NE(batchExtract, nullptr);
    delete batchExtract;
}

TEST(BatchExtract_addExtraction_UT, BatchExtract_addExtraction_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->addExtraction(QUrl::fromLocalFile("/home/chenglu/Desktop/1.tar.gz"));
}

TEST(BatchExtract_addExtraction_UT, BatchExtract_addExtraction_UT002)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->addExtraction(QUrl::fromLocalFile("/home/chenglu/Desktop/1.7z.001"));
}

TEST(BatchExtract_addExtraction_UT, BatchExtract_addExtraction_UT003)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->addExtraction(QUrl::fromLocalFile("/home/chenglu/Desktop/1.part01.rar"));
}

TEST(BatchExtract_addExtraction_UT, BatchExtract_addExtraction_UT004)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->addExtraction(QUrl::fromLocalFile("/home/chenglu/Desktop/1.part1.rar"));
}

TEST(BatchExtract_SlotProgressFile_UT, BatchExtract_SlotProgressFile_UT003)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->SlotProgressFile(nullptr, "pic.png");
}

//TEST(BatchExtract_doKill_UT, BatchExtract_doKill_UT001)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    bool kill = batchExtract->doKill();
//    EXPECT_EQ(kill, true);
//}

//TEST(BatchExtract_doKill_UT, BatchExtract_doKill_UT002)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    batchExtract->addExtraction(QUrl::fromLocalFile("/home/chenglu/Desktop/1.7z.001"));
//    bool kill = batchExtract->doKill();
//    EXPECT_EQ(kill, false);
//}

//TEST(BatchExtract_autoSubfolder_UT, BatchExtract_autoSubfolder_UT001)
//{
//    //该函数实际未被调用
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    bool autoSubfolder = batchExtract->autoSubfolder();
//    ASSERT_TRUE(autoSubfolder);
//}

TEST(BatchExtract_setAutoSubfolder_UT, BatchExtract_setAutoSubfolder_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setAutoSubfolder(true);
}

TEST(BatchExtract_setAutoSubfolder_UT, BatchExtract_setAutoSubfolder_UT002)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setAutoSubfolder(false);
}

TEST(BatchExtract_start_UT, BatchExtract_start_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->start();
}

class mystub
{
public:
    void loadPlugins_stub(void *obj)
    {
        PluginManager *o = (PluginManager *)obj;

        QCoreApplication::addLibraryPath("../../output/");
        const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("compressor-lib/"));
        QSet<QString> addedPlugins;
        for (const KPluginMetaData &metaData : plugins) {
            const auto pluginId = metaData.pluginId();
            // Filter out duplicate plugins.
            if (addedPlugins.contains(pluginId)) {
                continue;
            }

            Plugin *plugin = new Plugin(o, metaData);
            plugin->setEnabled(true);
            addedPlugins << pluginId;
            o->m_plugins << plugin;
        }
    }
};

// TEST(BatchExtract_slotStartJob_UT, BatchExtract_slotStartJob_UT001)
// {
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    QVector<Archive::Entry *> files;
//    ExtractionOptions options;
//    options.setBatchExtract(true);

//    QFileInfo fi("/home/chenglu/Desktop/pic.part1.rar");
//    QString fixedMimetype = determineMimeType(fi.filePath()).name();

//    Stub stub;
//    stub.set(ADDR(PluginManager, loadPlugins), ADDR(mystub,loadPlugins_stub));

//    ReadOnlyArchiveInterface *pIface = Archive::createInterface(fi.filePath(), fixedMimetype, true);
//    ExtractJob *job = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface);
//    batchExtract->addSubjob(job);
//    batchExtract->m_inputs << QUrl::fromLocalFile("/home/chenglu/Desktop/1.tar.gz")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.7z.001")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.part01.rar")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.part1.rar");
//    batchExtract->slotStartJob();
// }

//TEST(BatchExtract_slotResult_UT, BatchExtract_slotResult_UT001)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    QVector<Archive::Entry *> files;
//    ExtractionOptions options;
//    options.setBatchExtract(true);

//    QFileInfo fi("/home/chenglu/Desktop/pic.tar.gz");
//    QString fixedMimetype = determineMimeType(fi.filePath()).name();
//    ReadOnlyArchiveInterface *pIface = Archive::createInterface(fi.filePath(), fixedMimetype, true);
//    ExtractJob *job = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface);

//    QFileInfo fi1("/home/chenglu/Desktop/1.tar.gz");
//    QString fixedMimetype1 = determineMimeType(fi1.filePath()).name();
//    ReadOnlyArchiveInterface *pIface1 = Archive::createInterface(fi1.filePath(), fixedMimetype1, true);
//    ExtractJob *job1 = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface1);

//    batchExtract->addSubjob(job);
//    batchExtract->addSubjob(job1);
//    batchExtract->slotResult(job);
//    delete job;
//}

//TEST(BatchExtract_slotResult_UT, BatchExtract_slotResult_UT002)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    QVector<Archive::Entry *> files;
//    ExtractionOptions options;
//    options.setBatchExtract(true);

//    QFileInfo fi("/home/chenglu/Desktop/1.7z.001");
//    QString fixedMimetype = determineMimeType(fi.filePath()).name();
//    ReadOnlyArchiveInterface *pIface = Archive::createInterface(fi.filePath(), fixedMimetype, true);
//    ExtractJob *job = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface);
//    batchExtract->addSubjob(job);
//    job->setError(7);
//    batchExtract->slotResult(job);
//    delete job;
//}

//TEST(BatchExtract_slotResult_UT, BatchExtract_slotResult_UT003)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    QVector<Archive::Entry *> files;
//    ExtractionOptions options;
//    options.setBatchExtract(true);

//    QFileInfo fi("/home/chenglu/Desktop/1.tar.gz");
//    QString fixedMimetype = determineMimeType(fi.filePath()).name();
//    ReadOnlyArchiveInterface *pIface = Archive::createInterface(fi.filePath(), fixedMimetype, true);
//    ExtractJob *job = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface);
//    batchExtract->slotResult(job);
//    delete job;
//}

//TEST(BatchExtract_slotResult_UT, BatchExtract_slotResult_UT004)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    QVector<Archive::Entry *> files;
//    ExtractionOptions options;
//    options.setBatchExtract(true);

//    QFileInfo fi("/home/chenglu/Desktop/pic.part1.rar");
//    QString fixedMimetype = determineMimeType(fi.filePath()).name();
//    ReadOnlyArchiveInterface *pIface = Archive::createInterface(fi.filePath(), fixedMimetype, true);
//    ExtractJob *job = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface);
//    batchExtract->slotResult(job);
//    delete job;
//}

//TEST(BatchExtract_forwardProgress_UT, BatchExtract_forwardProgress_UT001)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    QVector<Archive::Entry *> files;
//    ExtractionOptions options;
//    options.setBatchExtract(true);

//    QFileInfo fi("/home/chenglu/Desktop/pic.part1.rar");
//    QString fixedMimetype = determineMimeType(fi.filePath()).name();
//    ReadOnlyArchiveInterface *pIface = Archive::createInterface(fi.filePath(), fixedMimetype, true);
//    ExtractJob *job = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface);
//    batchExtract->m_batchTotalSize = 106496;

//    batchExtract->m_numOfExtracting = 0;
//    batchExtract->m_inputs << QUrl::fromLocalFile("/home/chenglu/Desktop/1.tar.gz")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.7z.001")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.part01.rar")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.part1.rar");
//    batchExtract->forwardProgress(job, 0);
//}

//TEST(BatchExtract_forwardProgress_UT, BatchExtract_forwardProgress_UT002)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    QVector<Archive::Entry *> files;
//    ExtractionOptions options;
//    options.setBatchExtract(true);

//    QFileInfo fi("/home/chenglu/Desktop/pic.part1.rar");
//    QString fixedMimetype = determineMimeType(fi.filePath()).name();
//    ReadOnlyArchiveInterface *pIface = Archive::createInterface(fi.filePath(), fixedMimetype, true);
//    ExtractJob *job = new ExtractJob(files, "/home/chenglu/Desktop", options, pIface);
//    batchExtract->m_batchTotalSize = 106496;

//    batchExtract->m_numOfExtracting = 1;
//    batchExtract->m_inputs << QUrl::fromLocalFile("/home/chenglu/Desktop/1.tar.gz")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.7z.001")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.part01.rar")
//                           << QUrl::fromLocalFile("/home/chenglu/Desktop/1.part1.rar");
//    batchExtract->forwardProgress(job, 100);
//}

TEST(BatchExtract_addInput_UT, BatchExtract_addInput_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->addInput(QUrl::fromLocalFile("/home/chenglu/Desktop/1.7z.001"));
}

TEST(BatchExtract_addInput_UT, BatchExtract_addInput_UT002)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->addInput(QUrl::fromLocalFile("/home/chenglu/Desktop/2.7z.001"));
}

//TEST(BatchExtract_openDestinationAfterExtraction_UT, BatchExtract_openDestinationAfterExtraction_UT001)
//{
//    //该函数实际未被调用
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    bool open = batchExtract->openDestinationAfterExtraction();
//    ASSERT_EQ(open, true);
//}

TEST(BatchExtract_openDestinationAfterExtraction_UT, BatchExtract_openDestinationAfterExtraction_UT002)
{
    //该函数实际未被调用
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    bool open = batchExtract->openDestinationAfterExtraction();
    ASSERT_EQ(open, false);
}

TEST(BatchExtract_preservePaths_UT, BatchExtract_preservePaths_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    bool preserve = batchExtract->preservePaths();
    ASSERT_TRUE(preserve);
}

TEST(BatchExtract_destinationFolder_UT, BatchExtract_destinationFolder_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->m_destinationFolder = "/home/chenglu/Desktop/special";
    QString desFolder = batchExtract->destinationFolder();
    ASSERT_STREQ(desFolder.toStdString().c_str(), "/home/chenglu/Desktop/special");
}

//TEST(BatchExtract_destinationFolder_UT, BatchExtract_destinationFolder_UT002)
//{
//    BatchExtract *batchExtract = new BatchExtract(nullptr);
//    batchExtract->m_destinationFolder = "";
//    QString desFolder = batchExtract->destinationFolder();
//    ASSERT_STREQ(desFolder.toStdString().c_str(), "");
//}

TEST(BatchExtract_setDestinationFolder_UT, BatchExtract_setDestinationFolder_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setDestinationFolder("/home/chenglu/Desktop/special");
}

TEST(BatchExtract_setDestinationFolder_UT, BatchExtract_setDestinationFolder_UT002)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setDestinationFolder("/home/chenglu/Desktop/out.txt");
}

TEST(BatchExtract_setOpenDestinationAfterExtraction_UT, BatchExtract_setOpenDestinationAfterExtraction_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setOpenDestinationAfterExtraction(true);
}

TEST(BatchExtract_setOpenDestinationAfterExtraction_UT, BatchExtract_setOpenDestinationAfterExtraction_UT002)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setOpenDestinationAfterExtraction(false);
}

TEST(BatchExtract_setPreservePaths_UT, BatchExtract_setPreservePaths_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setPreservePaths(true);
}

TEST(BatchExtract_setPreservePaths_UT, BatchExtract_setPreservePaths_UT002)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setPreservePaths(false);
}

TEST(BatchExtract_setBatchTotalSize_UT, BatchExtract_setBatchTotalSize_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    batchExtract->setBatchTotalSize(20);
}

TEST(BatchExtract_showExtractDialog_UT, BatchExtract_showExtractDialog_UT001)
{
    BatchExtract *batchExtract = new BatchExtract(nullptr);
    bool ret = batchExtract->showExtractDialog();
    ASSERT_TRUE(ret);
}
