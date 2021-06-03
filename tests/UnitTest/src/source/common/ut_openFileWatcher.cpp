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
#include "openFileWatcher.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CompressPage
class TestOpenFileWatcher : public ::testing::Test
{
public:
    TestOpenFileWatcher(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenFileWatcher(nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenFileWatcher *m_tester;
};

TEST_F(TestOpenFileWatcher, initTest)
{

}

TEST_F(TestOpenFileWatcher, testreset)
{
    m_tester->m_mapFilePassword["1"] = "1";
    m_tester->reset();
    ASSERT_EQ(m_tester->m_mapFilePassword.isEmpty(), true);
}

TEST_F(TestOpenFileWatcher, testsetCurOpenFile)
{
    m_tester->setCurOpenFile("1.txt");
    ASSERT_EQ(m_tester->m_strOpenFile, "1.txt");
}

TEST_F(TestOpenFileWatcher, testaddCurOpenWatchFile)
{
    m_tester->setCurOpenFile("1.txt");
    m_tester->addCurOpenWatchFile();
    ASSERT_EQ(m_tester->m_mapFileHasModified["1.txt"], false);
}

TEST_F(TestOpenFileWatcher, testaddWatchFile)
{
    m_tester->addWatchFile("1.txt");
    ASSERT_EQ(m_tester->m_mapFileHasModified["1.txt"], false);
}

TEST_F(TestOpenFileWatcher, testgetFileHasModified)
{
    ASSERT_EQ(m_tester->getFileHasModified(), m_tester->m_mapFileHasModified);
}

TEST_F(TestOpenFileWatcher, testsetCurFilePassword)
{
    m_tester->setCurOpenFile("1.txt");
    m_tester->addCurOpenWatchFile();
    m_tester->setCurFilePassword("123456");
    ASSERT_EQ(m_tester->m_mapFilePassword["1.txt"], "123456");
}

TEST_F(TestOpenFileWatcher, testgetFilePassword)
{
    ASSERT_EQ(m_tester->getFilePassword(), m_tester->m_mapFilePassword);
}
