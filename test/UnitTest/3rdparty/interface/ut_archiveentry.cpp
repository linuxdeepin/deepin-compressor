#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include "archiveentry.h"
#include "archive_manager.h"
#include <QModelIndex>

TEST(Archive_Archive_UT, Archive_Archive_UT001)
{
    Archive::Entry *archive = nullptr;
    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    ASSERT_EQ(archive, nullptr);
}

TEST(Archive_Archive_UT, Archive_Archive_UT002)
{
    Archive::Entry *archive = new Archive::Entry();
    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    ASSERT_NE(archive, nullptr);
}

TEST(Archive_copyMetaData_UT, Archive_copyMetaData_UT003)
{
    Archive::Entry *archive = new Archive::Entry();
    Archive::Entry *archive3 = new Archive::Entry();
    archive->setProperty("fullPath", "/home1");
    archive->setProperty("permissions", "/home2");
    archive->setProperty("owner", "/home3");
    archive->setProperty("group", "/home4");
    archive->setProperty("size", 10);
    archive->setProperty("compressedSize", 20);
    archive->setProperty("link", "/home7");
    archive->setProperty("ratio", "/home8");
    archive->setProperty("CRC", "/home9");
    archive->setProperty("BLAKE2", "/home10");
    archive->setProperty("method", "/home11");
    archive->setProperty("version", "/home12");
    archive->setProperty("timestamp", QDateTime::fromString("2020021022", "yyyyMMDDHHIISS"));
    archive->setProperty("isDirectory", "/home14");
    archive->setProperty("isPasswordProtected", "/home15");
    archive3->copyMetaData(archive);

    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    ASSERT_EQ(archive3->property("fullPath"), "/home1");
    ASSERT_EQ(archive3->property("permissions"), "/home2");
    ASSERT_EQ(archive3->property("owner"), "/home3");
    ASSERT_EQ(archive3->property("group"), "/home4");
    ASSERT_EQ(archive3->property("size"), 10);
    ASSERT_EQ(archive3->property("compressedSize"), 20);
    ASSERT_EQ(archive3->property("link"), "/home7");
    ASSERT_EQ(archive3->property("ratio"), "/home8");
    ASSERT_EQ(archive3->property("CRC"), "/home9");
    ASSERT_EQ(archive3->property("BLAKE2"), "/home10");
    ASSERT_EQ(archive3->property("method"), "/home11");
    ASSERT_EQ(archive3->property("version"), "/home12");
    ASSERT_EQ(archive3->property("timestamp"), QDateTime::fromString("2020021022", "yyyyMMDDHHIISS"));
    ASSERT_EQ(archive3->property("isDirectory"), "/home14");
    ASSERT_EQ(archive3->property("isPasswordProtected"), "/home15");
    delete archive;
    delete archive3;
}

//TEST(Archive_entries_UT, Archive_entries_UT004)
//{
//    Archive::Entry *archive = new Archive::Entry();
//    Archive::Entry *archive2 = new Archive::Entry();
//    Archive::Entry *archive3 = new Archive::Entry();
//    QVector<Archive::Entry *> archivelist;
//    archivelist.append(archive2);
//    archivelist.append(archive3);
//    archive->setIsDirectory(true);
//    archive->appendEntry(archive2);
//    archive->appendEntry(archive3);
//    QVector<Archive::Entry *> res = archive->entries();
//    ASSERT_EQ(res, archivelist);
//    delete archive;
//    // delete  archive2;
//    // delete  archive3;
//    for (auto it = archivelist.begin(); it != archivelist.end(); ++it) {
//        if (*it != nullptr) {
//            delete (*it);
//            (*it) = nullptr;
//        }
//    }

//    //   const QVector<Archive::Entry *> archive4 =  archive->entries();
//}
bool getIsDir()
{
    return true;
}
TEST(Archive_entries_UT, Archive_entries_UT005)
{
    Archive::Entry *archiveParent = new Archive::Entry(nullptr, "/home/");

    const Archive::Entry *archive = new Archive::Entry(archiveParent, "/home/", "/home");
    Stub *stub = new Stub;
    stub->set(ADDR(Archive::Entry, isDir), getIsDir);
    //archiveParent->appendEntry(archive);
    const QVector<Archive::Entry *> archivelist;
    //  archive->setIsDirectory(true);
    const QVector<Archive::Entry *> res = archiveParent->entries();
    ASSERT_EQ(res, archivelist);
    delete archiveParent;
    //delete const_cast<Archive::Entry *>(archive);
    delete stub;
}

