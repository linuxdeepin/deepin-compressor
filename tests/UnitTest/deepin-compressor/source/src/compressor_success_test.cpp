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
#include "compressor_success.h"

TEST(Compressor_Success_Compressor_Success_UT, Compressor_Success_Compressor_Success_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    ASSERT_NE(page, nullptr);
    delete page;
}

TEST(Compressor_Success_setstringinfo_UT, Compressor_Success_setstringinfo_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    page->setstringinfo("test");
    int ret = page->m_stringinfolabel->text() == QLatin1String("test");
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Success_getPath_UT, Compressor_Success_getPath_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    page->setCompressPath("test");
    int ret = page->getPath() == "test";
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Success_setCompressFullPath_UT, Compressor_Success_setCompressFullPath_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    page->setCompressFullPath("test");
    int ret = page->m_fullpath == "test";
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Success_setCompressNewFullPath_UT, Compressor_Success_setCompressNewFullPath_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    page->setCompressNewFullPath("test");
    int ret = page->newCreatePath_ == "test";
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Success_clear_UT, Compressor_Success_clear_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    page->clear();
    int ret = ((page->m_fullpath == "") && (page->newCreatePath_ == ""));
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Success_setConvertType_UT, Compressor_Success_setConvertType_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    page->setConvertType("test");
    int ret = page->m_convertType == "test";
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Success_getShowfilebutton_UT, Compressor_Success_getShowfilebutton_UT001)
{
    Compressor_Success *page = new Compressor_Success(nullptr);
    //int ret = page->getShowfilebutton() == page->m_showfilebutton;
    // ASSERT_EQ(ret, true);
    delete page;
}
