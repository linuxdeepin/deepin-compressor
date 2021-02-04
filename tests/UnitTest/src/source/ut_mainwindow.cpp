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

#include "mainwindow.h"
#include "popupdialog.h"
#include "settingdialog.h"
#include "successpage.h"
#include "failurepage.h"
#include "gtest/src/stub.h"
#include "archivemanager.h"
#include "progresspage.h"
#include "compresssettingpage.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QSettings>
#include <DDialog>
#include <DLabel>
#include <QKeyEvent>
#include <DPushButton>
#include <DTitlebar>
#include <DWindowCloseButton>

#include <QStackedWidget>

DWIDGET_USE_NAMESPACE

class TestMainWindow : public ::testing::Test
{
public:
    TestMainWindow() {}

public:
    virtual void SetUp()
    {
        m_tester = new MainWindow;
        m_tester->initUI();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MainWindow *m_tester;
};

bool isWritable_stub()
{
    return false;
}

bool isExecutable_stub()
{
    return false;
}

int showDialog_stub(const QString &strDesText, const QString btnMsg, DDialog::ButtonType btnType)
{
    return false;
}

TEST_F(TestMainWindow, initTest)
{

}

TEST_F(TestMainWindow, testcheckHerePath)
{
    Stub stub;
    stub.set(ADDR(TipDialog, showDialog), showDialog_stub);
    stub.set(ADDR(QFileInfo, isWritable), isWritable_stub);
    stub.set(ADDR(QFileInfo, isExecutable), isExecutable_stub);

    QString strPath = QFileInfo("../UnitTest/test_sources/noPermissionDir").absoluteFilePath();

    ASSERT_EQ(m_tester->checkHerePath(strPath), false);
}

TEST_F(TestMainWindow, testcheckSettings)
{
}


TEST_F(TestMainWindow, testhandleApplicationTabEventNotify)
{
    m_tester->m_pUnCompressPage = nullptr;
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(nullptr, nullptr), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_001_TitleButtonVisible)
{
    m_tester->m_pTitleButton->setVisible(true);
    m_tester->m_pTitleButton->setEnabled(true);
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(m_tester->titlebar(), evt), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_001_TitleCommentButtonVisible)
{
    m_tester->m_pTitleCommentButton->setVisible(true);
    m_tester->m_pTitleCommentButton->setEnabled(true);
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(m_tester->titlebar(), evt), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_001_Other)
{
    m_tester->m_pTitleButton->setVisible(false);
    m_tester->m_pTitleCommentButton->setVisible(false);
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(m_tester->titlebar(), evt), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_002)
{
    QObject *p = new QObject(m_tester);
    p->setObjectName("CommentButton");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_003)
{
    QObject *p = new QObject(m_tester);
    p->setObjectName("TitleButton");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_004_UnCompress)
{
    m_tester->m_ePageID = PI_UnCompress;
    QObject *p = new QObject(m_tester);
    p->setObjectName("gotoPreviousLabel");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), true);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_004_Compress)
{
    m_tester->m_ePageID = PI_Compress;
    QObject *p = new QObject(m_tester);
    p->setObjectName("gotoPreviousLabel");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), true);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_004_Other)
{
    m_tester->m_ePageID = PI_Home;
    QObject *p = new QObject(m_tester);
    p->setObjectName("gotoPreviousLabel");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_005_UnCompress)
{
    m_tester->m_ePageID = PI_UnCompress;
    QObject *p = new QObject(m_tester);
    p->setObjectName("TableViewFile");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), true);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_005__Compress)
{
    m_tester->m_ePageID = PI_Compress;
    QObject *p = new QObject(m_tester);
    p->setObjectName("TableViewFile");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), true);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_005__Other)
{
    m_tester->m_ePageID = PI_Home;
    QObject *p = new QObject(m_tester);
    p->setObjectName("TableViewFile");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(p, evt), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_006)
{
    m_tester->m_ePageID = PI_UnCompress;
    DWindowCloseButton *closebtn = m_tester->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(closebtn, evt), true);
}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_001)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_002)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_003)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_004)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_005)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_006)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Left_001)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Left_002)
//{

