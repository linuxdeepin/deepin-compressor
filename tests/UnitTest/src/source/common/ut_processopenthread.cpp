// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processopenthread.h"
#include <gtest/gtest.h>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试ProcessOpenThread
class UT_ProcessOpenThread : public ::testing::Test
{
public:
    UT_ProcessOpenThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProcessOpenThread(nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ProcessOpenThread *m_tester;
};

TEST_F(UT_ProcessOpenThread, initTest)
{

}

TEST_F(UT_ProcessOpenThread, test_setProgramPath)
{
    m_tester->setProgramPath("xdg-open");
    EXPECT_EQ(m_tester->m_strProgramPath, "xdg-open");
}

TEST_F(UT_ProcessOpenThread, test_setArguments)
{
    QStringList listArguments = QStringList() << "1.txt";
    m_tester->setArguments(listArguments);
    EXPECT_EQ(m_tester->m_listArguments == listArguments, true);
}

TEST_F(UT_ProcessOpenThread, testrun)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString program = QStandardPaths::findExecutable(QStringLiteral("touch"));
    ASSERT_FALSE(program.isEmpty());
    const QString filePath = dir.filePath(QStringLiteral("file with spaces.txt"));
    m_tester->setProgramPath(program);
    m_tester->setArguments({filePath});
    m_tester->run();
    EXPECT_TRUE(QFileInfo::exists(filePath));
}
