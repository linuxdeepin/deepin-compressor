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
#include "compressor_fail.h"

TEST(Compressor_Fail_Compressor_Fail_UT, Compressor_Fail_Compressor_Fail_UT001)
{
    Compressor_Fail *page = new Compressor_Fail(nullptr);
    ASSERT_NE(page, nullptr);
    delete page;
}

TEST(Compressor_Fail_setFailStr_UT, Compressor_Fail_setFailStr_UT001)
{
    Compressor_Fail *page = new Compressor_Fail(nullptr);
    page->setFailStr("test");
    int ret = page->m_stringinfolabel->text() == "test";
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Fail_setFailStrDetail_UT, Compressor_Fail_setFailStrDetail_UT001)
{
    Compressor_Fail *page = new Compressor_Fail(nullptr);
    page->setFailStrDetail("test");
    int ret = page->m_stringdetaillabel->text() == "test";
    ASSERT_EQ(ret, true);
    delete page;
}

TEST(Compressor_Fail_getRetrybutton_UT, Compressor_Fail_getRetrybutton_UT001)
{
    Compressor_Fail *page = new Compressor_Fail(nullptr);
    int ret = page->getRetrybutton() == page->m_retrybutton;
    ASSERT_EQ(ret, true);
    delete page;
}
