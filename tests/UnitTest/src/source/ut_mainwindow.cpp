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
#include "uitools.h"
#include "compresspage.h"
#include "uncompresspage.h"
#include "archivejob.h"
#include "commonstruct.h"
#include "progressdialog.h"
#include "progresspage.h"
#include "queries.h"
#include "settingdialog.h"
#include "datamanager.h"
#include "successpage.h"
#include "failurepage.h"
#include "compresssettingpage.h"
#include "ddesktopservicesthread.h"
#include "openFileWatcher.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <QTest>
#include <QAction>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

/*******************************函数打桩************************************/
bool handleArguments_Open_stub(const QStringList &)
{
    return true;
}

bool handleArguments_RightMenu_stub(const QStringList &)
{
    return true;
}

bool handleArguments_Append_stub(const QStringList &)
{
    return true;
}

void slotChoosefiles_stub()
{
    return;
}

void loadArchive_stub(const QString &)
{
    return;
}

void compressPage_addCompressFiles_stub(const QStringList &)
{
    return;
}

QStringList compressPage_compressFiles_stub()
{
    return QStringList() << "1.txt";
}

void UnCompressPage_addNewFiles_stub(const QStringList &)
{
    return;
}

void handleJobNormalFinished_stub(ArchiveJob::JobType)
{
    return ;
}

void handleJobCancelFinished_stub(ArchiveJob::JobType)
{
    return ;
}

void handleJobErrorFinished_stub(ArchiveJob::JobType, ErrorType)
{
    return ;
}

void progressdialog_showDialog_stub()
{
    return ;
}

bool settingDialog_isAutoOpen_stub()
{
    return true;
}

bool settingDialog_isAutoCreatDir_stub()
{
    return true;
}

bool settingDialog_isAutoDeleteFile_stub()
{
    return true;
}
/*******************************函数打桩************************************/


class TestMainWindow : public ::testing::Test
{
public:
    TestMainWindow(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MainWindow;
        m_tester->initUI();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MainWindow *m_tester;
};

TEST_F(TestMainWindow, initTest)
{

}

TEST_F(TestMainWindow, testcheckHerePath)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, false);
    m_tester->checkHerePath("/home/Desktop");


    Stub stub1;
    CustomDialogStub::stub_TipDialog_showDialog(stub1, 0);
    QFileInfoStub::stub_QFileInfo_isWritable(stub1, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub1, true);
    m_tester->checkHerePath("/home/Desktop");
}

TEST_F(TestMainWindow, testrefreshPage)
{
    m_tester->m_ePageID = PI_Home;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_Compress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_CompressSetting;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_UnCompress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_AddCompressProgress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_CompressProgress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_UnCompressProgress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_DeleteProgress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_ConvertProgress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_CommentProgress;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_Success;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_Failure;
    m_tester->refreshPage();

    m_tester->m_ePageID = PI_Loading;
    m_tester->refreshPage();
}

TEST_F(TestMainWindow, testsetTitleButtonStyle)
{
    m_tester->setTitleButtonStyle(true, true, DStyle::SP_IncreaseElement);
    ASSERT_EQ(m_tester->m_pTitleButton->toolTip(), "Open file");
    m_tester->setTitleButtonStyle(true, true, DStyle::SP_ArrowLeave);
    ASSERT_EQ(m_tester->m_pTitleButton->toolTip(), "Back");
}

