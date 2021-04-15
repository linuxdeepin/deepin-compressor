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

#include "singleton.h"
#include "mimesappsmanager.h"
#include "durl.h"
#include "config.h"

#include <gtest/src/stub.h>
#include <gtest/gtest.h>

#include <QTest>

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

// 测试MimeAppsWorker
TEST_F(TestMimeAppsWorker, initTest)
{

}

TEST_F(TestMimeAppsWorker, testinitConnect)
{
    m_tester->initConnect();
    ASSERT_NE(m_tester->m_fileSystemWatcher, nullptr);
}

QStringList getDesktopFiles_stub()
{
    return QStringList() << TEST_SOURCES_PATH + QString("/deepin-compressor.desktop");
//    return QStringList();
}

QStringList getApplicationsFolders_stub()
{
    return QStringList();
}

TEST_F(TestMimeAppsWorker, teststartWatch)
{
    Stub stub;
    stub.set(ADDR(MimesAppsManager, getDesktopFiles), getDesktopFiles_stub);
    stub.set(ADDR(MimesAppsManager, getApplicationsFolders), getApplicationsFolders_stub);

    m_tester->m_fileSystemWatcher->removePaths(m_tester->m_fileSystemWatcher->files());
    m_tester->startWatch();
    QStringList list = m_tester->m_fileSystemWatcher->files();
    ASSERT_EQ(m_tester->m_fileSystemWatcher->files(), QStringList() << TEST_SOURCES_PATH + QString("/deepin-compressor.desktop"));
}

TEST_F(TestMimeAppsWorker, testhandleFileChanged)
{
    m_tester->handleDirectoryChanged("");
    ASSERT_NE(m_tester->m_updateCacheTimer, nullptr);

    m_tester->m_updateCacheTimer->stop();
}

TEST_F(TestMimeAppsWorker, testhandleDirectoryChanged)
{
    m_tester->handleFileChanged("");
    ASSERT_NE(m_tester->m_updateCacheTimer, nullptr);

    m_tester->m_updateCacheTimer->stop();
}

TEST_F(TestMimeAppsWorker, testupdateCache)
{
    m_tester->updateCache();
    ASSERT_EQ(mimeAppsManager->DesktopFiles.isEmpty(), false);
}

TEST_F(TestMimeAppsWorker, testwriteData)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QByteArray strContent = "write data";
    bool bResult = m_tester->writeData(strFile, strContent);
    ASSERT_EQ(bResult, true);
}

TEST_F(TestMimeAppsWorker, testreadData)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QByteArray strContent = m_tester->readData(strFile);
    ASSERT_EQ(strContent.isEmpty(), false);
}


// MimesAppsManager
TEST_F(TestMimesAppsManager, initTest)
{

}

TEST_F(TestMimesAppsManager, testgetMimeType)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QMimeType type = m_tester->getMimeType(strFile);
    ASSERT_EQ(type.name().toStdString(), "text/plain");
}

TEST_F(TestMimesAppsManager, testgetMimeTypeByFileName)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QString strName = m_tester->getMimeTypeByFileName(strFile);
    ASSERT_EQ(strName.toStdString(), "text/plain");
}

TEST_F(TestMimesAppsManager, testgetDefaultAppByFileName)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QString strName = m_tester->getDefaultAppByFileName(strFile);
    ASSERT_EQ(strName.isEmpty(), false);
}

TEST_F(TestMimesAppsManager, testgetDefaultAppByMimeType_001)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QMimeType type = m_tester->getMimeType(strFile);
    QString strName = m_tester->getDefaultAppByMimeType(type);
    ASSERT_EQ(strName.isEmpty(), false);
}

TEST_F(TestMimesAppsManager, testgetDefaultAppByMimeType_002)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QMimeType type = m_tester->getMimeType(strFile);
    QString strName = m_tester->getDefaultAppByMimeType(type.name());
    ASSERT_EQ(strName.isEmpty(), false);
}

TEST_F(TestMimesAppsManager, testgetDefaultAppDisplayNameByMimeType)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QMimeType type = m_tester->getMimeType(strFile);
    QString strName = m_tester->getDefaultAppDisplayNameByMimeType(type);
    ASSERT_EQ(strName.isEmpty(), false);
}

TEST_F(TestMimesAppsManager, testgetDefaultAppDisplayNameByGio)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QMimeType type = m_tester->getMimeType(strFile);
    QString strName = m_tester->getDefaultAppDisplayNameByGio(type.name());
    ASSERT_EQ(strName.isEmpty(), false);
}

TEST_F(TestMimesAppsManager, testgetDefaultAppDesktopFileByMimeType)
{
    QString strFile = TEST_SOURCES_PATH + QString("/test.txt");
    QMimeType type = m_tester->getMimeType(strFile);
    QString strName = m_tester->getDefaultAppDisplayNameByGio(type.name());
    ASSERT_EQ(strName.isEmpty(), false);
}
