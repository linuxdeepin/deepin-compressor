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

#include "singlejob.h"
#include "libzipplugin.h"
#include "commonstruct.h"
#include "uitools.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

/*******************************函数打桩************************************/

/*******************************函数打桩************************************/
// 测试LoadJob
class TestLoadJob : public ::testing::Test
{
public:
    TestLoadJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new LoadJob(m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    LoadJob *m_tester;
};

TEST_F(TestLoadJob, initTest)
{

}

TEST_F(TestLoadJob, testdoWork)
{
    m_tester->doWork();
}

TEST_F(TestLoadJob, testdoPause)
{
    m_tester->doPause();
}

TEST_F(TestLoadJob, testdoContinue)
{
    m_tester->doContinue();
}

TEST_F(TestLoadJob, testgetdptr)
{
    m_tester->getdptr();
}

TEST_F(TestLoadJob, testdoKill)
{
    m_tester->doKill();
}

TEST_F(TestLoadJob, testfinishJob)
{
    m_tester->finishJob();
}


// 测试AddJob
class TestAddJob : public ::testing::Test
{
public:
    TestAddJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new AddJob(QList<FileEntry>(), m_pInterface, CompressOptions(), nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    AddJob *m_tester;
};

TEST_F(TestAddJob, initTest)
{

}

TEST_F(TestAddJob, testdoWork)
{
    m_tester->doWork();
}


// 测试AddJob
class TestCreateJob : public ::testing::Test
{
public:
    TestCreateJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new CreateJob(QList<FileEntry>(), m_pInterface, CompressOptions(), nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    CreateJob *m_tester;
};

TEST_F(TestCreateJob, initTest)
{

}

TEST_F(TestCreateJob, testdoWork)
{
    m_tester->doWork();
}

TEST_F(TestCreateJob, testdoKill)
{
    m_tester->doKill();
}

TEST_F(TestCreateJob, testcleanCompressFileCancel)
{
    m_tester->m_stCompressOptions.bSplit = true;
    m_tester->cleanCompressFileCancel();
    m_tester->m_stCompressOptions.bSplit = false;
    m_tester->cleanCompressFileCancel();
}


// 测试ExtractJob
class TestExtractJob : public ::testing::Test
{
public:
    TestExtractJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new ExtractJob(QList<FileEntry>(), m_pInterface, ExtractionOptions(), nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    ExtractJob *m_tester;
};

TEST_F(TestExtractJob, initTest)
{

}

TEST_F(TestExtractJob, testdoWork)
{
    m_tester->doWork();
}


// 测试DeleteJob
class TestDeleteJob : public ::testing::Test
{
public:
    TestDeleteJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new DeleteJob(QList<FileEntry>(), m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    DeleteJob *m_tester;
};

TEST_F(TestDeleteJob, initTest)
{

}

TEST_F(TestDeleteJob, testdoWork)
{
    m_tester->doWork();
}


// 测试OpenJob
class TestOpenJob : public ::testing::Test
{
public:
    TestOpenJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new OpenJob(FileEntry(), "", "", m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    OpenJob *m_tester;
};

TEST_F(TestOpenJob, initTest)
{

}

TEST_F(TestOpenJob, testdoWork)
{
    m_tester->doWork();
}

TEST_F(TestOpenJob, testslotFinished)
{
    Stub stub;
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->slotFinished(PFT_Nomral);
}


// 测试UpdateJob
class TestUpdateJob : public ::testing::Test
{
public:
    TestUpdateJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new UpdateJob(UpdateOptions(), m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    UpdateJob *m_tester;
};

TEST_F(TestUpdateJob, initTest)
{

}

TEST_F(TestUpdateJob, teststart)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    m_tester->start();
}

TEST_F(TestUpdateJob, testdoWork)
{
    m_tester->doWork();
}


// 测试CommentJob
class TestCommentJob : public ::testing::Test
{
public:
    TestCommentJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        LibzipPlugin *pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new CommentJob("", pInterface, nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CommentJob *m_tester;
};

TEST_F(TestCommentJob, initTest)
{

}

TEST_F(TestCommentJob, testdoWork)
{
    m_tester->doWork();
}


// 测试ConvertJob
class TestConvertJob : public ::testing::Test
{
public:
    TestConvertJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ConvertJob("", "./", "", nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ConvertJob *m_tester;
};

TEST_F(TestConvertJob, initTest)
{

}

TEST_F(TestConvertJob, teststart)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
}

TEST_F(TestConvertJob, testslotHandleExtractFinished)
{
    //ReadOnlyArchiveInterface *pIface = new LibzipPlugin(nullptr, QVariantList());
    // Stub stub;
    //CommonStub::stub_UiTools_createInterface(stub, pIface);
    //JobStub::stub_CreateJob_doWork(stub);
    //JobStub::stub_ExtractJob_doWork(stub);
    // JobStub::stub_SingleJob_start(stub);
    // m_tester->start();
    // m_tester->m_pExtractJob->m_eFinishedType = PluginFinishType::PFT_Nomral;
    // ReadOnlyArchiveInterface *pIface1 = new LibzipPlugin(nullptr, QVariantList());
    //  Stub stub1;
//   CommonStub::stub_UiTools_createInterface(stub1, pIface1);
//    m_tester->slotHandleExtractFinished();
//    m_tester->m_pExtractJob->m_eFinishedType = PFT_Cancel;
//    m_tester->slotHandleExtractFinished();
//    m_tester->m_pExtractJob->m_eFinishedType = PFT_Error;
//    m_tester->slotHandleExtractFinished();
}

TEST_F(TestConvertJob, testdoPause)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
    m_tester->doPause();
}

TEST_F(TestConvertJob, testdoContinue)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
    m_tester->doContinue();
}

TEST_F(TestConvertJob, testdoKill)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
    m_tester->doKill();
}


// 测试StepExtractJob
class TestStepExtractJob : public ::testing::Test
{
public:
    TestStepExtractJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new StepExtractJob("", ExtractionOptions(), nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    StepExtractJob *m_tester;
};

TEST_F(TestStepExtractJob, initTest)
{

}

TEST_F(TestStepExtractJob, teststart)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
}

TEST_F(TestStepExtractJob, testslotHandleExtractFinished)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    m_tester->m_pExtractJob->m_eFinishedType = PluginFinishType::PFT_Nomral;
    ReadOnlyArchiveInterface *pIface1 = new LibzipPlugin(m_tester, QVariantList());
    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, pIface1);
    m_tester->slotHandleExtractFinished();
    m_tester->m_pExtractJob->m_eFinishedType = PFT_Cancel;
    m_tester->slotHandleExtractFinished();
    m_tester->m_pExtractJob->m_eFinishedType = PFT_Error;
    m_tester->slotHandleExtractFinished();
}

TEST_F(TestStepExtractJob, testdoKill)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    m_tester->m_pExtractJob2 = new ExtractJob(QList<FileEntry>(), pIface, ExtractionOptions(), m_tester);
    m_tester->doKill();
}


