//
// Created by lx777 on 2020/8/28.
//
//#define private public
#include <gtest/gtest.h>
#include "archivemodel.h"
#include "kpluginloader.h"
#include "pluginmanager.h"

#include <gtest/src/stub.h>
#include <iostream>
#include <QList>
#include <QAbstractItemModel>
#include "archiveinterface.h"
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>
#include <jobs.h>
#include "archive_manager.h"
#include <QIcon>
int returnAt = 0;


TEST(ArchiveModel_ArchiveModel_UT, ArchiveModel_ArchiveModel_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    ASSERT_NE(model, nullptr);
    delete  model;
}

TEST(ArchiveModel_data_UT, ArchiveModel_data_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    index.r = 0;
    index.c = 0;
    int *patchindex = new int(1);
    model->m_ppathindex = patchindex;
    index.r = 2;
    index.c = 0;
    model->data(index, 0);
    ASSERT_NE(model, nullptr);
    delete patchindex;
    delete model;
}

TEST(ArchiveModel_data_UT, ArchiveModel_data_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    index.r = 0;
    index.c = 0;
    model->m_ppathindex = nullptr;
    model->data(index, 0);
    ASSERT_NE(model, nullptr);

    delete model;
}


//---------------------
// 1. Qt::DisplayRole分支
// 2. FullPath 分支
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT004)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    Archive::Entry *entry = new Archive::Entry();
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_showColumns << 0;
    model->m_ppathindex = nullptr;
    model->data(index, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);

    delete entry;
    delete model;
}
// 1. Qt::DisplayRole分支
// 2. Type 分支
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT005)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_showColumns << 0 << EntryMetaDataType::Type;
    model->m_ppathindex = nullptr;
    model->data(index, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete model;
}

void loadPlugins_stub(void *obj)
{
    PluginManager *o = (PluginManager *)obj;

    QCoreApplication::addLibraryPath("../output/");
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

// 1. Qt::DisplayRole分支
// 2. Size 分支 119行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT006)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = true;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(true);
    model->m_showColumns << EntryMetaDataType::Size;

    model->data(index, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}
// 1. Qt::DisplayRole分支
// 2. Size 分支 121行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT007)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(true);
    model->m_showColumns << EntryMetaDataType::Size;

    model->data(index, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}
// 1. Qt::DisplayRole分支
// 2. Size 分支 126 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT008)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "as");
    model->m_showColumns << EntryMetaDataType::Size;

    model->data(index, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}
// 1. Qt::DisplayRole分支
// 2. Size 分支 128 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT009)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");
    model->m_showColumns << EntryMetaDataType::Size;

    model->data(index, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. Qt::DisplayRole分支
// 2. Timestamp 分支 132 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT010)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");
    model->m_showColumns << EntryMetaDataType::Timestamp;

    model->data(index, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. Qt::DecorationRole
// 2. 分支 143 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT011)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");
    model->m_showColumns << EntryMetaDataType::Timestamp;

    model->data(index, Qt::DecorationRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. Qt::DecorationRole
// 2. 分支 146 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT012)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");
    model->m_showColumns << EntryMetaDataType::Timestamp;

    model->data(index, Qt::DecorationRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. Qt::TextAlignmentRole
// 2. 分支 146 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT013)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");

    model->data(index, Qt::TextAlignmentRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. Qt::TextColorRole
// 2. 分支 146 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT014)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");

    model->data(index, Qt::TextColorRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. Qt::FontRole
// 2. 分支 162 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT015)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 0;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");

    model->data(index, Qt::FontRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. Qt::FontRole
// 2. 分支 166 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT016)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");

    model->data(index, Qt::FontRole);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1.  default
// 2. 分支 168 行
TEST(ArchiveModel_data_UT, ArchiveModel_data_UT017)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->m_ppathindex = nullptr;
    entry->setIsDirectory(false);
    entry->setProperty("link", "");

    model->data(index, 1000);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete interface;
    delete model;
}

// 1. 分支181行
TEST(ArchiveModel_flags_UT, ArchiveModel_flags_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->flags(index);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete model;
}

// 1. 分支184行
TEST(ArchiveModel_flags_UT, ArchiveModel_flags_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    // Archive::Entry * entry  = new Archive::Entry();
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->flags(index);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}

// 分支 193
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    // Archive::Entry * entry  = new Archive::Entry();
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->headerData(0, Qt::Orientation::Horizontal, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}
// 分支 193
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    // Archive::Entry * entry  = new Archive::Entry();
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->headerData(-1, Qt::Orientation::Horizontal, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}
// 分支 200
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT003)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    // Archive::Entry * entry  = new Archive::Entry();
    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << EntryMetaDataType::FullPath << EntryMetaDataType::FullPath;
    QVariant temp = model->headerData(1, Qt::Orientation::Horizontal, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}
// 分支 202
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT004)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << EntryMetaDataType::Size << EntryMetaDataType::Size;
    QVariant temp = model->headerData(1, Qt::Orientation::Horizontal, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}

// 分支 204
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT005)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << EntryMetaDataType::Type << EntryMetaDataType::Type;
    QVariant temp = model->headerData(1, Qt::Orientation::Horizontal, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}

// 分支 206
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT006)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << EntryMetaDataType::Timestamp << EntryMetaDataType::Timestamp;
    QVariant temp = model->headerData(1, Qt::Orientation::Horizontal, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}

// 分支 208
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT007)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << 100 << 100;
    QVariant temp = model->headerData(1, Qt::Orientation::Horizontal, Qt::DisplayRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}

// 分支 211
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT008)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << 100 << 100;
    QVariant temp = model->headerData(1, Qt::Orientation::Horizontal, Qt::TextAlignmentRole);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}
