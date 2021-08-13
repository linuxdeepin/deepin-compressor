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
#include "datamanager.h"
#include "popupdialog.h"
#include "datamodel.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <DFileDragServer>
#include <DFileDrag>

#include <QTest>
#include <QMimeData>
#include <QAction>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

/*******************************函数打桩************************************/
// 对UnCompressView的slotOpen进行打桩
void unCompressView_slotOpen_stub()
{
    return;
}

// 对UnCompressView的slotOpen进行打桩
void unCompressView_handleDoubleClick_stub(const QModelIndex &)
{
    return;
}

// 对QItemSelectionModel的selectedRows进行打桩
QModelIndexList qItemSelectionModel_selectedRows_stub(int &)
{
    return QModelIndexList() << QModelIndex();
}

// 对slotDeleteFile进行打桩
void slotDeleteFile_stub(const QModelIndex &)
{
    return;
}

// 对slotShowRightMenu进行打桩
void slotShowRightMenu_stub(const QPoint &)
{
    return;
}

// handleDoubleClick进行打桩
void handleDoubleClick_stub(const QModelIndex &)
{
    return;
}
/*******************************函数打桩************************************/


class TestUnCompressView : public ::testing::Test
{
public:
    TestUnCompressView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UnCompressView;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
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
    m_tester->m_iLevel = 1;

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();
    ASSERT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(TestUnCompressView, testsetArchivePath)
{
    m_tester->setArchivePath("1/2/3");
    ASSERT_EQ(m_tester->m_strArchive, "1/2/3");
}

TEST_F(TestUnCompressView, testsetDefaultUncompressPath)
{
    m_tester->setDefaultUncompressPath("/home/Desktop");
    ASSERT_EQ(m_tester->m_strUnCompressPath, "/home/Desktop");
}

TEST_F(TestUnCompressView, testmousePressEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(event);
    delete event;
}

TEST_F(TestUnCompressView, testmouseMoveEvent)
{
    m_tester->m_isPressed = true;
    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->m_lastTouchBeginPos = QPointF(200, 100);
    m_tester->mouseMoveEvent(event);
    delete event;

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();

    m_tester->m_isPressed = false;
    event = new QMouseEvent(QEvent::MouseMove, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->m_lastTouchBeginPos = QPointF(200, 100);
    m_tester->mouseMoveEvent(event);
    delete event;
}

TEST_F(TestUnCompressView, testclearDragData)
{
    m_tester->m_pFileDragServer = new DFileDragServer(m_tester);
    m_tester->m_pDrag = new DFileDrag(m_tester, m_tester->m_pFileDragServer);
    m_tester->clearDragData();
    ASSERT_EQ(m_tester->m_bReceive, false);
}

TEST_F(TestUnCompressView, testmouseDoubleClickEvent)
{
    QTest::mouseDClick(m_tester, Qt::LeftButton);
}

TEST_F(TestUnCompressView, testcalDirItemCount)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

    ASSERT_EQ(m_tester->calDirItemCount("1/"), 1);
}

TEST_F(TestUnCompressView, testhandleDoubleClick)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    entry.isDirectory = true;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    stub.set(ADDR(UnCompressView, slotOpen), unCompressView_slotOpen_stub);

    QModelIndex index = m_tester->model()->index(0, 0);

    m_tester->m_iLevel = 0;
    m_tester->handleDoubleClick(index);
    ASSERT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(TestUnCompressView, testhandleDoubleClick1)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    stub.set(ADDR(UnCompressView, slotOpen), unCompressView_slotOpen_stub);

    QModelIndex index = m_tester->model()->index(0, 0);

    m_tester->m_iLevel = 0;
    m_tester->handleDoubleClick(index);
    ASSERT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(TestUnCompressView, testrefreshDataByCurrentPath)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_iLevel = 0;
    m_tester->m_strCurrentPath = "/";
    m_tester->m_mapShowEntry.clear();
    m_tester->m_mapShowEntry[m_tester->m_strCurrentPath] = DataManager::get_instance().archiveData().listRootEntry;
    m_tester->refreshDataByCurrentPath();

    m_tester->m_iLevel = 1;
    m_tester->refreshDataByCurrentPath();
}

