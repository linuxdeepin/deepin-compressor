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
#include "mimetypedisplaymanager.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/


/*******************************单元测试************************************/
// 测试MimeTypeDisplayManager
class UT_MimeTypeDisplayManager : public ::testing::Test
{
public:
    UT_MimeTypeDisplayManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MimeTypeDisplayManager();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MimeTypeDisplayManager *m_tester;
};

TEST_F(UT_MimeTypeDisplayManager, initTest)
{

}

TEST_F(UT_MimeTypeDisplayManager, test_displayName)
{
    EXPECT_EQ(m_tester->displayName("inode/directory"), "Directory");
}

TEST_F(UT_MimeTypeDisplayManager, test_displayNameToEnum)
{
    EXPECT_EQ(m_tester->displayNameToEnum("application/x-desktop"), FileType::DesktopApplication);
    EXPECT_EQ(m_tester->displayNameToEnum("inode/directory"), FileType::Directory);
    EXPECT_EQ(m_tester->displayNameToEnum("application/x-executable"), FileType::Executable);
    EXPECT_EQ(m_tester->displayNameToEnum("video/"), FileType::Videos);
    EXPECT_EQ(m_tester->displayNameToEnum("audio/"), FileType::Audios);
    EXPECT_EQ(m_tester->displayNameToEnum("image/"), FileType::Images);
    EXPECT_EQ(m_tester->displayNameToEnum("text/"), FileType::Documents);
    if (MimeTypeDisplayManager::ArchiveMimeTypes.count() > 0) {
        EXPECT_EQ(m_tester->displayNameToEnum(MimeTypeDisplayManager::ArchiveMimeTypes[0]), FileType::Archives);
    }
    if (MimeTypeDisplayManager::BackupMimeTypes.count() > 0) {
        EXPECT_EQ(m_tester->displayNameToEnum(MimeTypeDisplayManager::BackupMimeTypes[0]), FileType::Backups);
    }
    EXPECT_EQ(m_tester->displayNameToEnum("sadsadasd"), FileType::Unknown);
}

TEST_F(UT_MimeTypeDisplayManager, test_defaultIcon)
{
    EXPECT_EQ(m_tester->defaultIcon("inode/directory"), "folder");
}

TEST_F(UT_MimeTypeDisplayManager, test_displayNames)
{
    EXPECT_EQ(m_tester->displayNames(), m_tester->m_displayNames);
}

TEST_F(UT_MimeTypeDisplayManager, test_supportArchiveMimetypes)
{
    EXPECT_EQ(m_tester->supportArchiveMimetypes() == MimeTypeDisplayManager::ArchiveMimeTypes, true);
}

TEST_F(UT_MimeTypeDisplayManager, test_supportVideoMimeTypes)
{
    EXPECT_EQ(m_tester->supportVideoMimeTypes() == MimeTypeDisplayManager::VideoMimeTypes, true);
}