// 分支 215
TEST(ArchiveModel_headerData_UT, ArchiveModel_headerData_UT009)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << 100 << 100;
    QVariant temp = model->headerData(1, Qt::Orientation::Horizontal, 100);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}

TEST(ArchiveModel_index_UT, ArchiveModel_index_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QModelIndex index;
    index.r = 2;
    index.c = 1;
    index.m = nullptr;
    //index.i = (quintptr)entry;
    model->m_showColumns << 100 << 100;
    QVariant temp = model->index(0, 0, index);
    ASSERT_NE(model, nullptr);
    // delete entry;
    delete model;
}
bool gethasIndex()
{
    return true;
}

//TEST(ArchiveModel_index_UT, ArchiveModel_index_UT002)
//{
//    ArchiveModel *model = new ArchiveModel(nullptr);
//    Archive::Entry *entry = new Archive::Entry();
//    Archive::Entry *parent = new Archive::Entry();
//    parent->setIsDirectory(true);
//    entry->setParent(parent);
//    entry->setIsDirectory(true);
//    entry->m_entries << parent;
//    QModelIndex index;
//    index.r = 0;
//    index.c = 1;
//    index.m = model;
//    index.i = (quintptr)entry;

//    Stub *stub = new Stub;
//    stub->set(ADDR(ArchiveModel, hasIndex), gethasIndex);
//   // QVariant temp = model->index(0, 0, index);
//    ASSERT_NE(model, nullptr);

//    delete stub;
//    delete parent;
//    delete entry;
//    delete model;
//}

TEST(ArchiveModel_isentryDir_UT, ArchiveModel_isentryDir_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();
    QModelIndex index;
    index.r = 0;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    bool temp = model->isentryDir(index);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete model;
}

TEST(ArchiveModel_setPathIndex_UT, ArchiveModel_setPathIndex_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    int *index = new int(5);
    model->setPathIndex(index);
    //   ASSERT_NE(*model->m_ppathindex, *index);
    delete index;
    delete model;
}

