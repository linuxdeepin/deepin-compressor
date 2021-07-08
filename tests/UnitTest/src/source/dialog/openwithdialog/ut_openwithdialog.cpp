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
#include "openwithdialog.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QDir>
#include <QResizeEvent>
#include <QStandardPaths>

/*******************************函数打桩************************************/
/*******************************单元测试************************************/
// 测试OpenWithDialogListItem
class TestOpenWithDialogListItem : public ::testing::Test
{
public:
    TestOpenWithDialogListItem(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenWithDialogListItem(QIcon(), "");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenWithDialogListItem *m_tester;
};

TEST_F(TestOpenWithDialogListItem, initTest)
{

}

TEST_F(TestOpenWithDialogListItem, testsetChecked)
{
    m_tester->setChecked(true);
    m_tester->setChecked(false);
}

TEST_F(TestOpenWithDialogListItem, testtext)
{
    m_tester->m_pTextLbl->setText("123");
    ASSERT_EQ(m_tester->text(), "123");
}

TEST_F(TestOpenWithDialogListItem, testresizeEvent)
{
    QResizeEvent *e = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(e);
    delete e;
}

TEST_F(TestOpenWithDialogListItem, testenterEvent)
{
    QEvent *e = new QEvent(QEvent::Enter);
    m_tester->enterEvent(e);
    delete e;
}

TEST_F(TestOpenWithDialogListItem, testleaveEvent)
{
    QEvent *e = new QEvent(QEvent::Leave);
    m_tester->leaveEvent(e);
    delete e;
}


// 测试OpenWithDialogListSparerItem
class TestOpenWithDialogListSparerItem : public ::testing::Test
{
public:
    TestOpenWithDialogListSparerItem(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenWithDialogListSparerItem("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenWithDialogListSparerItem *m_tester;
};

TEST_F(TestOpenWithDialogListSparerItem, initTest)
{

}


// 测试OpenWithDialog
class TestOpenWithDialog : public ::testing::Test
{
public:
    TestOpenWithDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenWithDialog("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenWithDialog *m_tester;
};

TEST_F(TestOpenWithDialog, initTest)
{

}

TEST_F(TestOpenWithDialog, testgetOpenStyle)
{
    m_tester->getOpenStyle("1.zip");
}

TEST_F(TestOpenWithDialog, testresizeEvent)
{
    QResizeEvent *e = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(e);
    delete e;
}

TEST_F(TestOpenWithDialog, testeventFilter)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem(QIcon(), "", m_tester);

    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->eventFilter(item, event);
    delete event;

    event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->eventFilter(item, event);
    delete event;
}

TEST_F(TestOpenWithDialog, testopenWithProgram)
{
    Stub stub;
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->openWithProgram("1.zip");
}

TEST_F(TestOpenWithDialog, testshowOpenWithDialog)
{
    m_tester->m_pCheckedItem = new OpenWithDialogListItem(QIcon(), "", m_tester);
    Stub stub;
    CommonStub::stub_DAbstractDialog_exec(stub, 1);
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->m_bOk = false;
    m_tester->showOpenWithDialog(OpenWithDialog::ShowType::OpenType);
    m_tester->m_bOk = true;
    m_tester->m_pSetToDefaultCheckBox->setChecked(true);
    m_tester->showOpenWithDialog(OpenWithDialog::ShowType::OpenType);
}

TEST_F(TestOpenWithDialog, testgetProgramPathByExec)
{
    m_tester->getProgramPathByExec("");
    m_tester->getProgramPathByExec("/usr/bin/deepin-compressor");
    m_tester->getProgramPathByExec("deepin-compressor");
}

TEST_F(TestOpenWithDialog, testcheckItem)
{
    m_tester->m_pCheckedItem = new OpenWithDialogListItem(QIcon(), "", m_tester);
    OpenWithDialogListItem *item = new OpenWithDialogListItem(QIcon(), "", m_tester);
    m_tester->checkItem(item);
}

TEST_F(TestOpenWithDialog, testcreateItem)
{
    m_tester->createItem(QIcon(), "", "", "");
}

TEST_F(TestOpenWithDialog, testslotUseOtherApplication)
{
    QString strPath = _SOURCEDIR;
    strPath += "/src/desktop/deepin-compressor.desktop";
    Stub stub;
    CommonStub::stub_QFileDialog_getOpenFileName(stub, strPath);
    m_tester->slotUseOtherApplication();

    Stub stub1;
    CommonStub::stub_QFileDialog_getOpenFileName(stub1, QStandardPaths::findExecutable("deepin-compressor"));
    m_tester->slotUseOtherApplication();
}

TEST_F(TestOpenWithDialog, testslotOpenFileByApp)
{
    m_tester->slotOpenFileByApp();
    m_tester->m_pCheckedItem = new OpenWithDialogListItem(QIcon(), "", m_tester);
    m_tester->slotOpenFileByApp();
}
