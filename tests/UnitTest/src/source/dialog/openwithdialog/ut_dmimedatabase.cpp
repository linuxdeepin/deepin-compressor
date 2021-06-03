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
#include "dmimedatabase.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试DMimeDatabase
class TestDMimeDatabase : public ::testing::Test
{
public:
    TestDMimeDatabase(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DMimeDatabase;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DMimeDatabase *m_tester;
};

TEST_F(TestDMimeDatabase, initTest)
{

}

TEST_F(TestDMimeDatabase, testmimeTypeForFile)
{
    m_tester->mimeTypeForFile("1.docx");
}
