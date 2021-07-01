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
class TestPluginManager : public ::testing::Test
{
public:
    TestPluginManager(): m_tester(nullptr) {}

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

TEST_F(TestPluginManager, initTest)
{

}

TEST_F(TestPluginManager, testget_instance)
{
    ASSERT_EQ(&m_tester->get_instance(), m_tester->m_instance);
}

TEST_F(TestPluginManager, testinstalledPlugins)
{
    m_tester->installedPlugins();
}

TEST_F(TestPluginManager, testavailablePlugins)
{
    m_tester->availablePlugins();
}

TEST_F(TestPluginManager, testavailableWritePlugins)
{
    m_tester->availableWritePlugins();
}

TEST_F(TestPluginManager, testenabledPlugins)
{
    m_tester->enabledPlugins();
}

TEST_F(TestPluginManager, testpreferredPluginsFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    m_tester->preferredPluginsFor(mimeType);
}

TEST_F(TestPluginManager, testpreferredWritePluginsFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    m_tester->preferredWritePluginsFor(mimeType);
}

TEST_F(TestPluginManager, testpreferredPluginFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    m_tester->preferredPluginFor(mimeType);
}

TEST_F(TestPluginManager, testpreferredWritePluginFor)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    m_tester->preferredWritePluginFor(mimeType);
}

TEST_F(TestPluginManager, testsupportedMimeTypes)
{
    m_tester->supportedMimeTypes(PluginManager::SortByComment);
}

TEST_F(TestPluginManager, testsupportedWriteMimeTypes)
{
    m_tester->supportedWriteMimeTypes(PluginManager::SortByComment);
}

TEST_F(TestPluginManager, testfilterBy)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    m_tester->filterBy(m_tester->availablePlugins(), mimeType);
}

TEST_F(TestPluginManager, testsetFileSize)
{
    m_tester->setFileSize(10);
}

TEST_F(TestPluginManager, testloadPlugins)
{
    m_tester->loadPlugins();
}

TEST_F(TestPluginManager, testpreferredPluginsFors)
{
    CustomMimeType mimeType;
    mimeType.m_bUnKnown = true;
    mimeType.m_strTypeName = "application/zip";
    m_tester->preferredPluginsFor(mimeType, true);
    m_tester->preferredPluginsFor(mimeType, false);
}

TEST_F(TestPluginManager, testsortByComment)
{
    QSet<QString> setValue;
    setValue.insert("application/zip");
    setValue.insert("application/x-7z-compressed");
    m_tester->sortByComment(setValue);
}

TEST_F(TestPluginManager, testlibarchiveHasLzo)
{
    m_tester->libarchiveHasLzo();
}