TEST_F(TestMainWindow, testloadArchive)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isReadable(stub, false);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->m_operationtype = Operation_NULL;
    m_tester->loadArchive("1.zip");
    ASSERT_EQ(m_tester->m_operationtype != Operation_Load, true);

    Stub stub1;
    QFileInfoStub::stub_QFileInfo_isReadable(stub1, true);
    QFileInfoStub::stub_QFileInfo_exists(stub1, false);
    m_tester->loadArchive("1.zip");
    ASSERT_EQ(m_tester->m_ePageID == PI_Failure, true);

    Stub stub2;
    QFileInfoStub::stub_QFileInfo_isReadable(stub2, true);
    QFileInfoStub::stub_QFileInfo_exists(stub2, true);
    ArchiveManagerstub::stub_ArchiveManager_loadArchive(stub2, true);
    m_tester->loadArchive("1.zip");
    ASSERT_EQ(m_tester->m_ePageID == PI_Loading, true);

    Stub stub3;
    QFileInfoStub::stub_QFileInfo_isReadable(stub3, true);
    QFileInfoStub::stub_QFileInfo_exists(stub3, true);
    ArchiveManagerstub::stub_ArchiveManager_loadArchive(stub3, false);
    m_tester->loadArchive("1.zip");
    ASSERT_EQ(m_tester->m_ePageID == PI_Home, true);
}

TEST_F(TestMainWindow, testcheckSettings)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);

    Stub stub1;
    QFileInfoStub::stub_QFileInfo_exists(stub1, false);
    ASSERT_EQ(m_tester->checkSettings("1.zip"), false);

    Stub stub2;
    QFileInfoStub::stub_QFileInfo_exists(stub2, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub2, false);
    ASSERT_EQ(m_tester->checkSettings("1.zip"), false);

    Stub stub3;
    QFileInfoStub::stub_QFileInfo_exists(stub3, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub3, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub3, true);
    ASSERT_EQ(m_tester->checkSettings("1/"), false);

    Stub stub4;
    QFileInfoStub::stub_QFileInfo_exists(stub4, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub4, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub4, false);
    ASSERT_EQ(m_tester->checkSettings(""), true);

    Stub stub5;
    QFileInfoStub::stub_QFileInfo_exists(stub5, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub5, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub5, false);
    m_tester->checkSettings("1.zip");
}

TEST_F(TestMainWindow, testhandleQuit)
{
    m_tester->handleQuit();
}

TEST_F(TestMainWindow, testslotHandleArguments)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleArguments_Open), handleArguments_Open_stub);
    stub.set(ADDR(MainWindow, handleArguments_RightMenu), handleArguments_RightMenu_stub);
    stub.set(ADDR(MainWindow, handleArguments_Append), handleArguments_Append_stub);

    m_tester->slotHandleArguments(QStringList(), MainWindow::ArgumentType::AT_Open);
    m_tester->slotHandleArguments(QStringList() << "1.zip", MainWindow::ArgumentType::AT_Open);
    m_tester->slotHandleArguments(QStringList() << "1.zip", MainWindow::ArgumentType::AT_RightMenu);
    m_tester->slotHandleArguments(QStringList() << "1.zip", MainWindow::ArgumentType::AT_DragDropAdd);

}

