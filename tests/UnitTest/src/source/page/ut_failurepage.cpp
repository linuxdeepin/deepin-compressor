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
#include "libzipplugin.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

class UT_FailurePage : public ::testing::Test
{
public:
    UT_FailurePage(): m_tester(nullptr) {}

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

TEST_F(UT_FailurePage, initTest)
{

}

TEST_F(UT_FailurePage, test_setFailuerDes)
{
    m_tester->setFailuerDes("111");
    EXPECT_EQ(m_tester->m_pFailureLbl->text(), "111");
}

TEST_F(UT_FailurePage, test_setFailureDetail)
{
    m_tester->setFailureDetail("111");
    EXPECT_EQ(m_tester->m_pDetailLbl->text(), "111");
}

TEST_F(UT_FailurePage, test_setRetryEnable)
{
    m_tester->setRetryEnable(true);
    EXPECT_EQ(m_tester->m_pRetrybutton->isEnabled(), true);
}

TEST_F(UT_FailurePage, test_setFailureInfo)
{
    m_tester->m_failureInfo = FailureInfo::FI_Compress;
    m_tester->setFailureInfo(FailureInfo::FI_Load);
    EXPECT_EQ(m_tester->m_failureInfo, FailureInfo::FI_Load);
}

TEST_F(UT_FailurePage, test_getFailureInfo)
{
    m_tester->m_failureInfo = FailureInfo::FI_Compress;
    EXPECT_EQ(m_tester->getFailureInfo(), FailureInfo::FI_Compress);
}
