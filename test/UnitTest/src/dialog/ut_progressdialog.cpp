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

#include "progressdialog.h"
#include "popupdialog.h"

#include <DFontSizeManager>

#include <QBoxLayout>
#include <QDebug>
#include <QFileInfo>
#include <QTimer>

#include <gtest/gtest.h>
#include <QTest>

class TestProgressDialog : public ::testing::Test
{
public:
    TestProgressDialog() {}

public:
    virtual void SetUp()
    {
        m_tester = new ProgressDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ProgressDialog *m_tester;
};

TEST_F(TestProgressDialog, initTest)
{

}

TEST_F(TestProgressDialog, initUI)
{
    m_tester->initUI();
}

TEST_F(TestProgressDialog, initConnect)
{
    m_tester->initConnect();
}

TEST_F(TestProgressDialog, setCurrentTask)
{
    m_tester->setCurrentTask("/home/chenglu/Desktop/ut/1.zip");
    EXPECT_EQ(m_tester->m_tasklable->text().endsWith("1.zip"), true);
}

TEST_F(TestProgressDialog, setCurrentFile)
{
    m_tester->setCurrentFile("1");
    EXPECT_EQ(m_tester->m_filelable->text().endsWith("1"), true);
}

TEST_F(TestProgressDialog, setProcess1)
{
    m_tester->m_dPerent = 10;
    m_tester->setProcess(10);
}

TEST_F(TestProgressDialog, setProcess2)
{
    m_tester->m_dPerent = 10;
    m_tester->setProcess(20);
    EXPECT_EQ(m_tester->m_circleprogress->value(), 20);
}

TEST_F(TestProgressDialog, setFinished)
{
    m_tester->setFinished();
}

TEST_F(TestProgressDialog, clearprocess)
{
    m_tester->clearprocess();
    EXPECT_EQ(m_tester->m_dPerent, 0);
    EXPECT_EQ(m_tester->m_circleprogress->value(), 0);
}

TEST_F(TestProgressDialog, closeEvent)
{

}

TEST_F(TestProgressDialog, slotextractpress1)
{
//    m_tester->slotextractpress(0);
}

TEST_F(TestProgressDialog, slotextractpress2)
{
    m_tester->slotextractpress(1);
}

class TestCommentProgressDialog : public ::testing::Test
{
public:
    TestCommentProgressDialog() {}

public:
    virtual void SetUp()
    {
        m_tester = new CommentProgressDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CommentProgressDialog *m_tester;
};

TEST_F(TestCommentProgressDialog, initTest)
{

}

TEST_F(TestCommentProgressDialog, initUI)
{
    m_tester->initUI();
}

TEST_F(TestCommentProgressDialog, showdialog)
{

}

TEST_F(TestCommentProgressDialog, setProgress)
{
    m_tester->setProgress(10);
    EXPECT_EQ(m_tester->m_progressBar->value(), 10);
}

TEST_F(TestCommentProgressDialog, setFinished)
{
    m_tester->setFinished();
    EXPECT_EQ(m_tester->m_progressBar->value(), 100);
}
