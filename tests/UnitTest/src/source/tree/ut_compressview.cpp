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

#include "compresspage.h"
#include "compressview.h"
#include "customwidget.h"
#include "uitools.h"
#include "popupdialog.h"

#include <gtest/gtest.h>
#include <QTest>

class TestCompressView : public ::testing::Test
{
public:
    TestCompressView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressView;
        m_tester->initUI();
        m_tester->initConnections();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressView *m_tester;
};

TEST_F(TestCompressView, initTest)
{

}

void showDialog_stub()
{
    return;
}

int getDialogResult_stub()
{
    return 1;
}

bool getApplyAll_stub()
{
    return true;
}

TEST_F(TestCompressView, testaddCompressFiles)
{
    QStringList listFiles = QStringList() << QFileInfo("../UnitTest/test_sources/zip/compress/test.txt").absoluteFilePath();
    m_tester->addCompressFiles(listFiles);
    ASSERT_EQ(m_tester->m_listSelFiles.isEmpty(), true);
}

TEST_F(TestCompressView, testgetCompressFiles)
{
    QStringList listFiles = QStringList() << QFileInfo("../UnitTest/test_sources/zip/compress/test.txt").absoluteFilePath();
    m_tester->addCompressFiles(listFiles);
    bool bResult = (m_tester->getCompressFiles() == listFiles);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestCompressView, testrefreshCompressedFiles)
{
    m_tester->refreshCompressedFiles(false);
    ASSERT_EQ(m_tester->m_listSelFiles.isEmpty(), true);
}

TEST_F(TestCompressView, testclear)
{
    m_tester->refreshCompressedFiles(false);
    ASSERT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(TestCompressView, testinitUI)
{
    m_tester->initUI();
    ASSERT_NE(m_tester->m_pFileWatcher, nullptr);
}

TEST_F(TestCompressView, testinitConnections)
{
    m_tester->initConnections();
    ASSERT_NE(m_tester->m_pFileWatcher, nullptr);
}

TEST_F(TestCompressView, testfileInfo2Entry)
{
    QFileInfo info("../UnitTest/test_sources/zip/compress/test.txt");
    FileEntry entry = m_tester->fileInfo2Entry(info);
    ASSERT_EQ(entry.strFileName, "test.txt");
}

TEST_F(TestCompressView, testhandleDoubleClick)
{
    QStringList listFiles = QStringList() << QFileInfo("../UnitTest/test_sources/zip/compress").absoluteFilePath();
    m_tester->addCompressFiles(listFiles);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    ASSERT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(TestCompressView, testgetCurrentDirFiles)
{
    QStringList listFiles = QStringList() << QFileInfo("../UnitTest/test_sources/zip/compress").absoluteFilePath();
    m_tester->addCompressFiles(listFiles);
    QList<FileEntry> listEntry = m_tester->getCurrentDirFiles();
    ASSERT_EQ(listEntry.isEmpty(), false);
}