//TEST(Archive_appendEntry_UT, Archive_appendEntry_UT006)
//{
//    Archive::Entry *archive = new Archive::Entry();
//    Archive::Entry *archive2 = new Archive::Entry();
//    Archive::Entry *archive3 = new Archive::Entry();
//    archive->setIsDirectory(true);
//    archive->appendEntry(archive2);
//    archive->appendEntry(archive3);
//    ASSERT_EQ(2, archive->m_iIndex);
//    delete archive;
//    delete archive2;
//    delete archive3;
//}

//TEST(Archive_removeEntryAt_UT, Archive_removeEntryAt_UT007)
//{
//    Archive::Entry *archive = new Archive::Entry();
//    Archive::Entry *archive2 = new Archive::Entry();
//    archive2->setFullPath("/home1");
//    Archive::Entry *archive3 = new Archive::Entry();
//    archive3->setFullPath("/home2");
//    archive->setIsDirectory(true);
//    archive->appendEntry(archive2);
//    archive->appendEntry(archive3);
//    archive->removeEntryAt(0);
//    ASSERT_EQ(1, archive->m_iIndex);

//    delete archive;
//    delete archive2;
//    delete archive3;
//}

TEST(Archive_getParent_UT, Archive_getParent_UT008)
{
    Archive::Entry *archive = new Archive::Entry();
    Archive::Entry *Parent = new Archive::Entry();
    archive->setParent(Parent);
    ASSERT_EQ(Parent, archive->getParent());

    delete archive;
    delete Parent;
}

TEST(Archive_setParent_UT, Archive_setParent_UT009)
{
    Archive::Entry *archive = new Archive::Entry();
    Archive::Entry *Parent = new Archive::Entry();
    archive->setParent(Parent);
    ASSERT_EQ(Parent, archive->getParent());
    delete archive;
    delete Parent;
}

TEST(Archive_getSize_UT, Archive_getSize_UT010)
{
    Archive::Entry *archive = new Archive::Entry();
    archive->m_size = 100;
    qint64 size = archive->getSize();
    ASSERT_EQ(100, size);
    delete archive;
}
// 134 行
TEST(Archive_calAllSize_UT, Archive_calAllSize_UT011)
{
    Archive::Entry *archive = new Archive::Entry();
    archive->setIsDirectory(false);
    archive->setSize(101);
    qint64 size = 0;
    archive->calAllSize(size);
    ASSERT_EQ(101, size);
    delete archive;
}
//// 148行
//TEST(Archive_calAllSize_UT, Archive_calAllSize_UT012)
//{
//    Archive::Entry *archiveDir = new Archive::Entry();
//    Archive::Entry *archive1 = new Archive::Entry();
//    Archive::Entry *archiveDir2 = new Archive::Entry();
//    Archive::Entry *archive2 = new Archive::Entry();
//    archiveDir->setIsDirectory(true);
//    archiveDir2->setIsDirectory(true);
//    archiveDir->appendEntry(archive1);
//    archive1->setSize(65);
//    archiveDir->appendEntry(archiveDir2);
//    archiveDir2->appendEntry(archive2);
//    archive2->setSize(100);
//    //archiveDir->appendEntry(archive2);
//    qint64 rsize = 0;
//    archiveDir->calAllSize(rsize);
//    ASSERT_EQ(165, rsize);

//    delete archiveDir;
//    delete archive1;
//    delete archiveDir2;
//    delete archive2;
//}

//// 152 行
//TEST(Archive_calEntriesCount_UT, Archive_calEntriesCount_UT013)
//{
//    Archive::Entry *archiveDir = new Archive::Entry();
//    Archive::Entry *archive1 = new Archive::Entry();
//    Archive::Entry *archiveDir2 = new Archive::Entry();
//    Archive::Entry *archive2 = new Archive::Entry();
//    archiveDir->setIsDirectory(true);
//    archiveDir2->setIsDirectory(true);
//    archiveDir->appendEntry(archive1);
//    archiveDir->appendEntry(archiveDir2);
//    archiveDir2->appendEntry(archive2);
//    qint64 rcount = 0;
//    archiveDir->calEntriesCount(rcount);
//    ASSERT_EQ(4, rcount);
//    delete archiveDir;
//    delete archive1;
//    delete archiveDir2;
//    delete archive2;
//}

