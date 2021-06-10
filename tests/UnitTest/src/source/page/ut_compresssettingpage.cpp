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

#include "compresssettingpage.h"
#include "customwidget.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QAction>

bool g_checkFileNameVaild_result = false;
bool g_checkCompressOptionValid_result = false;
/*******************************函数打桩************************************/
// 对CompressSettingPage的setDefaultName进行打桩
void setDefaultName_stub(QString name)
{
    Q_UNUSED(name)
    return;
}

// 对CompressSettingPage的showWarningDialog进行打桩
void showWarningDialog_stub(const QString &msg, const QString &strToolTip)
{
    Q_UNUSED(msg)
    Q_UNUSED(strToolTip)
    return;
}

// 对CompressSettingPage的checkFileNameVaild进行打桩
bool checkFileNameVaild_stub(const QString strText)
{
    Q_UNUSED(strText)
    return g_checkFileNameVaild_result;
}

// 对CompressSettingPage的checkCompressOptionValid进行打桩
bool checkCompressOptionValid_stub()
{
    return g_checkCompressOptionValid_result;
}
/*******************************函数打桩************************************/


// 测试TypeLabel
class TestTypeLabel : public ::testing::Test
{
public:
    TestTypeLabel(): m_tester(nullptr) {}

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

TEST_F(TestTypeLabel, testmousePressEvent)
{
    QTest::mousePress(m_tester, Qt::LeftButton);
}


// 测试TypeLabel
class TestCompressSettingPage : public ::testing::Test
{
public:
    TestCompressSettingPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressSettingPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressSettingPage *m_tester;
};

TEST_F(TestCompressSettingPage, initTest)
{

}

TEST_F(TestCompressSettingPage, testsetFileSize_noFiles)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles;
    m_tester->setFileSize(listFiles, 0);
}

TEST_F(TestCompressSettingPage, testsetFileSize_oneFilesAndisDir)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "path");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "path");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "path");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles = QStringList() << "path/";
    m_tester->setFileSize(listFiles, 0);
    ASSERT_EQ(m_tester->m_pSavePathEdt->text(), "path");
}

TEST_F(TestCompressSettingPage, testsetFileSize_oneFilesAndisFile)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "1");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1/1.txt");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles = QStringList() << "1/1.txt";
    m_tester->setFileSize(listFiles, 0);
    ASSERT_EQ(m_tester->m_pSavePathEdt->text(), "1");
}

TEST_F(TestCompressSettingPage, testsetFileSize_twoFiles)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "1");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1/1.txt");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles = QStringList() << "1/1.txt" << "2/2.txt";
    m_tester->setFileSize(listFiles, 0);
    ASSERT_EQ(m_tester->m_pSavePathEdt->text(), "1");
}

TEST_F(TestCompressSettingPage, testrefreshMenu)
{
    Stub stub;
    PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(stub);

    m_tester->refreshMenu();
}

TEST_F(TestCompressSettingPage, testgetClickLbl)
{
    ASSERT_EQ(m_tester->getClickLbl(), m_tester->m_pClickLbl);
}

TEST_F(TestCompressSettingPage, testgetCompressBtn)
{
    ASSERT_EQ(m_tester->getCompressBtn(), m_tester->m_pCompressBtn);
}

TEST_F(TestCompressSettingPage, testgetComment_enabled)
{
    m_tester->m_pCommentEdt->setPlainText("hhh");
    m_tester->m_pCommentEdt->setEnabled(true);
    ASSERT_EQ(m_tester->getComment(), "hhh");
}

TEST_F(TestCompressSettingPage, testgetComment_disabled)
{
    m_tester->m_pCommentEdt->setPlainText("hhh");
    m_tester->m_pCommentEdt->setEnabled(false);
    ASSERT_EQ(m_tester->getComment().isEmpty(), true);
}

TEST_F(TestCompressSettingPage, testgetComment_nullptr)
{
    ASSERT_EQ(m_tester->getComment().isEmpty(), true);
}

TEST_F(TestCompressSettingPage, testsetTypeImage)
{
    ASSERT_EQ(m_tester->checkFileNameVaild("1.zip"), true);
}

TEST_F(TestCompressSettingPage, testcheckFileNameVaild_length0)
{
    ASSERT_EQ(m_tester->checkFileNameVaild(""), false);
}

