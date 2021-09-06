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

#include "common.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <QTextCodec>
/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试Common
class UT_Common : public ::testing::Test
{
public:
    UT_Common(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Common;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Common *m_tester;
};

TEST_F(UT_Common, initTest)
{

}

TEST_F(UT_Common, test_codecConfidenceForData)
{
    EXPECT_EQ(m_tester->codecConfidenceForData(QTextCodec::codecForName("GBK"), "哈哈", QLocale::China), 1);
}

TEST_F(UT_Common, test_trans2uft8_001)
{
    QByteArray strCode;
    EXPECT_EQ(m_tester->trans2uft8("哈哈", strCode), "哈哈");
}

TEST_F(UT_Common, test_trans2uft8_002)
{
    QByteArray strCode;
    EXPECT_EQ(m_tester->trans2uft8("abc", strCode), "abc");
}

TEST_F(UT_Common, test_detectEncode)
{

}

TEST_F(UT_Common, test_ChartDet_DetectingTextCoding)
{

}

TEST_F(UT_Common, test_textCodecDetect)
{

}