//
TEST(Archive_setSize_UT, Archive_setSize_UT014)
{
    Archive::Entry *archive1 = new Archive::Entry();
    qint64 rsize = 100;
    archive1->setSize(rsize);
    ASSERT_EQ(100, archive1->m_size);
    delete archive1;
}

TEST(Archive_setFullPath_UT, Archive_setFullPath_UT015)
{
    Archive::Entry *archive1 = new Archive::Entry();
    QString path = "/home/asdf/a.cpp";
    archive1->setFullPath(path);
    ASSERT_EQ(archive1->m_fullPath, path);
    ASSERT_EQ(archive1->m_name, "a.cpp");
    delete archive1;
}

TEST(Archive_fullPath_UT, Archive_fullPath_UT016)
{
    Archive::Entry *archive1 = new Archive::Entry();
    QString path = "/home/asdf/";
    archive1->setFullPath(path);
    QString rpath = archive1->fullPath(NoTrailingSlash);
    ASSERT_EQ(rpath, "/home/asdf");
    delete archive1;
}

TEST(Archive_fullPath_UT, Archive_fullPath_UT017)
{
    Archive::Entry *archive1 = new Archive::Entry();
    QString path = "/home/asdf/a.cpp";
    archive1->setFullPath(path);
    QString rpath = archive1->fullPath();
    ASSERT_EQ(rpath, "/home/asdf/a.cpp");
    delete archive1;
}

TEST(Archive_name_UT, Archive_name_UT018)
{
    Archive::Entry *archive1 = new Archive::Entry();
    archive1->setFullPath("/home/asdf/a.cpp");
    QString name = archive1->name();
    ASSERT_EQ(name, "a.cpp");
    delete archive1;
}
TEST(Archive_setIsDirectory_UT, Archive_setIsDirectory_UT019)
{
    Archive::Entry *archive1 = new Archive::Entry();
    archive1->setIsDirectory(true);
    ASSERT_EQ(archive1->isDir(), true);
    delete archive1;
}

TEST(Archive_isDir_UT, Archive_isDir_UT020)
{
    Archive::Entry *archive1 = new Archive::Entry();
    archive1->setIsDirectory(true);
    ASSERT_EQ(archive1->isDir(), true);
    delete archive1;
}

//TEST(Archive_row_UT, Archive_row_UT021)
//{
//    Archive::Entry *archiveDir = new Archive::Entry();
//    Archive::Entry *archive1 = new Archive::Entry();
//    Archive::Entry *archiveDir2 = new Archive::Entry();
//    Archive::Entry *archive2 = new Archive::Entry();
//    Archive::Entry *archive3 = new Archive::Entry();
//    archiveDir->setIsDirectory(true);
//    archiveDir2->setIsDirectory(true);

//    archiveDir->appendEntry(archive1);
//    archive1->setParent(archiveDir);
//    archiveDir->appendEntry(archiveDir2);
//    archiveDir2->setParent(archiveDir);

//    archiveDir2->appendEntry(archive2);
//    archive2->setParent(archiveDir2);
//    archiveDir2->appendEntry(archive3);
//    archive3->setParent(archiveDir2);

//    int rrow1 = archiveDir->row();
//    int rrow2 = archive1->row();
//    int rrow3 = archiveDir2->row();
//    int rrow4 = archive3->row();
//    int rrow5 = archive2->row();
//    ASSERT_EQ(rrow1, 0);
//    ASSERT_EQ(rrow2, 0);
//    ASSERT_EQ(rrow3, 1);
//    ASSERT_EQ(rrow4, 1);
//    ASSERT_EQ(rrow5, 0);

//    delete archiveDir;
//    delete archive1;
//    delete archiveDir2;
//    delete archive3;
//    delete archive2;
//}

