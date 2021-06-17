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
#include "mimesappsmanager.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试MimeAppsWorker
class TestMimeAppsWorker : public ::testing::Test
{
public:
    TestMimeAppsWorker(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MimeAppsWorker;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MimeAppsWorker *m_tester;
};

TEST_F(TestMimeAppsWorker, initTest)
{

}

TEST_F(TestMimeAppsWorker, testhandleDirectoryChanged)
{
    m_tester->handleDirectoryChanged("");
}

TEST_F(TestMimeAppsWorker, testupdateCache)
{
    m_tester->updateCache();
}

TEST_F(TestMimeAppsWorker, testwriteData)
{
    m_tester->writeData("", "");
}

TEST_F(TestMimeAppsWorker, testreadData)
{
    m_tester->writeData("", "");
}


// 测试MimesAppsManager
class TestMimesAppsManager : public ::testing::Test
{
public:
    TestMimesAppsManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MimesAppsManager;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MimesAppsManager *m_tester;
};

TEST_F(TestMimesAppsManager, initTest)
{

}

TEST_F(TestMimesAppsManager, testgetMimeType)
{
    ASSERT_EQ(m_tester->getMimeType("1.zip").name(), "application/zip");
}

TEST_F(TestMimesAppsManager, testgetMimeTypeByFileName)
{
    ASSERT_EQ(m_tester->getMimeTypeByFileName("1.zip"), "application/zip");
}

TEST_F(TestMimesAppsManager, testgetDefaultAppByFileName)
{
    m_tester->getDefaultAppByFileName("1.zip");
}

TEST_F(TestMimesAppsManager, testgetDefaultAppByMimeType)
{
    m_tester->getDefaultAppByMimeType("1.zip");
}

TEST_F(TestMimesAppsManager, testgetDefaultAppByMimeType1)
{
    m_tester->getDefaultAppByMimeType(m_tester->getMimeType("1.zip"));
}

TEST_F(TestMimesAppsManager, testgetDefaultAppDisplayNameByMimeType)
{
    m_tester->getDefaultAppByMimeType("1.zip");
}

TEST_F(TestMimesAppsManager, testgetDefaultAppDisplayNameByGio)
{
    m_tester->getDefaultAppDisplayNameByGio("1.zip");
}

TEST_F(TestMimesAppsManager, testgetDefaultAppDesktopFileByMimeType)
{
    m_tester->getDefaultAppDesktopFileByMimeType("1.zip");
}

TEST_F(TestMimesAppsManager, testgetRecommendedAppsByQio)
{
    m_tester->getRecommendedAppsByQio(m_tester->getMimeType("1.zip"));
}

TEST_F(TestMimesAppsManager, testgetRecommendedAppsByGio)
{
    m_tester->getRecommendedAppsByGio("1.zip");
}

TEST_F(TestMimesAppsManager, testgetApplicationsFolders)
{
    m_tester->getApplicationsFolders();
}

TEST_F(TestMimesAppsManager, testgetMimeAppsCacheFile)
{
    m_tester->getMimeAppsCacheFile();
}

TEST_F(TestMimesAppsManager, testgetMimeInfoCacheFilePath)
{
    m_tester->getMimeInfoCacheFilePath();
}

TEST_F(TestMimesAppsManager, testgetMimeInfoCacheFileRootPath)
{
    m_tester->getMimeInfoCacheFileRootPath();
}

TEST_F(TestMimesAppsManager, testgetDesktopFilesCacheFile)
{
    m_tester->getDesktopFilesCacheFile();
}

TEST_F(TestMimesAppsManager, testgetDesktopIconsCacheFile)
{
    m_tester->getDesktopIconsCacheFile();
}

TEST_F(TestMimesAppsManager, testgetDesktopFiles)
{
    m_tester->getDesktopIconsCacheFile();
}

TEST_F(TestMimesAppsManager, testgetDDEMimeTypeFile)
{
    m_tester->getDDEMimeTypeFile();
}

TEST_F(TestMimesAppsManager, testgetDesktopObjs)
{
    m_tester->getDesktopObjs();
}

TEST_F(TestMimesAppsManager, testinitMimeTypeApps)
{
    m_tester->initMimeTypeApps();
}

TEST_F(TestMimesAppsManager, testloadDDEMimeTypes)
{
    m_tester->loadDDEMimeTypes();
}
