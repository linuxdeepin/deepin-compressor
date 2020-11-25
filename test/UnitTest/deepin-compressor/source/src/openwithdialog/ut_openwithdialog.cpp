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

#include <gtest/gtest.h>

class TestOpenWithDialog : public ::testing::Test
{
public:
    TestOpenWithDialog() {}

public:
    virtual void SetUp()
    {
        QString str = "../test/UnitTest/regress/new.zip";
        m_tester = new OpenWithDialog(DUrl(str));
        m_tester->exec();
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

TEST_F(TestOpenWithDialog, testaddMenuOpenAction)
{
    QMenu menu;
    QList<QAction *> listAction = OpenWithDialog::addMenuOpenAction("../test/UnitTest/regress/new.zip", &menu);
}

TEST_F(TestOpenWithDialog, testchooseOpen)
{
    QMenu menu;
    QList<QAction *> listAction = OpenWithDialog::addMenuOpenAction("../test/UnitTest/regress/new.zip", &menu);

    if (listAction.count() > 0) {
        OpenWithDialog::chooseOpen(listAction[0]->text(), "../test/UnitTest/regress/new.zip");
    }
}

TEST_F(TestOpenWithDialog, testincludePercentFile)
{
    OpenWithDialog::includePercentFile("../test/UnitTest/regress/new.zip");
}

TEST_F(TestOpenWithDialog, testSetShowType)
{
    m_tester->SetShowType(SelApp);
}

TEST_F(TestOpenWithDialog, testAppDisplayName)
{
    m_tester->AppDisplayName();
}