//TEST(Archive_find_UT, Archive_find_UT022)
//{
//    Archive::Entry *archiveDir = new Archive::Entry();
//    archiveDir->setFullPath("/home");
//    Archive::Entry *archive1 = new Archive::Entry();
//    archive1->setFullPath("/home/index.php");
//    Archive::Entry *archiveDir2 = new Archive::Entry();
//    archiveDir2->setFullPath("/home/www/");
//    Archive::Entry *archive2 = new Archive::Entry();
//    archive2->setFullPath("/home/www/a.html");
//    Archive::Entry *archive3 = new Archive::Entry();
//    archive3->setFullPath("/home/www/b.html");
//    archiveDir->setIsDirectory(true);
//    archiveDir2->setIsDirectory(true);
//    archiveDir->appendEntry(archive1);
//    archive1->setParent(archiveDir);
//    archiveDir->appendEntry(archiveDir2);
//    archiveDir2->setParent(archiveDir);
//    archiveDir2->appendEntry(archive2);
//    archive2->setParent(archiveDir2);
//    archiveDir2->appendEntry(archive3);
//    archive3->setParent(archiveDir2);
//    Archive::Entry *rEntry = archiveDir->find("index.php");

//    ASSERT_NE(rEntry, nullptr);

//    Archive::Entry *rEntry2 = archiveDir->find("indexa.php");
//    ASSERT_EQ(rEntry2, nullptr);
//    delete archiveDir;
//    delete archive1;
//    delete archiveDir2;
//    delete archive3;
//    delete archive2;
//}
// 230行
TEST(Archive_findByPath_UT, Archive_findByPath_UT023)
{
    QStringList pieces;
    pieces.append("home");
    pieces.append("temp");
    Archive::Entry *archive = new Archive::Entry();
    Archive::Entry *rarchive = archive->findByPath(pieces, 2);
    ASSERT_EQ(rarchive, nullptr);
    delete archive;
    delete rarchive;
}
//// 235行
//TEST(Archive_findByPath_UT, Archive_findByPath_UT024)
//{
//    QStringList pieces;
//    pieces.append("home");
//    pieces.append("temp");
//    Archive::Entry *archive = new Archive::Entry();
//    Archive::Entry *archive1 = new Archive::Entry();
//    archive1->setFullPath("/home/index.php");
//    archive->setIsDirectory(true);
//    archive->appendEntry(archive1);

//    Archive::Entry *rarchive = archive->findByPath(pieces, 1);
//    ASSERT_EQ(rarchive, nullptr);
//    delete archive;
//    delete archive1;
//    delete rarchive;
//}

//// 242行
//TEST(Archive_findByPath_UT, Archive_findByPath_UT025)
//{
//    QStringList pieces;
//    pieces.append("home");
//    pieces.append("temp");
//    Archive::Entry *archive = new Archive::Entry();
//    Archive::Entry *archive1 = new Archive::Entry();
//    archive1->setFullPath("/home/index.php");
//    archive->setIsDirectory(true);
//    archive->appendEntry(archive1);

//    Archive::Entry *rarchive = archive->findByPath(pieces, 0);
//    ASSERT_EQ(rarchive, nullptr);
//    delete archive;
//    delete archive1;
//    delete rarchive;
//}

//TEST(Archive_findByPath_UT, Archive_findByPath_UT026)
//{
//    QStringList pieces;
//    pieces.append("index.php");
//    pieces.append("temp");
//    Archive::Entry *archive = new Archive::Entry();
//    archive->setIsDirectory(true);
//    Archive::Entry *archive1 = new Archive::Entry();
//    archive1->setFullPath("/home/index.php");
//    archive->setIsDirectory(true);
//    archive->appendEntry(archive1);

//    Archive::Entry *rarchive = archive->findByPath(pieces, 0);
//    ASSERT_EQ(rarchive, nullptr);
//    delete archive;
//    delete archive1;
//    delete rarchive;
//}

TEST(Archive_countChildren_UT, Archive_countChildren_UT027)
{
    Archive::Entry *archive = new Archive::Entry();
    archive->setIsDirectory(false);
    uint dirs = 1;
    uint files = 1;
    archive->countChildren(dirs, files);
    ASSERT_EQ(dirs, 0);
    ASSERT_EQ(files, 0);
    delete archive;
}

