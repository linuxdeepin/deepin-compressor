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

#include "commonstruct.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>


/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试FileEntry
class TestFileEntry : public ::testing::Test
{
public:
    TestFileEntry(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new FileEntry;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    FileEntry *m_tester;
};

TEST_F(TestFileEntry, initTest)
{

}

TEST_F(TestFileEntry, testreset)
{
    ASSERT_EQ(m_tester->iIndex, -1);
}


// 测试ArchiveData
class TestArchiveData : public ::testing::Test
{
public:
    TestArchiveData(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ArchiveData;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ArchiveData *m_tester;
};

TEST_F(TestArchiveData, initTest)
{

}

TEST_F(TestArchiveData, testreset)
{
    ASSERT_EQ(m_tester->strPassword.isEmpty(), true);
}


// 测试UpdateOptions
class TestUpdateOptions : public ::testing::Test
{
public:
    TestUpdateOptions(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UpdateOptions;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UpdateOptions *m_tester;
};

TEST_F(TestUpdateOptions, initTest)
{

}

TEST_F(TestUpdateOptions, testreset)
{
    ASSERT_EQ(m_tester->qSize, 0);
}
