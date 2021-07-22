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
#include "properties.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/
// 对CompressView的addCompressFiles进行打桩
bool properties_contains_stub(const QString &key)
{
    if (key == "X-Deepin-AppID" || key == "X-Deepin-Vendor" || key == "NoDisplay" || key == "Hidden") {
        return true;
    }

    return false;
}

/*******************************单元测试************************************/
// 测试CompressParameter
class TestDesktopFile : public ::testing::Test
{
public:
    TestDesktopFile(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DesktopFile;
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

TEST_F(TestDesktopFile, testDesktopFile)
{
    Stub stub;
    QFileStub::stub_QFile_exists(stub, true);
    stub.set(ADDR(Properties, contains), properties_contains_stub);

    DesktopFile desktop1("1.desktop");
}

TEST_F(TestDesktopFile, testgetFileName)
{
    ASSERT_EQ(m_tester->getFileName(), m_tester->m_fileName);
}

TEST_F(TestDesktopFile, testgetPureFileName)
{
    QString strPath = _SOURCEDIR;
    strPath += "/src/desktop/deepin-compressor.desktop";
    m_tester->m_fileName = strPath;
    ASSERT_EQ(m_tester->getPureFileName(), "deepin-compressor");
}

TEST_F(TestDesktopFile, testgetName)
{
    m_tester->m_name = "deepin";
    ASSERT_EQ(m_tester->getName(), "deepin");
}

TEST_F(TestDesktopFile, testgetLocalName)
{
    m_tester->m_localName = "deepin";
    ASSERT_EQ(m_tester->getLocalName(), "deepin");
}

TEST_F(TestDesktopFile, testgetDisplayName)
{
    m_tester->m_deepinVendor = "deepin";
    m_tester->m_genericName = "compressor";
    ASSERT_EQ(m_tester->getDisplayName(), "compressor");

    m_tester->m_deepinVendor = "360";
    m_tester->m_localName = "test";
    ASSERT_EQ(m_tester->getDisplayName(), "test");
}

TEST_F(TestDesktopFile, testgetExec)
{
    ASSERT_EQ(m_tester->getExec(), m_tester->m_exec);
}

TEST_F(TestDesktopFile, testgetIcon)
{
    ASSERT_EQ(m_tester->getIcon(), m_tester->m_icon);
}

TEST_F(TestDesktopFile, testgetType)
{
    ASSERT_EQ(m_tester->getType(), m_tester->m_type);
}

TEST_F(TestDesktopFile, testgetDeepinId)
{
    ASSERT_EQ(m_tester->getDeepinId(), m_tester->m_deepinId);
}

TEST_F(TestDesktopFile, testgetDeepinVendor)
{
    ASSERT_EQ(m_tester->getDeepinVendor(), m_tester->m_deepinVendor);
}

TEST_F(TestDesktopFile, testgetNoShow)
{
    m_tester->m_noDisplay = true;
    ASSERT_EQ(m_tester->getNoShow(), true);
}

TEST_F(TestDesktopFile, testgetCategories)
{
    ASSERT_EQ(m_tester->getCategories() == m_tester->m_categories, true);
}

TEST_F(TestDesktopFile, testgetMimeType)
{
    ASSERT_EQ(m_tester->getMimeType() == m_tester->m_mimeType, true);
}
