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

#include "desktopfile.h"

#include <gtest/gtest.h>

#include <QDirIterator>

class TestDesktopFile : public ::testing::Test
{
public:
    TestDesktopFile() {}

public:
    virtual void SetUp()
    {
        QString strDesktopFolder = "/usr/share/applications/com.oray.sunlogin.client.desktop";
        m_tester = new DesktopFile(strDesktopFolder);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DesktopFile *m_tester;
};

TEST_F(TestDesktopFile, initTest)
{
}

//TEST_F(TestDesktopFile, testgetFileName)
//{
//    ASSERT_STREQ(m_tester->getFileName().toStdString().c_str(), "/usr/share/applications//usr/share/applications/com.oray.sunlogin.client.desktop");
//}

//TEST_F(TestDesktopFile, testgetPureFileName)
//{
//    ASSERT_STREQ(m_tester->getPureFileName().toStdString().c_str(), "/usr/share/applications/com.oray.sunlogin.client");
//}

//TEST_F(TestDesktopFile, testgetName)
//{
//    ASSERT_STREQ(m_tester->getName().toStdString().c_str(), "Sunlogin Client");
//}

TEST_F(TestDesktopFile, testgetLocalName)
{
    ASSERT_STREQ(m_tester->getLocalName().toStdString().c_str(), "");
}

//TEST_F(TestDesktopFile, testgetDisplayName)
//{
//    ASSERT_STREQ(m_tester->getDisplayName().toStdString().c_str(), "Sunlogin Client");
//}

TEST_F(TestDesktopFile, testgetExec)
{
    ASSERT_STREQ(m_tester->getExec().toStdString().c_str(), "");
}

TEST_F(TestDesktopFile, testgetIcon)
{
    ASSERT_STREQ(m_tester->getIcon().toStdString().c_str(), "");
}

TEST_F(TestDesktopFile, testgetType)
{
    ASSERT_STREQ(m_tester->getType().toStdString().c_str(), "");
}

TEST_F(TestDesktopFile, testgetDeepinId)
{
    ASSERT_STREQ(m_tester->getDeepinId().toStdString().c_str(), "");
}

TEST_F(TestDesktopFile, testgetDeepinVendor)
{
    ASSERT_STREQ(m_tester->getDeepinVendor().toStdString().c_str(), "");
}

TEST_F(TestDesktopFile, testgetNoShow)
{
    ASSERT_TRUE(m_tester->getNoShow() == false);
}

TEST_F(TestDesktopFile, testgetCategories)
{
    ASSERT_TRUE(m_tester->getCategories().empty());
}

TEST_F(TestDesktopFile, testgetMimeType)
{
    ASSERT_TRUE(m_tester->getMimeType().empty());
}
