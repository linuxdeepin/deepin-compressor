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

#include "mainwindow.h"

#include <gtest/gtest.h>
#include "jobs.h"
class TestMainWindow : public ::testing::Test
{
public:
    TestMainWindow() {}

public:
    virtual void SetUp()
    {
        m_tester = new MainWindow;
        m_tester->show();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MainWindow *m_tester;
};

TEST_F(TestMainWindow, initTest)
{
}

TEST_F(TestMainWindow, testInitUI)
{
    m_tester->InitUI();
    ASSERT_NE(m_tester->m_pOpenLoadingPage, nullptr);
}

TEST_F(TestMainWindow, testInitConnection)
{
    m_tester->InitUI();
    m_tester->InitConnection();
    ASSERT_NE(m_tester->m_pOpenLoadingPage, nullptr);
}

TEST_F(TestMainWindow, testinitTitleBar)
{
    m_tester->initTitleBar();
    ASSERT_NE(m_tester->m_pTitleButton, nullptr);
}

TEST_F(TestMainWindow, testcreateSettingsMenu)
{
    QMenu *pMenu = m_tester->createSettingsMenu();
    ASSERT_NE(pMenu, nullptr);
    delete pMenu;
}

TEST_F(TestMainWindow, testloadArchive)
{
    m_tester->InitUI();
    m_tester->InitConnection();

    m_tester->loadArchive("../UnitTest/regress/windows.7z.tar");

    LoadJob *temp = dynamic_cast<LoadJob *>(m_tester->m_pJob);
    ASSERT_NE(m_tester->m_pJob, nullptr);
    delete temp->m_archive;
    temp->m_archive = nullptr;
}