TEST_F(TestCompressSettingPage, testcheckFileNameVaild_toolong)
{
    ASSERT_EQ(m_tester->checkFileNameVaild("hdfjshfjksdhfkjshaflkashdfkjshfdksjdhfjhsadfrsahfsjahfdhsakjdfhsadifhasdhfasjkdhfjksadfhkjsadfhjksadhfkjsadhfjksadhfosdhfkjsadhfkjsadhfjklashdfkjsadhfkjasdhfkjasdhfkjlsahdfjksadhofusadkjfhhdfjshfjksdhfkjshaflkashdfkjshfdksjdhfjhsadfrsahfsjahfdhsakjdfhsadifhasdhfa.zip"), false);
}

TEST_F(TestCompressSettingPage, testcheckFileNameVaild_hasSeparator)
{
    ASSERT_EQ(m_tester->checkFileNameVaild("1/1.zip"), false);
}

TEST_F(TestCompressSettingPage, testsetEncryptedEnabled)
{
    m_tester->setEncryptedEnabled(false);
    ASSERT_EQ(m_tester->m_pPasswordEdt->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testsetListEncryptionEnabled)
{
    m_tester->setListEncryptionEnabled(false);
}

TEST_F(TestCompressSettingPage, testsetSplitEnabled_false)
{
    m_tester->setSplitEnabled(false);
    ASSERT_EQ(m_tester->m_pSplitValueEdt->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testsetSplitEnabled_true)
{
    m_tester->m_pSplitCkb->setCheckState(Qt::Checked);
    m_tester->setSplitEnabled(true);
    ASSERT_EQ(m_tester->m_pSplitValueEdt->isEnabled(), true);
}

TEST_F(TestCompressSettingPage, testrefreshCompressLevel_tar)
{
    m_tester->refreshCompressLevel("tar");
    ASSERT_EQ(m_tester->m_pCompressLevelLbl->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testrefreshCompressLevel_tarZ)
{
    m_tester->refreshCompressLevel("tar.Z");
    ASSERT_EQ(m_tester->m_pCompressLevelLbl->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testrefreshCompressLevel_other)
{
    m_tester->refreshCompressLevel("zip");
    ASSERT_EQ(m_tester->m_pCompressLevelLbl->isEnabled(), true);
}

TEST_F(TestCompressSettingPage, testsetCommentEnabled)
{
    m_tester->m_pCommentEdt->setPlainText("aaa");
    m_tester->setCommentEnabled(false);
    ASSERT_EQ(m_tester->m_pCommentEdt->toPlainText().isEmpty(), true);
}

TEST_F(TestCompressSettingPage, testcheckCompressOptionValid)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    m_tester->m_pSplitCkb->setChecked(false);
    ASSERT_EQ(m_tester->checkCompressOptionValid(), true);
}

TEST_F(TestCompressSettingPage, testcheckCompressOptionValid_nameUnVaild)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);

    m_tester->m_pFileNameEdt->setText("");
    ASSERT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(TestCompressSettingPage, testcheckCompressOptionValid_savePathEmpty)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText(" ");
    ASSERT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(TestCompressSettingPage, testcheckCompressOptionValid_savePathNoExists)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    ASSERT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(TestCompressSettingPage, testcheckCompressOptionValid_savePathNoWritable)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    ASSERT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(TestCompressSettingPage, testcheckCompressOptionValid_splitCkbUnChecked)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    m_tester->m_pSplitCkb->setChecked(true);
    m_tester->m_pSplitValueEdt->setValue(0.000000000000001);
    m_tester->m_strMimeType = "7z";
    ASSERT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(TestCompressSettingPage, testcheckFile)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);

    ASSERT_EQ(m_tester->checkFile(""), true);
}

TEST_F(TestCompressSettingPage, testcheckFile_isSymLink)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, true);

    ASSERT_EQ(m_tester->checkFile(""), false);
}

TEST_F(TestCompressSettingPage, testcheckFile_isNotSymLink)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, false);

    ASSERT_EQ(m_tester->checkFile(""), false);
}

TEST_F(TestCompressSettingPage, testcheckFile_isNotReadable)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, false);

    ASSERT_EQ(m_tester->checkFile(""), false);
}

TEST_F(TestCompressSettingPage, testcheckFile_isDir)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);

    ASSERT_EQ(m_tester->checkFile(""), true);
}

TEST_F(TestCompressSettingPage, testshowWarningDialog)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    ASSERT_EQ(m_tester->showWarningDialog("", ""), 0);
}

TEST_F(TestCompressSettingPage, testsetDefaultName)
{
    m_tester->setDefaultName("/home");
    ASSERT_EQ(m_tester->m_pFileNameEdt->lineEdit()->text(), "/home");
}

