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

#include <gtest/gtest.h>
#include <gtest/src/stub.h>

#include "mimetypes.h"

#include <QFileInfo>


TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT001)
{
    ASSERT_EQ(determineMimeType("asdf").name(), "application/octet-stream");
}

bool isReadable_f_stub()
{
    return false;
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT002)
{
    Stub stub;
    stub.set(ADDR(QFileInfo, isReadable), isReadable_f_stub);
    ASSERT_EQ(determineMimeType("test.tar.bz2").name(), "application/x-bzip-compressed-tar");
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT003)
{
    ASSERT_EQ(determineMimeType("test.tar.lz4").name(), "application/x-lz4-compressed-tar");
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT004)
{
    ASSERT_EQ(determineMimeType("test.tar.7z").name(), "application/x-7z-compressed");
}

bool isReadable_stub()
{
    return true;
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT005)
{
    ASSERT_EQ(determineMimeType("../UnitTest/test_sources/bz2/test.tar.bz2").name(), "application/x-bzip-compressed-tar");
}

bool isDefault_stub()
{
    return false;
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT006)
{
    ASSERT_EQ(determineMimeType("../UnitTest/test_sources/iso/test.iso").name(), "application/x-iso9660-image");
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT007)
{
    Stub stub;
    stub.set(ADDR(QFileInfo, isReadable), isReadable_stub);
    stub.set(ADDR(QMimeType, isDefault), isDefault_stub);

    ASSERT_EQ(determineMimeType("test.tar.lz4").name(), "application/octet-stream");
}