//}

//TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Left_003)
//{

//}

TEST_F(TestMainWindow, testhandleQuit)
{
    m_tester->close();
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestMainWindow, testinitUI)
{
    m_tester->initUI();
    ASSERT_NE(m_tester->m_pOpenFileWatcher, nullptr);
}

TEST_F(TestMainWindow, testinitTitleBar)
{
    m_tester->initTitleBar();
    ASSERT_EQ(m_tester->m_pTitleCommentButton->isVisible(), false);
}

TEST_F(TestMainWindow, testinitData)
{
    m_tester->initData();
    ASSERT_NE(m_tester->m_pSettings, nullptr);
}

TEST_F(TestMainWindow, testinitConnections)
{
    m_tester->initConnections();
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestMainWindow, testrefreshPage_Home)
{
    m_tester->m_ePageID = PI_Home;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 0);
}

TEST_F(TestMainWindow, testrefreshPage_Compress)
{
    m_tester->m_ePageID = PI_Compress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 1);
}

TEST_F(TestMainWindow, testrefreshPage_CompressSetting)
{
    m_tester->m_ePageID = PI_CompressSetting;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 2);
}

TEST_F(TestMainWindow, testrefreshPage_UnCompres)
{
    m_tester->m_ePageID = PI_UnCompress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 3);
}

