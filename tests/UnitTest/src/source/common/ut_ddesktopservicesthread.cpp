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
#include "gtest/src/stub.h"
#include "config.h"

#include <gtest/gtest.h>
#include <QTest>
#include <DDesktopServices>
#include <QString>

class TestDDesktopServicesThread : public ::testing::Test
{
public:
    TestDDesktopServicesThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DDesktopServicesThread;
        m_tester->setOpenFiles(m_listTempFile);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DDesktopServicesThread *m_tester;
    QStringList m_listTempFile = QStringList() << TEST_SOURCES_PATH + QString("/test.zip/testDir") << TEST_SOURCES_PATH + QString("/test.zip/test.txt");
};


TEST_F(TestDDesktopServicesThread, initTest)
{

}


TEST_F(TestDDesktopServicesThread, testsetOpenFiles)
{
    bool bEqual = true;

    for (int i = 0; i < m_listTempFile.count(); ++i) {
        if (m_tester->m_listFiles.count() > i) {
            if (m_listTempFile[i] != m_tester->m_listFiles[i]) {
                bEqual = false;
                break;
            }
        } else {
            bEqual = false;
            break;
        }
    }
    ASSERT_EQ(bEqual, true);
}

TEST_F(TestDDesktopServicesThread, testhasFiles)
{
    ASSERT_EQ(m_tester->hasFiles(), true);
}

bool showFolder_stub(QString localFilePath, const QString &startupId = QString())
{
    QFileInfo info(localFilePath);
    return info.isDir();
}

bool showFileItem_stub(QString localFilePath, const QString &startupId = QString())
{
    QFileInfo info(localFilePath);
    return info.isFile();
}

TEST_F(TestDDesktopServicesThread, testrun)
{
    typedef bool (*fptr)(QString, const QString &);
    fptr A_foo = (fptr)(&DDesktopServices::showFolder);
    fptr B_foo = (fptr)(&DDesktopServices::showFileItem);
    Stub stub;
    stub.set(A_foo, showFolder_stub);
    stub.set(B_foo, showFolder_stub);

    m_tester->run();

    ASSERT_EQ(m_tester->hasFiles(), true);
}