//TEST(Archive_countChildren_UT, Archive_countChildren_UT028)
//{
//    Archive::Entry *archiveParent = new Archive::Entry();
//    Archive::Entry *archivedir1 = new Archive::Entry();
//    Archive::Entry *archivedir2 = new Archive::Entry();
//    Archive::Entry *archivefile1 = new Archive::Entry();
//    Archive::Entry *archivefile2 = new Archive::Entry();
//    Archive::Entry *archivefile3 = new Archive::Entry();
//    archiveParent->setIsDirectory(true);
//    archivedir1->setIsDirectory(true);
//    archivedir2->setIsDirectory(true);
//    archiveParent->appendEntry(archivedir1);
//    archiveParent->appendEntry(archivedir2);
//    archiveParent->appendEntry(archivefile1);
//    archiveParent->appendEntry(archivefile2);
//    archiveParent->appendEntry(archivefile3);
//    uint dirs = 0;
//    uint files = 0;
//    archiveParent->countChildren(dirs, files);
//    ASSERT_EQ(dirs, 2);
//    ASSERT_EQ(files, 3);

//    delete archiveParent;
//    delete archivedir1;
//    delete archivedir2;
//    delete archivefile1;
//    delete archivefile2;
//    delete archivefile3;
//}

TEST(Archive_getAllNodesFullPath_UT, Archive_getAllNodesFullPath_UT029)
{
    Archive::Entry *archiveParent = new Archive::Entry();
    archiveParent->setFullPath("/home/temp/id");
    archiveParent->setIsDirectory(false);
    delete archiveParent;
}

//TEST(Archive_getAllNodesFullPath_UT, Archive_getAllNodesFullPath_UT030)
//{
//    Archive::Entry *archiveParent = new Archive::Entry();
//    Archive::Entry *archiveSub = new Archive::Entry();
//    Archive::Entry *archiveSubDir = new Archive::Entry();
//    Archive::Entry *archiveSubDir_file = new Archive::Entry();
//    archiveParent->setIsDirectory(true);
//    archiveSubDir->setIsDirectory(true);

//    archiveParent->setFullPath("/home/temp");
//    archiveSubDir->setFullPath("/home/temp/id");
//    archiveSubDir_file->setFullPath("/home/temp/id/b.php");
//    archiveSub->setFullPath("/home/temp/a.php");

//    archiveParent->appendEntry(archiveSubDir);
//    archiveParent->appendEntry(archiveSub);

//    archiveSubDir->appendEntry(archiveSubDir_file);

//    QStringList pList;
//    archiveParent->getAllNodesFullPath(pList);
//    delete archiveParent;
//    delete archiveSub;
//    delete archiveSubDir;
//    delete archiveSubDir_file;
//}

//TEST(Archive_getFilesCount_UT, Archive_getFilesCount_UT031)
//{
//    Archive::Entry *archiveParent = new Archive::Entry();
//    Archive::Entry *archiveSub = new Archive::Entry();
//    Archive::Entry *archiveSubDir = new Archive::Entry();
//    Archive::Entry *archiveSubDir_file = new Archive::Entry();
//    archiveParent->setIsDirectory(true);
//    archiveSubDir->setIsDirectory(true);
//    archiveParent->setFullPath("/home/temp");
//    archiveSubDir->setFullPath("/home/temp/id");
//    archiveSubDir_file->setFullPath("/home/temp/id/b.php");
//    archiveSub->setFullPath("/home/temp/a.php");
//    archiveParent->appendEntry(archiveSubDir);
//    archiveParent->appendEntry(archiveSub);
//    archiveSubDir->appendEntry(archiveSubDir_file);
//    int rcount = 0;
//    archiveParent->getFilesCount(archiveSubDir, rcount);
//    ASSERT_EQ(rcount, 1);
//    delete archiveParent;
//    delete archiveSub;
//    delete archiveSubDir;
//    delete archiveSubDir_file;
//}
// 288行
TEST(Archive_getFilesCount_UT, Archive_getFilesCount_UT032)
{
    Archive::Entry *archiveParent = new Archive::Entry();

    Archive::Entry *archiveSubDir_file = new Archive::Entry();
    archiveParent->setIsDirectory(true);
    archiveParent->setFullPath("/home/temp");
    archiveSubDir_file->setFullPath("/home/temp/id/b.php");
    int rcount = 0;
    archiveParent->getFilesCount(archiveSubDir_file, rcount);
    ASSERT_EQ(rcount, 1);
    delete archiveParent;
    delete archiveSubDir_file;
}

