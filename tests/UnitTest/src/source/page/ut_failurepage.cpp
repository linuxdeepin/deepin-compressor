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

#include "failurepage.h"
#include "customwidget.h"
#include "uistruct.h"

#include "gtest/src/stub.h"

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
        m_tester->disconnect();
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

TEST_F(TestFailurePage, testsetFailuerDes)
{
    m_tester->setFailuerDes("111");
    ASSERT_EQ(m_tester->m_pFailureLbl->text(), "111");
}

TEST_F(TestFailurePage, testsetFailureDetail)
{
    m_tester->setFailureDetail("111");
    ASSERT_EQ(m_tester->m_pDetailLbl->text(), "111");
}

TEST_F(TestFailurePage, testsetRetryEnable)
{
    m_tester->setRetryEnable(true);
    ASSERT_EQ(m_tester->m_pRetrybutton->isEnabled(), true);
}

TEST_F(TestFailurePage, testsetFailureInfo)
{
    m_tester->m_failureInfo = FailureInfo::FI_Compress;
    m_tester->setFailureInfo(FailureInfo::FI_Load);
    ASSERT_EQ(m_tester->m_failureInfo, FailureInfo::FI_Load);
}

TEST_F(TestFailurePage, testgetFailureInfo)
{
    m_tester->m_failureInfo = FailureInfo::FI_Compress;
    ASSERT_EQ(m_tester->getFailureInfo(), FailureInfo::FI_Compress);
}
