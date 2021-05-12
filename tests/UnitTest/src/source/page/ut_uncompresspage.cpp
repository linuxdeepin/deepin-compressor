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

#include "uncompresspage.h"
#include "uistruct.h"
#include "uncompressview.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

/*******************************函数打桩************************************/
// 对UnCompressView的addNewFiles进行打桩
void unCompressView_refreshDataByCurrentPathChanged_stub()
{
    return;
}

// 对UnCompressView的addNewFiles进行打桩
void unCompressView_addNewFiles_stub(const QStringList &listFiles)
{
    Q_UNUSED(listFiles)
    return;
}

// 对UnCompressView的addNewFiles进行打桩
QString unCompressView_getCurPath_stub()
{
    return "123";
}

// 对UnCompressView的width进行打桩
int unCompressPage_width_stub()
{
    return 50;
}

// 对QFontMetrics的width进行打桩
int qFontMetrics_width_stub(const QString &, int)
{
    return 120;
}
/*******************************函数打桩************************************/

class TestUnCompressPage : public ::testing::Test
{
public:
    TestUnCompressPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UnCompressPage;
        m_tester->disconnect();
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

TEST_F(TestUnCompressPage, testsetArchiveFullPath)
{
    UnCompressParameter stUnCompressParameter;
    stUnCompressParameter.eSplitVolume = UnCompressParameter::ST_No;
    stUnCompressParameter.bModifiable = true;
    m_tester->setArchiveFullPath("1.zip", stUnCompressParameter);
    ASSERT_EQ(m_tester->getUnCompressView()->isModifiable(), true);
}

TEST_F(TestUnCompressPage, testsetArchiveFullPath_No)
{
    UnCompressParameter stUnCompressParameter;
    stUnCompressParameter.eSplitVolume = UnCompressParameter::ST_Zip;
    stUnCompressParameter.bModifiable = false;
    m_tester->setArchiveFullPath("1.zip.001", stUnCompressParameter);
    ASSERT_EQ(m_tester->getUnCompressView()->isModifiable(), false);
}

TEST_F(TestUnCompressPage, testarchiveFullPath)
{
    UnCompressParameter stUnCompressParameter;
    m_tester->setArchiveFullPath("1.zip.001", stUnCompressParameter);
    ASSERT_EQ(m_tester->archiveFullPath(), "1.zip.001");
}

TEST_F(TestUnCompressPage, testsetDefaultUncompressPath)
{
    m_tester->setDefaultUncompressPath("/home/Desktop");
    ASSERT_EQ(m_tester->m_strUnCompressPath, "/home/Desktop");
}

TEST_F(TestUnCompressPage, testsetrefreshArchiveData)
{
    m_tester->refreshArchiveData();
}

TEST_F(TestUnCompressPage, testrefreshDataByCurrentPathChanged)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, refreshDataByCurrentPathChanged), unCompressView_refreshDataByCurrentPathChanged_stub);
    m_tester->refreshDataByCurrentPathChanged();
}

TEST_F(TestUnCompressPage, testaddNewFiles)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, addNewFiles), unCompressView_addNewFiles_stub);
    m_tester->addNewFiles(QStringList());
}

TEST_F(TestUnCompressPage, testgetCurPath)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, getCurPath), unCompressView_getCurPath_stub);
    ASSERT_EQ(m_tester->getCurPath(), "123");
}

TEST_F(TestUnCompressPage, testclear)
{
    m_tester->clear();
}

TEST_F(TestUnCompressPage, testgetUnCompressView)
{
    ASSERT_EQ(m_tester->getUnCompressView(), m_tester->m_pUnCompressView);
}

TEST_F(TestUnCompressPage, testgetUnCompressBtn)
{
    ASSERT_EQ(m_tester->getUnCompressBtn(), m_tester->m_pUnCompressBtn);
}

TEST_F(TestUnCompressPage, testgetUncompressPathBtn)
{
    ASSERT_EQ(m_tester->getUncompressPathBtn(), m_tester->m_pUncompressPathBtn);
}

TEST_F(TestUnCompressPage, testelidedExtractPath)
{
    Stub stub;
    stub.set(ADDR(UnCompressPage, width), unCompressPage_width_stub);
    stub.set((int(QFontMetrics::*)(const QString &, int)const)ADDR(QFontMetrics, width), qFontMetrics_width_stub);
    QString str = "123456789123456789123456789";
    m_tester->elidedExtractPath(str);
}

TEST_F(TestUnCompressPage, testslotUncompressClicked001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->slotUncompressClicked();
}

TEST_F(TestUnCompressPage, testslotUncompressClicked002)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->slotUncompressClicked();
}

TEST_F(TestUnCompressPage, testslotUncompressClicked003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->slotUncompressClicked();
}

TEST_F(TestUnCompressPage, testslotUnCompressPathClicked_Rejected)
{
    QList<QUrl> listUrl;
    listUrl << QUrl("/home/Desktop");
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 0);
    DFileDialogStub::stub_DFileDialog_selectedUrls(stub, listUrl);
    m_tester->slotUnCompressPathClicked();
}

TEST_F(TestUnCompressPage, testslotUnCompressPathClicked_Accepted)
{
    QList<QUrl> listUrl;
    listUrl << QUrl("/home/Desktop");
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    DFileDialogStub::stub_DFileDialog_selectedUrls(stub, listUrl);
    m_tester->slotUnCompressPathClicked();
}

TEST_F(TestUnCompressPage, testslotFileChoose)
{
    m_tester->getUnCompressView()->m_bModifiable = true;
    m_tester->slotFileChoose();
}
