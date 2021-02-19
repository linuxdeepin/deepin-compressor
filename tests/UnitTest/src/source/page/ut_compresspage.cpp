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

#include "compresspage.h"
#include "compressview.h"
#include "customwidget.h"
#include "uitools.h"
#include "popupdialog.h"

#include <gtest/gtest.h>
#include <QTest>

class TestCompressPage : public ::testing::Test
{
public:
    TestCompressPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressPage;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressPage *m_tester;
};

TEST_F(TestCompressPage, initTest)
{

}

TEST_F(TestCompressPage, addCompressFiles)
{
    m_tester->addCompressFiles(QStringList() << "/home/chenglu/Desktop/ut/3.txt");
}

TEST_F(TestCompressPage, compressFiles)
{
    m_tester->compressFiles();
}

TEST_F(TestCompressPage, refreshCompressedFiles)
{
    m_tester->refreshCompressedFiles(true, "");
}

TEST_F(TestCompressPage, clear)
{
    m_tester->clear();
}

TEST_F(TestCompressPage, initUI)
{
    m_tester->initUI();
}

TEST_F(TestCompressPage, initConnections)
{
    m_tester->initConnections();
}

TEST_F(TestCompressPage, slotCompressNextClicked1)
{
    // TipDialog
    m_tester->m_pCompressView->m_listCompressFiles << "/home/cehnglu/Desktop/ut/1";
    m_tester->slotCompressNextClicked();
}

TEST_F(TestCompressPage, slotCompressNextClicked2)
{
    // TipDialog
//    m_tester->slotCompressNextClicked();
}

TEST_F(TestCompressPage, slotCompressLevelChanged)
{
    m_tester->slotCompressLevelChanged(true);
}

TEST_F(TestCompressPage, getNextBtn)
{
    m_tester->getNextBtn();
}

TEST_F(TestCompressPage, getCompressView)
{
    m_tester->getCompressView();
}
