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
#include "properties.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QDir>
#include <QResizeEvent>
#include <QStandardPaths>

/*******************************函数打桩************************************/
/*******************************单元测试************************************/
// 测试Properties
class UT_Properties : public ::testing::Test
{
public:
    UT_Properties(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strPath = _SOURCEDIR;
        strPath += "/src/desktopdeepin-compressor.desktop";
        m_tester = new Properties(strPath, "Desktop Entry");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Properties *m_tester;
};

TEST_F(UT_Properties, initTest)
{

}

TEST_F(UT_Properties, test_Properties)
{
    Properties temp(*m_tester);
}

TEST_F(UT_Properties, test_getKeys)
{
    EXPECT_EQ(m_tester->getKeys().isEmpty(), true);
}
