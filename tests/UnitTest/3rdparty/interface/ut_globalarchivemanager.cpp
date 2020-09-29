#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include <QModelIndex>
#include "globalarchivemanager.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <jobs.h>
TEST(GlobalArchiveManager_GlobalArchiveManager_UT, GlobalArchiveManager_GlobalArchiveManager_UT001)
{
    GlobalArchiveManager *archive_format = nullptr;
    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    ASSERT_EQ(archive_format, nullptr);
}


TEST(GlobalArchiveManager_getCurrentArchive_UT, GlobalArchiveManager_getCurrentArchive_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="winow.txt";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    GlobalArchiveManager *archive_format = new GlobalArchiveManager();
    archive_format->setCurrentArchive(archive);
    archive = archive_format->getCurrentArchive();
    delete archive;
}

TEST(GlobalArchiveManager_setCurrentArchive_UT, GlobalArchiveManager_setCurrentArchive_UT001)
{
    GlobalArchiveManager *archive_format = new GlobalArchiveManager();
    archive_format->setCurrentArchive(nullptr);
     delete archive_format;
}

TEST(GlobalArchiveManager_getCurrentPlugin_UT, GlobalArchiveManager_getCurrentPlugin_UT001)
{
    GlobalArchiveManager *archive_format = new GlobalArchiveManager();
    archive_format->getCurrentPlugin();
    delete archive_format;
}

TEST(GlobalArchiveManager_getCurrentPlugin_UT, GlobalArchiveManager_getCurrentPlugin_UT002)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="winow.txt";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    GlobalArchiveManager *archive_format = new GlobalArchiveManager();
    archive_format->setCurrentArchive(archive);
    archive_format->getCurrentPlugin();
    delete archive;
    delete archive_format;
}

