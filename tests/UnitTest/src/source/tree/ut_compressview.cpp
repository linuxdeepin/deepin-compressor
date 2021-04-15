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
#include "treeheaderview.h"
#include "gtest/src/stub.h"
#include "ut_commonstub.h"
#include "config.h"

#include <DMenu>

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

TEST_F(TestCompressView, testaddCompressFiles)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress/test.txt");
    m_tester->addCompressFiles(listFiles);
    ASSERT_EQ(m_tester->m_listSelFiles.isEmpty(), true);
}

TEST_F(TestCompressView, testgetCompressFiles)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress/test.txt");
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
    QFileInfo info(TEST_SOURCES_PATH + QString("/zip/compress/test.txt"));
    FileEntry entry = m_tester->fileInfo2Entry(info);
    ASSERT_EQ(entry.strFileName, "test.txt");
}

TEST_F(TestCompressView, testhandleDoubleClick)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    ASSERT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(TestCompressView, testgetCurrentDirFiles)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);
    QList<FileEntry> listEntry = m_tester->getCurrentDirFiles();
    ASSERT_EQ(listEntry.isEmpty(), false);
}

TEST_F(TestCompressView, testhandleLevelChanged)
{
    m_tester->m_iLevel = 1;
    m_tester->handleLevelChanged();
    ASSERT_EQ(m_tester->acceptDrops(), false);
}

TEST_F(TestCompressView, testgetPrePathByLevel)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    QString str = m_tester->getPrePathByLevel(m_tester->m_strCurrentPath);
    ASSERT_EQ(str, "compress");
}

TEST_F(TestCompressView, testrefreshDataByCurrentPath)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);
    ASSERT_EQ(m_tester->m_pHeaderView->getpreLbl()->isVisible(), false);
}

TEST_F(TestCompressView, testslotShowRightMenu)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);

    CommonStub::stub_QMenu_exec(stub);

    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);
    QPoint p;
    m_tester->slotShowRightMenu(p);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    ASSERT_EQ(m_tester->m_stRightEntry.strFileName, "compress");
}

TEST_F(TestCompressView, testslotDeleteFile)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);
    m_tester->slotDeleteFile();
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCompressView, testslotDirChanged)
{
    m_tester->m_strCurrentPath = TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->slotDirChanged();
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCompressView, testslotDragFiles_isArchiveFile)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);

    Stub stub;
    CommonStub::stub_SimpleQueryDialog_showDialog(stub, 1);

    m_tester->slotDragFiles(QStringList() << TEST_SOURCES_PATH + QString("/zip/extract/test.zip"));

    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCompressView, testslotDragFiles_NormalFiles)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->slotDragFiles(listFiles);
    bool bResult = (m_tester->m_listCompressFiles == listFiles);
    ASSERT_EQ(bResult, true);
}

//QObject *sender_stub(CompressView *p)
//{
//    QAction *pAction = new QAction(p);
//    pAction->setText(QObject::tr("Select default program"));
//    return pAction;
//}
//
//TEST_F(TestCompressView, testslotOpenStyleClicked)
//{
//    m_tester->m_stRightEntry.strFullPath = QFileInfo("../UnitTest/test_sources/zip/compress/test.txt").absoluteFilePath();
//    Stub stub;
//    stub.set(ADDR(QObject, sender), sender_stub(m_tester));
//    m_tester->slotOpenStyleClicked();
//    ASSERT_NE(m_tester, nullptr);
//}

TEST_F(TestCompressView, testslotPreClicked)
{
    QStringList listFiles = QStringList() << TEST_SOURCES_PATH + QString("/zip/compress");
    m_tester->addCompressFiles(listFiles);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    m_tester->slotPreClicked();
    ASSERT_EQ(m_tester->m_iLevel, 0);
}