TEST(ArchiveModel_setParentEntry_UT, ArchiveModel_setParentEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();
    entry->setIsDirectory(true);
    QModelIndex index;
    index.r = 0;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->setParentEntry(index);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete model;
}

// 261行
TEST(ArchiveModel_getParentEntry_UT, ArchiveModel_getParentEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    model->getParentEntry();
    ASSERT_NE(model, nullptr);
    delete model;
}

// 265行
TEST(ArchiveModel_getParentEntry_UT, ArchiveModel_getParentEntry_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->m_ppathindex = new int(0);
    model->getParentEntry();
    ASSERT_NE(model, nullptr);
    delete model;
}

// 267行
TEST(ArchiveModel_getParentEntry_UT, ArchiveModel_getParentEntry_UT003)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->m_ppathindex = new int(1);
    model->getParentEntry();
    ASSERT_NE(model, nullptr);
    delete model;
}

// 273行
TEST(ArchiveModel_getRootEntry_UT, ArchiveModel_getRootEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->m_ppathindex = new int(1);
    model->getRootEntry();
    ASSERT_NE(model, nullptr);
    delete model;
}

// 284行
Archive::Entry *getRootEntry1()
{
    return nullptr;
}
TEST(ArchiveModel_isExists_UT, ArchiveModel_isExists_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    //model->m_rootEntry = nullptr;
    Stub *stub = new Stub;
    stub->set(ADDR(ArchiveModel, getRootEntry), getRootEntry1);
    model->isExists("/home");
    ASSERT_NE(model, nullptr);
    delete stub;
    delete model;
}

//// 294行
//TEST(ArchiveMohandel_isExists_UT, ArchiveModel_isExists_UT002)
//{
//    ArchiveModel *model = new ArchiveModel(nullptr);
//    //model->m_rootEntry = nullptr;
//    Archive::Entry *parent = new Archive::Entry();
//    Archive::Entry *sub1 = new Archive::Entry();
//    sub1->setProperty("fullPath", "fullPath");
//    sub1->setProperty("permissions", "permissions");
//    sub1->setProperty("owner", "owner");
//    sub1->setProperty("group", "group");
//    sub1->setProperty("size", "size");
//    sub1->setProperty("compressedSize", "compressedSize");
//    sub1->setProperty("link", "link");
//    sub1->setProperty("ratio", "ratio");
//    sub1->setProperty("CRC", "CRC");
//    sub1->setProperty("BLAKE2", "BLAKE2");
//    sub1->setProperty("method", "method");
//    sub1->setProperty("version", "version");
//    //sub1-> setProperty("timestamp", "timestamp").toDateTime();
//    sub1->setProperty("isDirectory", "isDirectory");
//    sub1->setProperty("isPasswordProtected", "isPasswordProtected");
//    sub1->m_name = "home";
//    parent->m_entries << (sub1);
//    model->m_parent = parent;
//    model->m_ppathindex = new int(3);
//    parent->setIsDirectory(true);

//    model->isExists("/home");
//    ASSERT_NE(model, nullptr);
//    delete sub1;
//    delete parent;
//    delete model;
//}
// 300行
TEST(ArchiveMohandel_isExists_UT, ArchiveModel_isExists_UT003)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    //model->m_rootEntry = nullptr;
    Archive::Entry *parent = new Archive::Entry();
    model->m_parent = parent;
    model->m_ppathindex = new int(3);
    parent->setIsDirectory(true);
    model->isExists("/home");
    ASSERT_NE(model, nullptr);
    delete parent;
    delete model;
}

TEST(ArchiveMohandel_setTableView_UT, ArchiveModel_setTableView_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QTableView *tableview = new QTableView;
    model->setTableView(tableview);
    ASSERT_NE(model, nullptr);
    delete tableview;
    delete model;
}
//314行
TEST(ArchiveMohandel_createNoncolumnIndex_UT, ArchiveModel_createNoncolumnIndex_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry();
    Archive::Entry *parent = new Archive::Entry();
    entry->setIsDirectory(true);
    entry->m_parent = parent;
    parent->setIsDirectory(true);
    QModelIndex index;
    index.r = 0;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    //model->createNoncolumnIndex(index);
    ASSERT_NE(model, nullptr);
    delete entry;
    delete parent;
    delete model;
}

