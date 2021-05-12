/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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
#include "uitools.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/
bool UiTools_isExistMimeType_stub()
{
    return true;
}

QString UiTools_readConf_stub()
{
    return "file_association.file_association_type.x-7z-compressed:true\nfile_association.file_association_type.x-archive:false";
}


/*******************************单元测试************************************/
// 测试CompressPage
class TestUiTools : public ::testing::Test
{
public:
    TestUiTools(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UiTools();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UiTools *m_tester;
};

TEST_F(TestUiTools, initTest)
{

}

TEST_F(TestUiTools, testgetConfigPath)
{
    Stub stub;
    QDirStub::stub_QDir_filePath(stub, "/a/b/c");
    ASSERT_EQ(m_tester->getConfigPath() == "/a/b/c", true);
}

TEST_F(TestUiTools, testrenderSVG)
{
    m_tester->renderSVG("/a/b/1.png", QSize(32, 32));
}

TEST_F(TestUiTools, testhumanReadableSize)
{
    ASSERT_EQ(m_tester->humanReadableSize(0, 1) == QLatin1String("-"), true);
    ASSERT_EQ(m_tester->humanReadableSize(1023, 1) == QLatin1String("1023.0 B"), true);
}

TEST_F(TestUiTools, testisArchiveFile)
{
    Stub stub;
    stub.set(ADDR(UiTools, isExistMimeType), UiTools_isExistMimeType_stub);

    ASSERT_EQ(m_tester->isArchiveFile("/a/b/1.zip"), true);
    ASSERT_EQ(m_tester->isArchiveFile("/a/b/1.crx"), true);
    ASSERT_EQ(m_tester->isArchiveFile("/a/b/1.deb"), false);
}

TEST_F(TestUiTools, testjudgeFileMime)
{
    ASSERT_EQ(m_tester->judgeFileMime("/a/b/1.7z.001") == QLatin1String("x-7z-compressed"), true);
    ASSERT_EQ(m_tester->judgeFileMime("/a/b/1..deb") == QLatin1String("vnd.debian.binary-package"), true);
}

TEST_F(TestUiTools, testisExistMimeType)
{
    Stub stub;
    stub.set(ADDR(UiTools, readConf), UiTools_readConf_stub);
    bool bArchive = false;
    ASSERT_EQ(m_tester->isExistMimeType("x-7z-compressed", bArchive), true);
    bArchive = false;
    ASSERT_EQ(m_tester->isExistMimeType("x-7z-compressedabc", bArchive), false);
    bArchive = false;
    ASSERT_EQ(m_tester->isExistMimeType("x-archive", bArchive), false);
}

TEST_F(TestUiTools, testreadConf)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileStub::stub_QFile_open(stub, true);
    QFileStub::stub_QFile_close(stub, true);
    QByteArray ba = QString("hello world").toLatin1();
    QFileStub::stub_QFile_readAll(stub, ba);

    ASSERT_EQ(m_tester->readConf() == QLatin1String("hello world"), true);
}
