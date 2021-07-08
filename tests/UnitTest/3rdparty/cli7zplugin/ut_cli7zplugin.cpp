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

#include "cli7zplugin.h"
#include "commonstruct.h"
#include "datamanager.h"
#include "queries.h"
#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>
#include <QStandardPaths>

Q_DECLARE_METATYPE(KPluginMetaData)

PluginFinishType g_cliInterface_handlePassword_result = PFT_Nomral;
bool g_cliInterface_handleFileExists_result = false;
bool g_QProcess_waitForStarted_result = false;
bool g_PasswordNeededQuery_responseCancelled_result = false;
bool g_OverwriteQuery_responseCancelled_result = false;
bool g_OverwriteQuery_responseSkip_result = false;
bool g_OverwriteQuery_responseSkipAll_result = false;
bool g_OverwriteQuery_responseOverwrite_result = false;
bool g_OverwriteQuery_responseOverwriteAll_result = false;
bool g_LoadCorruptQuery_responseYes_result = false;
/*******************************函数打桩************************************/
PluginFinishType cliInterface_handlePassword_stub()
{
    return g_cliInterface_handlePassword_result;
}

bool cliInterface_handleFileExists_stub(const QString &)
{
    return g_cliInterface_handleFileExists_result;
}

bool cliInterface_runProcess_stub(const QString &, const QStringList &)
{
    return true;
}

int kill_stub(__pid_t, int)
{
    return 0;
}

void kProcess_start_stub()
{
    return ;
}

qint64 kProcess_processId_stub()
{
    return 123456;
}

bool qProcess_waitForStarted_stub(int)
{
    return g_QProcess_waitForStarted_result;
}

QString qStandardPaths_findExecutable_stub(const QString &executableName, const QStringList &paths)
{
    if (executableName == "7z") {
        return "/usr/bin/7z";
    }
    return"";
}

void query_waitForResponse_stub()
{
    return ;
}

bool passwordNeededQuery_responseCancelled_stub()
{
    return g_PasswordNeededQuery_responseCancelled_result;
}

bool overwriteQuery_responseCancelled_stub()
{
    return g_OverwriteQuery_responseCancelled_result;
}

bool overwriteQuery_responseSkip_stub()
{
    return g_OverwriteQuery_responseSkip_result;
}

bool overwriteQuery_responseSkipAll_stub()
{
    return g_OverwriteQuery_responseSkipAll_result;
}

bool overwriteQuery_responseOverwrite_stub()
{
    return g_OverwriteQuery_responseOverwrite_result;
}

bool overwriteQuery_responseOverwriteAll_stub()
{
    return g_OverwriteQuery_responseOverwriteAll_result;
}

bool loadCorruptQuery_responseYes_stub()
{
    return g_LoadCorruptQuery_responseYes_result;
}

void writeToProcess_stub()
{
    return;
}

bool moveExtractTempFilesToDest_stub()
{
    return false;
}
/*******************************函数打桩************************************/

class TestCli7zPluginFactory : public QObject, public ::testing::Test
{
public:
    TestCli7zPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Cli7zPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Cli7zPluginFactory *m_tester;
};

class TestCli7zPlugin : public QObject, public ::testing::Test
{
public:
    TestCli7zPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("test.7z").absoluteFilePath();
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new Cli7zPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Cli7zPlugin *m_tester;
};


TEST_F(TestCli7zPluginFactory, initTest)
{

}

TEST_F(TestCli7zPlugin, initTest)
{

}

TEST_F(TestCli7zPlugin, testisPasswordPrompt)
{
    ASSERT_EQ(m_tester->isPasswordPrompt("Enter password (will not be echoed):"), true);
}

TEST_F(TestCli7zPlugin, testisWrongPasswordMsg)
{
    ASSERT_EQ(m_tester->isWrongPasswordMsg("Wrong password"), true);
}

TEST_F(TestCli7zPlugin, testisCorruptArchiveMsg)
{
    ASSERT_EQ(m_tester->isCorruptArchiveMsg("Unexpected end of archive"), true);
}

TEST_F(TestCli7zPlugin, testisisDiskFullMsg)
{
    ASSERT_EQ(m_tester->isDiskFullMsg("No space left on device"), true);
}

