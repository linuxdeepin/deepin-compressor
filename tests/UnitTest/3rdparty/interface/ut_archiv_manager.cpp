#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include "archiveentry.h"
#include "archive_manager.h"
#include <QModelIndex>
#include "pluginmanager.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
TEST(ArchiveManager_Archive_UT, Archive_Archive_UT001)
{
    Archive *archive = nullptr;
    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    ASSERT_EQ(archive, nullptr);
}

TEST(ArchiveManager_Archive_UT, Archive_createEmpty_UT001)
{
     ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
     QString fileName;
     QString mimeType;
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    delete archive;
    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
   // ASSERT_EQ(archive, nullptr);
}

TEST(ArchiveManager_Archive_UT, Archive_create_UT001)
{
     ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
     QString fileName;
     QString mimeType;
     fileName ="winow.iso";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->create(fileName,mimeType);
    delete archive;

}
TEST(ArchiveManager_Archive_UT, Archive_create_UT002)
{

     QString fileName;
     QString mimeType;
     fileName ="winow.iso";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->create(fileName,mimeType,false);
    delete archive;
}
TEST(ArchiveManager_Archive_UT, Archive_create_UT003)
{

     QString fileName;
     QString mimeType = "application/zip";
     fileName ="winow.iso";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->create(fileName,mimeType,true);
    delete archive;
}

TEST(ArchiveManager_Archive_UT, Archive_create_UT004)
{
     ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
     QString fileName;
     QString mimeType = "application/zip";
     fileName ="winow.zip";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->create(fileName,mimeType,true,nullptr,true);
    delete archive;
}


TEST(ArchiveManager_Archive_UT, Archive_create_UT005)
{
     ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
     QString fileName;
     QString mimeType = "7z";
     fileName ="winow.7z";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->create(fileName,mimeType,true,nullptr,true,false);
    delete archive;
}


TEST(ArchiveManager_createInterface_UT, Archive_createInterface_UT005)
{
     ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
     QString fileName;
     QString mimeType = "7z";
     fileName ="winow.7z";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->createInterface(fileName,mimeType);
    delete archive;
}

TEST(ArchiveManager_createInterface_UT, Archive_createInterface_UT006)
{
     PluginManager plugin;
     QString fileName;
     QString mimeType = "7z";
     fileName ="winow.iso";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    QMimeType subFileMimeType;
    Plugin *a = plugin.preferredPluginFor(subFileMimeType);
    archive->createInterface(fileName,a);
    delete archive;

}

TEST(ArchiveManager_create_UT, Archive_create_UT007)
{
     PluginManager plugin;
     QString fileName;
     QString mimeType = "7z";
     fileName ="winow.iso";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    QMimeType subFileMimeType;
    Plugin *a = plugin.preferredPluginFor(subFileMimeType);
    archive->create(fileName,a);
    delete archive;

}
TEST(ArchiveManager_create_UT, Archive_create_UT008)
{
     PluginManager plugin;
     QString fileName;
     QString mimeType = "7z";
     fileName ="winow.txt";

    Archive *archive =  Archive::createEmpty(fileName,mimeType);

    QMimeDatabase db;
    QMimeType subFileMimeType = db.mimeTypeForFile(fileName);
    Plugin *a = plugin.preferredPluginFor(subFileMimeType);
    archive->create(fileName,a);
    delete archive;
}

TEST(ArchiveManager_batchExtract_UT, Archive_batchExtract_UT001)
{
     PluginManager plugin;
     QString fileName;
     QString destination = "7z";

      Archive *a = Archive::createEmpty(fileName,destination);
     //BatchExtractJob * tmp =   Archive::batchExtract(fileName,destination,false,false,a);
}


TEST(ArchiveManager_create_UT, Archive_create_UT001)
{
     PluginManager plugin;
     QString fileName;
     QString destination = "7z";
     Archive::Entry *parent = new Archive::Entry();
     QVector<Archive::Entry *> entries;
     entries.push_back(parent);
     CompressionOptions  option;
      CreateJob *a = Archive::create(fileName,destination,entries,option);
}

TEST(ArchiveManager_CreateEntryNew_UT, Archive_CreateEntryNew_UT001)
{
    QString file = "/home/arv000/Downloads";
    QFileInfo fi(file);
    QHash<QString, QIcon> *map = new QHash<QString, QIcon>();
    Archive::Entry *entry = new Archive::Entry();
    entry->setIsDirectory(true);
    QString externalPath = fi.path() + QDir::separator();

    Archive::CreateEntryNew(fi.filePath(), entry, externalPath, map);
}

