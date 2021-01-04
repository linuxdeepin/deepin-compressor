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

#include "mainwindow.h"
#include "popupdialog.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <DDialog>
#include <DLabel>
#include <QKeyEvent>
#include <DPushButton>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

class TestMainWindow : public ::testing::Test
{
public:
    TestMainWindow() {}

public:
    virtual void SetUp()
    {
        m_tester = new MainWindow;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    MainWindow *m_tester;
};

bool isWritable_stub()
{
    return false;
}

bool isExecutable_stub()
{
    return false;
}

int showDialog_stub(const QString &strDesText, const QString btnMsg, DDialog::ButtonType btnType)
{
    return false;
}

TEST_F(TestMainWindow, initTest)
{

}

TEST_F(TestMainWindow, testcheckHerePath)
{
    Stub stub;
    stub.set(ADDR(TipDialog, showDialog), showDialog_stub);
    stub.set(ADDR(QFileInfo, isWritable), isWritable_stub);
    stub.set(ADDR(QFileInfo, isExecutable), isExecutable_stub);

    QString strPath = QFileInfo("../UnitTest/test_sources/noPermissionDir").absoluteFilePath();

    ASSERT_EQ(m_tester->checkHerePath(strPath), false);
}

TEST_F(TestMainWindow, testcheckSettings)
{
}


TEST_F(TestMainWindow, testhandleApplicationTabEventNotify)
{
    m_tester->m_pUnCompressPage = nullptr;
    ASSERT_EQ(m_tester->handleApplicationTabEventNotify(nullptr, nullptr), false);
}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_001)
{
    Dtk::Widget::DTitlebar *pp = m_tester->titlebar();
    QObject *obj = dynamic_cast<QObject *>(pp);
    QKeyEvent *evt = new QKeyEvent(QEvent::TabletPress, Qt::Key_Tab, Qt::NoModifier);
    m_tester->handleApplicationTabEventNotify(m_tester->titlebar(), evt);
//    ASSERT_EQ(, false);

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_002)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_003)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_004)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_005)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Tab_006)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_001)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_002)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_003)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_004)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_005)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_BackTab_006)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Left_001)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Left_002)
{

}

TEST_F(TestMainWindow, testhandleApplicationTabEventNotify_Left_003)
{

}
