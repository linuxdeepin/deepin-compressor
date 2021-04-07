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

#include "popupdialog.h"
#include "uitools.h"
#include "gtest/src/stub.h"


#include <gtest/gtest.h>
#include <QTest>


int exec_stub()
{
    return -1;
}


class TestTipDialog : public ::testing::Test
{
public:
    TestTipDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new TipDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    TipDialog *m_tester;
};

TEST_F(TestTipDialog, initTest)
{

}

TEST_F(TestTipDialog, testshowDialog)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, exec_stub);

    ASSERT_EQ(m_tester->showDialog(), -1);
}

TEST_F(TestTipDialog, testautoFeed)
{
    DLabel *label = new DLabel(m_tester);
    m_tester->autoFeed(label);
    ASSERT_EQ(m_tester->m_iDialogOldHeight, m_tester->height());
}

class TestSimpleQueryDialog : public ::testing::Test
{
public:
    TestSimpleQueryDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SimpleQueryDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SimpleQueryDialog *m_tester;
};

TEST_F(TestSimpleQueryDialog, initTest)
{

}

TEST_F(TestSimpleQueryDialog, testshowDialog)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, exec_stub);

    ASSERT_EQ(m_tester->showDialog(), -1);
}

TEST_F(TestSimpleQueryDialog, testautoFeed)
{
    DLabel *label = new DLabel(m_tester);
    m_tester->autoFeed(label);
    ASSERT_EQ(m_tester->m_iDialogOldHeight, m_tester->height());
}


class TestOverwriteQueryDialog : public ::testing::Test
{
public:
    TestOverwriteQueryDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OverwriteQueryDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OverwriteQueryDialog *m_tester;
};

TEST_F(TestOverwriteQueryDialog, initTest)
{

}

TEST_F(TestOverwriteQueryDialog, testshowDialog)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, exec_stub);
    m_tester->showDialog("sss");
    ASSERT_EQ(m_tester->m_retType, OR_Cancel);
}

TEST_F(TestOverwriteQueryDialog, testautoFeed)
{
    DLabel *label = new DLabel(m_tester);
    DLabel *label2 = new DLabel(m_tester);
    m_tester->autoFeed(label, label2);
    ASSERT_EQ(m_tester->m_iDialogOldHeight, m_tester->height());
}

TEST_F(TestOverwriteQueryDialog, getDialogResult)
{
    m_tester->m_ret = -1;
    EXPECT_EQ(m_tester->getDialogResult(), - 1);
}

TEST_F(TestOverwriteQueryDialog, getQueryResult)
{
    m_tester->m_retType = OR_Cancel;
    EXPECT_EQ(m_tester->getQueryResult(), OR_Cancel);
}

TEST_F(TestOverwriteQueryDialog, getApplyAll)
{
    m_tester->m_applyAll = false;
    EXPECT_EQ(m_tester->getApplyAll(), false);
}

class TestConvertDialog : public ::testing::Test
{
public:
    TestConvertDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ConvertDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ConvertDialog *m_tester;
};

TEST_F(TestConvertDialog, initTest)
{

}

TEST_F(TestConvertDialog, testshowDialog)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, exec_stub);
    QStringList list;
    list  << "false" << "" << "false" << "none";
    bool b = (list == m_tester->showDialog());
    ASSERT_EQ(b, true);
}

TEST_F(TestConvertDialog, testautoFeed)
{
    DLabel *label = new DLabel(m_tester);
    m_tester->autoFeed(label);
    ASSERT_EQ(m_tester->m_iDialogOldHeight, m_tester->height());
}


class TestAppendDialog : public ::testing::Test
{
public:
    TestAppendDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new AppendDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    AppendDialog *m_tester;
};

TEST_F(TestAppendDialog, initTest)
{

}

TEST_F(TestAppendDialog, testshowDialog)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, exec_stub);
    m_tester->showDialog(false);
    ASSERT_EQ(m_tester->m_strPassword.isEmpty(), true);
}

TEST_F(TestAppendDialog, testautoFeed)
{
    DLabel *label = new DLabel(m_tester);
    m_tester->autoFeed(label);
    ASSERT_EQ(m_tester->m_iDialogOldHeight, m_tester->height());
}

TEST_F(TestAppendDialog, testpassword)
{
    m_tester->m_strPassword = "123";
    ASSERT_EQ(m_tester->password(), "123");
}