TEST_F(TestCli7zPlugin, testisFileExistsMsg)
{
    ASSERT_EQ(m_tester->isFileExistsMsg("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);
}

TEST_F(TestCli7zPlugin, testisFileExistsFileName)
{
    ASSERT_EQ(m_tester->isFileExistsFileName("file ./"), true);
}

TEST_F(TestCli7zPlugin, testisMultiPasswordPrompt)
{
    ASSERT_EQ(m_tester->isMultiPasswordPrompt("ssssss"), false);
}

TEST_F(TestCli7zPlugin, testisOpenFileFailed)
{
    ASSERT_EQ(m_tester->isOpenFileFailed("ERROR: Can not open output file :"), true);
}

TEST_F(TestCli7zPlugin, testsetupCliProperties)
{
    m_tester->setupCliProperties();
}

TEST_F(TestCli7zPlugin, testkillProcess)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->m_bWaitingPassword = true;
    Stub stub;
    stub.set(kill, kill_stub);
    stub.set(ADDR(KProcess, processId), kProcess_processId_stub);
    m_tester->m_bWaitingPassword = true;
    m_tester->killProcess(true);
    m_tester->m_bWaitingPassword = false;
    m_tester->killProcess(true);
}

TEST_F(TestCli7zPlugin, testreadListLine)
{
    ASSERT_EQ(m_tester->readListLine("Open ERROR: Can not open the file as [7z] archive"), false);

    m_tester->m_parseState = ParseStateTitle;
    ASSERT_EQ(m_tester->readListLine("7-Zip [64] 16.02 : Copyright (c) 1999-2016 Igor Pavlov : 2016-05-21"), true);

    m_tester->m_parseState = ParseStateHeader;
    ASSERT_EQ(m_tester->readListLine("--"), true);

    m_tester->m_parseState = ParseStateArchiveInformation;
    ASSERT_EQ(m_tester->readListLine("----------"), true);
    m_tester->m_parseState = ParseStateArchiveInformation;
    ASSERT_EQ(m_tester->readListLine("Type = 7z"), true);
    ASSERT_EQ(m_tester->readListLine("Type = bzip2"), true);
    ASSERT_EQ(m_tester->readListLine("Type = gzip"), true);
    ASSERT_EQ(m_tester->readListLine("Type = xz"), true);
    ASSERT_EQ(m_tester->readListLine("Type = tar"), true);
    ASSERT_EQ(m_tester->readListLine("Type = zip"), true);
    ASSERT_EQ(m_tester->readListLine("Type = Rar"), true);
    ASSERT_EQ(m_tester->readListLine("Type = Split"), true);
    ASSERT_EQ(m_tester->readListLine("Type = Udf"), true);
    ASSERT_EQ(m_tester->readListLine("Type = Iso"), true);
    ASSERT_EQ(m_tester->readListLine("Type = crx"), false);

    m_tester->m_parseState = ParseStateEntryInformation;
    ASSERT_EQ(m_tester->readListLine("Path = /home"), true);
    ASSERT_EQ(m_tester->readListLine("Size = 8172"), true);
    m_tester->m_archiveType = Cli7zPlugin::ArchiveType::ArchiveTypeIso;
    ASSERT_EQ(m_tester->readListLine("Modified = 2021-06-18 15:27:01"), true);
    ASSERT_EQ(m_tester->readListLine("Attributes = D...."), true);
    ASSERT_EQ(m_tester->readListLine("Attributes = ....."), true);
    ASSERT_EQ(m_tester->readListLine("Block = "), true);
    ASSERT_EQ(m_tester->readListLine("Folder = -"), true);
}

TEST_F(TestCli7zPlugin, testhandleLine)
{
    Stub stub1;
    stub1.set(ADDR(CliInterface, handlePassword), cliInterface_handlePassword_stub);
    g_cliInterface_handlePassword_result = PFT_Cancel;
    ASSERT_EQ(m_tester->handleLine("Enter password (will not be echoed):", WT_List), false);
    g_cliInterface_handlePassword_result = PFT_Nomral;
    ASSERT_EQ(m_tester->handleLine("Enter password (will not be echoed):", WT_List), true);
    ASSERT_EQ(m_tester->handleLine("Wrong password", WT_List), false);
    ASSERT_EQ(m_tester->handleLine("No space left on device", WT_List), false);

    ASSERT_EQ(m_tester->handleLine("Unexpected end of archive", WT_List), true);
    m_tester->m_isEmptyArchive = false;
    ASSERT_EQ(m_tester->handleLine("Unexpected end of archive", WT_Extract), true);

    Stub stub2;
    g_cliInterface_handleFileExists_result = true;
    stub2.set(ADDR(CliInterface, handleFileExists), cliInterface_handleFileExists_stub);
    ASSERT_EQ(m_tester->handleLine("No files to process", WT_Extract), true);

    Stub stub3;
    g_cliInterface_handleFileExists_result = false;
    stub3.set(ADDR(CliInterface, handleFileExists), cliInterface_handleFileExists_stub);
    ASSERT_EQ(m_tester->handleLine("ERROR: Can not open output file : sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss", WT_Extract), false);

    ASSERT_EQ(m_tester->handleLine("System ERROR:28", WT_Add), false);
    ASSERT_EQ(m_tester->handleLine("ERROR: E_FAIL", WT_Extract), false);
    m_tester->m_eErrorType = ET_WrongPassword;
    ASSERT_EQ(m_tester->handleLine("E_FAIL", WT_Delete), false);
    ASSERT_EQ(m_tester->handleLine("MAX_PATHNAME_LEN", WT_Add), false);
}

TEST_F(TestCli7zPlugin, testisNoFilesArchive)
{
    ASSERT_EQ(m_tester->isNoFilesArchive("No files to process"), true);
}

TEST_F(TestCli7zPlugin, testlist)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);
    ASSERT_EQ(m_tester->list(), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testtestArchive)
{
    ASSERT_EQ(m_tester->testArchive(), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testextractFiles)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    ExtractionOptions options;
    options.strTargetPath = "/home/Desktop/";
    options.bAllExtract = true;
    m_tester->extractFiles(QList<FileEntry>(), options);


    FileEntry entry;
    entry.strFullPath = "1.txt";
    entry.strFileName = "1.txt";
    options.strTargetPath = "/home/Desktop/";
    options.bAllExtract = false;
    m_tester->extractFiles(QList<FileEntry>() << entry, options);
}

