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
class UT_FileEntry : public ::testing::Test
{
public:
    UT_FileEntry(): m_tester(nullptr) {}

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

TEST_F(UT_FileEntry, initTest)
{

}

TEST_F(UT_FileEntry, UT_FileEntry_reset)
{
    EXPECT_EQ(m_tester->iIndex, -1);
}


// 测试ArchiveData
class UT_ArchiveData : public ::testing::Test
{
public:
    UT_ArchiveData(): m_tester(nullptr) {}

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

TEST_F(UT_ArchiveData, initTest)
{

}

TEST_F(UT_ArchiveData, test_reset)
{
    EXPECT_EQ(m_tester->strPassword.isEmpty(), true);
}


// 测试UpdateOptions
class UT_UpdateOptions : public ::testing::Test
{
public:
    UT_UpdateOptions(): m_tester(nullptr) {}

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

TEST_F(UT_UpdateOptions, initTest)
{

}

TEST_F(UT_UpdateOptions, test_reset)
{
    EXPECT_EQ(m_tester->qSize, 0);
}
