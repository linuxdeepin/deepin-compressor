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

#include "dfmstandardpaths.h"
#include "durl.h"

#include <gtest/gtest.h>
#include <QTest>

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

//TEST_F(TestDFMStandardPaths, testgetCachePath)
//{
//    QString defaultPath = m_tester->getCachePath();
//    ASSERT_EQ(QFile::exists(defaultPath), true);
//}

TEST_F(TestDFMStandardPaths, testlocation_TrashPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::TrashPath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.local/share/Trash");
}

TEST_F(TestDFMStandardPaths, testlocation_TrashFilesPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::TrashFilesPath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.local/share/Trash/files");
}

TEST_F(TestDFMStandardPaths, testlocation_TrashInfosPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::TrashInfosPath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.local/share/Trash/info");
}

TEST_F(TestDFMStandardPaths, testlocation_ThumbnailPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::ThumbnailPath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.cache/thumbnails");
}

TEST_F(TestDFMStandardPaths, testlocation_ThumbnailFailPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::ThumbnailFailPath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.cache/thumbnails/fail");
}

TEST_F(TestDFMStandardPaths, testlocation_ThumbnailLargePath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::ThumbnailLargePath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.cache/thumbnails/large");
}

TEST_F(TestDFMStandardPaths, testlocation_ThumbnailNormalPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::ThumbnailNormalPath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.cache/thumbnails/normal");
}

TEST_F(TestDFMStandardPaths, testlocation_ThumbnailSmallPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::ThumbnailSmallPath);
    ASSERT_EQ(strLocation, QDir::homePath() + "/.cache/thumbnails/small");
}

TEST_F(TestDFMStandardPaths, testlocation_RecentPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::RecentPath);
    ASSERT_EQ(strLocation, "recent:///");
}

TEST_F(TestDFMStandardPaths, testlocation_HomePath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::HomePath);
    ASSERT_EQ(strLocation, QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
}

TEST_F(TestDFMStandardPaths, testlocation_DesktopPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::DesktopPath);
    ASSERT_EQ(strLocation, QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
}

TEST_F(TestDFMStandardPaths, testlocation_VideosPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::VideosPath);
    ASSERT_EQ(strLocation, QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first());
}

TEST_F(TestDFMStandardPaths, testlocation_MusicPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::MusicPath);
    ASSERT_EQ(strLocation, QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
}

TEST_F(TestDFMStandardPaths, testlocation_PicturesPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::PicturesPath);
    ASSERT_EQ(strLocation, QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first());
}

TEST_F(TestDFMStandardPaths, testlocation_DocumentsPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::DocumentsPath);
    ASSERT_EQ(strLocation, QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
}

TEST_F(TestDFMStandardPaths, testlocation_DownloadsPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::DownloadsPath);
    ASSERT_EQ(strLocation, QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first());
}

TEST_F(TestDFMStandardPaths, testlocation_CachePath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::CachePath);
    ASSERT_EQ(strLocation, m_tester->getCachePath());
}

TEST_F(TestDFMStandardPaths, testlocation_DiskPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::DiskPath);
    ASSERT_EQ(strLocation, QDir::rootPath());
}

TEST_F(TestDFMStandardPaths, testlocation_NetworkRootPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::NetworkRootPath);
//    ASSERT_EQ(strLocation, NETWORK_ROOT);
}

TEST_F(TestDFMStandardPaths, testlocation_UserShareRootPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::UserShareRootPath);
//    ASSERT_EQ(strLocation, USERSHARE_ROOT);
}

TEST_F(TestDFMStandardPaths, testlocation_ComputerRootPath)
{
    QString strLocation = m_tester->location(DFMStandardPaths::ComputerRootPath);
//    ASSERT_EQ(strLocation, COMPUTER_ROOT);
}

TEST_F(TestDFMStandardPaths, testlocation_Root)
{
    QString strLocation = m_tester->location(DFMStandardPaths::Root);
    ASSERT_EQ(strLocation, "/");
}


