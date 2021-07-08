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

#include "batchjob.h"
#include "singlejob.h"
#include "uitools.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

/*******************************函数打桩************************************/
bool batchExtractJob_addExtractItem_stub(const QFileInfo &)
{
    return true;
}
/*******************************函数打桩************************************/

// 测试BatchExtractJob
class TestBatchExtractJob : public ::testing::Test
{
public:
    TestBatchExtractJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BatchExtractJob;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    BatchExtractJob *m_tester;
};

TEST_F(TestBatchExtractJob, initTest)
{

}

TEST_F(TestBatchExtractJob, testaddSubjob)
{
    ASSERT_EQ(m_tester->addSubjob(nullptr), false);
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    ASSERT_EQ(m_tester->addSubjob(pJob), true);
}

TEST_F(TestBatchExtractJob, testremoveSubjob)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    ASSERT_EQ(m_tester->removeSubjob(pJob), true);
    ASSERT_EQ(m_tester->removeSubjob(nullptr), false);
}

TEST_F(TestBatchExtractJob, testhasSubjobs)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    ASSERT_EQ(m_tester->hasSubjobs(), true);
}

TEST_F(TestBatchExtractJob, testsubjobs)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    ASSERT_EQ(m_tester->subjobs(), m_tester->m_listSubjobs);
}

TEST_F(TestBatchExtractJob, testclearSubjobs)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    m_tester->clearSubjobs();
}

TEST_F(TestBatchExtractJob, testdoPause)
{
    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);
    m_tester->doPause();
}

TEST_F(TestBatchExtractJob, testdoContinue)
{
    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);
    m_tester->doContinue();
}

TEST_F(TestBatchExtractJob, teststart)
{
    Stub stub;
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);
    m_tester->start();
}

TEST_F(TestBatchExtractJob, testsetExtractPath)
{
    m_tester->setExtractPath("/home/Desktop");
    ASSERT_EQ(m_tester->m_strExtractPath, "/home/Desktop");
}

TEST_F(TestBatchExtractJob, testsetArchiveFiles)
{
    Stub stub;
    stub.set(ADDR(BatchExtractJob, setArchiveFiles), batchExtractJob_addExtractItem_stub);
    ASSERT_EQ(m_tester->setArchiveFiles(QStringList() << "1.zip" << "2.zip"), true);
}

TEST_F(TestBatchExtractJob, testaddExtractItem)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);

    ASSERT_EQ(m_tester->addExtractItem(QFileInfo("1.zip")), true);
    ASSERT_EQ(m_tester->addExtractItem(QFileInfo("1.tar.7z")), true);
}

TEST_F(TestBatchExtractJob, testslotHandleSingleJobProgress)
{
    m_tester->m_listFiles << "1.txt";
    m_tester->m_qBatchTotalSize = 1024;
    m_tester->m_iCurArchiveIndex = 0;
    m_tester->slotHandleSingleJobProgress(50);
}

TEST_F(TestBatchExtractJob, testslotHandleSingleJobCurFileName)
{
    m_tester->slotHandleSingleJobCurFileName("1.txt");
}

TEST_F(TestBatchExtractJob, testslotHandleSingleJobFinished)
{
    Stub stub;
    JobStub::stub_SingleJob_start(stub);

    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);

    m_tester->m_pCurJob->m_eFinishedType = PFT_Error;
    m_tester->slotHandleSingleJobFinished();

    m_tester->m_pCurJob->m_eFinishedType = PFT_Nomral;
    m_tester->slotHandleSingleJobFinished();

    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);
    m_tester->m_pCurJob->m_eFinishedType = PFT_Nomral;
    m_tester->m_iCurArchiveIndex = 0;
    m_tester->m_listFiles << "1.txt" << "2.txt";
    m_tester->slotHandleSingleJobFinished();
}

TEST_F(TestBatchExtractJob, testdoKill)
{
    m_tester->doKill();
}