TEST_F(TestUnCompressView, testrefreshDataByCurrentPathChanged)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_iLevel = 0;
    m_tester->m_strCurrentPath = "/";
    m_tester->m_mapShowEntry.clear();
    m_tester->m_mapShowEntry[m_tester->m_strCurrentPath] = DataManager::get_instance().archiveData().listRootEntry;
    m_tester->refreshDataByCurrentPathChanged();

    m_tester->m_iLevel = 1;
    m_tester->m_strCurrentPath = "1/";
    m_tester->m_eChangeType = UnCompressView::CT_Add;
    m_tester->refreshDataByCurrentPathChanged();
}

TEST_F(TestUnCompressView, testaddNewFiles)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_AppendDialog_showDialog(stub, 1);
    QFileInfoStub::stub_QFileInfo_path(stub, "");


    m_tester->m_strArchive = "/1.zip";

    m_tester->addNewFiles(QStringList() << "/1.zip");
    ASSERT_EQ(m_tester->m_eChangeType, UnCompressView::CT_None);

    QFileInfoStub::stub_QFileInfo_filePath(stub, "/1.txt");
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, Overwrite_Result::OR_Cancel);
    m_tester->addNewFiles(QStringList() << "/1.txt");
    ASSERT_EQ(m_tester->m_eChangeType, UnCompressView::CT_None);

    Stub stub1;
    QFileInfoStub::stub_QFileInfo_filePath(stub1, "/1.txt");
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub1, Overwrite_Result::OR_Skip);
    m_tester->addNewFiles(QStringList() << "/1.txt");

    Stub stub2;
    QFileInfoStub::stub_QFileInfo_filePath(stub2, "/1.txt");
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub2, Overwrite_Result::OR_Overwrite);
    m_tester->addNewFiles(QStringList() << "/1.txt");

}

TEST_F(TestUnCompressView, testgetCurPath)
{
    m_tester->m_iLevel = 0;
    ASSERT_EQ(m_tester->getCurPath(), "");
    m_tester->m_strCurrentPath = "/";
    m_tester->m_iLevel = 1;
    ASSERT_EQ(m_tester->getCurPath(), "/");
}

TEST_F(TestUnCompressView, testsetModifiable)
{
    m_tester->setModifiable(false, false);
    ASSERT_EQ(m_tester->m_bMultiplePassword, false);
}

TEST_F(TestUnCompressView, testisModifiable)
{
    m_tester->m_bModifiable = false;
    ASSERT_EQ(m_tester->isModifiable(), false);
}

TEST_F(TestUnCompressView, testclear)
{
    m_tester->clear();
    ASSERT_EQ(m_tester->m_bReceive, false);
}

TEST_F(TestUnCompressView, testgetCurPathFiles)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_strCurrentPath = "1/";
    ASSERT_EQ(m_tester->getCurPathFiles().count(), 1);
}

TEST_F(TestUnCompressView, testgetSelEntry)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();
    ASSERT_EQ(m_tester->getSelEntry().count(), 1);
}

TEST_F(TestUnCompressView, testextract2Path)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();
    m_tester->extract2Path("/home/Desktop");
}

TEST_F(TestUnCompressView, testcalEntrySizeByParentPath)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1/1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

    qint64 qSize;
    m_tester->calEntrySizeByParentPath("1/", qSize);
    ASSERT_EQ(qSize, 10);
}

TEST_F(TestUnCompressView, testslotDragFiles)
{
    m_tester->slotDragFiles(QStringList());
}

TEST_F(TestUnCompressView, testslotShowRightMenu)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1/1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_stRightEntry.isDirectory = true;
    m_tester->slotShowRightMenu(QPoint(10, 50));

    m_tester->m_stRightEntry.isDirectory = false;
    m_tester->slotShowRightMenu(QPoint(10, 50));
}

TEST_F(TestUnCompressView, testslotExtract)
{
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 0);
    m_tester->slotExtract();

    Stub stub1;
    QList<QUrl> listUrl;
    listUrl << QUrl("/home/Desktop");
    DFileDialogStub::stub_DFileDialog_selectedUrls(stub1, listUrl);
    DFileDialogStub::stub_DFileDialog_exec(stub1, 1);
    m_tester->slotExtract();
}

TEST_F(TestUnCompressView, testslotExtract2Here)
{
    m_tester->extract2Path("/home/Desktop");
}

TEST_F(TestUnCompressView, testslotDeleteFile)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();
    m_tester->m_bModifiable = true;
    m_tester->slotDeleteFile();
    ASSERT_EQ(m_tester->m_eChangeType, UnCompressView::CT_Delete);
}

