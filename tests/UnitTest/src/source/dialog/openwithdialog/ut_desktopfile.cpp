/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenglu <chenglu@uniontech.com>
*
* Maintainer: chenglu <chenglu@uniontech.com>
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
#include "config.h"

#include <gtest/gtest.h>
#include <QTest>

class TestDesktopFile : public ::testing::Test
{
public:
    TestDesktopFile(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strPath = TEST_SOURCES_PATH + QString("/deepin-compressor.desktop");
        m_tester = new DesktopFile(strPath);
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

TEST_F(TestDesktopFile, testgetFileName)
{
    QString strPath = TEST_SOURCES_PATH + QString("/deepin-compressor.desktop");
    ASSERT_EQ(m_tester->getFileName(), strPath);
}

TEST_F(TestDesktopFile, testgetPureFileName)
{
    ASSERT_EQ(m_tester->getPureFileName(), "deepin-compressor");
}

TEST_F(TestDesktopFile, testgetName)
{
    ASSERT_EQ(m_tester->getName().isEmpty(), true);
}

//TEST_F(TestDesktopFile, testgetLocalName)
//{
//    ASSERT_EQ(m_tester->getLocalName(), "归档管理器");
//}

//TEST_F(TestDesktopFile, testgetDisplayName)
//{
//    ASSERT_EQ(m_tester->getDisplayName(), "归档管理器");
//}

TEST_F(TestDesktopFile, testgetExec)
{
    ASSERT_EQ(m_tester->getExec(), "/usr/bin/deepin-compressor  %F");
}

TEST_F(TestDesktopFile, testgetIcon)
{
    ASSERT_EQ(m_tester->getIcon(), "deepin-compressor");
}

TEST_F(TestDesktopFile, testgetType)
{
    ASSERT_EQ(m_tester->getType(), "Application");
}

TEST_F(TestDesktopFile, testgetDeepinId)
{
    ASSERT_EQ(m_tester->getDeepinId(), "");
}

TEST_F(TestDesktopFile, testgetDeepinVendor)
{
    ASSERT_EQ(m_tester->getDeepinVendor(), "user-custom");
}

TEST_F(TestDesktopFile, testgetNoShow)
{
    ASSERT_EQ(m_tester->getNoShow(), false);
}

TEST_F(TestDesktopFile, testgetCategories)
{
    QStringList list = QStringList() << "Qt" << "Archiving" << "Compression" << "";
    ASSERT_EQ(m_tester->getCategories(), list);
}

TEST_F(TestDesktopFile, testgetMimeType)
{
    ASSERT_EQ(m_tester->getMimeType().contains("application/x-tar"), true);
}