TEST_F(TestMainWindow, testslotTitleBtnClicked)
{
    Stub stub;
    stub.set(ADDR(MainWindow, slotChoosefiles), slotChoosefiles_stub);

    m_tester->m_ePageID = PI_Home;
    m_tester->slotTitleBtnClicked();
    m_tester->m_ePageID = PI_CompressSetting;
    m_tester->slotTitleBtnClicked();
    ASSERT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(TestMainWindow, testslotChoosefiles)
{
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 0);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    stub.set(ADDR(CompressPage, addCompressFiles), compressPage_addCompressFiles_stub);
    stub.set(ADDR(UnCompressPage, addNewFiles), UnCompressPage_addNewFiles_stub);
    m_tester->slotChoosefiles();

    Stub stub1;
    DFileDialogStub::stub_DFileDialog_exec(stub1, 1);
    DFileDialogStub::stub_DFileDialog_selectedFiles(stub1, QStringList() << "1.txt");
    CommonStub::stub_UiTools_isLocalDeviceFile(stub1, true);
    CommonStub::stub_UiTools_isArchiveFile(stub1, true);
    m_tester->m_ePageID = PI_Home;
    m_tester->slotChoosefiles();
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);


    Stub stub2;
    DFileDialogStub::stub_DFileDialog_exec(stub2, 1);
    DFileDialogStub::stub_DFileDialog_selectedFiles(stub2, QStringList() << "1.txt");
    CommonStub::stub_UiTools_isLocalDeviceFile(stub2, true);
    CommonStub::stub_UiTools_isArchiveFile(stub2, false);
    m_tester->m_ePageID = PI_Home;
    m_tester->slotChoosefiles();
    ASSERT_EQ(m_tester->m_ePageID, PI_Compress);

    m_tester->m_ePageID = PI_Compress;
    m_tester->slotChoosefiles();
    ASSERT_EQ(m_tester->m_ePageID, PI_Compress);

    m_tester->m_ePageID = PI_UnCompress;
    m_tester->slotChoosefiles();
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->m_ePageID = PI_CompressSetting;
    m_tester->slotChoosefiles();
    ASSERT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(TestMainWindow, testslotDragSelectedFiles)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    stub.set(ADDR(CompressPage, addCompressFiles), compressPage_addCompressFiles_stub);

    m_tester->slotDragSelectedFiles(QStringList());

    m_tester->slotDragSelectedFiles(QStringList() << "1.txt" << "2.txt");
    ASSERT_EQ(m_tester->m_ePageID, PI_Compress);

    Stub stub1;
    CommonStub::stub_UiTools_isArchiveFile(stub1, true);
    m_tester->slotDragSelectedFiles(QStringList() << "1.zip");

    Stub stub2;
    CommonStub::stub_UiTools_isArchiveFile(stub2, false);
    m_tester->slotDragSelectedFiles(QStringList() << "1.txt");
    ASSERT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(TestMainWindow, testslotCompressLevelChanged)
{
    m_tester->slotCompressLevelChanged(true);
    ASSERT_EQ(m_tester->m_pOpenAction->isEnabled(), true);
}

TEST_F(TestMainWindow, testslotCompressNext)
{
    m_tester->slotCompressNext();
    ASSERT_EQ(m_tester->m_ePageID, PI_CompressSetting);
}

TEST_F(TestMainWindow, testslotCompress)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_createArchive(stub, true);
    stub.set(ADDR(CompressPage, compressFiles), compressPage_compressFiles_stub);
    m_tester->slotCompress(QVariant());
    ASSERT_EQ(m_tester->m_ePageID, PI_CompressProgress);

    Stub stub1;
    ArchiveManagerstub::stub_ArchiveManager_createArchive(stub1, false);
    m_tester->slotCompress(QVariant());
    ASSERT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(TestMainWindow, testslotJobFinished)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleJobNormalFinished), handleJobNormalFinished_stub);
    stub.set(ADDR(MainWindow, handleJobCancelFinished), handleJobCancelFinished_stub);
    stub.set(ADDR(MainWindow, handleJobErrorFinished), handleJobErrorFinished_stub);

    m_tester->slotJobFinished(ArchiveJob::JT_Create, PFT_Nomral, ET_NoError);
    m_tester->slotJobFinished(ArchiveJob::JT_Create, PFT_Cancel, ET_UserCancelOpertion);
    m_tester->slotJobFinished(ArchiveJob::JT_Create, PFT_Error, ET_FileWriteError);
}

TEST_F(TestMainWindow, testslotUncompressClicked)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_extractFiles(stub, true);
    m_tester->slotUncompressClicked("/home/Desktop");
}

TEST_F(TestMainWindow, testslotReceiveProgress)
{
    Stub stub;
    stub.set(ADDR(ProgressDialog, showDialog), progressdialog_showDialog_stub);
    CommonStub::stub_QDialog_exec(stub, 0);

    m_tester->m_operationtype = Operation_SingleExtract;
    m_tester->slotReceiveProgress(50);
    ASSERT_EQ(m_tester->m_pProgressdialog->m_dPerent, 50);

    m_tester->m_operationtype = Operation_Update_Comment;
    m_tester->slotReceiveProgress(50);
    ASSERT_EQ(m_tester->m_pProgressdialog->m_dPerent, 50);

    m_tester->m_operationtype = Operation_Extract;
    m_tester->slotReceiveProgress(50);
    ASSERT_EQ(m_tester->m_pProgressPage->m_iPerent, 50);
}

