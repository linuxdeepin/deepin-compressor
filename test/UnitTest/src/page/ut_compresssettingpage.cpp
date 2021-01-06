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

#include "compresssettingpage.h"
#include "customwidget.h"
#include "pluginmanager.h"
#include "uitools.h"
#include "uistruct.h"
#include "popupdialog.h"
#include "DebugTimeManager.h"

#include <QStandardPaths>
#include <QFocusEvent>
#include <QScrollArea>
#include <QFileIconProvider>
#include <QMenu>
#include <QMimeDatabase>

#include <cmath>

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QTest>

class TestTypeLabel : public ::testing::Test
{
public:
    TestTypeLabel() {}

public:
    virtual void SetUp()
    {
        m_tester = new TypeLabel;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    TypeLabel *m_tester;
};

TEST_F(TestTypeLabel, initTest)
{

}

TEST_F(TestTypeLabel, mousePressEvent)
{

}

TEST_F(TestTypeLabel, paintEvent)
{

}

TEST_F(TestTypeLabel, focusInEvent)
{

}

TEST_F(TestTypeLabel, focusOutEvent)
{

}

class TestCompressSettingPage : public ::testing::Test
{
public:
    TestCompressSettingPage() {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressSettingPage;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressSettingPage *m_tester;
};

int length_stub()
{
    return 30000;
}

bool isChecked_true_stub()
{
    return true;
}

bool isChecked_false_stub()
{
    return false;
}

TEST_F(TestCompressSettingPage, initTest)
{

}

TEST_F(TestCompressSettingPage, setFileSize1)
{
    // 单文件
    m_tester->setFileSize(QStringList() << "/home/chenglu/Desktop/ut/3.txt", 3);
}

TEST_F(TestCompressSettingPage, setFileSize2)
{
    // 文件处于不同位置
    m_tester->setFileSize(QStringList() << "/home/chenglu/Desktop/ut/3.txt" << "/home/chenglu/Desktop/ut/1/1.txt", 6);
}

TEST_F(TestCompressSettingPage, setFileSize3)
{
    // 单文件夹
    m_tester->setFileSize(QStringList() << "/home/chenglu/Desktop/ut/1", 9);
}

TEST_F(TestCompressSettingPage, refreshMenu)
{
    m_tester->m_listSupportedMimeTypes.clear();
    m_tester->refreshMenu();
}

TEST_F(TestCompressSettingPage, initUI)
{
    m_tester->initUI();
}

TEST_F(TestCompressSettingPage, initConnections)
{
    m_tester->initConnections();
}

TEST_F(TestCompressSettingPage, setTypeImage)
{
    m_tester->setTypeImage("zip");
}

TEST_F(TestCompressSettingPage, checkFileNameVaild1)
{
    // 文件名为空返回错误
    bool ret = m_tester->checkFileNameVaild("");
    EXPECT_EQ(ret, false);
}

TEST_F(TestCompressSettingPage, checkFileNameVaild2)
{
    // 文件名过长返回错误
//    Stub stub;
//    stub.set(ADDR(QString, length), length_stub);
    bool ret = m_tester->checkFileNameVaild("dfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfgdfgdgdfgfg");
    EXPECT_EQ(ret, false);
}

TEST_F(TestCompressSettingPage, checkFileNameVaild3)
{
    // 如果文件名中包含"/"，返回错误
    bool ret = m_tester->checkFileNameVaild("a/b");
    EXPECT_EQ(ret, false);
}

TEST_F(TestCompressSettingPage, setEncryptedEnabled)
{
    m_tester->setEncryptedEnabled(false);
}

TEST_F(TestCompressSettingPage, slotEchoModeChanged)
{
    m_tester->slotEchoModeChanged(true);
}

TEST_F(TestCompressSettingPage, setListEncryptionEnabled)
{
    m_tester->setListEncryptionEnabled(false);
}

TEST_F(TestCompressSettingPage, setSplitEnabled1)
{
    m_tester->setSplitEnabled(true);
}

TEST_F(TestCompressSettingPage, setSplitEnabled2)
{
    m_tester->setSplitEnabled(false);
}

TEST_F(TestCompressSettingPage, refreshCompressLevel1)
{
    m_tester->refreshCompressLevel("tar");
}

TEST_F(TestCompressSettingPage, refreshCompressLevel2)
{
    m_tester->refreshCompressLevel("tar.Z");
}

TEST_F(TestCompressSettingPage, refreshCompressLevel3)
{
    m_tester->refreshCompressLevel("zip");
}

TEST_F(TestCompressSettingPage, setCommentEnabled)
{
    m_tester->setCommentEnabled(false);
}

TEST_F(TestCompressSettingPage, checkCompressOptionValid)
{
//    m_tester->m_pFileNameEdt->setText("utTest");
//    m_tester->m_pSavePathEdt->setText("/home/chenglu/Desktop/ut");
//    m_tester->m_listFiles << "/home/chenglu/Desktop/ut/3.txt";
//    Stub stub;
//    stub.set(ADDR(CustomCheckBox, isChecked), isChecked_false_stub());
//    bool ret = m_tester->checkCompressOptionValid();
//    EXPECT_EQ(ret, true);
}

TEST_F(TestCompressSettingPage, checkFilePermission)
{
    bool ret = m_tester->checkFilePermission("/home/cehnglu/Desktop/ut/1");
    EXPECT_EQ(ret, true);
}

TEST_F(TestCompressSettingPage, showWarningDialog)
{
    // TipDialog
//    m_tester->showWarningDialog("文件名无效");
}

TEST_F(TestCompressSettingPage, setDefaultName)
{
    m_tester->setDefaultName("utTest.zip");
}

TEST_F(TestCompressSettingPage, slotShowRightMenu)
{

}

TEST_F(TestCompressSettingPage, slotTypeChanged1)
{
    m_tester->slotTypeChanged(nullptr);
}

TEST_F(TestCompressSettingPage, slotTypeChanged2)
{
    QAction *action = new QAction("tar.7z", m_tester->m_pTypeMenu);
    m_tester->slotTypeChanged(action);
    EXPECT_EQ(m_tester->m_pCompressTypeLbl->text(), "tar.7z");
    EXPECT_EQ(m_tester->m_pPasswordEdt->isEnabled(), true);
    EXPECT_EQ(m_tester->m_pListEncryptionBtn->isEnabled(), true);
    EXPECT_EQ(m_tester->m_pSplitCkb->isEnabled(), false);
    EXPECT_EQ(m_tester->m_pCommentEdt->isEnabled(), false);
    ASSERT_NE(action, nullptr);
    delete action;
}

TEST_F(TestCompressSettingPage, slotTypeChanged3)
{
    QAction *action = new QAction("7z", m_tester->m_pTypeMenu);
    m_tester->slotTypeChanged(action);
    EXPECT_EQ(m_tester->m_pCompressTypeLbl->text(), "7z");
    EXPECT_EQ(m_tester->m_pPasswordEdt->isEnabled(), true);
    EXPECT_EQ(m_tester->m_pListEncryptionBtn->isEnabled(), true);
    EXPECT_EQ(m_tester->m_pSplitCkb->isEnabled(), true);
    EXPECT_EQ(m_tester->m_pCommentEdt->isEnabled(), false);
    ASSERT_NE(action, nullptr);
    delete action;
}

TEST_F(TestCompressSettingPage, slotTypeChanged4)
{
    QAction *action = new QAction("zip", m_tester->m_pTypeMenu);
    m_tester->slotTypeChanged(action);
    EXPECT_EQ(m_tester->m_pCompressTypeLbl->text(), "zip");
    EXPECT_EQ(m_tester->m_pPasswordEdt->isEnabled(), true);
    EXPECT_EQ(m_tester->m_pListEncryptionBtn->isEnabled(), false);
    EXPECT_EQ(m_tester->m_pSplitCkb->isEnabled(), true);
//    EXPECT_EQ(m_tester->m_pCommentEdt->isEnabled(), false);
    ASSERT_NE(action, nullptr);
    delete action;
}

TEST_F(TestCompressSettingPage, slotTypeChanged5)
{
    QAction *action = new QAction("jar", m_tester->m_pTypeMenu);
    m_tester->slotTypeChanged(action);
    EXPECT_EQ(m_tester->m_pCompressTypeLbl->text(), "jar");
    EXPECT_EQ(m_tester->m_pPasswordEdt->isEnabled(), false);
    EXPECT_EQ(m_tester->m_pListEncryptionBtn->isEnabled(), false);
    EXPECT_EQ(m_tester->m_pSplitCkb->isEnabled(), false);
    EXPECT_EQ(m_tester->m_pCommentEdt->isEnabled(), false);
    ASSERT_NE(action, nullptr);
    delete action;
}

TEST_F(TestCompressSettingPage, slotFileNameChanged1)
{
    m_tester->slotFileNameChanged("");
}

TEST_F(TestCompressSettingPage, slotFileNameChanged2)
{
    m_tester->slotFileNameChanged("a/b");
}

TEST_F(TestCompressSettingPage, slotFileNameChanged3)
{
    m_tester->slotFileNameChanged("utTest");
}

TEST_F(TestCompressSettingPage, slotAdvancedEnabled)
{
    m_tester->slotAdvancedEnabled(false);
}

TEST_F(TestCompressSettingPage, slotSplitEdtEnabled)
{
    Stub stub;
    stub.set(ADDR(CustomCheckBox, isChecked), isChecked_true_stub);
    m_tester->m_strMimeType = "zip";
    m_tester->slotSplitEdtEnabled();
}

TEST_F(TestCompressSettingPage, slotSplitEdtEnabled2)
{
    Stub stub;
    stub.set(ADDR(CustomCheckBox, isChecked), isChecked_false_stub);
    m_tester->m_strMimeType = "zip";
    m_tester->slotSplitEdtEnabled();
}

void compressParameter_stub()
{
    CompressParameter compressInfo;
    compressInfo.strTargetPath = "/home/chenglu/Desktop/ut";
    compressInfo.strArchiveName = "slotCompressClicked.zip";
//    return compressInfo;
}

bool checkCompressOptionValid_true_stub()
{
    return true;
}

bool checkCompressOptionValid_false_stub()
{
    return false;
}

TEST_F(TestCompressSettingPage, slotCompressClicked1)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_false_stub);
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, slotCompressClicked2)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_true_stub);

    m_tester->m_pFileNameEdt->setText("/home/chenglu/Desktop/ut/3.txt");
    m_tester->m_pCompressTypeLbl->setText("tar");
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, slotCompressClicked3)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_true_stub);

    m_tester->m_pFileNameEdt->setText("/home/chenglu/Desktop/ut/3.txt");
    m_tester->m_pCompressTypeLbl->setText("tar.bz2");
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, slotCompressClicked4)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_true_stub);

    m_tester->m_pFileNameEdt->setText("/home/chenglu/Desktop/ut/3.txt");
    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, slotCommentTextChanged1)
{
//    Stub stub;
//    stub.set(ADDR(QString, size), length_stub);

//    m_tester->slotCommentTextChanged();
}

TEST_F(TestCompressSettingPage, slotCommentTextChanged2)
{
    m_tester->slotCommentTextChanged();
}

TEST_F(TestCompressSettingPage, getCompressBtn)
{
    CustomPushButton *ret = m_tester->getCompressBtn();
    ASSERT_NE(ret, nullptr);
    delete ret;
}

TEST_F(TestCompressSettingPage, getComment1)
{
    m_tester->m_pCommentEdt = nullptr;
    QString ret = m_tester->getComment();
    EXPECT_EQ(ret, "");
}

TEST_F(TestCompressSettingPage, getComment2)
{
    m_tester->m_pCommentEdt = new DTextEdit(m_tester);
    m_tester->getComment();
}

TEST_F(TestCompressSettingPage, eventFilter)
{

}

TEST_F(TestCompressSettingPage, getClickLbl)
{
    TypeLabel *ret = m_tester->getClickLbl();
    ASSERT_NE(ret, nullptr);
    delete ret;
}
