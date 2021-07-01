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

#include "archiveformat.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <QTextCodec>
#include <QFileInfo>
/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试ArchiveFormat
class TestArchiveFormat : public ::testing::Test
{
public:
    TestArchiveFormat() {}

public:
    virtual void SetUp()
    {
        QString strPath = _SOURCEDIR;
        strPath += "/3rdparty/libzipplugin/kerfuffle_libzip.json";
        CustomMimeType mimeType;
        mimeType.m_bUnKnown = true;
        mimeType.m_strTypeName = "application/zip";
        KPluginMetaData metadata(strPath);
        m_tester = ArchiveFormat::fromMetadata(mimeType, metadata);
    }

    virtual void TearDown()
    {

    }

protected:
    ArchiveFormat m_tester;
};

TEST_F(TestArchiveFormat, initTest)
{

}

TEST_F(TestArchiveFormat, testencryptionType)
{
    m_tester.encryptionType();
}

TEST_F(TestArchiveFormat, testminCompressionLevel)
{
    m_tester.minCompressionLevel();
}

TEST_F(TestArchiveFormat, testmaxCompressionLevel)
{
    m_tester.maxCompressionLevel();
}

TEST_F(TestArchiveFormat, testdefaultCompressionLevel)
{
    m_tester.maxCompressionLevel();
}

TEST_F(TestArchiveFormat, testsupportsWriteComment)
{
    m_tester.maxCompressionLevel();
}

TEST_F(TestArchiveFormat, testsupportsTesting)
{
    m_tester.supportsTesting();
}

TEST_F(TestArchiveFormat, testsupportsMultiVolume)
{
    m_tester.supportsMultiVolume();
}

TEST_F(TestArchiveFormat, testcompressionMethods)
{
    m_tester.compressionMethods();
}

TEST_F(TestArchiveFormat, testdefaultCompressionMethod)
{
    m_tester.defaultCompressionMethod();
}

TEST_F(TestArchiveFormat, testencryptionMethods)
{
    m_tester.encryptionMethods();
}

TEST_F(TestArchiveFormat, testdefaultEncryptionMethod)
{
    m_tester.defaultEncryptionMethod();
}


