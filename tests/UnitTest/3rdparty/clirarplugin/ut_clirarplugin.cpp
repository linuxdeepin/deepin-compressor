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

#include "clirarplugin.h"
#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>
#include <QStandardPaths>

Q_DECLARE_METATYPE(KPluginMetaData)

/*******************************函数打桩************************************/
int kill_stub(__pid_t, int)
{
    return 0;
}

qint64 kPtyProcess_processId_stub()
{
    return 123456;
}
/*******************************函数打桩************************************/

class TestCliRarPluginFactory : public QObject, public ::testing::Test
{
public:
    TestCliRarPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CliRarPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CliRarPluginFactory *m_tester;
};

class TestCliRarPlugin : public QObject, public ::testing::Test
{
public:
    TestCliRarPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("test.rar").absoluteFilePath();
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new CliRarPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CliRarPlugin *m_tester;
};


TEST_F(TestCliRarPluginFactory, initTest)
{

}

TEST_F(TestCliRarPlugin, initTest)
{

}

TEST_F(TestCliRarPlugin, testsetupCliProperties)
{
    m_tester->setupCliProperties();
}

TEST_F(TestCliRarPlugin, testisPasswordPrompt)
{
    ASSERT_EQ(m_tester->isPasswordPrompt("Enter password (will not be echoed) for : "), true);
}
TEST_F(TestCliRarPlugin, testisWrongPasswordMsg)
{
    ASSERT_EQ(m_tester->isWrongPasswordMsg("The specified password is incorrect"), true);
}

TEST_F(TestCliRarPlugin, testisCorruptArchiveMsg)
{
    ASSERT_EQ(m_tester->isCorruptArchiveMsg("Unexpected end of archive"), true);
}

TEST_F(TestCliRarPlugin, testisDiskFullMsg)
{
    ASSERT_EQ(m_tester->isDiskFullMsg("No space left on device"), true);
}

TEST_F(TestCliRarPlugin, testisFileExistsMsg)
{
    ASSERT_EQ(m_tester->isFileExistsMsg("[Y]es, [N]o, [A]ll, n[E]ver, [R]ename, [Q]uit "), true);
}

TEST_F(TestCliRarPlugin, testisFileExistsFileName)
{
    ASSERT_EQ(m_tester->isFileExistsFileName("Would you like to replace the existing file "), true);
}

TEST_F(TestCliRarPlugin, testisMultiPasswordPrompt)
{
    ASSERT_EQ(m_tester->isMultiPasswordPrompt("use current password ? [Y]es, [N]o, [A]ll"), true);
}

TEST_F(TestCliRarPlugin, testisOpenFileFailed)
{
    ASSERT_EQ(m_tester->isOpenFileFailed("Cannot create "), true);
}

TEST_F(TestCliRarPlugin, testkillProcess)
{
    Stub stub;
    stub.set(kill, kill_stub);
    stub.set(ADDR(KPtyProcess, processId), kPtyProcess_processId_stub);

    m_tester->m_isProcessKilled = false;
    m_tester->killProcess(true);
    ASSERT_EQ(m_tester->m_isProcessKilled, false);

    m_tester->m_isProcessKilled = false;
    m_tester->m_process = new KPtyProcess;
    m_tester->killProcess(true);
    ASSERT_EQ(m_tester->m_isProcessKilled, true);
}

TEST_F(TestCliRarPlugin, testreadListLine)
{
    m_tester->m_parseState = ParseStateTitle;
    m_tester->readListLine("UNRAR 5.61 beta 1 freeware      Copyright (c) 1993-2018 Alexander Roshal");
    ASSERT_EQ(m_tester->m_parseState, ParseStateArchiveInformation);

    m_tester->m_parseState = ParseStateArchiveInformation;
    m_tester->readListLine("Archive:");
    m_tester->readListLine("Details:");
    m_tester->readListLine("sfasf:");

    m_tester->m_parseState = ParseStateEntryInformation;
    m_tester->readListLine("Name: 电影歌曲.zip");
    m_tester->readListLine("        Type: File");
    m_tester->readListLine("        Type: Directory");
    m_tester->readListLine("        Size: 2066668493");
    m_tester->readListLine("       mtime: 2020-09-23 10:11:43,000000000");
    m_tester->m_parseState = ParseStateEntryInformation;
}

TEST_F(TestCliRarPlugin, testhandleLine)
{

}
