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

#include "datamanager.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>


/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试DataManager
class TestDataManager : public ::testing::Test
{
public:
    TestDataManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DataManager;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DataManager *m_tester;
};

TEST_F(TestDataManager, initTest)
{

}

TEST_F(TestDataManager, testresetArchiveData)
{
    m_tester->m_stArchiveData.qSize = 100;
    m_tester->resetArchiveData();
    ASSERT_EQ(m_tester->m_stArchiveData.qSize, 0);
}

TEST_F(TestDataManager, testarchiveData)
{
    m_tester->m_stArchiveData.qSize = 100;
    ASSERT_EQ(m_tester->archiveData().qSize, 100);
}

TEST_F(TestDataManager, testget_instance)
{
    ASSERT_EQ((&m_tester->get_instance()) == m_tester->m_instance, true);
}

