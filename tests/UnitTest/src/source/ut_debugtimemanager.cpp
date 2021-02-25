/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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

#include "DebugTimeManager.h"

#include <gtest/gtest.h>
#include <QTest>

class TestDebugTimeManager : public ::testing::Test
{
public:
    TestDebugTimeManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DebugTimeManager();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DebugTimeManager *m_tester;
};

TEST_F(TestDebugTimeManager, initTest)
{

}

TEST_F(TestDebugTimeManager, testclear)
{
    m_tester->beginPointQt("test1");
    m_tester->clear();
    ASSERT_EQ(m_tester->m_MapPoint.isEmpty(), true);
}

TEST_F(TestDebugTimeManager, testbeginPointQt)
{
    m_tester->beginPointQt("test2");
    ASSERT_EQ(m_tester->m_MapPoint.isEmpty(), false);
}

TEST_F(TestDebugTimeManager, testendPointQt)
{
    m_tester->beginPointQt("test3");
    m_tester->beginPointQt("test4");
    m_tester->endPointQt("test3");
    ASSERT_EQ(m_tester->m_MapPoint.size(), 1);
}

TEST_F(TestDebugTimeManager, testbeginPointLinux)
{
    m_tester->beginPointLinux("test2");
    ASSERT_EQ(m_tester->m_MapPoint.isEmpty(), false);
}

TEST_F(TestDebugTimeManager, testendPointLinux)
{
    m_tester->beginPointLinux("test3");
    m_tester->beginPointLinux("test4");
    m_tester->endPointLinux("test3");
    ASSERT_EQ(m_tester->m_MapPoint.size(), 1);
}
