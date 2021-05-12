/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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

#include "datamodel.h"
#include "uistruct.h"
#include "mimetypes.h"
#include "mimetypedisplaymanager.h"
#include "uitools.h"

#include "gtest/src/stub.h"

#include <QItemSelection>

#include <gtest/gtest.h>

/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


class TestDataModel : public ::testing::Test
{
public:
    TestDataModel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DataModel;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DataModel *m_tester;
};

TEST_F(TestDataModel, initTest)
{

}

TEST_F(TestDataModel, testheaderData)
{
    ASSERT_EQ(m_tester->headerData(0, Qt::Horizontal, Qt::DisplayRole), QObject::tr("Name"));
    ASSERT_EQ(m_tester->headerData(0, Qt::Horizontal, Qt::TextAlignmentRole), QVariant(Qt::AlignLeft | Qt::AlignVCenter));
    ASSERT_EQ(m_tester->headerData(0, Qt::Horizontal, Qt::UserRole), QVariant());
}

TEST_F(TestDataModel, testdata)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Name);
    ASSERT_EQ(m_tester->data(index, Qt::DisplayRole), m_tester->m_listEntry[0].strFileName);
    index = m_tester->index(0, DC_Time);
    ASSERT_EQ(m_tester->data(index, Qt::DisplayRole), QDateTime::fromTime_t(m_tester->m_listEntry[0].uLastModifiedTime).toString("yyyy/MM/dd hh:mm:ss"));
    index = m_tester->index(0, DC_Type);
    CustomMimeType mimetype = determineMimeType(m_tester->m_listEntry[0].strFullPath);  // 根据全路径获取类型
    MimeTypeDisplayManager *pMimetype = new MimeTypeDisplayManager(m_tester);
    ASSERT_EQ(m_tester->data(index, Qt::DisplayRole), pMimetype->displayName(mimetype.name()));
    index = m_tester->index(0, DC_Size);
    ASSERT_EQ(m_tester->data(index, Qt::DisplayRole), UiTools::humanReadableSize(m_tester->m_listEntry[0].qSize, 1));
    index = m_tester->index(1, DC_Size);
    QString str = m_tester->data(index, Qt::DisplayRole).toString();
    ASSERT_EQ(m_tester->data(index, Qt::DisplayRole), QString::number(m_tester->m_listEntry[1].qSize) + " " + QObject::tr("item(s)"));
    index = m_tester->index(0, DC_Name);
    ASSERT_EQ(m_tester->data(index, Qt::UserRole), QVariant::fromValue(m_tester->m_listEntry[0]));
    index = m_tester->index(0, DC_Name);
    ASSERT_EQ(m_tester->data(index, Qt::DecorationRole).isValid(), true);
    index = m_tester->index(0, DC_Time);
    ASSERT_EQ(m_tester->data(index, Qt::DecorationRole), QVariant());
    index = m_tester->index(0, DC_Time);
    ASSERT_EQ(m_tester->data(index, Qt::StatusTipRole), QVariant());
}

TEST_F(TestDataModel, testrowCount)
{
    ASSERT_EQ(m_tester->rowCount(QModelIndex()), m_tester->m_listEntry.count());
}

TEST_F(TestDataModel, testcolumnCount)
{
    ASSERT_EQ(m_tester->columnCount(QModelIndex()), m_tester->m_listColumn.count());
}

TEST_F(TestDataModel, testrefreshFileEntry)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;

    QList<FileEntry> listEntry;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);

    ASSERT_EQ(m_tester->m_listEntry, listEntry);
}

TEST_F(TestDataModel, testgetSelectItem)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;

    QList<FileEntry> listEntry;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    QItemSelection sel = m_tester->getSelectItem(QStringList() << "1.txt");

    ASSERT_EQ(sel.count() != 0, true);
}

TEST_F(TestDataModel, testgetListEntryIndex)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;

    QList<FileEntry> listEntry;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);

    ASSERT_EQ(m_tester->getListEntryIndex("1.txt").row(), 0);
    ASSERT_EQ(m_tester->getListEntryIndex("2.txt").row(), -1);
}

TEST_F(TestDataModel, testsort)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);

    m_tester->sort(5, Qt::DescendingOrder);
    m_tester->sort(0, Qt::DescendingOrder);
    m_tester->sort(0, Qt::AscendingOrder);
    m_tester->sort(1, Qt::DescendingOrder);
    m_tester->sort(1, Qt::AscendingOrder);
    m_tester->sort(2, Qt::DescendingOrder);
    m_tester->sort(2, Qt::AscendingOrder);
    m_tester->sort(3, Qt::DescendingOrder);
    m_tester->sort(3, Qt::AscendingOrder);
}
