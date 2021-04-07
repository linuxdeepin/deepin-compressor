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

#include "singlejob.h"
#include "gtest/src/stub.h"
#include "uitools.h"

#include <gtest/gtest.h>
#include <QTest>

class TestSingleJobThread : public ::testing::Test
{
public:
    TestSingleJobThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pLoadJob = new LoadJob(pInterface);
        m_tester = new SingleJobThread(m_pLoadJob);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ReadOnlyArchiveInterface *pInterface = nullptr;
    LoadJob *m_pLoadJob;
    SingleJobThread *m_tester;
};

TEST_F(TestSingleJobThread, initTest)
{

}

void doWork_stub()
{
    return;
}

TEST_F(TestSingleJobThread, testrun)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&LoadJob::doWork);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, doWork_stub);
    m_tester->run();
    ASSERT_NE(m_tester, nullptr);
}