TEST_F(TestMainWindow, testslotReceiveCurFileName)
{
    m_tester->m_operationtype = Operation_SingleExtract;
    m_tester->slotReceiveCurFileName("1.txt");

    m_tester->m_operationtype = Operation_Extract;
    m_tester->slotReceiveCurFileName("1.txt");
}

TEST_F(TestMainWindow, testslotReceiveFileWriteErrorName)
{
    m_tester->m_fileWriteErrorName.clear();
    m_tester->slotReceiveFileWriteErrorName("1.txt");
    ASSERT_EQ(m_tester->m_fileWriteErrorName, "1.txt");
}

TEST_F(TestMainWindow, testslotQuery)
{
    Stub stub;
    CommonStub::stub_OverwriteQuery_execute(stub);
    OverwriteQuery query("", m_tester);
    m_tester->slotQuery(&query);
}

TEST_F(TestMainWindow, testExtract2PathFinish)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoOpen), settingDialog_isAutoOpen_stub);
    m_tester->Extract2PathFinish("ddd");
}

TEST_F(TestMainWindow, testcreateTempPath)
{
    ASSERT_EQ(m_tester->createTempPath().isEmpty(), false);
}

TEST_F(TestMainWindow, testgetExtractPath)
{
    Stub stub;
    stub.set(ADDR(SettingDialog, isAutoCreatDir), settingDialog_isAutoCreatDir_stub);
    ASSERT_EQ(m_tester->getExtractPath("/home/Desktop/1.zip"), "1");
}

TEST_F(TestMainWindow, testhandleJobNormalFinished)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerstub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerstub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_iCompressedWatchTimerID = 1000;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Create);
    ASSERT_EQ(m_tester->m_ePageID, PI_Success);

    m_tester->m_eStartupType = StartupType::ST_DragDropAdd;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Add);
    ASSERT_EQ(m_tester->m_ePageID, PI_Success);
    m_tester->m_eStartupType = StartupType::ST_Normal;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Add);
    ASSERT_EQ(m_tester->m_ePageID, PI_Loading);

    m_tester->handleJobNormalFinished(ArchiveJob::JT_Load);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
    DataManager::get_instance().archiveData().reset();
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Load);
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);

    m_tester->m_operationtype = Archive_OperationType::Operation_SingleExtract;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
    m_tester->m_operationtype = Archive_OperationType::Operation_Extract;
    DataManager::get_instance().archiveData().reset();
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    ASSERT_EQ(m_tester->m_ePageID, PI_Failure);
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->m_stUnCompressParameter.bBatch = true;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    ASSERT_EQ(m_tester->m_ePageID, PI_Success);
    m_tester->m_stUnCompressParameter.bBatch = false;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    ASSERT_EQ(m_tester->m_ePageID, PI_Success);

    m_tester->handleJobNormalFinished(ArchiveJob::JT_Delete);
    ASSERT_EQ(m_tester->m_ePageID, PI_Loading);

    m_tester->m_stUnCompressParameter.bModifiable = true;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Open);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->handleJobNormalFinished(ArchiveJob::JT_Convert);
    ASSERT_EQ(m_tester->m_ePageID, PI_Success);

    m_tester->handleJobNormalFinished(ArchiveJob::JT_Update);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    DataManager::get_instance().archiveData().reset();
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Update);
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);

    m_tester->m_operationtype = Operation_Update_Comment;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Comment);
    ASSERT_EQ(m_tester->m_commentProgressDialog->m_progressBar->value(), 100);
    m_tester->m_operationtype = Operation_NULL;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Comment);
    ASSERT_EQ(m_tester->m_ePageID, PI_Success);
}

