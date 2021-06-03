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
#include "ddesktopservicesthread.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/


/*******************************单元测试************************************/
// 测试DDesktopServicesThread
class TestDDesktopServicesThread : public ::testing::Test
{
public:
    TestDDesktopServicesThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DDesktopServicesThread();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DDesktopServicesThread *m_tester;
};

TEST_F(TestDDesktopServicesThread, initTest)
{

}

TEST_F(TestDDesktopServicesThread, testsetOpenFiles)
{
    QStringList listFiles = QStringList() << "1/" << "1.txt";
    m_tester->setOpenFiles(listFiles);
    ASSERT_EQ(m_tester->m_listFiles == listFiles, true);
}

TEST_F(TestDDesktopServicesThread, testhasFiles)
{
    QStringList listFiles = QStringList() << "1/" << "1.txt";
    m_tester->setOpenFiles(listFiles);
    ASSERT_EQ(m_tester->hasFiles(), true);
}

TEST_F(TestDDesktopServicesThread, testrun)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    DDesktopServicestub::stub_DDesktopServicestub_showFolder(stub);
    m_tester->m_listFiles.clear();
    m_tester->m_listFiles << "1/";
    m_tester->run();

    Stub stub1;
    DDesktopServicestub::stub_DDesktopServicestub_showFileItem(stub1);
    QFileInfoStub::stub_QFileInfo_isDir(stub1, false);
    m_tester->m_listFiles.clear();
    m_tester->m_listFiles << "1.txt";
    m_tester->run();
}

