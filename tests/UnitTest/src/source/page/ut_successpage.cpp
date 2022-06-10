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

#include "successpage.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

class UT_SuccessPage : public ::testing::Test
{
public:
    UT_SuccessPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SuccessPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SuccessPage *m_tester;
};

TEST_F(UT_SuccessPage, initTest)
{

}

TEST_F(UT_SuccessPage, test_setCompressFullPath)
{
    m_tester->setCompressFullPath("111");
    EXPECT_EQ(m_tester->m_strFullPath, "111");
}

TEST_F(UT_SuccessPage, test_setSuccessDes)
{
    m_tester->setSuccessDes("222");
    EXPECT_EQ(m_tester->m_pSuccessLbl->text(), "222");
}

TEST_F(UT_SuccessPage, test_setSuccessType)
{
    m_tester->setSuccessType(SI_Compress);
    EXPECT_EQ(m_tester->m_successInfoType, SI_Compress);
}

TEST_F(UT_SuccessPage, test_getSuccessType)
{
    m_tester->setSuccessType(SI_Compress);
    EXPECT_EQ(m_tester->getSuccessType(), SI_Compress);
}
