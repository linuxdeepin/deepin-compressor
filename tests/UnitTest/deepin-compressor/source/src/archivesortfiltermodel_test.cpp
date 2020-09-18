#include <gtest/gtest.h>
#include "archivesortfiltermodel.h"
#include <gtest/src/stub.h>
#include <iostream>
#include <QList>
#include <QModelIndex>
#include <archive.h>
#include <archive_entry.h>
#include <archive_manager.h>
#include <QModelIndex>
#include "archivemodel.h"

TEST(ArchiveSortFilterModel_ArchiveSortFilterModel_UT, ArchiveModel_ArchiveSortFilterModel_UT001)
{
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    ASSERT_NE(model, nullptr);
    delete model;
}
// 41行返回  return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT002)
{
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    bool res = model->lessThan(leftModel, rightModel);
    // leftModel.c = -1;
    ASSERT_FALSE(res);
    delete model;
}
// 41行返回  return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT003)
{
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    bool res = model->lessThan(leftModel, rightModel);
    //    rightModel.c = -1;
    ASSERT_FALSE(res);
    delete model;
}

// 52 return false;
Qt::SortOrder MyDescendingOrder()
{
    return Qt::SortOrder::DescendingOrder;
}
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT004)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    Archive::Entry *rightEntry = new Archive::Entry();
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = false;
    armodel->m_showColumns.push_back(10);
    // model->sort(1,Qt::DescendingOrder);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 1;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;
    model->sort(1, Qt::SortOrder::DescendingOrder);
    Stub *stub = new Stub;
    stub->set(ADDR(ArchiveSortFilterModel, sortOrder), MyDescendingOrder);
    armodel->m_showColumns.push_back(1);
    bool res = model->lessThan(leftModel, rightModel);
    stub->reset(ADDR(ArchiveSortFilterModel, sortOrder));
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
    delete stub;
}
// 52 return true;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT005)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    armodel->m_showColumns.push_back(10);
    armodel->m_showColumns.push_back(10);
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    Archive::Entry *rightEntry = new Archive::Entry();
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = false;
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 1;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_TRUE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
// 57   return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT006)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    armodel->m_showColumns.push_back(1);
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    Archive::Entry *rightEntry = new Archive::Entry();
    leftEntry->m_isDirectory = false;
    rightEntry->m_isDirectory = true;
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 0;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 0;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
//117行 FullPath分支     return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT007)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);

    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "你好";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "好你2";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(0);
    armodel->m_showColumns.push_back(0);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 1;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
//115行 FullPath分支     return true;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT008)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "好你2";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "你好";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(0);
    armodel->m_showColumns.push_back(0);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 1;
    rightModel.c = 2;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_TRUE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
//120行 FullPath分支     return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT009)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "好你2";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "fff";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(0);
    armodel->m_showColumns.push_back(0);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 1;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
//120行 FullPath分支     return true;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT010)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "好你2";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(0);
    armodel->m_showColumns.push_back(0);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 1;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_TRUE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
//129行 FullPath分支     return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT011)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "eee";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(0);
    armodel->m_showColumns.push_back(0);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 1;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
//127行 FullPath 分支 return true;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT012)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "eee";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "fff";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(0);
    armodel->m_showColumns.push_back(0);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 1;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 1;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_TRUE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}

// Size 分支 80  return files_l < files_r;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT014)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "eee";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(1);
    armodel->m_showColumns.push_back(2);
    armodel->m_showColumns.push_back(3);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 2;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 2;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