TEST_F(TestMainWindow, testhandleJobCancelFinished)
{
    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Create);
    ASSERT_EQ(m_tester->m_operationtype, Operation_NULL);
    m_tester->m_eStartupType = StartupType::ST_Compress;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Create);
    ASSERT_EQ(m_tester->m_ePageID, PI_Compress);

    m_tester->m_eStartupType = StartupType::ST_DragDropAdd;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Add);
    ASSERT_EQ(m_tester->m_operationtype, Operation_NULL);
    m_tester->m_eStartupType = StartupType::ST_Compress;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Add);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->handleJobCancelFinished(ArchiveJob::JT_Load);
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);

    m_tester->m_operationtype = Archive_OperationType::Operation_SingleExtract;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Extract);
    m_tester->m_operationtype = Archive_OperationType::Operation_Extract;
    m_tester->m_eStartupType = ST_ExtractHere;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Extract);
    ASSERT_EQ(m_tester->m_operationtype, Operation_NULL);
    m_tester->m_eStartupType = ST_Extract;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Extract);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->handleJobCancelFinished(ArchiveJob::JT_Delete);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->handleJobCancelFinished(ArchiveJob::JT_Open);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->handleJobCancelFinished(ArchiveJob::JT_Convert);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(TestMainWindow, testhandleJobErrorFinished)
{
    m_tester->m_iCompressedWatchTimerID = 1000;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Create, ET_InsufficientDiskSpace);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Create, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Create, ET_FileWriteError);

    m_tester->m_eStartupType = ST_Extract;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_WrongPassword);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_FileWriteError);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
    m_tester->m_eStartupType = ST_DragDropAdd;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_FileWriteError);

    m_tester->handleJobErrorFinished(ArchiveJob::JT_Load, ET_ArchiveDamaged);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Load, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Load, ET_FileWriteError);

    m_tester->m_operationtype = Operation_SingleExtract;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_FileWriteError);
    m_tester->m_operationtype = Operation_Extract;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_ArchiveDamaged);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_FileWriteError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_MissingVolume);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_InsufficientDiskSpace);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_PluginError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_NeedPassword);

    m_tester->handleJobErrorFinished(ArchiveJob::JT_Delete, ET_FileWriteError);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_ArchiveDamaged);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_FileWriteError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_InsufficientDiskSpace);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_PluginError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_NeedPassword);

    m_tester->m_operationtype = Operation_TempExtract_Open;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Open, ET_WrongPassword);
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(TestMainWindow, testaddFiles2Archive)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    m_tester->addFiles2Archive(QStringList() << "1.txt");
    ASSERT_EQ(m_tester->m_ePageID, PI_AddCompressProgress);
}

TEST_F(TestMainWindow, testresetMainwindow)
{
    m_tester->resetMainwindow();
    ASSERT_EQ(m_tester->m_eStartupType, StartupType::ST_Normal);
}

TEST_F(TestMainWindow, testdeleteWhenJobFinish)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
    m_tester->m_stCompressParameter.listCompressFiles << "1/";
    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Create);

    Stub stub1;
    QFileInfoStub::stub_QFileInfo_isDir(stub1, false);
    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
    m_tester->m_stCompressParameter.listCompressFiles << "1.txt";
    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Create);

    m_tester->m_stUnCompressParameter.listBatchFiles << "1.txt";
    m_tester->m_stUnCompressParameter.strFullPath = "1.txt";
    m_tester->m_eStartupType = StartupType::ST_ExtractHere;
    m_tester->m_stUnCompressParameter.bBatch = true;
    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Extract);
    m_tester->m_stUnCompressParameter.bBatch = false;
    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Extract);
}

TEST_F(TestMainWindow, testConstructAddOptions)
{
    Stub stub;
    CommonStub::stub_QThreadPool_waitForDone(stub);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);

    m_tester->ConstructAddOptions(QStringList() << "1.txt");
}

TEST_F(TestMainWindow, testConstructAddOptionsByThread)
{
    Stub stub;
    QDirStub::stub_QDir_exists(stub, true);
    QFileInfoList listInfo = QFileInfoList() << QFileInfo("1.txt");
    QDirStub::stub_QDir_entryInfoList(stub, listInfo);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);

    m_tester->ConstructAddOptionsByThread("/home/Desktop");
}

