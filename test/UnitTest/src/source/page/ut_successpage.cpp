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

#include "successpage.h"
#include "uitools.h"
#include "customwidget.h"

#include <gtest/gtest.h>
#include <QTest>

class TestSuccessPage : public ::testing::Test
{
public:
    TestSuccessPage() {}

public:
    virtual void SetUp()
    {
        m_tester = new SuccessPage;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SuccessPage *m_tester;
};

TEST_F(TestSuccessPage, initTest)
{

}

TEST_F(TestSuccessPage, setCompressFullPath)
{
    m_tester->setCompressFullPath("/home/chhenglu/Desktop/ut");
    EXPECT_EQ(m_tester->m_strFullPath, "/home/chhenglu/Desktop/ut");
}

TEST_F(TestSuccessPage, setSuccessDes)
{
    m_tester->setSuccessDes("压缩成功");
    EXPECT_EQ(m_tester->m_pSuccessLbl->text() == "压缩成功", true);
}

TEST_F(TestSuccessPage, setSuccessType)
{
    m_tester->setSuccessType(SI_Compress);
    EXPECT_EQ(m_tester->m_successInfoType, SI_Compress);
}

TEST_F(TestSuccessPage, getSuccessType)
{
    m_tester->m_successInfoType = SI_Compress;
    SuccessInfo info = m_tester->getSuccessType();
    EXPECT_EQ(info, SI_Compress);
}

TEST_F(TestSuccessPage, initUI)
{
    m_tester->initUI();
}

TEST_F(TestSuccessPage, initConnections)
{
    m_tester->initConnections();
}
