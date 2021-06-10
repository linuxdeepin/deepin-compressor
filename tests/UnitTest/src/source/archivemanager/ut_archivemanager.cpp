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

#include "archivemanager.h"
#include "libzipplugin.h"
#include "batchjob.h"
#include "singlejob.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

bool g_batchExtractJob_setArchiveFiles_result = false;
/*******************************函数打桩************************************/
bool batchExtractJob_setArchiveFiles_stub(const QStringList &)
{
    return g_batchExtractJob_setArchiveFiles_result;
}
/*******************************函数打桩************************************/

// 测试TypeLabel
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
    ASSERT_EQ(m_tester->get_instance(), m_tester->m_instance);
}

TEST_F(TestArchiveManager, testdestory_instance)
{
    m_tester->destory_instance();
    ASSERT_EQ(nullptr, m_tester->m_instance);
}

TEST_F(TestArchiveManager, testcreateArchive)
{
    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    ASSERT_EQ(m_tester->createArchive(QList<FileEntry>(), "/home/Desktop", CompressOptions(), UiTools::APT_Auto), true);
    delete m_tester->m_pTempInterface;
    m_tester->m_pTempInterface = nullptr;

    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    ASSERT_EQ(m_tester->createArchive(QList<FileEntry>(), "/home/Desktop", CompressOptions(), UiTools::APT_Auto), false);
}

TEST_F(TestArchiveManager, testloadArchive)
{
    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    ASSERT_EQ(m_tester->loadArchive("1.zip", UiTools::APT_Auto), true);

    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    ASSERT_EQ(m_tester->loadArchive("1.zip", UiTools::APT_Auto), false);
}

TEST_F(TestArchiveManager, testaddFiles)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    ASSERT_EQ(m_tester->addFiles("1.zip", QList<FileEntry>(), CompressOptions()), true);
    delete m_tester->m_pTempInterface;
    m_tester->m_pTempInterface = nullptr;

    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    ASSERT_EQ(m_tester->addFiles("1.zip", QList<FileEntry>(), CompressOptions()), false);
}

TEST_F(TestArchiveManager, testextractFiles)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    JobStub::stub_SingleJob_start(stub);
    JobStub::stub_StepExtractJob_start(stub);

    SAFE_DELETE_ELE(m_tester->m_pInterface);
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    ExtractionOptions option;
    option.bTar_7z = false;
    ASSERT_EQ(m_tester->extractFiles("1.zip", QList<FileEntry>(), option, UiTools::APT_Auto), true);

    SAFE_DELETE_ELE(m_tester->m_pInterface);
    SAFE_DELETE_ELE(m_tester->m_pArchiveJob);
    pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, pInterface);
    option.bTar_7z = true;
    ASSERT_EQ(m_tester->extractFiles("1.zip", QList<FileEntry>(), option, UiTools::APT_Auto), true);

    Stub stub2;
    CommonStub::stub_UiTools_createInterface(stub2, nullptr);
    SAFE_DELETE_ELE(m_tester->m_pInterface);
    SAFE_DELETE_ELE(m_tester->m_pArchiveJob);
    ASSERT_EQ(m_tester->extractFiles("1.zip", QList<FileEntry>(), option, UiTools::APT_Auto), false);
}

TEST_F(TestArchiveManager, testextractFiles2Path)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    ASSERT_EQ(m_tester->extractFiles2Path("1.zip", QList<FileEntry>(), ExtractionOptions()), true);

    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    SAFE_DELETE_ELE(m_tester->m_pInterface);
    ASSERT_EQ(m_tester->extractFiles2Path("1.zip", QList<FileEntry>(), ExtractionOptions()), false);
}

TEST_F(TestArchiveManager, testdeleteFiles)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    ASSERT_EQ(m_tester->deleteFiles("1.zip", QList<FileEntry>()), true);

    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    SAFE_DELETE_ELE(m_tester->m_pInterface);
    ASSERT_EQ(m_tester->deleteFiles("1.zip", QList<FileEntry>()), false);
}

TEST_F(TestArchiveManager, testbatchExtractFiles)
{
    g_batchExtractJob_setArchiveFiles_result = true;
    Stub stub;
    stub.set(ADDR(BatchExtractJob, setArchiveFiles), batchExtractJob_setArchiveFiles_stub);
    ASSERT_EQ(m_tester->batchExtractFiles(QStringList(), ""), true);


    g_batchExtractJob_setArchiveFiles_result = false;
    Stub stub1;
    stub1.set(ADDR(BatchExtractJob, setArchiveFiles), batchExtractJob_setArchiveFiles_stub);
    ASSERT_EQ(m_tester->batchExtractFiles(QStringList(), ""), false);
}

TEST_F(TestArchiveManager, testopenFile)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    ASSERT_EQ(m_tester->openFile("1.zip", FileEntry(), "", ""), true);

    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    SAFE_DELETE_ELE(m_tester->m_pInterface);
    ASSERT_EQ(m_tester->openFile("1.zip", FileEntry(), "", ""), false);
}

TEST_F(TestArchiveManager, testupdateArchiveCacheData)
{
    Stub stub;
    JobStub::stub_UpdateJob_start(stub);

    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    ASSERT_EQ(m_tester->updateArchiveCacheData(UpdateOptions()), true);
    SAFE_DELETE_ELE(m_tester->m_pInterface);

    ASSERT_EQ(m_tester->updateArchiveCacheData(UpdateOptions()), false);
}

TEST_F(TestArchiveManager, testupdateArchiveComment)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    ASSERT_EQ(m_tester->updateArchiveComment("1.zip", ""), true);

    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    ASSERT_EQ(m_tester->updateArchiveComment("1.zip", ""), false);
}

TEST_F(TestArchiveManager, testconvertArchive)
{
    Stub stub;
    JobStub::stub_ConvertJob_start(stub);

    ASSERT_EQ(m_tester->convertArchive("", "", ""), true);
}

TEST_F(TestArchiveManager, testpauseOperation)
{
    Stub stub;
    JobStub::stub_SingleJob_doPause(stub, true);

    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    ASSERT_EQ(m_tester->pauseOperation(), true);
    SAFE_DELETE_ELE(m_tester->m_pArchiveJob);
    ASSERT_EQ(m_tester->pauseOperation(), false);
}

TEST_F(TestArchiveManager, testcontinueOperation)
{
    Stub stub;
    JobStub::stub_SingleJob_doContinue(stub, true);

    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    ASSERT_EQ(m_tester->continueOperation(), true);
    SAFE_DELETE_ELE(m_tester->m_pArchiveJob);
    ASSERT_EQ(m_tester->continueOperation(), false);
}

TEST_F(TestArchiveManager, testcancelOperation)
{
    Stub stub;
    JobStub::stub_ArchiveJob_kill(stub);

    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    ASSERT_EQ(m_tester->cancelOperation(), true);
    SAFE_DELETE_ELE(m_tester->m_pArchiveJob);
    ASSERT_EQ(m_tester->cancelOperation(), false);
}

TEST_F(TestArchiveManager, testgetCurFilePassword)
{
    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    m_tester->m_pInterface->m_strPassword = "123";
    ASSERT_EQ(m_tester->getCurFilePassword(), "123");
    SAFE_DELETE_ELE(m_tester->m_pInterface);
    ASSERT_EQ(m_tester->getCurFilePassword(), "");
}

TEST_F(TestArchiveManager, testslotJobFinished)
{
    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    m_tester->slotJobFinished();
    ASSERT_EQ(m_tester->m_pArchiveJob, nullptr);
}
