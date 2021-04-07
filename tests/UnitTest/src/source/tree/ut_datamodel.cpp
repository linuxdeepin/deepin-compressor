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
#include "compressview.h"
#include "gtest/src/stub.h"

#include <DMenu>

#include <gtest/gtest.h>
#include <QTest>

class TestDataModel : public ::testing::Test
{
public:
    TestDataModel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DataModel;
        m_view = new CompressView;

        m_tester->m_listEntry << m_view->fileInfo2Entry(QFileInfo("../UnitTest/test_sources/zip/compress/test.txt"));
        m_tester->m_listEntry << m_view->fileInfo2Entry(QFileInfo("../UnitTest/test_sources/zip/compress/test"));

    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_view;
    }

protected:
    DataModel *m_tester;
    CompressView *m_view;
};

TEST_F(TestDataModel, initTest)
{

}

TEST_F(TestDataModel, testheaderData_DisplayRole)
{
    int section = 0;
    Qt::Orientation orientation;
    int role = Qt::DisplayRole;
    QVariant var = m_tester->headerData(section, orientation, role);
    ASSERT_EQ((var.toString() == QObject::tr("Name")), true);
}

TEST_F(TestDataModel, testheaderData_TextAlignmentRole)
{
    int section = 0;
    Qt::Orientation orientation;
    int role = Qt::TextAlignmentRole;
    QVariant var = m_tester->headerData(section, orientation, role);
    ASSERT_EQ((var == QVariant(Qt::AlignLeft | Qt::AlignVCenter)), true);
}

TEST_F(TestDataModel, testdata)
{
    m_tester->data(m_tester->index(0, 0), Qt::DisplayRole);
    m_tester->data(m_tester->index(0, 1), Qt::DisplayRole);
    m_tester->data(m_tester->index(0, 2), Qt::DisplayRole);
    m_tester->data(m_tester->index(0, 3), Qt::DisplayRole);
    m_tester->data(m_tester->index(1, 0), Qt::DisplayRole);
    m_tester->data(m_tester->index(1, 1), Qt::DisplayRole);
    m_tester->data(m_tester->index(1, 2), Qt::DisplayRole);
    m_tester->data(m_tester->index(1, 3), Qt::DisplayRole);
    m_tester->data(m_tester->index(0, 0), Qt::UserRole);
    m_tester->data(m_tester->index(0, 1), Qt::UserRole);
    m_tester->data(m_tester->index(0, 2), Qt::UserRole);
    m_tester->data(m_tester->index(0, 3), Qt::UserRole);
    m_tester->data(m_tester->index(0, 0), Qt::DecorationRole);
    m_tester->data(m_tester->index(0, 1), Qt::DecorationRole);
    m_tester->data(m_tester->index(0, 2), Qt::DecorationRole);
    m_tester->data(m_tester->index(0, 3), Qt::DecorationRole);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestDataModel, testrowCount)
{
    QModelIndex index;
    ASSERT_EQ(m_tester->rowCount(index), 2);
}

TEST_F(TestDataModel, testcolumnCount)
{
    QModelIndex index;
    ASSERT_EQ(m_tester->columnCount(index), 4);
}

TEST_F(TestDataModel, testrefreshFileEntry)
{
    FileEntry entry =  m_view->fileInfo2Entry(QFileInfo("../UnitTest/test_sources/zip/compress/test1.txt"));
    QList<FileEntry> listEntry = QList<FileEntry>() << entry;
    m_tester->refreshFileEntry(listEntry);
    bool bResult = (m_tester->m_listEntry == listEntry);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestDataModel, testgetSelectItem)
{
    QStringList listName;
    listName << "test.txt";
    QItemSelection selection = m_tester->getSelectItem(listName);
    ASSERT_EQ(selection.contains(m_tester->index(0, DC_Name)), true);
}

TEST_F(TestDataModel, testgetListEntryIndex)
{
    QModelIndex index = m_tester->getListEntryIndex("test.txt");
    ASSERT_EQ((index == m_tester->index(0, DC_Name)), true);
}

TEST_F(TestDataModel, testsort)
{
    m_tester->sort(DC_Name, Qt::DescendingOrder);
    m_tester->sort(DC_Name, Qt::AscendingOrder);
    m_tester->sort(DC_Time, Qt::DescendingOrder);
    m_tester->sort(DC_Time, Qt::AscendingOrder);
    m_tester->sort(DC_Type, Qt::DescendingOrder);
    m_tester->sort(DC_Type, Qt::AscendingOrder);
    m_tester->sort(DC_Size, Qt::DescendingOrder);
    m_tester->sort(DC_Size, Qt::AscendingOrder);
    ASSERT_NE(m_tester, nullptr);
}
