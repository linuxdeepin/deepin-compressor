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
#include "popupdialog.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/
/*******************************单元测试************************************/
// 测试TipDialog
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
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);

    ASSERT_EQ(m_tester->showDialog(), 1);
}

TEST_F(TestTipDialog, testautoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
}

TEST_F(TestTipDialog, testchangeEvent)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    pDesLbl->setObjectName("ContentLabel");
    QEvent *event = new QEvent(QEvent::FontChange);
    m_tester->changeEvent(event);
    delete event;
}



// 测试ConvertDialog
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
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);
    QStringList listValue = m_tester->showDialog();
    ASSERT_EQ(listValue.contains("true"), true);

    Stub stub1;
    CommonStub::stub_DDialog_exec(stub1, 0);
    listValue = m_tester->showDialog();
    ASSERT_EQ(listValue.contains("false"), true);
}

TEST_F(TestConvertDialog, testautoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
}



// 测试SimpleQueryDialog
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
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);

    ASSERT_EQ(m_tester->showDialog(), 1);
}

TEST_F(TestSimpleQueryDialog, testautoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
}

TEST_F(TestSimpleQueryDialog, testchangeEvent)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    pDesLbl->setObjectName("ContentLabel");
    QEvent *event = new QEvent(QEvent::FontChange);
    m_tester->changeEvent(event);
    delete event;
}



// 测试OverwriteQueryDialog
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
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 0);

    m_tester->showDialog("1/", true);
    ASSERT_EQ(m_tester->m_retType, OR_Skip);

    Stub stub1;
    CommonStub::stub_DDialog_exec(stub1, 1);
    m_tester->showDialog("1.txt", false);
    ASSERT_EQ(m_tester->m_retType, OR_Overwrite);

    Stub stub2;
    CommonStub::stub_DDialog_exec(stub2, -1);
    m_tester->showDialog("1.txt", false);
    qInfo() << m_tester->m_retType;
    ASSERT_EQ(m_tester->m_retType, OR_Cancel);
}

TEST_F(TestOverwriteQueryDialog, testgetDialogResult)
{
    m_tester->m_retType = OR_Skip;
    ASSERT_EQ(m_tester->getDialogResult(), OR_Skip);
}

TEST_F(TestOverwriteQueryDialog, testgetApplyAll)
{
    m_tester->m_applyAll = true;
    ASSERT_EQ(m_tester->getApplyAll(), true);
}

TEST_F(TestOverwriteQueryDialog, testautoFeed)
{
    DLabel *pDesLbl1 = new DLabel(m_tester);
    DLabel *pDesLbl2 = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl1, pDesLbl2);
}



// 测试AppendDialog
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
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);

    ASSERT_EQ(m_tester->showDialog(true), 1);
}

TEST_F(TestAppendDialog, testpassword)
{
    m_tester->m_strPassword = "123";
    ASSERT_EQ(m_tester->m_strPassword, "123");
}

TEST_F(TestAppendDialog, testautoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
}

TEST_F(TestAppendDialog, testchangeEvent)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    pDesLbl->setObjectName("ContentLabel");
    QEvent *event = new QEvent(QEvent::FontChange);
    m_tester->changeEvent(event);
    delete event;
}

