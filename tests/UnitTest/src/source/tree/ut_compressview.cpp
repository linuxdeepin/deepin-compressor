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

#include "compressview.h"
#include "treeheaderview.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QAction>

/*******************************函数打桩************************************/
// 对CompressView的handleDoubleClick进行打桩
void compressView_handleDoubleClick_stub(const QModelIndex &)
{
    return;
}
/*******************************函数打桩************************************/

class TestStyleTreeViewDelegate : public ::testing::Test
{
public:
    TestStyleTreeViewDelegate(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new StyleTreeViewDelegate;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    StyleTreeViewDelegate *m_tester;
};

TEST_F(TestStyleTreeViewDelegate, initTest)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    ASSERT_EQ(m_tester->sizeHint(option, index).height(), 36);
}


class TestCompressView : public ::testing::Test
{
public:
    TestCompressView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressView;
        m_tester->disconnect();
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

TEST_F(TestCompressView, testgetHeaderView)
{
    ASSERT_EQ(m_tester->getHeaderView(), m_tester->m_pHeaderView);
}

TEST_F(TestCompressView, testaddCompressFiles_Cancel)
{
    Stub stub;
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_OverwriteQueryDialog_getApplyAll(stub, true);
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, OR_Cancel);

    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << "1.txt";
    m_tester->addCompressFiles(listFiles);

    ASSERT_EQ(m_tester->m_listCompressFiles == listFiles, true);
}

TEST_F(TestCompressView, testaddCompressFiles_Overwrite)
{
    Stub stub;
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_OverwriteQueryDialog_getApplyAll(stub, true);
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, OR_Overwrite);

    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << "1.txt";
    m_tester->addCompressFiles(listFiles);

    ASSERT_EQ(m_tester->m_listCompressFiles == listFiles, true);
}

TEST_F(TestCompressView, testgetCompressFiles)
{
    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << listFiles;

    ASSERT_EQ(m_tester->getCompressFiles(), listFiles);
}

TEST_F(TestCompressView, testrefreshCompressedFiles)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, false);

    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << listFiles;
    m_tester->m_listSelFiles << "2.txt";
    m_tester->refreshCompressedFiles(true, "1.txt");

    ASSERT_EQ(m_tester->m_listSelFiles.isEmpty(), true);
}

TEST_F(TestCompressView, testclear)
{
    m_tester->clear();

    ASSERT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(TestCompressView, testmouseDoubleClickEvent)
{
    typedef void (*fptr)(CompressView *, const QModelIndex &);
    fptr A_foo = (fptr)(&CompressView::handleDoubleClick);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, compressView_handleDoubleClick_stub);

    QPoint point(0, 40);
    QTest::mouseDClick(m_tester->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), point);
}

TEST_F(TestCompressView, testfileInfo2Entry_Directory)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");

    QFileInfo fileInfo;
    FileEntry entry = m_tester->fileInfo2Entry(fileInfo);
    ASSERT_EQ(entry.strFileName, "1.txt");
}

TEST_F(TestCompressView, testfileInfo2Entry_File)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "2.txt");

    QFileInfo fileInfo;
    FileEntry entry = m_tester->fileInfo2Entry(fileInfo);
    ASSERT_EQ(entry.strFileName, "2.txt");
}

TEST_F(TestCompressView, testhandleDoubleClick)
{
    m_tester->m_iLevel = 0;
    QStringList listFiles = QStringList() << "/home/Desktop/compress/";


    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    m_tester->addCompressFiles(listFiles);
    QModelIndex index = m_tester->model()->index(0, 0);
    CommonStub::stub_ProcessOpenThread_start(stub);
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);

    m_tester->handleDoubleClick(index);
    ASSERT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(TestCompressView, testhandleDoubleClick1)
{
    m_tester->m_iLevel = 0;
    QStringList listFiles = QStringList() << "/home/Desktop/compress/";


    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    m_tester->addCompressFiles(listFiles);
    QModelIndex index = m_tester->model()->index(0, 0);
    CommonStub::stub_ProcessOpenThread_start(stub);
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);

    m_tester->handleDoubleClick(index);
    ASSERT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(TestCompressView, testgetCurrentDirFiles)
{
    QFileInfo fi1;
    fi1.setFile("1.txt");
    QFileInfoList filist{fi1};

    Stub stub;
    QDirStub::stub_QDir_entryInfoList(stub, filist);

    QList<FileEntry> listEntry = m_tester->getCurrentDirFiles();

    if (listEntry.count() > 0)
        ASSERT_EQ(listEntry[0].strFileName, "1.txt");
}

TEST_F(TestCompressView, testhandleLevelChanged)
{
    m_tester->m_iLevel = 0;
    m_tester->handleLevelChanged();
    ASSERT_EQ(m_tester->acceptDrops(), true);
}

