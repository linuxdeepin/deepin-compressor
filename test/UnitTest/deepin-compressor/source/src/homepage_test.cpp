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
#include "homepage.h"

TEST(HomePage_HomePage_UT, HomePage_HomePage_UT001)
{
    HomePage *page = new HomePage(nullptr);
    ASSERT_NE(page, nullptr);
    delete page;
}

TEST(HomePage_setIconPixmap_UT, HomePage_setIconPixmap_UT001)
{
    HomePage *page = new HomePage(nullptr);
    page->setIconPixmap(true);
    int ret = page->m_iconLabel->pixmap()->isNull();
    ASSERT_EQ(ret, false);
    delete page;
}

TEST(HomePage_getChooseBtn_UT, HomePage_getChooseBtn_UT001)
{
    HomePage *page = new HomePage(nullptr);
    bool ret = page->getChooseBtn() == page->m_chooseBtn;
    ASSERT_EQ(ret, true);
    delete page;
}

//TEST(HomePage_onChooseBtnClicked_UT, HomePage_onChooseBtnClicked_UT001)
//{
//    HomePage *page = new HomePage(nullptr);
//    page->onChooseBtnClicked();
//    delete page;
//}
