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

TEST_F(TestUiTools, testtoShortString)
{
    QString str = "123456789123465789";
    ASSERT_EQ(m_tester->toShortString(str, 4, 4), "12...89");
}

TEST_F(TestUiTools, testhandleFileName)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1.tar");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "1.tar.lz");
    ASSERT_EQ(m_tester->handleFileName("1.tar.lz"), "1");


    Stub stub1;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub1, "1.7z");
    QFileInfoStub::stub_QFileInfo_filePath(stub1, "1.7z.001");
    ASSERT_EQ(m_tester->handleFileName("1.7z.001"), "1");


    Stub stub2;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub2, "1.part01");
    QFileInfoStub::stub_QFileInfo_filePath(stub2, "1.part01.rar");
    ASSERT_EQ(m_tester->handleFileName("1.part01.rar"), "1");


    Stub stub3;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub3, "1.part1");
    QFileInfoStub::stub_QFileInfo_filePath(stub3, "1.part1.rar");
    ASSERT_EQ(m_tester->handleFileName("1.part1.rar"), "1");


    Stub stub4;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub4, "1.zip");
    QFileInfoStub::stub_QFileInfo_filePath(stub4, "1.zip.001");
    ASSERT_EQ(m_tester->handleFileName("1.zip.001"), "1");
}

TEST_F(TestUiTools, testisLocalDeviceFile)
{
    m_tester->isLocalDeviceFile("1.txt");
}

TEST_F(TestUiTools, testremoveSameFileName)
{
    QStringList listFiles = QStringList() << "1.txt" << "1.txt";
    QStringList listResult = m_tester->removeSameFileName(listFiles);
    bool bResult = (listResult.count() == 1 && listResult[0] == "1.txt");
    ASSERT_EQ(bResult, true);
}

TEST_F(TestUiTools, testtransSplitFileName)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);

    QString str = "1.7z.001";
    UnCompressParameter::SplitType type;
    m_tester->transSplitFileName(str, type);
    ASSERT_EQ(type, UnCompressParameter::ST_Other);

    str = "1.rar.001";
    m_tester->transSplitFileName(str, type);
    ASSERT_EQ(type, UnCompressParameter::ST_Other);

    str = "1.zip.001";
    m_tester->transSplitFileName(str, type);
    ASSERT_EQ(type, UnCompressParameter::ST_Zip);

    str = "1.z01";
    m_tester->transSplitFileName(str, type);
    ASSERT_EQ(type, UnCompressParameter::ST_Zip);
}