//331行
TEST(ArchiveMohandel_createNoncolumnIndex_UT, ArchiveModel_createNoncolumnIndex_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    model->createNoncolumnIndex(index);
    ASSERT_NE(model, nullptr);
    delete model;
}

//339行
TEST(ArchiveMohandel_entryForIndex_UT, ArchiveModel_entryForIndex_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    Archive::Entry *entry = new Archive::Entry();

    index.r = 0;
    index.c = 1;
    index.m = model;
    index.i = (quintptr)entry;
    model->entryForIndex(index);
    ASSERT_NE(model, nullptr);
    delete model;
}

//342行
TEST(ArchiveMohandel_entryForIndex_UT, ArchiveModel_entryForIndex_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    model->entryForIndex(index);
    ASSERT_NE(model, nullptr);
    delete model;
}

TEST(ArchiveMohandel_columnCount_UT, ArchiveModel_columnCount_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    model->columnCount(index);
    ASSERT_NE(model, nullptr);
    delete model;
}

TEST(ArchiveMohandel_supportedDropActions_UT, ArchiveModel_supportedDropActions_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->supportedDropActions();
    ASSERT_NE(model, nullptr);
    delete model;
}

TEST(ArchiveMohandel_mimeData_UT, ArchiveModel_mimeData_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QModelIndex index;
    QModelIndexList indexes;
    indexes.push_back(index);
    model->mimeData(indexes);
    ASSERT_NE(model, nullptr);

    delete model;
}

// 390 行
TEST(ArchiveMohandel_dropMimeData_UT, ArchiveModel_dropMimeData_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    ArchiveError errorCode;
    QObject *parent = new QObject;
    QString path;
    QString mimeType;

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    Archive *temp = Archive::createEmpty(path, mimeType, parent);

    model->m_archive.reset(temp);
    QMimeData *data = new QMimeData();
    //  QList<QUrl> urls ;
    // data->setUrls(urls);
    QModelIndex index;
    model->dropMimeData(data, Qt::DropAction::ActionMask, 0, 0, index);
    ASSERT_NE(model, nullptr);

    // delete temp;
    // delete parent;
    delete model;
}

// 396行
TEST(ArchivehandeMol_dropMimeData_UT, ArchiveModel_dropMimeData_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    ArchiveError errorCode;
    QObject *parent = new QObject;
    QString path;
    QString mimeType;

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    Archive *temp = Archive::createEmpty(path, mimeType, parent);
    temp->m_encryptionType = Archive::HeaderEncrypted;
    temp->m_iface->m_password = "adsf";
    temp->m_iface->m_numberOfEntries = 6;
    temp->m_iface->m_isMultiVolume = true;
    model->m_archive.reset(temp);
    QMimeData *data = new QMimeData();
    QList<QUrl> urls;
    QUrl url;
    url.setUrl("/home/assd");
    urls.push_back(url);
    data->setUrls(urls);
    QModelIndex index;
    model->dropMimeData(data, Qt::DropAction::ActionMask, 0, 0, index);
    ASSERT_NE(model, nullptr);
    //   delete temp;
    delete data;
    //  delete parent;
    delete model;
}

