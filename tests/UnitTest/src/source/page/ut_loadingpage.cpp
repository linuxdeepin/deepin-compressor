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

#include "loadingpage.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

/*******************************函数打桩************************************/
// 对DSpinner的start进行打桩
void dSpinner_start_stub()
{
    return;
}

// 对DSpinner的start进行打桩
void dSpinner_stop_stub()
{
    return;
}
/*******************************函数打桩************************************/

class UT_LoadingPage : public ::testing::Test
{
public:
    UT_LoadingPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LoadingPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LoadingPage *m_tester;
};

TEST_F(UT_LoadingPage, initTest)
{

}

TEST_F(UT_LoadingPage, test_startLoading)
{
    Stub stub;
    stub.set(ADDR(DSpinner, start), dSpinner_start_stub);
    m_tester->startLoading();
}

TEST_F(UT_LoadingPage, test_stopLoading)
{
    Stub stub;
    stub.set(ADDR(DSpinner, stop), dSpinner_stop_stub);
    m_tester->stopLoading();
}

TEST_F(UT_LoadingPage, test_setDes)
{
    m_tester->setDes("123");
    EXPECT_EQ(m_tester->m_pTextLbl->text(), "123");
}
