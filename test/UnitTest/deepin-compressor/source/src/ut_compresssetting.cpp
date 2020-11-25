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

#include "compresssetting.h"

#include <gtest/gtest.h>

class TestCompressSetting : public ::testing::Test
{
public:
    TestCompressSetting() {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressSetting;
        m_tester->show();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressSetting *m_tester;
};

TEST_F(TestCompressSetting, initTest)
{
}

TEST_F(TestCompressSetting, testInitUI)
{
    m_tester->InitUI();
    ASSERT_NE(m_tester->m_nextbutton, nullptr);
}

TEST_F(TestCompressSetting, testInitConnection)
{
    m_tester->InitConnection();
    ASSERT_NE(m_tester->m_nextbutton, nullptr);
}

TEST_F(TestCompressSetting, testinitWidget)
{
    m_tester->initWidget();
    ASSERT_NE(m_tester->m_nextbutton, nullptr);
}

TEST_F(TestCompressSetting, testsetTypeImage)
{
    m_tester->setTypeImage("zip");
    ASSERT_NE(m_tester->m_pixmaplabel, nullptr);
}

TEST_F(TestCompressSetting, testsetDefaultPath)
{
    m_tester->setDefaultPath("/home/gaoxiang/Desktop");
    ASSERT_EQ(m_tester->m_savepath->directoryUrl(), QUrl("/home/gaoxiang/Desktop"));
}

TEST_F(TestCompressSetting, testsetDefaultName)
{
    m_tester->setDefaultName("Create New Archive");
    ASSERT_EQ(m_tester->m_filename->lineEdit()->text(), "Create New Archive");
}

TEST_F(TestCompressSetting, testdirFileSize)
{
    m_tester->setDefaultName("Create New Archive");
    ASSERT_EQ(m_tester->m_filename->lineEdit()->text(), "Create New Archive");
}
