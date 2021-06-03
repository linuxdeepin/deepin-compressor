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
#include "plugin.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试Plugin
class TestPlugin : public ::testing::Test
{
public:
    TestPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Plugin(nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Plugin *m_tester;
};

TEST_F(TestPlugin, initTest)
{

}

TEST_F(TestPlugin, testpriority)
{
    ASSERT_EQ(m_tester->priority(), 0);
}

TEST_F(TestPlugin, testisEnabled)
{
    ASSERT_EQ(m_tester->isEnabled(), m_tester->m_enabled);
}

TEST_F(TestPlugin, testsetEnabled)
{
    m_tester->m_enabled = false;
    m_tester->setEnabled(true);
    ASSERT_EQ(m_tester->m_enabled, true);
}

TEST_F(TestPlugin, testisReadWrite)
{
    ASSERT_EQ(m_tester->isReadWrite(), false);
}

TEST_F(TestPlugin, testreadOnlyExecutables)
{
    ASSERT_EQ(m_tester->isReadWrite(), false);
}