TEST_F(TestMainWindow, testshowSuccessInfo)
{
    m_tester->showSuccessInfo(SI_Compress);
    ASSERT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Compression successful");
    m_tester->showSuccessInfo(SI_UnCompress);
    ASSERT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Extraction successful");
    m_tester->showSuccessInfo(SI_Convert);
    ASSERT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Conversion successful");
}

TEST_F(TestMainWindow, testshowErrorMessage)
{
    m_tester->showErrorMessage(FI_Compress, EI_NoPlugin);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Plugin error");
    m_tester->showErrorMessage(FI_Compress, EI_CreatArchiveFailed);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Failed to create file");
    m_tester->showErrorMessage(FI_Compress, EI_InsufficientDiskSpace);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Insufficient disk space");

    m_tester->showErrorMessage(FI_Load, EI_NoPlugin);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Plugin error");
    m_tester->showErrorMessage(FI_Load, EI_ArchiveDamaged);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "The archive is damaged");
    m_tester->showErrorMessage(FI_Load, EI_WrongPassword);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Wrong password");
    m_tester->showErrorMessage(FI_Load, EI_ArchiveMissingVolume);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Some volumes are missing");

    m_tester->showErrorMessage(FI_Uncompress, EI_NoPlugin);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Plugin error");
    m_tester->showErrorMessage(FI_Uncompress, EI_ArchiveDamaged);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "The archive is damaged");
    m_tester->showErrorMessage(FI_Uncompress, EI_ArchiveMissingVolume);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Some volumes are missing");
    m_tester->showErrorMessage(FI_Uncompress, EI_WrongPassword);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Wrong password, please retry");
    m_tester->showErrorMessage(FI_Uncompress, EI_LongFileName);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "File name too long");
    m_tester->showErrorMessage(FI_Uncompress, EI_CreatFileFailed);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text().contains("Failed to create"), true);
    m_tester->showErrorMessage(FI_Uncompress, EI_ArchiveNoData);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "No data in it");
    m_tester->showErrorMessage(FI_Uncompress, EI_InsufficientDiskSpace);
    ASSERT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Insufficient disk space");
}

TEST_F(TestMainWindow, testgetConfigWinSize)
{
    m_tester->getConfigWinSize();
}

TEST_F(TestMainWindow, testsaveConfigWinSize)
{
    m_tester->saveConfigWinSize(800, 600);
}

TEST_F(TestMainWindow, testconvertArchive)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_convertArchive(stub, true);
    m_tester->convertArchive("zip");
}

TEST_F(TestMainWindow, testupdateArchiveComment)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_updateArchiveComment(stub, true);
    m_tester->updateArchiveComment();
    ASSERT_EQ(m_tester->m_operationtype, Operation_Update_Comment);
}

TEST_F(TestMainWindow, testaddArchiveComment)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_updateArchiveComment(stub, true);
    m_tester->m_pCompressSettingPage->m_pCommentEdt->setPlainText("123456");
    m_tester->addArchiveComment();
    ASSERT_EQ(m_tester->m_ePageID, PI_CommentProgress);
}

TEST_F(TestMainWindow, testwatcherArchiveFile)
{
    m_tester->watcherArchiveFile("1.txt");
}

TEST_F(TestMainWindow, testcreatShorcutJson)
{
    m_tester->creatShorcutJson();
}

TEST_F(TestMainWindow, testhandleArguments_Open)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);

    ASSERT_EQ(m_tester->handleArguments_Open(QStringList()), false);
    ASSERT_EQ(m_tester->handleArguments_Open(QStringList() << "1.zip"), true);
}

