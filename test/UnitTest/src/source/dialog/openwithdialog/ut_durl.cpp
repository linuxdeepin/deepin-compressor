///*
//* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
//*
//* Author:     chenglu <chenglu@uniontech.com>
//*
//* Maintainer: chenglu <chenglu@uniontech.com>
//*
//* This program is free software: you can redistribute it and/or modify
//* it under the terms of the GNU General Public License as published by
//* the Free Software Foundation, either version 3 of the License, or
//* any later version.
//*
//* This program is distributed in the hope that it will be useful,
//* but WITHOUT ANY WARRANTY; without even the implied warranty of
//* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//* GNU General Public License for more details.
//*
//* You should have received a copy of the GNU General Public License
//* along with this program.  If not, see <http://www.gnu.org/licenses/>.
//*/

//#include "durl.h"

//#include <gtest/gtest.h>
//#include <QTest>

//class TestDUrl : public ::testing::Test
//{
//public:
//    TestDUrl() {}

//public:
//    virtual void SetUp()
//    {
//        m_tester = new DUrl;
//    }

//    virtual void TearDown()
//    {
//        delete m_tester;
//    }

//protected:
//    DUrl *m_tester;
//};

//TEST_F(TestDUrl, initTest)
//{

//}

//TEST_F(TestDUrl, testsetPath)
//{
//    QString strPath = QFileInfo("../UnitTest/test_sources").absoluteFilePath();
//    m_tester->setPath(strPath);
//    ASSERT_NE(m_tester, nullptr);
//}

//TEST_F(TestDUrl, testsetScheme)
//{
//    m_tester->setScheme(TRASH_SCHEME);
//    ASSERT_EQ(m_tester->isTrashFile(), true);
//}

//TEST_F(TestDUrl, testsetUrl)
//{
//    QString strPath = QFileInfo("../UnitTest/test_sources").absoluteFilePath();
//    m_tester->setUrl(strPath);
//    ASSERT_NE(m_tester, nullptr);
//}

//TEST_F(TestDUrl, testisTrashFile)
//{
//    m_tester->setScheme(TRASH_SCHEME);
//    ASSERT_EQ(m_tester->isTrashFile(), true);
//}

//TEST_F(TestDUrl, testisRecentFile)
//{
//    m_tester->setScheme(RECENT_SCHEME);
//    ASSERT_EQ(m_tester->isRecentFile(), true);
//}

//TEST_F(TestDUrl, testisBookMarkFile)
//{
//    m_tester->setScheme(BOOKMARK_SCHEME);
//    ASSERT_EQ(m_tester->isBookMarkFile(), true);
//}

//TEST_F(TestDUrl, testisSearchFile)
//{
//    m_tester->setScheme(SEARCH_SCHEME);
//    ASSERT_EQ(m_tester->isSearchFile(), true);
//}

//TEST_F(TestDUrl, testisComputerFile)
//{
//    m_tester->setScheme(COMPUTER_SCHEME);
//    ASSERT_EQ(m_tester->isComputerFile(), true);
//}

//TEST_F(TestDUrl, testisNetWorkFile)
//{
//    m_tester->setScheme(NETWORK_SCHEME);
//    ASSERT_EQ(m_tester->isNetWorkFile(), true);
//}

//TEST_F(TestDUrl, testisSMBFile)
//{
//    m_tester->setScheme(SMB_SCHEME);
//    ASSERT_EQ(m_tester->isSMBFile(), true);
//}

//TEST_F(TestDUrl, testisAFCFile)
//{
//    m_tester->setScheme(AFC_SCHEME);
//    ASSERT_EQ(m_tester->isAFCFile(), true);
//}

//TEST_F(TestDUrl, testisMTPFile)
//{
//    m_tester->setScheme(MTP_SCHEME);
//    ASSERT_EQ(m_tester->isMTPFile(), true);
//}

//TEST_F(TestDUrl, testisUserShareFile)
//{
//    m_tester->setScheme(USERSHARE_SCHEME);
//    ASSERT_EQ(m_tester->isUserShareFile(), true);
//}

//TEST_F(TestDUrl, testisAVFSFile)
//{
//    m_tester->setScheme(AVFS_SCHEME);
//    ASSERT_EQ(m_tester->isAVFSFile(), true);
//}

//TEST_F(TestDUrl, testisFTPFile)
//{
//    m_tester->setScheme(FTP_SCHEME);
//    ASSERT_EQ(m_tester->isFTPFile(), true);
//}

//TEST_F(TestDUrl, testisSFTPFile)
//{
//    m_tester->setScheme(SFTP_SCHEME);
//    ASSERT_EQ(m_tester->isSFTPFile(), true);
//}

//TEST_F(TestDUrl, testisTaggedFile)
//{
//    m_tester->setScheme(TAG_SCHEME);
//    ASSERT_EQ(m_tester->isTaggedFile(), true);
//}

//TEST_F(TestDUrl, testtoString)
//{
//    QString str = m_tester->toString();
//    ASSERT_EQ(str, "");
//}

