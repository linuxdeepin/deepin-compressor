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
class TestCommon : public ::testing::Test
{
public:
    TestCommon(): m_tester(nullptr) {}

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

TEST_F(TestCommon, initTest)
{

}

TEST_F(TestCommon, testcodecConfidenceForData)
{
    m_tester->codecConfidenceForData(QTextCodec::codecForName("GBK"), "哈哈", QLocale::China);
}

TEST_F(TestCommon, testtrans2uft8)
{
    QByteArray strCode;
    m_tester->trans2uft8("哈哈", strCode);
    m_tester->trans2uft8("abc", strCode);
}

TEST_F(TestCommon, testdetectEncode)
{

}

TEST_F(TestCommon, testChartDet_DetectingTextCoding)
{

}

TEST_F(TestCommon, testtextCodecDetect)
{

}