TEST_F(TestMainWindow, testhandleArguments_RightMenu)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 0);
    ArchiveManagerstub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerstub::stub_ArchiveManager_extractFiles(stub, true);
    ASSERT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip"), false);
    ASSERT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.txt" << "compress"), true);
    ASSERT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.txt" << "compress_to_7z"), true);
    ASSERT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "compress_to_7z"), true);
    ASSERT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "2.zip" << "compress_to_7z"), true);
    ASSERT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "extract_here"), true);
    ASSERT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "extract_to_specifypath"), true);
}

TEST_F(TestMainWindow, testhandleArguments_Append)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CommonStub::stub_QThread_start(stub);
    PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(stub);

    ASSERT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "dragdropadd"), false);
    ASSERT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "1.zip" << "dragdropadd"), false);
    Stub stub1;
    QFileInfoStub::stub_QFileInfo_isWritable(stub1, false);
    ASSERT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "1.zip" << "dragdropadd"), false);
    Stub stub2;
    QFileInfoStub::stub_QFileInfo_isWritable(stub2, true);
    ASSERT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "1.txt" << "dragdropadd"), true);
}

TEST_F(TestMainWindow, testrightExtract2Path)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_extractFiles(stub, true);
    ArchiveManagerstub::stub_ArchiveManager_batchExtractFiles(stub, true);

    m_tester->rightExtract2Path(ST_Extract, QStringList());

    m_tester->m_eStartupType = StartupType::ST_ExtractHere;
    m_tester->rightExtract2Path(ST_Extract, QStringList() << "1.zip");
    m_tester->m_eStartupType = StartupType::ST_Extract;
    m_tester->rightExtract2Path(ST_Extract, QStringList() << "1.zip");
    m_tester->m_eStartupType = StartupType::ST_ExtractHere;
    m_tester->rightExtract2Path(ST_Extract, QStringList() << "1.zip" << "2.zip");
}

TEST_F(TestMainWindow, testshowWarningDialog)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->showWarningDialog("", "");
}

TEST_F(TestMainWindow, testmoveDialogToCenter)
{
    DDialog dialog(m_tester);
    m_tester->moveDialogToCenter(&dialog);
}

TEST_F(TestMainWindow, testdelayQuitApp)
{
    m_tester->delayQuitApp();
}

TEST_F(TestMainWindow, testslotExtract2Path)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_extractFiles2Path(stub, false);
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    listEntry << entry;
    m_tester->slotExtract2Path(listEntry, ExtractionOptions());
    ASSERT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(TestMainWindow, testslotDelFiles)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_deleteFiles(stub, true);
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    listEntry << entry;
    m_tester->slotDelFiles(listEntry, 10);
    ASSERT_EQ(m_tester->m_ePageID, PI_DeleteProgress);

    Stub stub1;
    ArchiveManagerstub::stub_ArchiveManager_deleteFiles(stub1, false);
    m_tester->slotDelFiles(listEntry, 10);
    ASSERT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(TestMainWindow, testslotReceiveCurArchiveName)
{
    m_tester->slotReceiveCurArchiveName("1.zip");
    ASSERT_EQ(m_tester->m_pProgressPage->m_strArchiveName, "1.zip");
}

TEST_F(TestMainWindow, testslotOpenFile)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_openFile(stub, true);
    FileEntry entry;
    entry.strFileName = "1.txt";
    m_tester->slotOpenFile(entry, "");
    ASSERT_EQ(m_tester->m_ePageID, PI_Loading);

    Stub stub1;
    ArchiveManagerstub::stub_ArchiveManager_openFile(stub1, false);
    m_tester->slotOpenFile(entry, "");
    ASSERT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(TestMainWindow, testslotOpenFileChanged)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    m_tester->m_pOpenFileWatcher->m_mapFileHasModified["1.txt"] = false;
    m_tester->m_stUnCompressParameter.bModifiable = false;
    m_tester->slotOpenFileChanged("1.txt");
}

TEST_F(TestMainWindow, testslotPause)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_pauseOperation(stub, true);
    m_tester->slotPause();
}

TEST_F(TestMainWindow, testslotContinue)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_continueOperation(stub, true);
    m_tester->slotContinue();
}

