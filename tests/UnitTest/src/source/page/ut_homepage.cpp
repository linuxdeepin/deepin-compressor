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

#include "homepage.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <QTest>

DGUI_USE_NAMESPACE

class TestHomePage : public ::testing::Test
{
public:
    TestHomePage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new HomePage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    HomePage *m_tester;
};

TEST_F(TestHomePage, initTest)
{

}

TEST_F(TestHomePage, dragEnterEvent)
{

}

TEST_F(TestHomePage, dragMoveEvent)
{

}

TEST_F(TestHomePage, dropEvent)
{

}

TEST_F(TestHomePage, slotThemeChanged_LightType)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::LightType);
    m_tester->slotThemeChanged();
}

TEST_F(TestHomePage, slotThemeChanged_DarkType)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::DarkType);
    m_tester->slotThemeChanged();
}

TEST_F(TestHomePage, slotThemeChanged_UnknownType)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::UnknownType);
    m_tester->slotThemeChanged();
}
