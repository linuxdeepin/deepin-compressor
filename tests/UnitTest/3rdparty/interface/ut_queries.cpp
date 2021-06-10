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

#include "queries.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <DLabel>

#include <QUrl>

int g_Dialog_exec_result1 = 0;
/*******************************函数打桩************************************/
QUrl qUrl_fromLocalFile_stub(const QString &)
{
    return QUrl("file://file:dsfdsfsdf.txt");
}

int dialog_exec_stub()
{
    return g_Dialog_exec_result1;
}
/*******************************函数打桩************************************/
// 测试CustomDDialog
class TestCustomDDialog : public ::testing::Test
{
public:
    TestCustomDDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomDDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomDDialog *m_tester;
};

TEST_F(TestCustomDDialog, initTest)
{

}


// 测试OverwriteQuery
class TestOverwriteQuery : public ::testing::Test
{
public:
    TestOverwriteQuery(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OverwriteQuery("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OverwriteQuery *m_tester;
};

TEST_F(TestOverwriteQuery, initTest)
{

}

TEST_F(TestOverwriteQuery, testexecute)
{
    Stub stub;
    stub.set(ADDR(QUrl, fromLocalFile), qUrl_fromLocalFile_stub);

    g_Dialog_exec_result1 = -1;
    Stub stub1;
    typedef int (*fptr)(DDialog *);
    fptr A_foo1 = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub1.set(A_foo1, dialog_exec_stub);
    m_tester->execute();

    g_Dialog_exec_result1 = 0;
    Stub stub2;
    typedef int (*fptr)(DDialog *);
    fptr A_foo2 = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub2.set(A_foo2, dialog_exec_stub);
    m_tester->execute();

    g_Dialog_exec_result1 = 1;
    Stub stub3;
    typedef int (*fptr)(DDialog *);
    fptr A_foo3 = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub3.set(A_foo3, dialog_exec_stub);
    m_tester->execute();
}

TEST_F(TestOverwriteQuery, testresponseCancelled)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Cancel;
    ASSERT_EQ(m_tester->responseCancelled(), true);
}

TEST_F(TestOverwriteQuery, testresponseSkip)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Skip;
    ASSERT_EQ(m_tester->responseSkip(), true);
}

TEST_F(TestOverwriteQuery, testresponseSkipAll)
{
    m_tester->m_data[QStringLiteral("response")] = Result_SkipAll;
    ASSERT_EQ(m_tester->responseSkipAll(), true);
}

TEST_F(TestOverwriteQuery, testresponseOverwrite)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Overwrite;
    ASSERT_EQ(m_tester->responseOverwrite(), true);
}

TEST_F(TestOverwriteQuery, testresponseOverwriteAll)
{
    m_tester->m_data[QStringLiteral("response")] = Result_OverwriteAll;
    ASSERT_EQ(m_tester->responseOverwriteAll(), true);
}

TEST_F(TestOverwriteQuery, testautoFeed)
{
    DLabel *label1 = new DLabel();
    DLabel *label2 = new DLabel();
    CustomDDialog *dialog = new CustomDDialog();

    m_tester->autoFeed(label1, label2, dialog);

    delete label1;
    delete label2;
    delete dialog;
}

TEST_F(TestOverwriteQuery, testsetWidgetColor)
{
    QWidget *pWgt = new QWidget;
    DPalette::ColorRole ct = DPalette::ToolTipText;
    double alphaF = 0.5;

    m_tester->setWidgetColor(pWgt, ct, alphaF);
    delete pWgt;
}

TEST_F(TestOverwriteQuery, testsetWidgetType)
{
    QWidget *pWgt = new QWidget;
    DPalette::ColorType ct;
    double alphaF = 0.5;

    m_tester->setWidgetType(pWgt, ct, alphaF);
    delete pWgt;
}


// 测试PasswordNeededQuery
class TestPasswordNeededQuery : public ::testing::Test
{
public:
    TestPasswordNeededQuery(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new PasswordNeededQuery("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    PasswordNeededQuery *m_tester;
};

TEST_F(TestPasswordNeededQuery, initTest)
{

}

TEST_F(TestPasswordNeededQuery, testexecute)
{
    g_Dialog_exec_result1 = -1;
    Stub stub1;
    typedef int (*fptr)(DDialog *);
    fptr A_foo1 = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub1.set(A_foo1, dialog_exec_stub);
    m_tester->execute();

    g_Dialog_exec_result1 = 1;
    Stub stub2;
    typedef int (*fptr)(DDialog *);
    fptr A_foo2 = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub2.set(A_foo2, dialog_exec_stub);
    m_tester->execute();
}

TEST_F(TestPasswordNeededQuery, testautoFeed)
{
    DLabel *label1 = new DLabel();
    DLabel *label2 = new DLabel();
    CustomDDialog *dialog = new CustomDDialog();

    m_tester->autoFeed(label1, label2, dialog);

    delete label1;
    delete label2;
    delete dialog;
}

TEST_F(TestPasswordNeededQuery, testresponseCancelled)
{
    m_tester->m_data[QStringLiteral("response")] = false;
    ASSERT_EQ(m_tester->responseCancelled(), true);
}

TEST_F(TestPasswordNeededQuery, testpassword)
{
    m_tester->m_data[QStringLiteral("password")] = "123";
    ASSERT_EQ(m_tester->password(), "123");
}


// 测试PasswordNeededQuery
class TestLoadCorruptQuery : public ::testing::Test
{
public:
    TestLoadCorruptQuery(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LoadCorruptQuery("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LoadCorruptQuery *m_tester;
};

TEST_F(TestLoadCorruptQuery, initTest)
{

}

TEST_F(TestLoadCorruptQuery, testexecute)
{
    g_Dialog_exec_result1 = -1;
    Stub stub1;
    typedef int (*fptr)(DDialog *);
    fptr A_foo1 = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub1.set(A_foo1, dialog_exec_stub);
    m_tester->execute();

    g_Dialog_exec_result1 = 0;
    Stub stub2;
    typedef int (*fptr)(DDialog *);
    fptr A_foo2 = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub2.set(A_foo2, dialog_exec_stub);
    m_tester->execute();
}

TEST_F(TestLoadCorruptQuery, testresponseYes)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Readonly;
    ASSERT_EQ(m_tester->responseYes(), true);
}

TEST_F(TestLoadCorruptQuery, testautoFeed)
{
    DLabel *label = new DLabel();
    CustomDDialog *dialog = new CustomDDialog();

    m_tester->autoFeed(label, dialog);

    delete label;
    delete dialog;
}