TEST_F(TestMainWindow, testrefreshPage_AddCompressProgress)
{
    m_tester->m_ePageID = PI_AddCompressProgress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(TestMainWindow, testrefreshPage_CompressProgress)
{
    m_tester->m_ePageID = PI_CompressProgress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(TestMainWindow, testrefreshPage_UnCompressProgress)
{
    m_tester->m_ePageID = PI_UnCompressProgress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(TestMainWindow, testrefreshPage_DeleteProgress)
{
    m_tester->m_ePageID = PI_DeleteProgress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(TestMainWindow, testrefreshPage_ConvertProgress)
{
    m_tester->m_ePageID = PI_ConvertProgress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(TestMainWindow, testrefreshPage_CommentProgress)
{
    m_tester->m_ePageID = PI_CommentProgress;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(TestMainWindow, testrefreshPage_Success)
{
    m_tester->m_ePageID = PI_Success;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 5);
}

TEST_F(TestMainWindow, testrefreshPage_Failure)
{
    m_tester->m_ePageID = PI_Failure;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 6);
}

TEST_F(TestMainWindow, testrefreshPage_Loading)
{
    m_tester->m_ePageID = PI_Loading;
    m_tester->refreshPage();
    ASSERT_EQ(m_tester->m_pMainWidget->currentIndex(), 7);
}

TEST_F(TestMainWindow, testcalSelectedTotalFileSize)
{
    QString strPath = QFileInfo("../UnitTest/test_sources/calSize").absoluteFilePath();
    ASSERT_EQ(m_tester->calSelectedTotalFileSize(QStringList() << strPath), 30);
}

TEST_F(TestMainWindow, testcalFileSizeByThread)
{
    m_tester->m_stCompressParameter.qSize = 0;
    QString strPath = QFileInfo("../UnitTest/test_sources/calSize").absoluteFilePath();
    m_tester->calFileSizeByThread(strPath);
    ASSERT_EQ(m_tester->m_stCompressParameter.qSize, 30);
}

TEST_F(TestMainWindow, testsetTitleButtonStyle_IncreaseElement)
{
    m_tester->setTitleButtonStyle(true, false, DStyle::StandardPixmap::SP_IncreaseElement);
    ASSERT_EQ(m_tester->m_pTitleButton->toolTip(), QObject::tr("Open file"));
}

TEST_F(TestMainWindow, testsetTitleButtonStyle_ArrowLeave)
{
    m_tester->setTitleButtonStyle(true, false, DStyle::StandardPixmap::SP_ArrowLeave);
    ASSERT_EQ(m_tester->m_pTitleButton->toolTip(), QObject::tr("Back"));
}

TEST_F(TestMainWindow, testloadArchive)
{
    QString strPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    m_tester->loadArchive(strPath);
    ASSERT_EQ(m_tester->m_ePageID, PI_Loading);
}

TEST_F(TestMainWindow, testExtract2PathFinish)
{
    m_tester->Extract2PathFinish("sss");
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestMainWindow, testcreateUUID)
{
    ASSERT_EQ(m_tester->createUUID().isEmpty(), false);
}

bool isAutoCreatDir_stub_true()
{
    return true;
}

bool isAutoCreatDir_stub_false()
{
    return false;
}

TEST_F(TestMainWindow, testgetExtractPath_AutoCreatDir)
{
    Stub stub;
    stub.set(ADDR(SettingDialog, isAutoCreatDir), isAutoCreatDir_stub_true);
    QString strPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    ASSERT_EQ(m_tester->getExtractPath(strPath), "test");
}

TEST_F(TestMainWindow, testgetExtractPath_NoAutoCreatDir)
{
    Stub stub;
    stub.set(ADDR(SettingDialog, isAutoCreatDir), isAutoCreatDir_stub_false);
    QString strPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    ASSERT_EQ(m_tester->getExtractPath(strPath), "");
}

TEST_F(TestMainWindow, testhandleJobNormalFinished)
{

}

TEST_F(TestMainWindow, testhandleJobCancelFinished)
{

}

TEST_F(TestMainWindow, testhandleJobErrorFinished)
{

}

TEST_F(TestMainWindow, testaddFiles2Archive)
{

}

TEST_F(TestMainWindow, testresetMainwindow)
{
    m_tester->m_eStartupType = StartupType::ST_Compress;
    m_tester->resetMainwindow();
    ASSERT_EQ(m_tester->m_eStartupType, StartupType::ST_Normal);
}

TEST_F(TestMainWindow, testdeleteWhenJobFinish)
{
    m_tester->deleteWhenJobFinish(ArchiveJob::JobType::JT_Add);
//ASSERT_EQ(m_tester->m_eStartupType, StartupType::ST_Normal);

}
TEST_F(TestMainWindow, testConstructAddOptionsh)
{
    QStringList files{"file1", "file2"};
    m_tester->ConstructAddOptions(files);
    ASSERT_EQ(m_tester->m_stUpdateOptions.listEntry.size(), files.size());
}

TEST_F(TestMainWindow, testshowSuccessInfo_001)
{
    m_tester->showSuccessInfo(SuccessInfo::SI_Compress);
    ASSERT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Compression successful");
}

TEST_F(TestMainWindow, testshowSuccessInfo_002)
{
    m_tester->showSuccessInfo(SuccessInfo::SI_UnCompress);
    ASSERT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Extraction successful");
}

TEST_F(TestMainWindow, testshowSuccessInfo_003)
{
    m_tester->showSuccessInfo(SuccessInfo::SI_Convert);
    ASSERT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Conversion successful");
}

TEST_F(TestMainWindow, testshowErrorMessage_001)
{
    m_tester->showErrorMessage(FailureInfo::FI_Compress, ErrorInfo::EI_NoPlugin);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Plugin error");
}

TEST_F(TestMainWindow, testshowErrorMessage_002)
{
    m_tester->showErrorMessage(FailureInfo::FI_Uncompress, ErrorInfo::EI_InsufficientDiskSpace);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Insufficient disk space");
}

QVariant value_stub(const QString &key, const QVariant &defaultValue = QVariant())
{
    return 0;
}

TEST_F(TestMainWindow, testgetConfigWinSize)
{
    Stub stub;
    stub.set(ADDR(QSettings, value), value_stub);
    QSize s = m_tester->getConfigWinSize();
    ASSERT_EQ(s.width(), MAINWINDOW_DEFAULTW);
    ASSERT_EQ(s.height(), MAINWINDOW_DEFAULTH);
}

TEST_F(TestMainWindow, testsaveConfigWinSize)
{
    m_tester->saveConfigWinSize(1024, 768);
}

TEST_F(TestMainWindow, testgetDefaultApp)
{
    m_tester->getDefaultApp("vnd.rar");
}

bool convertArchive_stub(const QString strOriginalArchiveFullPath, const QString strTargetFullPath, const QString strNewArchiveFullPath)
{
    return true;
}

TEST_F(TestMainWindow, testconvertArchive)
{
    Stub stub;
    stub.set(ADDR(ArchiveManager, convertArchive), convertArchive_stub);
    m_tester->m_stUnCompressParameter.strFullPath = "test.rar";
    m_tester->convertArchive("zip");
    ASSERT_EQ(m_tester->m_pProgressPage->m_eType, Progress_Type::PT_Convert);
}

bool updateArchiveComment_stub(const QString &strArchiveFullPath, const QString &strComment)
{
    return true;
}

TEST_F(TestMainWindow, testupdateArchiveComment)
{
    Stub stub;
    stub.set(ADDR(ArchiveManager, updateArchiveComment), updateArchiveComment_stub);
    m_tester->m_stUnCompressParameter.strFullPath = "test.zip";
    m_tester->m_comment = "one";
    m_tester->updateArchiveComment();
}

QString getComment_stub()
{
    return "two";
}

TEST_F(TestMainWindow, testaddArchiveComment)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, getComment), getComment_stub);
    stub.set(ADDR(ArchiveManager, updateArchiveComment), updateArchiveComment_stub);
    m_tester->m_stCompressParameter.strTargetPath = "./";
    m_tester->m_stCompressParameter.strArchiveName = "test.zip";

    m_tester->addArchiveComment();
    ASSERT_EQ(m_tester->m_pProgressPage->m_eType, Progress_Type::PT_Comment);
}

TEST_F(TestMainWindow, testcreatShorcutJson)
{
    ASSERT_EQ(m_tester->creatShorcutJson().value("shortcut").isUndefined(), false);
}

void loadArchive_stub(const QString &strArchiveFullPath)
{
    return;
}

TEST_F(TestMainWindow, testhandleArguments_Open)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);

    QStringList listParam;
    listParam << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("b");
    m_tester->handleArguments_Open(listParam);
    ASSERT_EQ(m_tester->m_eStartupType, StartupType::ST_Normal);
}