TEST_F(TestCompressView, testgetPrePathByLevel_Level0)
{
    m_tester->m_iLevel = 0;
    m_tester->getPrePathByLevel("");
}

TEST_F(TestCompressView, testgetPrePathByLevel_Level1)
{
    m_tester->m_iLevel = 1;
    QString strResult = m_tester->getPrePathByLevel("/home/Desktop");
    ASSERT_EQ(strResult, "Desktop");
}

TEST_F(TestCompressView, testrefreshDataByCurrentPath_Level0)
{
    m_tester->m_iLevel = 0;
    m_tester->refreshDataByCurrentPath();
    ASSERT_EQ(m_tester->m_pHeaderView->getpreLbl()->isVisible(), false);
}

TEST_F(TestCompressView, testrefreshDataByCurrentPath_Level1)
{
    Stub stub;
    CommonStub::stub_QWidget_isVisible(stub, true);
    m_tester->m_iLevel = 1;
    m_tester->m_strCurrentPath = "/home/Desktop";
    m_tester->refreshDataByCurrentPath();
    ASSERT_EQ(m_tester->m_pHeaderView->getpreLbl()->isVisible(), true);
}

TEST_F(TestCompressView, testslotShowRightMenu_dir)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);

    m_tester->m_stRightEntry.isDirectory = true;
    QStringList listFiles = QStringList() << "/home/Desktop/compress";
    m_tester->addCompressFiles(listFiles);
    QPoint p;
    m_tester->slotShowRightMenu(p);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    ASSERT_EQ(m_tester->m_stRightEntry.strFileName, "compress");
}

TEST_F(TestCompressView, testslotShowRightMenu_file)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);

    m_tester->m_stRightEntry.isDirectory = false;
    QStringList listFiles = QStringList() << "/home/Desktop/compress.txt";
    m_tester->addCompressFiles(listFiles);
    QPoint p;
    m_tester->slotShowRightMenu(p);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    ASSERT_EQ(m_tester->m_stRightEntry.strFileName, "compress.txt");
}

TEST_F(TestCompressView, testslotDeleteFile_Level0)
{
    QStringList listFiles = QStringList() << "/home/Desktop/compress";
    m_tester->addCompressFiles(listFiles);
    m_tester->slotDeleteFile();
}

TEST_F(TestCompressView, testslotDeleteFile_Level1)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);

    QStringList listFiles = QStringList() << "/home/Desktop/compress.txt";
    m_tester->addCompressFiles(listFiles);
    m_tester->m_iLevel = 1;
    m_tester->slotDeleteFile();
}

TEST_F(TestCompressView, testslotDirChanged)
{
    m_tester->slotDirChanged();
}

TEST_F(TestCompressView, testslotDragFiles_isArchiveFile_1)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    CommonStub::stub_UiTools_isArchiveFile(stub, true);
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->slotDragFiles(QStringList() << "1.zip");
}

TEST_F(TestCompressView, testslotDragFiles_isArchiveFile_2)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    CommonStub::stub_UiTools_isArchiveFile(stub, true);
    m_tester->slotDragFiles(QStringList() << "1.zip");
}

TEST_F(TestCompressView, testslotDragFiles_isArchiveFile)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    CommonStub::stub_UiTools_isArchiveFile(stub, false);
    m_tester->slotDragFiles(QStringList() << "1.txt");
}

TEST_F(TestCompressView, testslotOpenStyleClicked_Nullptr)
{
    QAction *pAction = nullptr;
    Stub stub;
    CommonStub::stub_QObject_sender(stub, pAction);
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);
    m_tester->slotOpenStyleClicked();
}

TEST_F(TestCompressView, testslotOpenStyleClicked)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText(QObject::tr("Select default program"));
    Stub stub;
    CommonStub::stub_QObject_sender(stub, pAction);
    CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(stub, "");
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);
    m_tester->slotOpenStyleClicked();
}

TEST_F(TestCompressView, testslotOpenStyleClicked_Other)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText(QObject::tr("other"));
    Stub stub;
    CommonStub::stub_QObject_sender(stub, pAction);
    CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(stub, "");
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);
    m_tester->slotOpenStyleClicked();
}

TEST_F(TestCompressView, testslotPreClicked)
{
    m_tester->m_iLevel = 1;
    m_tester->m_vPre << "compress";
    m_tester->slotPreClicked();
    ASSERT_EQ(m_tester->m_iLevel, 0);

    m_tester->m_iLevel = 2;
    m_tester->m_vPre << "compress";
    m_tester->slotPreClicked();
    ASSERT_EQ(m_tester->m_iLevel, 1);
}