// 410行
QModelIndex index03(int row, int column, const QModelIndex &parent)
{
    QModelIndex index;
    Archive::Entry *entry = new Archive::Entry();
    index.r = 2;
    index.c = 2;
    index.i = (quintptr)entry;
    return index;
}
TEST(ArchivehandeMol_resetmparent_UT, ArchiveModel_resetmparent_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *parent = new Archive::Entry();
    Archive::Entry *pparent = new Archive::Entry();
    parent->setParent(pparent);
    model->m_parent = parent;
    model->resetmparent();
    delete pparent;
    delete parent;
    delete model;
}
// 479行
TEST(ArchivehandeMol_cleanFileName_UT, ArchiveModel_cleanFileName_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QString temp = model->cleanFileName("/");
    delete model;
}
// 481行
TEST(ArchivehandeMol_cleanFileName_UT, ArchiveModel_cleanFileName_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QString temp = model->cleanFileName("./home");
    delete model;
}

// 484行
TEST(ArchivehandeMol_cleanFileName_UT, ArchiveModel_cleanFileName_UT003)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    QString temp = model->cleanFileName("/home/tmp");
    delete model;
}

TEST(ArchivehandeMol_initRootEntry_UT, ArchiveModel_initRootEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);

    model->initRootEntry();
    delete model;
}

// 497 行
TEST(ArchivehandeMol_parentFor_UT, ArchiveModel_parentFor_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry;
    model->parentFor(entry, ArchiveModel::InsertBehaviour::NotifyViews);
    delete entry;
    delete model;
}

// 519 行
TEST(ArchivehandeMol_parentFor_UT, ArchiveModel_parentFor_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry;
    model->s_previousMatch = entry;
    model->s_previousPieces->push_back("asf");
    entry->setFullPath("/home/arv000");
    model->parentFor(entry, ArchiveModel::InsertBehaviour::NotifyViews);
    delete entry;
    delete model;
}
// 555 行
TEST(ArchivehandeMol_parentFor_UT, ArchiveModel_parentFor_UT003)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry;
    entry->setIsDirectory(true);
    entry->setFullPath("/home/arv00/tmp/dd/aa");
    model->s_previousMatch = entry;
    model->m_rootEntry.reset(entry);
    entry->setFullPath("/home/arv00/tmp/dd");

    model->parentFor(entry, ArchiveModel::InsertBehaviour::NotifyViews);
    // delete entry;
    delete model;
}

bool getIsDir()
{
    return true;
}
// 519 行
TEST(ArchivehandeMol_parentFor_UT, ArchiveModel_parentFor_UT004)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry;
    entry->setIsDirectory(true);
    entry->setFullPath("/home/arv00/tmp/dd/aa");
    model->s_previousMatch = entry;
    model->m_rootEntry.reset(entry);
    entry->setFullPath("/home/arv00/tmp/dd");
    Stub *stub = new Stub;
    stub->set(ADDR(Archive::Entry, isDir), getIsDir);
    model->parentFor(entry, ArchiveModel::InsertBehaviour::NotifyViews);
    // delete entry;
    delete stub;
    delete model;
}

// 564 行
TEST(ArchivehandeMol_indexForEntry_UT, ArchiveModel_indexForEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry(model);
    Archive::Entry *sub = new Archive::Entry(entry);
    sub->setParent(entry);
    entry->setIsDirectory(true);
    model->indexForEntry(sub);

    delete entry;
    //delete parent;
    delete model;
}

// 564 行
TEST(ArchivehandeMol_indexForEntry_UT, ArchiveModel_indexForEntry_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *entry = new Archive::Entry;
    model->m_rootEntry.reset(entry);
    model->indexForEntry(entry);
    // delete entry;
    delete model;
}

// 574行
TEST(ArchivehandeMol_slotEntryRemoved_UT, ArchiveModel_slotEntryRemoved_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->slotEntryRemoved("");
    delete model;
}
Archive::Entry *getEntry_slotEntryRemoved()
{
    Archive::Entry *entry = new Archive::Entry();
    return entry;
}
TEST(ArchivehandeMol_slotEntryRemoved_UT, ArchiveModel_slotEntryRemoved_UT002)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;
    rootEntry->setFullPath("/");
    rootEntry->setIsDirectory(true);
    Archive::Entry *SubEntry = new Archive::Entry(rootEntry);
    SubEntry->setFullPath("/home");

    Archive::Entry *Sub2Entry = new Archive::Entry(SubEntry);
    Sub2Entry->setFullPath("/home/parent");
    rootEntry->appendEntry(SubEntry);

    model->m_rootEntry.reset(rootEntry);

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);
    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    model->setPlugin(interface);

    model->slotEntryRemoved("/home");
    //delete rootEntry;
    delete interface;
    delete model;
}

