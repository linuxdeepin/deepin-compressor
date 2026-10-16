// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processopenthread.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试ProcessOpenThread
class UT_ProcessOpenThread : public ::testing::Test
{
public:
    UT_ProcessOpenThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProcessOpenThread(nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ProcessOpenThread *m_tester;
};

TEST_F(UT_ProcessOpenThread, initTest)
{

}

TEST_F(UT_ProcessOpenThread, test_setProgramPath)
{
    m_tester->setProgramPath("xdg-open");
    EXPECT_EQ(m_tester->m_strProgramPath, "xdg-open");
}

TEST_F(UT_ProcessOpenThread, test_setArguments)
{
    QStringList listArguments = QStringList() << "1.txt";
    m_tester->setArguments(listArguments);
    EXPECT_EQ(m_tester->m_listArguments == listArguments, true);
}

// 测试 QProcess::startDetached 分支（指定程序路径）
TEST_F(UT_ProcessOpenThread, testrun_startDetached)
{
    Stub stub;
    CommonStub::stub_QProcess_startDetached(stub);
    m_tester->setProgramPath("test-app");
    m_tester->setArguments(QStringList() << "1.txt");
    m_tester->run();
}

// 测试 QDesktopServices::openUrl 分支（程序路径为空）
TEST_F(UT_ProcessOpenThread, testrun_openUrl_emptyProgramPath)
{
    Stub stub;
    CommonStub::stub_QDesktopServices_openUrl(stub);
    m_tester->setProgramPath("");
    m_tester->setArguments(QStringList() << "1.txt");
    m_tester->run();
}

// 测试 QDesktopServices::openUrl 分支（程序路径为 xdg-open）
TEST_F(UT_ProcessOpenThread, testrun_openUrl_xdgOpen)
{
    Stub stub;
    CommonStub::stub_QDesktopServices_openUrl(stub);
    m_tester->setProgramPath("/usr/bin/xdg-open");
    m_tester->setArguments(QStringList() << "1.txt");
    m_tester->run();
}