TEST_F(TestMainWindow, testslotCancel)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_cancelOperation(stub, true);
    m_tester->slotCancel();
}

TEST_F(TestMainWindow, testslotAddFiles)
{
    m_tester->slotAddFiles(QStringList(), "");
}

TEST_F(TestMainWindow, testslotSuccessView)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    CommonStub::stub_QThread_start(stub);

    m_tester->m_pSuccessPage->m_successInfoType = SI_Convert;
    m_tester->slotSuccessView();

    if (m_tester->m_pDDesktopServicesThread == nullptr) {
        m_tester->m_pDDesktopServicesThread = new DDesktopServicesThread(m_tester);
    }

    m_tester->m_pSuccessPage->m_successInfoType = SI_UnCompress;
    m_tester->m_pDDesktopServicesThread->m_listFiles << "1.txt";
    m_tester->slotSuccessView();
}

TEST_F(TestMainWindow, testslotSuccessReturn)
{
    m_tester->m_pSuccessPage->m_successInfoType = SI_Compress;
    m_tester->slotSuccessReturn();
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);

    m_tester->m_pSuccessPage->m_successInfoType = SI_UnCompress;
    m_tester->slotSuccessReturn();
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);

    m_tester->m_pSuccessPage->m_successInfoType = SI_Convert;
    m_tester->slotSuccessReturn();
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(TestMainWindow, testslotFailureRetry)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Compress;
    m_tester->slotFailureRetry();
    ASSERT_EQ(m_tester->m_ePageID, PI_CompressSetting);

    m_tester->m_pFailurePage->m_failureInfo = FI_Uncompress;
    m_tester->m_eStartupType = ST_Extract;
    m_tester->slotFailureRetry();
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->m_pFailurePage->m_failureInfo = FI_Convert;
    m_tester->slotFailureRetry();
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(TestMainWindow, testslotFailureReturn)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Compress;
    m_tester->slotFailureReturn();
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);

    m_tester->m_pFailurePage->m_failureInfo = FI_Uncompress;
    m_tester->slotFailureReturn();
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);

    m_tester->m_pFailurePage->m_failureInfo = FI_Convert;
    m_tester->slotFailureReturn();
    ASSERT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(TestMainWindow, testslotTitleCommentButtonPressed)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_updateArchiveComment(stub, false);
    m_tester->m_isFirstViewComment = true;
    m_tester->m_ePageID = PI_UnCompress;
    CommonStub::stub_DDialog_exec(stub, 0);
    m_tester->slotTitleCommentButtonPressed();
}

TEST_F(TestMainWindow, testslotThemeChanged)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::LightType);
    m_tester->slotThemeChanged();

    Stub stub1;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub1, DGuiApplicationHelper::DarkType);
    m_tester->slotThemeChanged();

    Stub stub2;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub2, DGuiApplicationHelper::UnknownType);
    m_tester->slotThemeChanged();
}

TEST_F(TestMainWindow, testslotShowShortcutTip)
{
    m_tester->slotShowShortcutTip();
}

TEST_F(TestMainWindow, testslotFinishCalculateSize)
{
    Stub stub;
    ArchiveManagerstub::stub_ArchiveManager_createArchive(stub, true);
    ArchiveManagerstub::stub_ArchiveManager_addFiles(stub, true);

    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
    m_tester->slotFinishCalculateSize(10, "1.txt", QList<FileEntry>(), CompressOptions(), QList<FileEntry>());

    m_tester->m_eStartupType = StartupType::ST_Extract;
    m_tester->slotFinishCalculateSize(10, "1.txt", QList<FileEntry>(), CompressOptions(), QList<FileEntry>());
}

TEST_F(TestMainWindow, testslotCheckFinished)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->m_eStartupType = ST_Compresstozip7z;
    m_tester->slotCheckFinished("ddd", "hhh");

    m_tester->m_eStartupType = ST_Compress;
    m_tester->slotCheckFinished("ddd", "hhh");
    ASSERT_EQ(m_tester->m_ePageID, PI_UnCompress);
}