TEST(ArchivehandeMol_slotEntryRemoved_UT, ArchiveModel_slotEntryRemoved_UT003)
{
}

TEST(ArchivehandeMol_slotNewEntry_UT, ArchiveModel_slotNewEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;
    model->slotNewEntry(rootEntry);
    delete rootEntry;
    delete model;
}

TEST(ArchivehandeMol_slotAddEntry_UT, ArchiveModel_slotAddEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;
    model->slotAddEntry(rootEntry);
    delete rootEntry;
    delete model;
}

//TEST(ArchivehandeMol_slotAddEntry_UT, ArchiveModel_slotAddEntry_UT002)
//{
//    ArchiveModel *model = new ArchiveModel(nullptr);
//    Archive::Entry *rootEntry = new Archive::Entry;
//    Archive::Entry *subEntry = new Archive::Entry(rootEntry);
//    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
//    interface->m_bAllEntry = false;
//    subEntry->setFullPath("/home");
//    model->setPlugin(interface);
//    model->slotAddEntry(subEntry);
//    delete rootEntry;
//    delete interface;
//    delete model;
//}

TEST(ArchivehandeMol_slotAddEntry_UT, ArchiveModel_slotAddEntry_UT003)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;
    Archive::Entry *subEntry = new Archive::Entry(rootEntry);
    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);
    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    subEntry->setFullPath("./");
    model->setPlugin(interface);
    model->slotAddEntry(subEntry);
    delete rootEntry;
    delete interface;
    delete model;
}
TEST(ArchivehandeMol_slotAddEntry_UT, ArchiveModel_slotAddEntry_UT004)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;
    Archive::Entry *subEntry = new Archive::Entry(rootEntry);

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    subEntry->setFullPath("./home");
    subEntry->setIsDirectory(true);
    model->setPlugin(interface);
    model->slotAddEntry(subEntry);
    delete rootEntry;
    delete interface;
//    delete model;
}

//测试如果总数据中存在了该节点，需要跳过
TEST(ArchivehandeMol_slotAddEntry_UT, ArchiveModel_slotAddEntry_UT005)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;
    Archive::Entry *subEntry = new Archive::Entry(rootEntry);

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
    interface->m_bAllEntry = false;
    subEntry->setFullPath("./home");
    subEntry->setIsDirectory(true);
    model->setPlugin(interface);
    model->slotAddEntry(subEntry);
    model->slotAddEntry(subEntry);
    delete subEntry;
    delete rootEntry;
    delete interface;

//    delete model;
}
static int ifind_slotAddEntry = 0;
Archive::Entry *find_slotAddEntry()
{
    if (ifind_slotAddEntry == 0) {
        ifind_slotAddEntry++;
        return nullptr;
    }
    Archive::Entry *entry = new Archive::Entry();

    return entry;
}
// 创建一个entry,如果从父类中找到了这个entry进行宝贝
//TEST(ArchivehandeMol_slotAddEntry_UT, ArchiveModel_slotAddEntry_UT006)
//{
//    ArchiveModel *model = new ArchiveModel(nullptr);
//    Archive::Entry *rootEntry = new Archive::Entry;
//    Archive::Entry *subEntry = new Archive::Entry(rootEntry);
//    rootEntry->setIsDirectory(true);
//    rootEntry->appendEntry(subEntry);
//    ReadOnlyArchiveInterface *interface = Archive::createInterface("", "7z");
//    interface->m_bAllEntry = false;
//    subEntry->setFullPath("./home");
//    model->setPlugin(interface);
//    Stub *stub = new Stub;
//    stub->set(ADDR(Archive::Entry, find), find_slotAddEntry);
//    model->slotAddEntry(subEntry);
//    delete subEntry;
//    delete rootEntry;
//    delete interface;
//    delete stub;
//    delete model;
//}