TEST_F(TestCli7zPlugin, testpauseOperation)
{
    Stub stub;
    stub.set(kill, kill_stub);
    m_tester->m_childProcessId << 123456 << 234567;
    m_tester->m_processId = 012345;
    m_tester->pauseOperation();
}

TEST_F(TestCli7zPlugin, testcontinueOperation)
{
    Stub stub;
    stub.set(kill, kill_stub);
    m_tester->m_childProcessId << 123456 << 234567;
    m_tester->m_processId = 012345;
    m_tester->continueOperation();
}

TEST_F(TestCli7zPlugin, testdoKill)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->doKill();
}

TEST_F(TestCli7zPlugin, testaddFiles)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    QList<FileEntry> files;
    CompressOptions options;
    options.bTar_7z = true;

    m_tester->m_filesSize = 10;

    options.strDestination = "/a/b";
    m_tester->addFiles(files, options);

    options.strDestination.clear();
    m_tester->addFiles(files, options);

    options.bTar_7z = false;
    m_tester->addFiles(files, options);

}

TEST_F(TestCli7zPlugin, testmoveFiles)
{
    ASSERT_EQ(m_tester->moveFiles(QList<FileEntry>(), CompressOptions()), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testcopyFiles)
{
    ASSERT_EQ(m_tester->copyFiles(QList<FileEntry>(), CompressOptions()), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testdeleteFiles)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    ASSERT_EQ(m_tester->deleteFiles(QList<FileEntry>()), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testaddComment)
{
    ASSERT_EQ(m_tester->addComment(""), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testupdateArchiveData)
{
    UpdateOptions options;

    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    FileEntry entry;
    entry.strFullPath = "1/";
    entry.strFileName = "1";
    entry.isDirectory = true;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    options.listEntry << entry;
    entry.strFullPath = "2/";
    entry.strFileName = "2";
    entry.isDirectory = true;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    entry.strFullPath = "1.txt";
    entry.strFileName = "1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    options.listEntry << entry;
    entry.strFullPath = "1/1.txt";
    entry.strFileName = "1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;

    options.eType = UpdateOptions::Delete;
    ASSERT_EQ(m_tester->updateArchiveData(options), PFT_Nomral);

    entry.strFullPath = "1/2.txt";
    entry.strFileName = "2.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    options.eType = UpdateOptions::Add;
    ASSERT_EQ(m_tester->updateArchiveData(options), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testsetListEmptyLines)
{
    m_tester->setListEmptyLines(true);
    ASSERT_EQ(m_tester->m_listEmptyLines, true);
}

TEST_F(TestCli7zPlugin, testrunProcess)
{
    Stub stub;
    stub.set(ADDR(KProcess, start), kProcess_start_stub);

    Stub stub1;
    stub1.set(ADDR(QStandardPaths, findExecutable), qStandardPaths_findExecutable_stub);
    ASSERT_EQ(m_tester->runProcess("asdasd", QStringList()), false);
    m_tester->deleteProcess();

    Stub stub2;
    stub2.set(ADDR(QStandardPaths, findExecutable), qStandardPaths_findExecutable_stub);
    stub2.set(ADDR(QProcess, waitForStarted), qProcess_waitForStarted_stub);
    g_QProcess_waitForStarted_result = true;
    m_tester->m_workStatus = WT_Extract;
    m_tester->m_isTar7z = true;
    ASSERT_EQ(m_tester->runProcess("7z", QStringList()), true);
    m_tester->deleteProcess();

    Stub stub3;
    stub3.set(ADDR(QProcess, waitForStarted), qProcess_waitForStarted_stub);
    g_QProcess_waitForStarted_result = false;
    m_tester->m_workStatus = WT_Add;
    m_tester->m_isTar7z = false;
    ASSERT_EQ(m_tester->runProcess("7z", QStringList()), true);
}

TEST_F(TestCli7zPlugin, testdeleteProcess)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->deleteProcess();
}

TEST_F(TestCli7zPlugin, testhandleProgress)
{
    m_tester->m_filesSize = 10;
    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("7z");
    m_tester->m_workStatus = WT_Extract;
    m_tester->handleProgress("22% \b\b\b\b 73593 - 1/2/3.js");
    m_tester->m_workStatus = WT_Delete;
    m_tester->handleProgress("% = 1/2/3.js");
    m_tester->m_workStatus = WT_Extract;
    m_tester->handleProgress("0% \b\b\b\b 73593 - 1/2/3.js");

    m_tester->m_process->setProgram("unrar");
    m_tester->handleProgress("Extracting  安装包/新建文件夹 - 副本 (2)/TabTip.exe.mui                         OK");

    m_tester->m_process->setProgram("bash");
    m_tester->handleProgress("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b  7M + [Content]");
}

TEST_F(TestCli7zPlugin, testhandlePassword)
{
    m_tester->m_process = new KPtyProcess;

    Stub stub;
    stub.set(ADDR(PasswordNeededQuery, waitForResponse), query_waitForResponse_stub);

    g_PasswordNeededQuery_responseCancelled_result = true;
    Stub stub1;
    stub1.set(ADDR(PasswordNeededQuery, responseCancelled), passwordNeededQuery_responseCancelled_stub);
    m_tester->m_process->setProgram("7z");
    ASSERT_EQ(m_tester->handlePassword(), PFT_Cancel);


    g_PasswordNeededQuery_responseCancelled_result = false;
    Stub stub2;
    stub2.set(ADDR(PasswordNeededQuery, responseCancelled), passwordNeededQuery_responseCancelled_stub);
    m_tester->m_process->setProgram("7z");
    ASSERT_EQ(m_tester->handlePassword(), PFT_Nomral);


    Stub stub3;
    stub3.set(ADDR(PasswordNeededQuery, responseCancelled), passwordNeededQuery_responseCancelled_stub);

    m_tester->m_process->setProgram("unrar");
    ASSERT_EQ(m_tester->handlePassword(), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testhandleFileExists)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(CliInterface, writeToProcess), writeToProcess_stub);

    ASSERT_EQ(m_tester->handleFileExists("file ./"), false);

    Stub stub1;
    stub1.set(ADDR(OverwriteQuery, responseCancelled), overwriteQuery_responseCancelled_stub);
    stub1.set(ADDR(OverwriteQuery, responseSkip), overwriteQuery_responseSkip_stub);
    stub1.set(ADDR(OverwriteQuery, responseSkipAll), overwriteQuery_responseSkipAll_stub);
    stub1.set(ADDR(OverwriteQuery, responseOverwrite), overwriteQuery_responseOverwrite_stub);
    stub1.set(ADDR(OverwriteQuery, responseOverwriteAll), overwriteQuery_responseOverwriteAll_stub);

    g_OverwriteQuery_responseCancelled_result = true;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    ASSERT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = true;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    ASSERT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = true;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    ASSERT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = true;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    ASSERT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = true;
    ASSERT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);

    ASSERT_EQ(m_tester->handleFileExists("sssssssss"), false);
}

TEST_F(TestCli7zPlugin, testhandleCorrupt)
{
    Stub stub;
    stub.set(ADDR(LoadCorruptQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(LoadCorruptQuery, responseYes), loadCorruptQuery_responseYes_stub);

    g_LoadCorruptQuery_responseYes_result = false;
    ASSERT_EQ(m_tester->handleCorrupt(), PFT_Error);

    g_LoadCorruptQuery_responseYes_result = true;
    ASSERT_EQ(m_tester->handleCorrupt(), PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testwriteToProcess)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->writeToProcess("");
}

TEST_F(TestCli7zPlugin, testmoveExtractTempFilesToDest)
{

}

TEST_F(TestCli7zPlugin, testreadStdout)
{

}

TEST_F(TestCli7zPlugin, testprocessFinished)
{
    Stub stub;
    stub.set(ADDR(LoadCorruptQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(LoadCorruptQuery, responseYes), loadCorruptQuery_responseYes_stub);

    g_LoadCorruptQuery_responseYes_result = false;

    m_tester->m_process = new KPtyProcess;
    m_tester->m_isCorruptArchive = true;
    m_tester->processFinished(0, QProcess::NormalExit);
    ASSERT_EQ(m_tester->m_finishType, PFT_Nomral);
}

TEST_F(TestCli7zPlugin, testextractProcessFinished)
{
    Stub stub;
    stub.set(ADDR(CliInterface, moveExtractTempFilesToDest), moveExtractTempFilesToDest_stub);

    m_tester->m_process = new KPtyProcess;
    m_tester->m_extractOptions.bAllExtract = false;
    m_tester->m_extractOptions.strTargetPath = "/home";
    m_tester->extractProcessFinished(0, QProcess::NormalExit);
}

TEST_F(TestCli7zPlugin, testgetChildProcessId)
{

}

TEST_F(TestCli7zPlugin, testgetTargetPath)
{
    m_tester->m_extractOptions.strTargetPath = "a/b";
    ASSERT_EQ(m_tester->getTargetPath(), "a/b");
}

TEST_F(TestCli7zPlugin, testwaitForFinished)
{
    m_tester->m_bWaitForFinished = true;
    ASSERT_EQ(m_tester->waitForFinished(), true);
}

TEST_F(TestCli7zPlugin, testsetPassword)
{
    m_tester->setPassword("123456");
    ASSERT_EQ(m_tester->m_strPassword, "123456");
}

TEST_F(TestCli7zPlugin, testgetPassword)
{
    m_tester->m_strPassword = "123456";
    ASSERT_EQ(m_tester->getPassword(), "123456");
}

TEST_F(TestCli7zPlugin, testerrorType)
{
    m_tester->m_eErrorType = ErrorType::ET_NoError;
    ASSERT_EQ(m_tester->errorType(), ErrorType::ET_NoError);
}

TEST_F(TestCli7zPlugin, testsetWaitForFinishedSignal)
{
    m_tester->setWaitForFinishedSignal(true);
    ASSERT_EQ(m_tester->m_bWaitForFinished, true);
}

TEST_F(TestCli7zPlugin, testgetPermissions)
{
    mode_t perm = 0;
    m_tester->getPermissions(perm);
    perm = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
    m_tester->getPermissions(perm);
}
