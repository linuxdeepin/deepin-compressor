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

#include "cliproperties.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <QTextCodec>
#include <QFileInfo>
/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试CliProperties
class TestCliProperties : public ::testing::Test
{
public:
    TestCliProperties(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strPath = _SOURCEDIR;
        strPath += "/3rdparty/libzipplugin/kerfuffle_libzip.json";
        CustomMimeType mimeType;
        mimeType.m_bUnKnown = true;
        mimeType.m_strTypeName = "application/zip";
        KPluginMetaData metadata(strPath);
        m_tester = new CliProperties(nullptr, metadata, mimeType);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CliProperties *m_tester;
};

TEST_F(TestCliProperties, initTest)
{

}

TEST_F(TestCliProperties, testaddArgs)
{
    m_tester->m_compressionLevelSwitch = "ON";
    m_tester->m_passwordSwitch = QStringList() << "123";
    m_tester->m_compressionMethodSwitch["application/zip"] = "ON";
    m_tester->addArgs("1.zip", QStringList() << "1.txt", "123", false, 3, "BZip2", "AES256", 0, true, "");
    m_tester->addArgs("1.zip", QStringList() << "1.txt", "123", false, 3, "BZip2", "AES256", 0, false, "");
}

TEST_F(TestCliProperties, testcommentArgs)
{
    m_tester->m_commentSwitch = QStringList() << "ON";
    m_tester->commentArgs("1.zip", "123456789");
}

TEST_F(TestCliProperties, testdeleteArgs)
{
    QList<FileEntry> files;
    FileEntry entry;
    entry.strFullPath = "1/";
    files << entry;
    m_tester->m_passwordSwitch = QStringList() << "123";
    m_tester->deleteArgs("1.zip", files, "123456");
}

TEST_F(TestCliProperties, testextractArgs)
{
    m_tester->m_extractSwitch = QStringList() << "1/";
    m_tester->m_passwordSwitch = QStringList() << "123";
    m_tester->extractArgs("1.zip", QStringList() << "1.txt", true, "123456");
    m_tester->extractArgs("1.zip", QStringList() << "1.txt", false, "123456");
}

TEST_F(TestCliProperties, testlistArgs)
{
    m_tester->listArgs("1.zip", "123456");
}

TEST_F(TestCliProperties, testmoveArgs)
{
    QString destination;
    m_tester->m_passwordSwitch = QStringList() << "123";
    m_tester->moveArgs("1.zip", QList<FileEntry>(), destination, "");
}

TEST_F(TestCliProperties, testtestArgs)
{
    m_tester->m_testSwitch = QStringList() << "t";
    m_tester->m_passwordSwitch = QStringList() << "123";
    m_tester->testArgs("1.zip", "123");
}

TEST_F(TestCliProperties, testsubstituteCommentSwitch)
{
    m_tester->m_commentSwitch = QStringList() << "ON";
    m_tester->substituteCommentSwitch("123");
}
