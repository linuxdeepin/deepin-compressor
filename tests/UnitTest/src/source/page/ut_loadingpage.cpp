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

#include "loadingpage.h"

#include <gtest/gtest.h>
#include <QTest>

class TestLoadingPage : public ::testing::Test
{
public:
    TestLoadingPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LoadingPage;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LoadingPage *m_tester;
};

TEST_F(TestLoadingPage, initTest)
{

}

TEST_F(TestLoadingPage, startLoading)
{
    m_tester->startLoading();
}

TEST_F(TestLoadingPage, stopLoading)
{
    m_tester->stopLoading();
}

TEST_F(TestLoadingPage, setDes)
{
    m_tester->setDes("加载中，请稍等...");
}

TEST_F(TestLoadingPage, initUI)
{

}
