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

#include "uncompressview.h"
#include "gtest/src/stub.h"
#include "datamanager.h"
#include "popupdialog.h"
#include "ut_commonstub.h"
#include "config.h"

#include <DMenu>
#include <DDialog>

#include <gtest/gtest.h>
#include <QTest>

class TestUnCompressView : public ::testing::Test
{
public:
    TestUnCompressView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        DataManager::get_instance().resetArchiveData();

        FileEntry entry;

        entry.isDirectory = true;
        entry.strFullPath = "1/";
        entry.strFileName = "1/";
        DataManager::get_instance().archiveData().listRootEntry << entry;
        DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
        entry.isDirectory = true;
        entry.strFullPath = "1/1.txt";
        entry.strFileName = "1.txt";
        entry.qSize = 10;
        DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

        m_tester = new UnCompressView;
        m_tester->initUI();
        m_tester->initConnections();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        DataManager::get_instance().resetArchiveData();
        delete m_tester;
    }

protected:
    UnCompressView *m_tester;
};

TEST_F(TestUnCompressView, initTest)
{

}

TEST_F(TestUnCompressView, testrefreshArchiveData)
{
    m_tester->refreshArchiveData();
    ASSERT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(TestUnCompressView, testsetArchivePath)
{
    QFileInfo fileInfo(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"));
    m_tester->setArchivePath(fileInfo.absoluteFilePath());
    ASSERT_EQ(m_tester->m_strArchive.toStdString(), fileInfo.absoluteFilePath().toStdString());
}

TEST_F(TestUnCompressView, testsetDefaultUncompressPath)
{
    QFileInfo fileInfo(TEST_SOURCES_PATH + QString("/zip/extract"));
    m_tester->setDefaultUncompressPath(fileInfo.absoluteFilePath());
    ASSERT_EQ(m_tester->m_strUnCompressPath.toStdString(), fileInfo.absoluteFilePath().toStdString());
}

TEST_F(TestUnCompressView, testrefreshDataByCurrentPathChanged_Level0)
{
    m_tester->refreshArchiveData();
    m_tester->refreshDataByCurrentPathChanged();
    ASSERT_EQ(m_tester->m_eChangeType, UnCompressView::CT_None);
}

TEST_F(TestUnCompressView, testrefreshDataByCurrentPathChanged_Level1)
{
    m_tester->refreshArchiveData();
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    m_tester->refreshDataByCurrentPathChanged();
    ASSERT_EQ(m_tester->m_eChangeType, UnCompressView::CT_None);
}

TEST_F(TestUnCompressView, testaddNewFiles_self)
{
    Stub stub;
    CommonStub::stub_TipDialog_showDialog(stub, 1);
    CommonStub::stub_AppendDialog_showDialog(stub, 1);

    m_tester->m_eChangeType = UnCompressView::CT_None;
    QFileInfo fileInfo(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"));
    m_tester->m_strArchive = fileInfo.absoluteFilePath();
    QStringList lisFiles = QStringList() << m_tester->m_strArchive;
    m_tester->addNewFiles(lisFiles);
    ASSERT_EQ(m_tester->m_eChangeType, UnCompressView::CT_None);
}

TEST_F(TestUnCompressView, testaddNewFiles_add)
{
    Stub stub;
    CommonStub::stub_TipDialog_showDialog(stub, 1);
    CommonStub::stub_AppendDialog_showDialog(stub, 1);
    CommonStub::stub_OverwriteQueryDialog_showDialog(stub, 2, true);

    m_tester->m_eChangeType = UnCompressView::CT_None;
    QFileInfo fileInfo(TEST_SOURCES_PATH + QString("/zip/compress/test.txt"));
    QStringList lisFiles = QStringList() << fileInfo.absoluteFilePath();
    m_tester->addNewFiles(lisFiles);
    ASSERT_EQ(m_tester->m_eChangeType, UnCompressView::CT_Add);
}

TEST_F(TestUnCompressView, testgetCurPath_level0)
{
    m_tester->m_iLevel = 0;
    ASSERT_EQ(m_tester->getCurPath(), "");
}

TEST_F(TestUnCompressView, testgetCurPath_levelother)
{
    m_tester->m_strCurrentPath = "aaa";
    m_tester->m_iLevel = 1;
    ASSERT_EQ(m_tester->getCurPath(), m_tester->m_strCurrentPath);
}

TEST_F(TestUnCompressView, testsetModifiable)
{
    m_tester->setModifiable(false, false);
    ASSERT_EQ(m_tester->m_bMultiplePassword, false);
}

TEST_F(TestUnCompressView, testisModifiable)
{
    m_tester->setModifiable(false, false);
    ASSERT_EQ(m_tester->isModifiable(), false);
}

TEST_F(TestUnCompressView, testclear)
{
    m_tester->clear();
    ASSERT_EQ(m_tester->m_bReceive, false);
}

TEST_F(TestUnCompressView, testmousePressEvent)
{
    QTest::mousePress(m_tester->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(20, 20));
    ASSERT_EQ((m_tester->m_dragPos == QPoint(20, 20)), true);
}

TEST_F(TestUnCompressView, testmouseMoveEvent)
{
    m_tester->m_isPressed = true;
    QTest::mouseMove(m_tester->viewport());
    ASSERT_EQ((m_tester->m_lastTouchBeginPos == QPoint(0, 0)), true);
}

TEST_F(TestUnCompressView, testmouseDoubleClickEvent)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    m_tester->refreshArchiveData();
    QPoint point(0, 40);
    QTest::mouseDClick(m_tester->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), point);
    ASSERT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(TestUnCompressView, testcalDirItemCount)
{
    QString str = QString("1/");
    qlonglong num = m_tester->calDirItemCount(str);

    ASSERT_EQ(num, 1);
}

TEST_F(TestUnCompressView, testhandleDoubleClick)
{
    m_tester->refreshArchiveData();
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    ASSERT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(TestUnCompressView, testrefreshDataByCurrentPath)
{
    m_tester->refreshArchiveData();
    m_tester->refreshDataByCurrentPath();
    ASSERT_EQ(m_tester->m_mapShowEntry.isEmpty(), false);
}

TEST_F(TestUnCompressView, testgetCurPathFiles)
{
    m_tester->refreshArchiveData();
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    ASSERT_EQ(m_tester->getCurPathFiles().isEmpty(), false);
}

QModelIndexList selectedRows_stub(void *obj, int column)
{
    QModelIndexList listIndex;
    QItemSelectionModel *p = (QItemSelectionModel *)obj;
    if (p) {
        listIndex << p->model()->index(0, 0);
    }

    return listIndex;
}

TEST_F(TestUnCompressView, testgetSelEntry)
{
    Stub stub;
    stub.set((QModelIndexList(QItemSelectionModel::*)(int))ADDR(QItemSelectionModel, selectedRows), selectedRows_stub);
    m_tester->refreshArchiveData();
    QList<FileEntry> listSelEntry = m_tester->getSelEntry();
    ASSERT_EQ(listSelEntry.isEmpty(), false);
}

TEST_F(TestUnCompressView, testextract2Path)
{
    Stub stub;
    stub.set((QModelIndexList(QItemSelectionModel::*)(int))ADDR(QItemSelectionModel, selectedRows), selectedRows_stub);
    m_tester->refreshArchiveData();
    QList<FileEntry> listSelEntry = m_tester->getSelEntry();
    ASSERT_EQ(listSelEntry.isEmpty(), false);
}

TEST_F(TestUnCompressView, testcalEntrySizeByParentPath)
{
    qint64 qSize;
    m_tester->calEntrySizeByParentPath("1/", qSize);
    ASSERT_EQ(qSize, 10);
}

TEST_F(TestUnCompressView, testclearDragData)
{
    m_tester->clearDragData();
    ASSERT_EQ(m_tester->m_bReceive, false);
}

TEST_F(TestUnCompressView, testslotPreClicked)
{
    m_tester->refreshArchiveData();
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    m_tester->slotPreClicked();
    ASSERT_EQ(m_tester->m_iLevel, 0);
}

void addNewFiles_stub(const QStringList &listFiles)
{
    return;
}

TEST_F(TestUnCompressView, testslotDragFiles)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, addNewFiles), addNewFiles_stub);
    m_tester->slotDragFiles(QStringList());
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestUnCompressView, testslotShowRightMenu)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, addNewFiles), addNewFiles_stub);
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);
    m_tester->slotShowRightMenu(QPoint());
    ASSERT_NE(m_tester->m_stRightEntry.strFullPath.isEmpty(), false);
}



TEST_F(TestUnCompressView, testslotExtract)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, addNewFiles), addNewFiles_stub);
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);
    m_tester->slotShowRightMenu(QPoint());
    ASSERT_NE(m_tester->m_stRightEntry.strFullPath.isEmpty(), false);
}

