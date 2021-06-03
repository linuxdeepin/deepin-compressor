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
#include "progressdialog.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试ProgressDialog
class TestProgressDialog : public ::testing::Test
{
public:
    TestProgressDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProgressDialog;
        m_tester->disconnect();
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

TEST_F(TestProgressDialog, testsetCurrentTask)
{
    m_tester->setCurrentTask("1.txt");
    ASSERT_EQ(m_tester->m_tasklable->text(), QString("Task:1.txt"));
}

TEST_F(TestProgressDialog, testsetCurrentFile)
{
    m_tester->setCurrentFile("1.txt");
}

TEST_F(TestProgressDialog, testsetProcess)
{
    m_tester->m_dPerent = 0;
    m_tester->setProcess(0);
    ASSERT_EQ(m_tester->m_dPerent, 0);


    m_tester->setProcess(10);
    ASSERT_EQ(m_tester->m_dPerent, 10);
}

TEST_F(TestProgressDialog, testsetFinished)
{
    m_tester->m_dPerent = 100;
    m_tester->setFinished();
    ASSERT_EQ(m_tester->m_dPerent, 0);
}

TEST_F(TestProgressDialog, testclearprocess)
{
    m_tester->m_dPerent = 100;
    m_tester->m_circleprogress->setValue(100);
    m_tester->clearprocess();
    ASSERT_EQ(m_tester->m_circleprogress->value(), 0);
}

TEST_F(TestProgressDialog, testshowDialog)
{
    Stub stub;
    CommonStub::stub_QDialog_open(stub);

    m_tester->showDialog();
}

TEST_F(TestProgressDialog, testslotextractpress)
{
    Stub stub;
    CommonStub::stub_QDialog_exec(stub, -1);

    m_tester->slotextractpress(1);
    m_tester->slotextractpress(0);
}



// 测试CommentProgressDialog
class TestCommentProgressDialog : public ::testing::Test
{
public:
    TestCommentProgressDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CommentProgressDialog;
        m_tester->disconnect();
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

TEST_F(TestCommentProgressDialog, testshowdialog)
{
    Stub stub;
    CommonStub::stub_QDialog_exec(stub, -1);

    m_tester->showdialog();
}

TEST_F(TestCommentProgressDialog, testsetProgress)
{
    m_tester->m_progressBar->setValue(0);
    m_tester->setProgress(10);
    ASSERT_EQ(m_tester->m_progressBar->value(), 10);
}

TEST_F(TestCommentProgressDialog, testsetFinished)
{
    m_tester->m_progressBar->setValue(10);
    m_tester->setFinished();
    ASSERT_EQ(m_tester->m_progressBar->value(), 100);
}