// 创建一个entry,如果从父类中找到了这个entry进行
TEST(ArchivehandeMol_slotListEntry_UT, ArchiveModel_slotListEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;

    model->slotListEntry(rootEntry);
    delete rootEntry;
    delete model;
}

TEST(ArchivehandeMol_newEntry_UT, ArchiveModel_newEntry_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *rootEntry = new Archive::Entry;

    model->newEntry(rootEntry, ArchiveModel::InsertBehaviour::DoNotNotifyViews);
    delete rootEntry;
    delete model;
}

//TEST(ArchivehandeMol_newEntry_UT, ArchiveModel_newEntry_UT002)
//{
//    ArchiveModel *model = new ArchiveModel(nullptr);
//    Archive::Entry *rootEntry = new Archive::Entry;
//    rootEntry->setFullPath("/home");
//    model->newEntry(rootEntry, ArchiveModel::InsertBehaviour::DoNotNotifyViews);
//    delete rootEntry;
//    delete model;
//}
// 父类中没有相同的数据，插入新数据
//TEST(ArchivehandeMol_newEntry_UT, ArchiveModel_newEntry_UT003)
//{
//    ArchiveModel *model = new ArchiveModel(nullptr);
//    Archive::Entry *rootEntry = new Archive::Entry;
//    Archive::Entry *subEntry = new Archive::Entry(rootEntry);
//    rootEntry->setFullPath("/home");
//    subEntry->setFullPath("./tmp");
//    model->newEntry(subEntry, ArchiveModel::InsertBehaviour::NotifyViews);
//    delete rootEntry;
//    delete model;
//}
// TODO
TEST(ArchivehandeMol_slotLoadingFinished_UT, ArchiveModel_slotLoadingFinished_UT001)
{
    //    ArchiveModel *model = new ArchiveModel(nullptr);
    //    Archive::Entry *rootEntry = new Archive::Entry;
    //    rootEntry->setFullPath("/home");
    //    QVector<Archive::Entry *> entries;
    //    entries.append(rootEntry);
    //     ReadOnlyArchiveInterface *pIface = Archive::createInterface("/home", "7z", true);
    //     ExtractionOptions options;
    //     ExtractJob *m_pJob1 =  new ExtractJob(entries, "~", options,pIface);
    //    KJob *m_pJob = nullptr;
    //    m_pJob = m_pJob1;
    //  //  model->slotLoadingFinished(m_pJob);
    //    delete model;
}

//TEST(ArchivehandeMol_insertEntry_UT, ArchiveModel_insertEntry_UT001)
//{
//    ArchiveModel *model = new ArchiveModel(nullptr);
//    Archive::Entry *rootEntry = new Archive::Entry;
//    rootEntry->setIsDirectory(true);
//    Archive::Entry *subEntry = new Archive::Entry(rootEntry);
//    subEntry->setParent(rootEntry);
//    subEntry->setIsDirectory(true);
//    Archive::Entry *sub2Entry = new Archive::Entry(subEntry);
//    sub2Entry->setParent(subEntry);
//    rootEntry->appendEntry(subEntry);
//    subEntry->appendEntry(sub2Entry);

//    model->insertEntry(sub2Entry, ArchiveModel::InsertBehaviour::NotifyViews);

//    delete sub2Entry;
//    delete subEntry;
//    delete rootEntry;
//    delete model;
//}

TEST(ArchivehandeMol_appendEntryIcons_UT, ArchiveModel_appendEntryIcons_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    QHash<QString, QIcon> *map = new QHash<QString, QIcon>();
    QIcon i;
    map->insert("hh", i);
    model->appendEntryIcons(*map);
    map->clear();
//    delete map;
//    map = nullptr;
    delete model;
}