TEST(ArchiveManager_CreateEntryNew_UT, Archive_CreateEntryNew_UT002)
{
    QString file = "./home/arv000/Downloads";
    QFileInfo fi(file);
    QHash<QString, QIcon> *map = new QHash<QString, QIcon>();
    Archive::Entry *entry = new Archive::Entry();
    entry->setIsDirectory(true);
    QString externalPath = fi.path() + QDir::separator();

    Archive::CreateEntryNew(fi.filePath(), entry, externalPath, map);
}


TEST(ArchiveManager_createEmpty_UT, Archive_createEmpty_UT001)
{
    QString file = "./home/arv000/Downloads";
    QFileInfo fi(file);
    QString mimeType = "zip";
    QString externalPath = fi.path() + QDir::separator();

    Archive::createEmpty(fi.filePath(),mimeType);
}

TEST(ArchiveManager_load_UT, Archive_load_UT001)
{
    QString file = "./home/arv000/Downloads";

    QString mimeType = "zip";
    QWidget *parent =new QWidget();
    Archive::load(file,parent);
}

TEST(ArchiveManager_load_UT, Archive_load_UT002)
{
    QString file = "./home/arv000/Downloads";

    QString mimeType = "zip";
    QWidget *parent =new QWidget();
    Archive::load(file,mimeType,nullptr);
}

TEST(ArchiveManager_load_UT, Archive_load_UT003)
{

    PluginManager plugin;
    QMimeType subFileMimeType;
    Plugin *a = plugin.preferredPluginFor(subFileMimeType);
    QString file = "./home/arv000/Downloads";

    QString mimeType = "zip";
    QWidget *parent =new QWidget();
    Archive::load(file,mimeType,a);
}

TEST(ArchiveManager_load_UT, Archive_load_UT004)
{
   QString file = "./home/arv000/Downloads";
   Archive::load(file,false);
}

TEST(ArchiveManager_onCompressionMethodFound_UT, Archive_onCompressionMethodFound_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="winow.txt";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->onCompressionMethodFound("sort");
}


TEST(ArchiveManager_onEncryptionMethodFound_UT, Archive_onEncryptionMethodFound_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="winow.txt";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->onCompressionMethodFound("sort");
}

TEST(ArchiveManager_completeBaseName_UT, Archive_completeBaseName_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.tar.tar";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->completeBaseName();
}

TEST(ArchiveManager_completeBaseName_UT, Archive_completeBaseName_UT002)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.7z.tar";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->completeBaseName();
}
TEST(ArchiveManager_completeBaseName_UT, Archive_completeBaseName_UT003)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.zip.tar";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->completeBaseName();
}

TEST(ArchiveManager_completeBaseName_UT, Archive_completeBaseName_UT004)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->completeBaseName();
}


TEST(ArchiveManager_fileName_UT, Archive_fileName_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->fileName();
}

TEST(ArchiveManager_comment_UT, Archive_comment_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";

   Archive *archive =  Archive::createEmpty(fileName,mimeType);
   archive->comment();
}

TEST(ArchiveManager_addComment_UT, Archive_addComment_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";

    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->addComment(fileName);
}

TEST(ArchiveManager_mimeType_UT, Archive_mimeType_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->mimeType();
}

TEST(ArchiveManager_isEmpty_UT, Archive_isEmpty_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->isEmpty();
}

TEST(ArchiveManager_isReadOnly_UT, Archive_isReadOnly_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->isReadOnly();
}


TEST(ArchiveManager_isSingleFile_UT, Archive_isSingleFile_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->isSingleFile();
}

TEST(ArchiveManager_isSingleFolder_UT, Archive_isSingleFolder_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
    archive->isSingleFolder();
}

TEST(ArchiveManager_hasComment_UT, Archive_hasComment_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
   // archive->m_iface = nullptr;
    archive->hasComment();
}

TEST(ArchiveManager_isMultiVolume_UT, Archive_isMultiVolume_UT001)
{
    QString fileName;
    QString mimeType = "7z";
    fileName ="../UnitTest/regress/windows.rar.rar";
    Archive *archive =  Archive::createEmpty(fileName,mimeType);
   // archive->m_iface = nullptr;
    archive->isMultiVolume();
}