TEST_F(TestMainWindow, testhandleArguments_RightMenu_001)
{
    QStringList listParam;
    listParam << QStringLiteral("a");
    ASSERT_EQ(m_tester->handleArguments_RightMenu(listParam), false);
}

TEST_F(TestMainWindow, testhandleArguments_Open_002)
{
    QStringList listParam;
    listParam << QStringLiteral("a") << QStringLiteral("compress");
    ASSERT_EQ(m_tester->handleArguments_RightMenu(listParam), true);
}

TEST_F(TestMainWindow, testhandleArguments_Append_001)
{
    QStringList listParam;
    listParam << QStringLiteral("a") << QStringLiteral("compress");
    ASSERT_EQ(m_tester->handleArguments_Append(listParam), false);
}

TEST_F(TestMainWindow, testhandleArguments_Append_002)
{

}

TEST_F(TestMainWindow, testrightExtract2Path)
{

}

QString archiveFullPath_stub()
{
    return QFileInfo("../UnitTest/test_sources/tar/extract").absoluteFilePath() + "/test.tar";
}

bool extractFiles2Path(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions)
{
    return true;
}

TEST_F(TestMainWindow, testslotExtract2Path)
{
    ExtractionOptions stOptions;
    stOptions.strTargetPath = QFileInfo("../UnitTest/test_sources/tar/extract").absoluteFilePath();

    QList<FileEntry> listSelEntry;
    FileEntry fileentry1;
    fileentry1.strFullPath = "test1.txt";
    listSelEntry << fileentry1;
    m_tester->slotExtract2Path(listSelEntry, stOptions);
    ASSERT_EQ(QFileInfo(stOptions.strTargetPath + "/test1.txt").exists(), true);
    QFile::remove(stOptions.strTargetPath + "/test1.txt");
}

//TEST_F(TestMainWindow, testslotDelFiles)
//{

//}
