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

TEST_F(UT_ProcessOpenThread, testrun)
{
    Stub stub;
    CommonStub::stub_KProcess_start(stub);
    m_tester->run();
}
