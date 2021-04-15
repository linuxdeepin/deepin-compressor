/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenglu <chenglu@uniontech.com>
*
* Maintainer: chenglu <chenglu@uniontech.com>
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

#include "archivemanager.h"
#include "singlejob.h"
#include "gtest/src/stub.h"
#include "ut_commonstub.h"
#include "config.h"

#include <gtest/gtest.h>
#include <QTest>

class TestArchiveManager : public ::testing::Test
{
public:
    TestArchiveManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ArchiveManager;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ArchiveManager *m_tester;
};

TEST_F(TestArchiveManager, initTest)
{

}

TEST_F(TestArchiveManager, testget_instance)
{
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestArchiveManager, testcreateArchive)
{
    QList<FileEntry> files;
    FileEntry entry;
    entry.strFullPath = TEST_SOURCES_PATH + QString("/zip/compress/test.txt");
    files << entry;
    QString strDestination = TEST_SOURCES_PATH + QString("/zip/compress/test.zip");
    CompressOptions options;
    UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto;

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->createArchive(files, strDestination, options, eType);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testloadArchive)
{
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");
    UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto;

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->loadArchive(strArchiveFullPath, eType);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testaddFiles)
{
    QList<FileEntry> listAddEntry;
    FileEntry entry;
    entry.strFullPath = TEST_SOURCES_PATH + QString("/zip/compress/test.txt");
    listAddEntry << entry;
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/compress/test.zip");
    CompressOptions options;

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->addFiles(strArchiveFullPath, listAddEntry, options);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testextractFiles)
{
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");
    ExtractionOptions stOptions;
    stOptions.bAllExtract = true;
    UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto;

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->extractFiles(strArchiveFullPath, QList<FileEntry>(), stOptions, eType);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testextractFiles2Path)
{
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");
    ExtractionOptions stOptions;
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFullPath = "test.txt";
    listEntry << entry;

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->extractFiles2Path(strArchiveFullPath, listEntry, stOptions);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testdeleteFiles)
{
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFullPath = "test.txt";
    listEntry << entry;

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->deleteFiles(strArchiveFullPath, listEntry);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testbatchExtractFiles)
{
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->batchExtractFiles(QStringList() << strArchiveFullPath, TEST_SOURCES_PATH + QString("/zip/extract"));
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testopenFile)
{
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");

    FileEntry entry;
    entry.strFullPath = "test.txt";

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->openFile(strArchiveFullPath, entry, TEST_SOURCES_PATH + QString("/zip/extract"), "deepin-editor");
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testupdateArchiveCacheData)
{
    m_tester->m_pInterface = UiTools::createInterface(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"), false);

    UpdateOptions options;

    Stub stub;
    CommonStub::stub_UpdateJob_start(stub);

    bool bResult = m_tester->updateArchiveCacheData(options);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testupdateArchiveComment)
{
    QString strArchiveFullPath = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");

    Stub stub;
    CommonStub::stub_SingleJob_start(stub);

    bool bResult = m_tester->updateArchiveComment(strArchiveFullPath, "ddddd");
    ASSERT_EQ(bResult, true);
}

//TEST_F(TestArchiveManager, testconvertArchive)
//{
//    QString strArchiveFullPath1 = TEST_SOURCES_PATH + QString("/zip/extract/test.zip");
//    QString strArchiveFullPath2 = TEST_SOURCES_PATH + QString("/zip/extract/test2.zip");

//    Stub stub;
//    CommonStub::stub_ConvertJob_start(stub);

//    bool bResult = m_tester->convertArchive(strArchiveFullPath1, TEST_SOURCES_PATH + QString("/zip/extract"), strArchiveFullPath2);
//    ASSERT_EQ(bResult, true);
//}

//TEST_F(TestArchiveManager, testpauseOperation)
//{
//    m_tester->m_pInterface = UiTools::createInterface(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"), false);
//    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);

//    Stub stub;
//    CommonStub::stub_SingleJob_doPause(stub);

//    bool bResult = m_tester->pauseOperation();
//    ASSERT_EQ(bResult, true);
//}

//TEST_F(TestArchiveManager, testcontinueOperation)
//{
//    m_tester->m_pInterface = UiTools::createInterface(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"), false);
//    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);

//    Stub stub;
//    CommonStub::stub_SingleJob_doContinue(stub);

//    bool bResult = m_tester->continueOperation();
//    ASSERT_EQ(bResult, true);
//}

//TEST_F(TestArchiveManager, testcancelOperation)
//{
//    m_tester->m_pInterface = UiTools::createInterface(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"), false);
//    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);

//    Stub stub;
//    CommonStub::stub_SingleJob_kill(stub);

//    bool bResult = m_tester->cancelOperation();
//    ASSERT_EQ(bResult, true);
//}

//TEST_F(TestArchiveManager, testgetCurFilePassword)
//{
//    m_tester->m_pInterface = UiTools::createInterface(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"), false);
//    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);
//    QString strPassword = "123";
//    m_tester->m_pInterface->setPassword(strPassword);

//    ASSERT_EQ(m_tester->getCurFilePassword(), strPassword);
//}

//TEST_F(TestArchiveManager, testslotJobFinished)
//{
//    m_tester->m_pInterface = UiTools::createInterface(TEST_SOURCES_PATH + QString("/zip/extract/test.zip"), false);
//    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);
//    m_tester->slotJobFinished();

//    ASSERT_EQ(m_tester->m_pArchiveJob, nullptr);
//}