TEST_F(TestCompressSettingPage, testslotTypeChanged_tar7z)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("tar.7z");
    m_tester->slotTypeChanged(pAction);
    ASSERT_EQ(m_tester->m_pCommentLbl->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testslotTypeChanged_7z)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("7z");
    m_tester->slotTypeChanged(pAction);
    ASSERT_EQ(m_tester->m_pCommentLbl->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testslotTypeChanged_zip)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("zip");
    m_tester->slotTypeChanged(pAction);
    ASSERT_EQ(m_tester->m_pSplitCkb->isEnabled(), true);
}

TEST_F(TestCompressSettingPage, testslotTypeChanged_other)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("tar");
    m_tester->slotTypeChanged(pAction);
    ASSERT_EQ(m_tester->m_pCommentLbl->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testslotTypeChanged_nullptr)
{
    QAction *pAction = nullptr;
    m_tester->slotTypeChanged(pAction);
}

TEST_F(TestCompressSettingPage, testslotFileNameChanged)
{
    m_tester->m_pFileNameEdt->setText("");
    m_tester->slotRefreshFileNameEdit();
}

TEST_F(TestCompressSettingPage, testslotFileNameChanged_inValid)
{
    g_checkFileNameVaild_result = false;
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkFileNameVaild), checkFileNameVaild_stub);
    m_tester->m_pFileNameEdt->setText("hh");
    m_tester->slotRefreshFileNameEdit();
}

TEST_F(TestCompressSettingPage, testslotFileNameChanged_valid)
{
    g_checkFileNameVaild_result = true;
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkFileNameVaild), checkFileNameVaild_stub);
    m_tester->m_pFileNameEdt->setText("hh");
    m_tester->slotRefreshFileNameEdit();
}

TEST_F(TestCompressSettingPage, testslotAdvancedEnabled_enabled)
{
    m_tester->m_pSplitValueEdt->setValue(1.0);
    m_tester->slotAdvancedEnabled(true);
    ASSERT_EQ(m_tester->m_pSplitValueEdt->value(), 1.0);
}

TEST_F(TestCompressSettingPage, testslotAdvancedEnabled_unEnabled)
{
    m_tester->m_pSplitValueEdt->setValue(1.0);
    m_tester->slotAdvancedEnabled(false);
    ASSERT_EQ(m_tester->m_pSplitValueEdt->value(), 0.0);
}

TEST_F(TestCompressSettingPage, testslotSplitEdtEnabled_SplitCkbUnChecked)
{
    m_tester->m_pSplitCkb->setChecked(false);
    m_tester->m_pSplitValueEdt->setValue(1.0);
    m_tester->slotSplitEdtEnabled();
    ASSERT_EQ(m_tester->m_pSplitValueEdt->value(), 0.0);
}

TEST_F(TestCompressSettingPage, testslotSplitEdtEnabled_SplitCkbChecked)
{
    m_tester->m_pSplitCkb->setChecked(true);
    m_tester->m_strMimeType = "zip";
    m_tester->slotSplitEdtEnabled();
    ASSERT_EQ(m_tester->m_pCommentEdt->isEnabled(), false);
}

TEST_F(TestCompressSettingPage, testslotCompressClicked_containsSelf)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);

    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("/home");
    m_tester->m_listFiles << "/home/1.zip";
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, testslotCompressClicked_zipVolumePassword)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    g_checkCompressOptionValid_result = false;
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, testslotCompressClicked_tar)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("tar");
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, testslotCompressClicked_zip)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, testslotCompressClicked_archiveExists)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    QFileStub::stub_QFile_remove(stub, true);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, testslotCompressClicked_archiveExists001)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    QFileStub::stub_QFile_remove(stub, false);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->slotCompressClicked();
}

TEST_F(TestCompressSettingPage, testslotCommentTextChanged)
{
    QString str;
    QString strTemp;
    for (int i = 0; i < 10001; ++i) {
        str += "1";
        if (i < 10000)
            strTemp += "1";
    }
    m_tester->m_pCommentEdt->setPlainText(str);
    m_tester->slotCommentTextChanged();
    ASSERT_EQ(m_tester->m_pCommentEdt->toPlainText(), strTemp);
}

TEST_F(TestCompressSettingPage, testslotPasswordChanged)
{
    m_tester->m_pPasswordEdt->setText("123《》");
    m_tester->slotPasswordChanged();
    ASSERT_EQ(m_tester->m_pPasswordEdt->lineEdit()->text(), "123");
}

TEST_F(TestCompressSettingPage, testslotEchoModeChanged)
{
    m_tester->slotEchoModeChanged(false);
}
