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

#include "uncompresspage.h"
#include "uncompressview.h"
#include "customwidget.h"
#include "popupdialog.h"
#include "DebugTimeManager.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "datamanager.h"

#include <gtest/gtest.h>
#include <QTest>

class TestUnCompressPage : public ::testing::Test
{
public:
    TestUnCompressPage() {}

public:
    virtual void SetUp()
    {
        m_tester = new UnCompressPage;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UnCompressPage *m_tester;
};

TEST_F(TestUnCompressPage, initTest)
{

}

TEST_F(TestUnCompressPage, setArchiveFullPath)
{
    UnCompressParameter parameter;
    parameter.bModifiable = true;
    parameter.bMultiplePassword = true;
    parameter.eSplitVolume = UnCompressParameter::ST_No;
    m_tester->setArchiveFullPath("/home/chenglu/Desktop/ut/1.zip", parameter);
    EXPECT_EQ(m_tester->m_pUnCompressView->m_bModifiable, true);
    EXPECT_EQ(m_tester->m_pUnCompressView->m_bMultiplePassword, true);
}

TEST_F(TestUnCompressPage, setArchiveFullPath2)
{
    UnCompressParameter parameter;
    parameter.eSplitVolume = UnCompressParameter::ST_Zip;
    m_tester->setArchiveFullPath("/home/chenglu/Desktop/ut/01.zip.001", parameter);
    EXPECT_EQ(m_tester->m_pUnCompressView->m_bModifiable, false);
    EXPECT_EQ(m_tester->m_pUnCompressView->m_bMultiplePassword, false);
}

TEST_F(TestUnCompressPage, archiveFullPath)
{
    m_tester->m_strArchiveFullPath = "/home/chenglu/Desktop/ut/1.zip";
    m_tester->archiveFullPath();
    EXPECT_EQ(m_tester->m_strArchiveFullPath, "/home/chenglu/Desktop/ut/1.zip");
}

TEST_F(TestUnCompressPage, setDefaultUncompressPath)
{
    m_tester->setDefaultUncompressPath("/home/chenglu/Desktop/ut");
}

TEST_F(TestUnCompressPage, refreshArchiveData)
{
    m_tester->refreshArchiveData();
}

TEST_F(TestUnCompressPage, resizeEvent)
{

}

TEST_F(TestUnCompressPage, refreshDataByCurrentPathChanged)
{
    m_tester->refreshDataByCurrentPathChanged();
}

TEST_F(TestUnCompressPage, addNewFiles)
{
//    m_tester->addNewFiles(QStringList() << "/home/chenglu/Desktop/ut/3.txt");
}

TEST_F(TestUnCompressPage, getCurPath)
{
    m_tester->getCurPath();
}

TEST_F(TestUnCompressPage, clear)
{
    m_tester->clear();
}

TEST_F(TestUnCompressPage, initUI)
{
    m_tester->initUI();
}

TEST_F(TestUnCompressPage, initConnections)
{
    m_tester->initConnections();
}

TEST_F(TestUnCompressPage, elidedExtractPath)
{
    QString ret = m_tester->elidedExtractPath("/home/chenglu/Desktop/ut");
//    qInfo() << "////" << m_tester->width(); // 640
    EXPECT_EQ(ret, "/home/chenglu/Desktop/ut");
}

TEST_F(TestUnCompressPage, slotUncompressClicked)
{

}

TEST_F(TestUnCompressPage, slotUnCompressPathClicked)
{

}

TEST_F(TestUnCompressPage, getUncompressPathBtn)
{
    m_tester->getUncompressPathBtn();
}

TEST_F(TestUnCompressPage, getUnCompressBtn)
{
    m_tester->getUnCompressBtn();
}

TEST_F(TestUnCompressPage, getUnCompressView)
{
    m_tester->getUnCompressView();
}