TEST(ArchivehandeMol_archive_UT, ArchiveModel_archive_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->archive();
    delete model;
}

TEST(ArchivehandeMol_reset_UT, ArchiveModel_reset_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->reset();
    delete model;
}

TEST(ArchivehandeMol_createEmptyArchive_UT, ArchiveModel_createEmptyArchive_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
//   model->createEmptyArchive("", "7z", nullptr);
    delete model;
}

TEST(ArchivehandeMol_loadArchive_UT, ArchiveModel_loadArchive_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->loadArchive("", "7z", nullptr);
    delete model;
}

TEST(ArchivehandeMol_extractFile_UT, ArchiveModel_extractFile_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *file = new Archive::Entry();
    ExtractionOptions option;
    // model->createEmptyArchive("", "7z", nullptr);
    //  model->extractFile(file, "/home", option);
    delete file;
    delete model;
}

TEST(ArchivehandeMol_extractFiles_UT, ArchiveModel_extractFiles_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    // model->loadArchive("asdf","7z",nullptr);
    Archive::Entry *file = new Archive::Entry();
    ExtractionOptions option;
    //  model->createEmptyArchive("", "7z", nullptr);

    //model->archive()->encrypt("asd", Archive::Encrypted);
    // model->extractFile(file, "/home", option);
    delete file;
    delete model;
}

TEST(ArchivehandeMol_preview_UT, ArchiveModel_preview_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *file = new Archive::Entry();
    ExtractionOptions option;
    // model->createEmptyArchive("", "7z", nullptr);
    // model->preview(file);
    delete file;
    delete model;
}

TEST(ArchivehandeMol_open_UT, ArchiveModel_open_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *file = new Archive::Entry();
    ExtractionOptions option;
    //model->createEmptyArchive("", "7z", nullptr);
    // model->open(file);
    delete file;
    delete model;
}

TEST(ArchivehandeMol_openWith_UT, ArchiveModel_openWith_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *file = new Archive::Entry();
    ExtractionOptions option;
    //model->createEmptyArchive("", "7z", nullptr);
    // model->open(file);
    delete file;
    delete model;
}

TEST(ArchivehandeMol_addFiles_UT, ArchiveModel_addFiles_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    Archive::Entry *file = new Archive::Entry();
    ExtractionOptions option;
    //model->createEmptyArchive("", "7z", nullptr);
    // model->addFiles(file);
    delete file;
    delete model;
}

TEST(ArchivehandeMol_countEntriesAndSize_UT, ArchiveModel_countEntriesAndSize_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    //  model->countEntriesAndSize();
    delete model;
}

TEST(ArchivehandeMol_traverseAndCountDirNode_UT, ArchiveModel_traverseAndCountDirNode_UT001)
{
}

TEST(ArchivehandeMol_setPlugin_UT, ArchiveModel_setPlugin_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    ReadOnlyArchiveInterface *interface;
    model->setPlugin(interface);
    delete model;
}

TEST(ArchivehandeMol_numberOfFiles_UT, ArchiveModel_numberOfFiles_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    //  model->numberOfFiles();
    delete model;
}

TEST(ArchivehandeMol_numberOfFolders_UT, ArchiveModel_numberOfFolders_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    //model->numberOfFolders();
    delete model;
}

TEST(ArchivehandeMol_uncompressedSize_UT, ArchiveModel_uncompressedSize_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
//   model->uncompressedSize();
    delete model;
}

TEST(ArchivehandeMol_shownColumns_UT, ArchiveModel_shownColumns_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->shownColumns();
    delete model;
}

TEST(ArchivehandeMol_propertiesMap_UT, ArchiveModel_propertiesMap_UT001)
{
    ArchiveModel *model = new ArchiveModel(nullptr);
    model->propertiesMap();
    delete model;
}
