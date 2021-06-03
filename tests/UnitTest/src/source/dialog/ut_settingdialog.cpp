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
#include "settingdialog.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/
QVariant dSettings_value_stub(const QString &key)
{
    return QVariant(true);
}
/*******************************单元测试************************************/
// 测试SettingDialog
class TestSettingDialog : public ::testing::Test
{
public:
    TestSettingDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SettingDialog;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SettingDialog *m_tester;
};

TEST_F(TestSettingDialog, initTest)
{

}

TEST_F(TestSettingDialog, testgetDefaultExtractPath)
{
    m_tester->m_curpath = "1/2";
    ASSERT_EQ(m_tester->getDefaultExtractPath(), "1/2");
}

TEST_F(TestSettingDialog, testisAutoCreatDir)
{
    Stub stub;
    stub.set(ADDR(DSettings, value), dSettings_value_stub);

    ASSERT_EQ(m_tester->isAutoCreatDir(), true);
}

TEST_F(TestSettingDialog, testisAutoOpen)
{
    Stub stub;
    stub.set(ADDR(DSettings, value), dSettings_value_stub);

    ASSERT_EQ(m_tester->isAutoOpen(), true);
}

TEST_F(TestSettingDialog, testisAutoDeleteFile)
{
    Stub stub;
    stub.set(ADDR(DSettings, value), dSettings_value_stub);

    ASSERT_EQ(m_tester->isAutoDeleteFile(), true);
}

TEST_F(TestSettingDialog, testisAutoDeleteArchive)
{
    m_tester->m_deleteArchiveOption->setValue("Never");
    ASSERT_EQ(m_tester->isAutoDeleteArchive(), "Never");
}

