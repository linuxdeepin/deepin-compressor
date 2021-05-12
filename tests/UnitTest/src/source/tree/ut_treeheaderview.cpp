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

#include <QTest>

#include <gtest/gtest.h>

/*******************************函数打桩************************************/
// 对QHeaderView的sectionSizeFromContents进行打桩
QSize qHeaderView_sectionSizeFromContents_stub(int logicalIndex)
{
    return QSize(30, 30);
}
/*******************************函数打桩************************************/


class TestPreviousLabel : public ::testing::Test
{
public:
    TestPreviousLabel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new PreviousLabel;
        m_tester->disconnect();
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
    m_tester->setPrePath("123");
}

TEST_F(TestPreviousLabel, testmouseDoubleClickEvent)
{
    QTest::mouseDClick(m_tester, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint());
}

TEST_F(TestPreviousLabel, testkeyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Tab);
}



class TestTreeHeaderView : public ::testing::Test
{
public:
    TestTreeHeaderView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new TreeHeaderView(Qt::Horizontal);
        m_tester->disconnect();
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
    typedef QSize(*fptr)(QHeaderView *, int);
    fptr A_foo = (fptr)(&QHeaderView::sectionSizeFromContents);   // 获取虚函数地址
    Stub stub;
    stub.set(A_foo, qHeaderView_sectionSizeFromContents_stub);

    ASSERT_EQ(m_tester->sizeHint().width(), 30);
}

TEST_F(TestTreeHeaderView, testgetpreLbl)
{
    ASSERT_EQ(m_tester->getpreLbl(), m_tester->m_pPreLbl);
}

TEST_F(TestTreeHeaderView, testsetPreLblVisible)
{
    m_tester->setPreLblVisible(true);
    ASSERT_EQ(m_tester->height(), 76);
    m_tester->setPreLblVisible(false);
    ASSERT_EQ(m_tester->height(), 38);
}
