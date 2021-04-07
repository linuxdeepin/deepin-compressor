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

#include "treeheaderview.h"
#include "gtest/src/stub.h"

#include <DMenu>

#include <gtest/gtest.h>
#include <QTest>

class TestPreviousLabel : public ::testing::Test
{
public:
    TestPreviousLabel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new PreviousLabel;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    PreviousLabel *m_tester;
};

TEST_F(TestPreviousLabel, initTest)
{

}

TEST_F(TestPreviousLabel, testsetPrePath)
{
    m_tester->setPrePath(QString());
    ASSERT_EQ(m_tester->text(), "     .. " + QObject::tr("Back to: %1").arg('/'));
}



class TestTreeHeaderView : public ::testing::Test
{
public:
    TestTreeHeaderView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new TreeHeaderView(Qt::Horizontal);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    TreeHeaderView *m_tester;
};

TEST_F(TestTreeHeaderView, initTest)
{

}

TEST_F(TestTreeHeaderView, testsizeHint)
{
    QSize size = m_tester->sizeHint();
    ASSERT_EQ(size.height(), 31);
}

TEST_F(TestTreeHeaderView, testgetpreLbl)
{
    PreviousLabel *pLbl = m_tester->getpreLbl();
    ASSERT_EQ(pLbl, m_tester->m_pPreLbl);
}

TEST_F(TestTreeHeaderView, testsetPreLblVisible_true)
{
    m_tester->setPreLblVisible(true);
    ASSERT_EQ(m_tester->height(), 76);
}

TEST_F(TestTreeHeaderView, testsetPreLblVisible_false)
{
    m_tester->setPreLblVisible(false);
    ASSERT_EQ(m_tester->height(), 38);
}