//TEST(Archive_getVector_UT, Archive_getVector_UT033)
//{
//    Archive::Entry *archiveParent = new Archive::Entry();
//    Archive::Entry *archiveSub = new Archive::Entry();
//    Archive::Entry *archiveSubDir = new Archive::Entry();
//    Archive::Entry *archiveSubDir_file = new Archive::Entry();
//    archiveParent->setIsDirectory(true);
//    archiveSubDir->setIsDirectory(true);
//    archiveParent->setFullPath("/home/temp");
//    archiveSubDir->setFullPath("/home/temp/id");
//    archiveSubDir_file->setFullPath("/home/temp/id/b.php");
//    archiveSub->setFullPath("/home/temp/a.php");
//    archiveParent->appendEntry(archiveSubDir);
//    archiveParent->appendEntry(archiveSub);
//    archiveSubDir->appendEntry(archiveSubDir_file);
//    QVector<Archive::Entry *> vector;
//    archiveParent->getVector(archiveParent, vector);
//    int rcount = vector.count();
//    ASSERT_EQ(rcount, 4);
//    for (auto it = vector.begin(); it != vector.end(); ++it) {
//        if (*it != nullptr) {
//            delete (*it);
//            (*it) = nullptr;
//        }
//    }
//}

TEST(Archive_operator_UT, Archive_operator_UT034)
{
    Archive::Entry *archiveParent = new Archive::Entry();
    const Archive::Entry *archiveSub = new Archive::Entry(nullptr, "/home");
    archiveParent->setFullPath("/home");
    //  archiveSub->setFullPath("/home");
    bool r = *archiveParent == *archiveSub;

    ASSERT_EQ(r, true);
    delete (archiveParent);
}
static int i33 = 0;
bool getIsDir2()
{
    if (i33 == 1) {
        return true;
    }
    i33++;
    return false;
}

TEST(Archive_clean_UT, Archive_clean_UT035)
{
    Archive::Entry *archiveSubDir_file = new Archive::Entry();
    archiveSubDir_file->setFullPath("/home/temp/id/b.php");
    Stub *stub = new Stub;
    stub->set(ADDR(Archive::Entry, isDir), getIsDir2);
    archiveSubDir_file->clean();
    ASSERT_NE(archiveSubDir_file, nullptr);
    // delete archiveSubDir_file;
}

//TEST(Archive_clean_UT, Archive_clean_UT036)
//{
//    Archive::Entry *archiveParent = new Archive::Entry();
//    Archive::Entry *archiveSub = new Archive::Entry();
//    Archive::Entry *archiveSubDir = new Archive::Entry();
//    Archive::Entry *archiveSubDir_file = new Archive::Entry();
//    archiveParent->setIsDirectory(true);
//    archiveSubDir->setIsDirectory(true);
//    archiveParent->setFullPath("/home/temp");
//    archiveSubDir->setFullPath("/home/temp/id");
//    archiveSubDir_file->setFullPath("/home/temp/id/b.php");
//    archiveSub->setFullPath("/home/temp/a.php");
//    archiveParent->appendEntry(archiveSubDir);
//    archiveParent->appendEntry(archiveSub);
//    archiveSubDir->appendEntry(archiveSubDir_file);
//    archiveParent->clean();
//    ASSERT_NE(archiveParent, nullptr);
//}

TEST(Archive_operator_UT, Archive_operator_UT037)
{
    //Archive::Entry *archiveParent = new Archive::Entry();
    Archive::Entry archiveSub;
    archiveSub.rootNode = "/home";
    qDebug() << archiveSub;
}
TEST(Archive_operator_UT, Archive_operator_UT038)
{
    Archive::Entry *archiveParent = new Archive::Entry();

    archiveParent->rootNode = "/home";
    qDebug() << archiveParent;
    delete archiveParent;
}
