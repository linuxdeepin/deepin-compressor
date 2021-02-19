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

#include "failurepage.h"
#include "customwidget.h"
#include "uitools.h"

#include <gtest/gtest.h>
#include <QTest>

class TestFailurePage : public ::testing::Test
{
public:
    TestFailurePage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new FailurePage;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    FailurePage *m_tester;
};

TEST_F(TestFailurePage, initTest)
{

}

TEST_F(TestFailurePage, setFailuerDes)
{
    m_tester->setFailuerDes("解压失败");
}

TEST_F(TestFailurePage, setFailureDetail)
{
    m_tester->setFailureDetail("密码错误");
}

TEST_F(TestFailurePage, setRetryEnable)
{
    m_tester->setRetryEnable(true);
}

TEST_F(TestFailurePage, setFailureInfo)
{
    m_tester->setFailureInfo(FI_Uncompress);
}

TEST_F(TestFailurePage, getFailureInfo)
{
    m_tester->m_failureInfo = FI_Uncompress;
    FailureInfo info = m_tester->getFailureInfo();
    EXPECT_EQ(info, FI_Uncompress);
}

TEST_F(TestFailurePage, initUI)
{
    m_tester->initUI();
}

TEST_F(TestFailurePage, initConnections)
{
    m_tester->initConnections();
}
