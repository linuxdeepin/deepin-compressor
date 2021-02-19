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

void start_stub()
{
    return;
}

TEST_F(TestArchiveManager, testcreateArchive)
{
    QList<FileEntry> files;
    FileEntry entry;
    entry.strFullPath = QFileInfo("../UnitTest/test_sources/zip/compress/test.txt").absoluteFilePath();
    files << entry;
    QString strDestination = QFileInfo("../UnitTest/test_sources/zip/compress/test.zip").absoluteFilePath();
    CompressOptions options;
    UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto;

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->createArchive(files, strDestination, options, eType);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testloadArchive)
{
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto;

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->loadArchive(strArchiveFullPath, eType);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testaddFiles)
{
    QList<FileEntry> listAddEntry;
    FileEntry entry;
    entry.strFullPath = QFileInfo("../UnitTest/test_sources/zip/compress/test.txt").absoluteFilePath();
    listAddEntry << entry;
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/compress/test.zip").absoluteFilePath();
    CompressOptions options;

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->addFiles(strArchiveFullPath, listAddEntry, options);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testextractFiles)
{
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    ExtractionOptions stOptions;
    stOptions.bAllExtract = true;
    UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto;

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->extractFiles(strArchiveFullPath, QList<FileEntry>(), stOptions, eType);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testextractFiles2Path)
{
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    ExtractionOptions stOptions;
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFullPath = "test.txt";
    listEntry << entry;

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->extractFiles2Path(strArchiveFullPath, listEntry, stOptions);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testdeleteFiles)
{
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFullPath = "test.txt";
    listEntry << entry;

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->deleteFiles(strArchiveFullPath, listEntry);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testbatchExtractFiles)
{
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->batchExtractFiles(QStringList() << strArchiveFullPath, QFileInfo("../UnitTest/test_sources/zip/extract").absoluteFilePath());
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testopenFile)
{
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();

    FileEntry entry;
    entry.strFullPath = "test.txt";

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->openFile(strArchiveFullPath, entry, QFileInfo("../UnitTest/test_sources/zip/extract").absoluteFilePath(), "deepin-editor");
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testupdateArchiveCacheData)
{
    m_tester->m_pInterface = UiTools::createInterface(QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath(), false);

    UpdateOptions options;

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->updateArchiveCacheData(options);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testupdateArchiveComment)
{
    QString strArchiveFullPath = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->updateArchiveComment(strArchiveFullPath, "ddddd");
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testconvertArchive)
{
    QString strArchiveFullPath1 = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
    QString strArchiveFullPath2 = QFileInfo("../UnitTest/test_sources/zip/extract/test2.zip").absoluteFilePath();

    typedef void (*fptr)();
    fptr A_foo = (fptr)(&ConvertJob::start);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, start_stub);

    bool bResult = m_tester->convertArchive(strArchiveFullPath1, QFileInfo("../UnitTest/test_sources/zip/extract").absoluteFilePath(), strArchiveFullPath2);
    ASSERT_EQ(bResult, true);
}

void doPause_stub()
{
    return;
}

TEST_F(TestArchiveManager, testpauseOperation)
{
    m_tester->m_pInterface = UiTools::createInterface(QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath(), false);
    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);

    typedef void (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::doPause);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, doPause_stub);

    bool bResult = m_tester->pauseOperation();
    ASSERT_EQ(bResult, true);
}

void doContinue_stub()
{
    return;
}

TEST_F(TestArchiveManager, testcontinueOperation)
{
    m_tester->m_pInterface = UiTools::createInterface(QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath(), false);
    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);

    typedef void (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::doContinue);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, doContinue_stub);

    bool bResult = m_tester->continueOperation();
    ASSERT_EQ(bResult, true);
}

void kill_stub()
{
    return;
}

TEST_F(TestArchiveManager, testcancelOperation)
{
    m_tester->m_pInterface = UiTools::createInterface(QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath(), false);
    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);

    typedef void (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::kill);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, kill_stub);

    bool bResult = m_tester->cancelOperation();
    ASSERT_EQ(bResult, true);
}

TEST_F(TestArchiveManager, testgetCurFilePassword)
{
    m_tester->m_pInterface = UiTools::createInterface(QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath(), false);
    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);
    QString strPassword = "123";
    m_tester->m_pInterface->setPassword(strPassword);

    ASSERT_EQ(m_tester->getCurFilePassword(), strPassword);
}

TEST_F(TestArchiveManager, testslotJobFinished)
{
    m_tester->m_pInterface = UiTools::createInterface(QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath(), false);
    m_tester->m_pArchiveJob = new LoadJob(m_tester->m_pInterface);
    m_tester->slotJobFinished();

    ASSERT_EQ(m_tester->m_pArchiveJob, nullptr);
}