TEST_F(TestUnCompressView, testslotOpen)
{
    Stub stub;
    typedef void (*fptr)(UnCompressView *, const QModelIndex &);
    fptr A_foo = (fptr)(&UnCompressView::handleDoubleClick);   // 获取虚函数地址
    stub.set(A_foo, unCompressView_handleDoubleClick_stub);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    entry.isDirectory = true;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->setCurrentIndex(m_tester->m_pModel->index(0, 0));
    m_tester->slotOpen();

    m_tester->setCurrentIndex(m_tester->m_pModel->index(1, 0));
    m_tester->slotOpen();

}

TEST_F(TestUnCompressView, testfocusInEvent)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn, Qt::TabFocusReason);
    m_tester->focusInEvent(event);

    m_tester->selectionModel()->setCurrentIndex(m_tester->model()->index(0, 0), QItemSelectionModel::Select);
    m_tester->focusInEvent(event);
    delete event;
}

TEST_F(TestUnCompressView, testdragEnterEvent)
{
    QMimeData *data = new QMimeData;
    QDragEnterEvent *e = new QDragEnterEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);
    m_tester->dragEnterEvent(e);

    QList<QUrl> listUrls;
    listUrls << QUrl("123456");
    data->setUrls(listUrls);
    m_tester->dragEnterEvent(e);

    delete data;
    delete e;
}

TEST_F(TestUnCompressView, testdragMoveEvent)
{
    QMimeData *data = new QMimeData;
    QDragMoveEvent *e = new QDragMoveEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);
    m_tester->dragMoveEvent(e);

    delete data;
    delete e;
}

TEST_F(TestUnCompressView, testdropEvent)
{
    QMimeData *data = new QMimeData;
    QDropEvent *e = new QDropEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);
    m_tester->dropEvent(e);

    QList<QUrl> listUrls;
    listUrls << QUrl("123456");
    data->setUrls(listUrls);
    m_tester->dropEvent(e);

    delete data;
    delete e;
}

TEST_F(TestUnCompressView, testresizeEvent)
{
    QResizeEvent *event = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(event);
    delete event;
}

TEST_F(TestUnCompressView, testevent)
{
    QTouchDevice *pDevice = new QTouchDevice;
    pDevice->setType(QTouchDevice::TouchScreen);
    QTouchEvent *e = new QTouchEvent(QEvent::TouchBegin, pDevice, Qt::NoModifier, Qt::TouchPointPressed);
    e->setTouchPoints(QList<QTouchEvent::TouchPoint>() << QTouchEvent::TouchPoint());
    m_tester->m_isPressed = false;
    m_tester->event(e);

    delete pDevice;
    delete e;
}

TEST_F(TestUnCompressView, testkeyPressEvent)
{
    Stub stub;
    stub.set(ADDR(QItemSelectionModel, selectedRows), qItemSelectionModel_selectedRows_stub);
    typedef void (*fptr)(UnCompressView *);
    fptr A_foo = (fptr)(&UnCompressView::slotDeleteFile);   // 获取虚函数地址
    stub.set(A_foo, slotDeleteFile_stub);

    typedef void (*fptr1)(UnCompressView *, const QPoint &);
    fptr1 A_foo1 = (fptr1)(&UnCompressView::slotShowRightMenu);   // 获取虚函数地址
    stub.set(A_foo1, slotShowRightMenu_stub);

    typedef void (*fptr2)(UnCompressView *, const QModelIndex &);
    fptr2 A_foo2 = (fptr2)(&UnCompressView::handleDoubleClick);   // 获取虚函数地址
    stub.set(A_foo2, handleDoubleClick_stub);
    QTest::keyPress(m_tester, Qt::Key_Delete);
    QTest::keyPress(m_tester, Qt::Key_M, Qt::AltModifier);
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Tab);
}

TEST_F(TestUnCompressView, testslotOpenStyleClicked)
{
    Stub stub;
    CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(stub, "");

    QAction *pAction = new QAction("Select default program", m_tester);
    CommonStub::stub_QObject_sender(stub, pAction);
    m_tester->setPreLblVisible(true, "");

    pAction = new QAction("sss", m_tester);
    Stub stub1;
    CommonStub::stub_QObject_sender(stub1, pAction);
}

TEST_F(TestUnCompressView, testslotPreClicked)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();
    m_tester->m_vPre << "/";
    m_tester->m_iLevel = 0;
    m_tester->slotPreClicked();

    m_tester->m_iLevel = 1;
    m_tester->m_vPre << "1/";
    m_tester->slotPreClicked();
}
