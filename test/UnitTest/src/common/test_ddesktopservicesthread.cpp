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

#include "ddesktopservicesthread.h"

#include <gtest/gtest.h>
#include <QTest>

class TestDDesktopServicesThread : public ::testing::Test
{
public:
    TestDDesktopServicesThread() {}

public:
    virtual void SetUp()
    {
        m_tester = new DDesktopServicesThread;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DDesktopServicesThread *m_tester;
};


TEST_F(TestDDesktopServicesThread, initTest)
{

}


TEST_F(TestDDesktopServicesThread, testsetOpenFiles)
{
    m_tester->setOpenFiles(QStringList() << "5555");
}

TEST_F(TestDDesktopServicesThread, testhasFiles)
{
//    QTest::keyPress(m_tester, Qt::Key_Delete);
//    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestDDesktopServicesThread, testrun)
{
//    QTest::keyPress(m_tester, Qt::Key_Delete);
//    ASSERT_NE(m_tester, nullptr);
}