static int c_iDirType15 = 0;
bool getDirType15()
{
    c_iDirType15++;
    if (c_iDirType15 == 8) {
        return false;
    }
    return true;
}
// Size 分支 82 return true;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT015)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "eee";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(1);
    armodel->m_showColumns.push_back(2);
    armodel->m_showColumns.push_back(3);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 2;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 2;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    Stub *stub = new Stub;
    stub->set(ADDR(Archive::Entry, isDir), getDirType15);

    bool res = model->lessThan(leftModel, rightModel);

    stub->reset(ADDR(Archive::Entry, isDir));
    //  rightModel.c = -1;
   // ASSERT_TRUE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
    delete stub;
}
static int c_iDirType16 = 0;
bool getDirType16()
{
    c_iDirType16++;
    if (c_iDirType16 == 8) {
        return false;
    }
    if (c_iDirType16 == 9) {
        return false;
    }
    return true;
}
// Size 分支 84 return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT016)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "eee";
    leftEntry->m_isDirectory = true;
    rightEntry->m_isDirectory = true;
    armodel->m_showColumns.push_back(1);
    armodel->m_showColumns.push_back(2);
    armodel->m_showColumns.push_back(3);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 2;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 2;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;

    Stub *stub = new Stub;
    stub->set(ADDR(Archive::Entry, isDir), getDirType16);

    bool res = model->lessThan(leftModel, rightModel);

    stub->reset(ADDR(Archive::Entry, isDir));
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
    delete stub;
}

// Size 分支 88  return true;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT017)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "eee";
    leftEntry->m_isDirectory = false;
    leftEntry->m_size = 100;
    rightEntry->m_isDirectory = false;
    rightEntry->m_size = 101;
    armodel->m_showColumns.push_back(1);
    armodel->m_showColumns.push_back(2);
    armodel->m_showColumns.push_back(3);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 2;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 2;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;
    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_TRUE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}

// 66行 Type Timestamp  return true;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT018)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "eee";
    leftEntry->m_isDirectory = false;
    leftEntry->m_size = 100;

    QDateTime g = QDateTime::fromString(QString("20200903102002"), "yyyyMMddHHmmss");
    leftEntry->setProperty("timestamp", g);
    rightEntry->m_isDirectory = false;
    rightEntry->m_size = 101;
    QDateTime g2 = QDateTime::fromString(QString("20200903102003"), "yyyyMMddHHmmss");
    rightEntry->setProperty("timestamp", g2);
    armodel->m_showColumns.push_back(1);
    armodel->m_showColumns.push_back(2);
    armodel->m_showColumns.push_back(3);
    armodel->m_showColumns.push_back(1);
    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 3;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 3;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;
    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_TRUE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}
// 139行   return false;
TEST(ArchiveSortFilterModel_lessThan_UT, ArchiveModel_ArchiveSortFilterModel_UT019)
{
    ArchiveModel *armodel = new ArchiveModel(nullptr);
    //armodel->setItemData()
    armodel->m_showColumns.push_back(10);
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex leftModel;
    QModelIndex rightModel;
    Archive::Entry *leftEntry = new Archive::Entry();
    leftEntry->m_name = "fff";
    Archive::Entry *rightEntry = new Archive::Entry();
    rightEntry->m_name = "eee";
    leftEntry->m_isDirectory = false;
    leftEntry->m_size = 100;

    QDateTime g = QDateTime::fromString(QString("20200903102002"), "yyyyMMddHHmmss");
    leftEntry->setProperty("timestamp", g);
    rightEntry->m_isDirectory = false;
    rightEntry->m_size = 101;
    QDateTime g2 = QDateTime::fromString(QString("20200903102003"), "yyyyMMddHHmmss");
    rightEntry->setProperty("timestamp", g2);

    model->setSourceModel(armodel);
    leftModel.r = 1;
    leftModel.c = 0;
    leftModel.m = armodel;
    leftModel.i = (quintptr)leftEntry;
    rightModel.r = 2;
    rightModel.c = 0;
    rightModel.m = armodel;
    rightModel.i = (quintptr)rightEntry;
    bool res = model->lessThan(leftModel, rightModel);
    //  rightModel.c = -1;
    ASSERT_FALSE(res);
    delete armodel;
    delete model;
    delete leftEntry;
    delete rightEntry;
}

TEST(ArchiveSortFilterModel_removeRows_UT, ArchiveModel_ArchiveSortFilterModel_UT020)
{
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex parent;
    bool res = model->removeRows(0, 1, parent);
    ASSERT_TRUE(res);
    delete model;
}

TEST(ArchiveSortFilterModel_refreshNow_UT, ArchiveModel_refreshNow_UT001)
{
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    QModelIndex parent;
    model->refreshNow();
    delete model;
}
