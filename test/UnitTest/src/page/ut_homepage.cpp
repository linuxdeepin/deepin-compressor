/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenglu <chenglu@uniontech.com>
*
* Maintainer: chenglu <chenglu@uniontech.com>
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
#include "customwidget.h"
#include "uitools.h"

#include <DFileDialog>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QSettings>
#include <QSettings>
#include <QShortcut>
#include <QDragEnterEvent>
#include <QMimeData>

#include <gtest/gtest.h>
#include <QTest>

class TestHomePage : public ::testing::Test
{
public:
    TestHomePage() {}

public:
    virtual void SetUp()
    {
        m_tester = new HomePage;
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

TEST_F(TestHomePage, initUI)
{
    m_tester->initUI();
}

TEST_F(TestHomePage, initConnections)
{
    m_tester->initConnections();
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

TEST_F(TestHomePage, slotThemeChanged)
{
    m_tester->slotThemeChanged();
}

