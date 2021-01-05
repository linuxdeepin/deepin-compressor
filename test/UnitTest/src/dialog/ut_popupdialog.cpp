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

#include <gtest/gtest.h>
#include <QTest>

class TestTipDialog : public ::testing::Test
{
public:
    TestTipDialog() {}

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

TEST_F(TestTipDialog, showDialog)
{

}

class TestSimpleQueryDialog : public ::testing::Test
{
public:
    TestSimpleQueryDialog() {}

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

TEST_F(TestSimpleQueryDialog, showDialog)
{

}

class TestOverwriteQueryDialog : public ::testing::Test
{
public:
    TestOverwriteQueryDialog() {}

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

TEST_F(TestOverwriteQueryDialog, showDialog)
{

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
    TestConvertDialog() {}

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

TEST_F(TestConvertDialog, showDialog)
{

}
