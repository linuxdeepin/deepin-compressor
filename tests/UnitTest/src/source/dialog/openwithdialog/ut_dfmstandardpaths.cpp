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
#include "dfmstandardpaths.h"
#include "durl.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CompressParameter
class TestDFMStandardPaths : public ::testing::Test
{
public:
    TestDFMStandardPaths(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DFMStandardPaths;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DFMStandardPaths *m_tester;
};

TEST_F(TestDFMStandardPaths, initTest)
{

}

TEST_F(TestDFMStandardPaths, testlocation)
{
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::TrashPath), QDir::homePath() + "/.local/share/Trash");
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::TrashFilesPath), QDir::homePath() + "/.local/share/Trash/files");
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::TrashInfosPath), QDir::homePath() + "/.local/share/Trash/info");
#ifdef APPSHAREDIR
#endif
#ifdef PLUGINDIR
#endif
#ifdef QMAKE_TARGET
#endif
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath), QDir::homePath() + "/.cache/thumbnails");
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailFailPath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/fail");
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailLargePath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/large");
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailNormalPath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/normal");
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailSmallPath), m_tester->location(DFMStandardPaths::StandardLocation::ThumbnailPath) + "/small");
#ifdef APPSHAREDIR
#endif
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::RecentPath), "recent:///");
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::HomePath), QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DesktopPath), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::VideosPath), QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::MusicPath), QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::PicturesPath), QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DocumentsPath), QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DownloadsPath), QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::CachePath), m_tester->getCachePath());
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::DiskPath), QDir::rootPath());
#ifdef NETWORK_ROOT
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::NetworkRootPath), NETWORK_ROOT);
#endif
#ifdef USERSHARE_ROOT
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::UserShareRootPath), USERSHARE_ROOT);
#endif
#ifdef COMPUTER_ROOT
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::ComputerRootPath), COMPUTER_ROOT);
#endif
    ASSERT_EQ(m_tester->location(DFMStandardPaths::StandardLocation::Root), "/");
}

TEST_F(TestDFMStandardPaths, testgetCachePath)
{
    m_tester->getCachePath();
}

