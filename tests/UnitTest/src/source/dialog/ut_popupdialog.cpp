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
class UT_TipDialog : public ::testing::Test
{
public:
    UT_TipDialog(): m_tester(nullptr) {}

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

TEST_F(UT_TipDialog, initTest)
{

}

TEST_F(UT_TipDialog, test_showDialog)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);

    EXPECT_EQ(m_tester->showDialog(), 1);
    EXPECT_EQ(m_tester->m_strDesText, "");
}

TEST_F(UT_TipDialog, test_autoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
    EXPECT_EQ(m_tester->m_iLabelOldHeight, 0);
    EXPECT_EQ(m_tester->m_iDialogOldHeight, 100);
}

TEST_F(UT_TipDialog, test_changeEvent)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    pDesLbl->setObjectName("ContentLabel");
    QEvent *event = new QEvent(QEvent::FontChange);
    m_tester->changeEvent(event);
    delete event;
}



// 测试ConvertDialog
class UT_ConvertDialog : public ::testing::Test
{
public:
    UT_ConvertDialog(): m_tester(nullptr) {}

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

TEST_F(UT_ConvertDialog, initTest)
{

}

TEST_F(UT_ConvertDialog, test_showDialog_001)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);
    QStringList listValue = m_tester->showDialog();
    EXPECT_EQ(listValue.contains("true"), true);
}

TEST_F(UT_ConvertDialog, test_showDialog_002)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 0);
    QStringList listValue = m_tester->showDialog();
    EXPECT_EQ(listValue.contains("false"), true);
}

TEST_F(UT_ConvertDialog, test_autoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
    EXPECT_EQ(m_tester->m_iLabelOldHeight, 0);
    EXPECT_EQ(m_tester->m_iLabelOld1Height, 0);
    EXPECT_EQ(m_tester->m_iDialogOldHeight, 100);
}



// 测试SimpleQueryDialog
class UT_SimpleQueryDialog : public ::testing::Test
{
public:
    UT_SimpleQueryDialog(): m_tester(nullptr) {}

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

TEST_F(UT_SimpleQueryDialog, initTest)
{

}

TEST_F(UT_SimpleQueryDialog, test_showDialog)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);

    EXPECT_EQ(m_tester->showDialog(), 1);
    EXPECT_EQ(m_tester->m_strDesText, "");
}

TEST_F(UT_SimpleQueryDialog, test_autoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
    EXPECT_EQ(m_tester->m_iLabelOldHeight, 0);
    EXPECT_EQ(m_tester->m_iDialogOldHeight, 100);
}

TEST_F(UT_SimpleQueryDialog, test_changeEvent)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    pDesLbl->setObjectName("ContentLabel");
    QEvent *event = new QEvent(QEvent::FontChange);
    m_tester->changeEvent(event);
    delete event;
}



// 测试OverwriteQueryDialog
class UT_OverwriteQueryDialog : public ::testing::Test
{
public:
    UT_OverwriteQueryDialog(): m_tester(nullptr) {}

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

TEST_F(UT_OverwriteQueryDialog, initTest)
{

}

TEST_F(UT_OverwriteQueryDialog, test_showDialog_001)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 0);

    m_tester->showDialog("1/", true);
    EXPECT_EQ(m_tester->m_retType, OR_Skip);
}

TEST_F(UT_OverwriteQueryDialog, test_showDialog_002)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);
    m_tester->showDialog("1.txt", false);
    EXPECT_EQ(m_tester->m_retType, OR_Overwrite);
}

TEST_F(UT_OverwriteQueryDialog, test_showDialog_003)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, -1);
    m_tester->showDialog("1.txt", false);
    EXPECT_EQ(m_tester->m_retType, OR_Cancel);
}

TEST_F(UT_OverwriteQueryDialog, test_getDialogResult)
{
    m_tester->m_retType = OR_Skip;
    EXPECT_EQ(m_tester->getDialogResult(), OR_Skip);
}

TEST_F(UT_OverwriteQueryDialog, test_getApplyAll)
{
    m_tester->m_applyAll = true;
    EXPECT_EQ(m_tester->getApplyAll(), true);
}

TEST_F(UT_OverwriteQueryDialog, test_autoFeed)
{
    DLabel *pDesLbl1 = new DLabel(m_tester);
    DLabel *pDesLbl2 = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl1, pDesLbl2);
}



// 测试AppendDialog
class UT_AppendDialog : public ::testing::Test
{
public:
    UT_AppendDialog(): m_tester(nullptr) {}

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

TEST_F(UT_AppendDialog, initTest)
{

}

TEST_F(UT_AppendDialog, test_showDialog)
{
    Stub stub;
    CommonStub::stub_DDialog_exec(stub, 1);

    EXPECT_EQ(m_tester->showDialog(true), 1);
}

TEST_F(UT_AppendDialog, test_password)
{
    m_tester->m_strPassword = "123";
    EXPECT_EQ(m_tester->m_strPassword, "123");
}

TEST_F(UT_AppendDialog, test_autoFeed)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    m_tester->autoFeed(pDesLbl);
}

TEST_F(UT_AppendDialog, test_changeEvent)
{
    DLabel *pDesLbl = new DLabel(m_tester);
    pDesLbl->setObjectName("ContentLabel");
    QEvent *event = new QEvent(QEvent::FontChange);
    m_tester->changeEvent(event);
    delete event;
}

