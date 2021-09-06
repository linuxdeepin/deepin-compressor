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
#include "uistruct.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CompressParameter
class UT_CompressParameter : public ::testing::Test
{
public:
    UT_CompressParameter(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressParameter;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressParameter *m_tester;
};

TEST_F(UT_CompressParameter, initTest)
{

}



// 测试UnCompressParameter
class UT_UnCompressParameter : public ::testing::Test
{
public:
    UT_UnCompressParameter(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UnCompressParameter;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UnCompressParameter *m_tester;
};

TEST_F(UT_UnCompressParameter, initTest)
{

}
