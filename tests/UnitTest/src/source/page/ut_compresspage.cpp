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

#include "compresspage.h"
#include "compressview.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QFileSystemWatcher>


/*******************************函数打桩************************************/
// 对CompressView的addCompressFiles进行打桩
void compressView_addCompressFiles_stub(const QStringList &listFiles)
{
    Q_UNUSED(listFiles)
    return;
}

// 对CompressView的addCompressFiles进行打桩
QStringList compressView_getCompressFiles_empty_stub()
{
    return QStringList();
}

QStringList compressView_getCompressFiles_stub()
{
    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    return listFiles;
}

// 对CompressView的refreshCompressedFiles进行打桩
void compressView_refreshCompressedFiles_stub(bool bChanged, const QString &strFileName)
{
    Q_UNUSED(bChanged)
    Q_UNUSED(strFileName)
    return;
}

// 对CompressView的clear进行打桩
void compressView_clear_stub()
{
    return;
}
/*******************************函数打桩************************************/


// 测试CompressPage
class UT_CompressPage : public ::testing::Test
{
public:
    UT_CompressPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressPage *m_tester;
};

TEST_F(UT_CompressPage, initTest)
{

}

// 测试CompressPage的addCompressFiles函数
TEST_F(UT_CompressPage, test_addCompressFiles)
{
    Stub stub;
    stub.set(ADDR(CompressView, addCompressFiles), compressView_addCompressFiles_stub);
    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->addCompressFiles(listFiles);
    EXPECT_EQ(m_tester->m_pCompressView->m_listCompressFiles.isEmpty(), true);
}

// 测试CompressPage的compressFiles函数
TEST_F(UT_CompressPage, test_compressFiles)
{
    Stub stub;
    stub.set(ADDR(CompressView, getCompressFiles), compressView_getCompressFiles_stub);

    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    QStringList listFileResult = m_tester->compressFiles();
    bool bResult = (listFiles == listFileResult) ? true : false;
    EXPECT_EQ(bResult, true);
}

// 测试CompressPage的refreshCompressedFiles函数
TEST_F(UT_CompressPage, test_refreshCompressedFiles)
{
    Stub stub;
    stub.set(ADDR(CompressView, refreshCompressedFiles), compressView_refreshCompressedFiles_stub);
    m_tester->refreshCompressedFiles(false, "");
    EXPECT_EQ(m_tester->m_pCompressView->m_listCompressFiles.isEmpty(), true);
}

// 测试CompressPage的clear函数
TEST_F(UT_CompressPage, test_clear)
{
    Stub stub;
    stub.set(ADDR(CompressView, clear), compressView_clear_stub);
    m_tester->clear();
    EXPECT_EQ(m_tester->m_pCompressView->m_pFileWatcher->files().isEmpty(), true);
    EXPECT_EQ(m_tester->m_pCompressView->m_listCompressFiles.isEmpty(), true);
    EXPECT_EQ(m_tester->m_pCompressView->m_listEntry.isEmpty(), true);
}

// 测试CompressPage的getCompressView函数
TEST_F(UT_CompressPage, test_getCompressView)
{
    EXPECT_EQ(m_tester->getCompressView(), m_tester->m_pCompressView);
}

// 测试CompressPage的getNextBtn函数
TEST_F(UT_CompressPage, test_getNextBtn)
{
    EXPECT_EQ(m_tester->getNextBtn(), m_tester->m_pNextBtn);
}

// 测试CompressPage的slotCompressNextClicked函数
TEST_F(UT_CompressPage, test_slotCompressNextClicked)
{
    Stub stub;
    stub.set(ADDR(CompressView, getCompressFiles), compressView_getCompressFiles_stub);
    m_tester->slotCompressNextClicked();
}

// 测试CompressPage的slotCompressNextClicked函数
TEST_F(UT_CompressPage, test_slotCompressNextClicked_empty)
{
    Stub stub;
    stub.set(ADDR(CompressView, getCompressFiles), compressView_getCompressFiles_empty_stub);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->slotCompressNextClicked();
}

// 测试CompressPage的slotCompressLevelChanged函数
TEST_F(UT_CompressPage, test_slotCompressLevelChanged)
{
    m_tester->slotCompressLevelChanged(true);
    EXPECT_EQ(m_tester->m_bRootIndex, true);
}

// 测试CompressPage的slotCompressLevelChanged函数
TEST_F(UT_CompressPage, test_slotFileChoose)
{
    m_tester->m_bRootIndex = true;
    m_tester->slotFileChoose();
    EXPECT_EQ(m_tester->m_bRootIndex, true);
}
