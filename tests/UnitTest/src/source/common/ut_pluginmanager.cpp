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
#include "pluginmanager.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试PluginManager
class UT_PluginManager : public ::testing::Test
{
public:
    UT_PluginManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new PluginManager;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    PluginManager *m_tester;
};

TEST_F(UT_PluginManager, initTest)
{

}

TEST_F(UT_PluginManager, test_get_instance)
{
    EXPECT_EQ(&m_tester->get_instance(), m_tester->m_instance);
}

TEST_F(UT_PluginManager, test_installedPlugins)
{
    m_tester->installedPlugins();
    EXPECT_EQ(m_tester->m_plugins.isEmpty(), false);
}

TEST_F(UT_PluginManager, test_availablePlugins)
{
    EXPECT_EQ(m_tester->availablePlugins().isEmpty(), false);
}

TEST_F(UT_PluginManager, test_availableWritePlugins)
{
    EXPECT_EQ(m_tester->availableWritePlugins().isEmpty(), false);
}

TEST_F(UT_PluginManager, test_enabledPlugins)
{
    EXPECT_EQ(m_tester->enabledPlugins().isEmpty(), false);
}

TEST_F(UT_PluginManager, testpreferredPluginsFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    EXPECT_EQ(m_tester->preferredPluginsFor(mimeType).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_preferredWritePluginsFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    EXPECT_EQ(m_tester->preferredWritePluginsFor(mimeType).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_preferredPluginFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    EXPECT_NE(m_tester->preferredPluginFor(mimeType), nullptr);
}

TEST_F(UT_PluginManager, test_preferredWritePluginFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    EXPECT_NE(m_tester->preferredWritePluginFor(mimeType), nullptr);
}

TEST_F(UT_PluginManager, test_supportedMimeTypes)
{
    EXPECT_EQ(m_tester->supportedMimeTypes(PluginManager::SortByComment).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_supportedWriteMimeTypes)
{
    EXPECT_EQ(m_tester->supportedWriteMimeTypes(PluginManager::SortByComment).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_filterBy)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    EXPECT_EQ(m_tester->filterBy(m_tester->availablePlugins(), mimeType).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_setFileSize)
{
    m_tester->setFileSize(10);
    EXPECT_EQ(m_tester->m_filesize, 10);
}

TEST_F(UT_PluginManager, test_loadPlugins)
{
    m_tester->loadPlugins();
}

TEST_F(UT_PluginManager, test_preferredPluginsFors_001)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    EXPECT_EQ(m_tester->preferredPluginsFor(mimeType, true).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_preferredPluginsFors_002)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    EXPECT_EQ(m_tester->preferredPluginsFor(mimeType, false).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_sortByComment)
{
    QSet<QString> setValue;
    setValue.insert("application/zip");
    setValue.insert("application/x-7z-compressed");
    EXPECT_EQ(m_tester->sortByComment(setValue).isEmpty(), false);
}

TEST_F(UT_PluginManager, test_libarchiveHasLzo)
{
    EXPECT_EQ(m_tester->libarchiveHasLzo(), false);
}

