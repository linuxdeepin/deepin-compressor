/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <gaoxiang@uniontech.com>
*
* Maintainer: chendu <gaoxiang@uniontech.com>
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

#include "datamanager.h"
#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QString>
#include <QTextCodec>

class TestDataManager : public ::testing::Test
{
public:
    TestDataManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DataManager;
        m_tester->resetArchiveData();
    }

    virtual void TearDown()
    {
        DataManager::get_instance().resetArchiveData();
        delete m_tester;
    }

protected:
    DataManager *m_tester;
};

TEST_F(TestDataManager, initTest)
{

}

TEST_F(TestDataManager, testresetArchiveData)
{
    m_tester->resetArchiveData();
    ASSERT_EQ(m_tester->m_stArchiveData.qSize, 0);
}

TEST_F(TestDataManager, testarchiveData)
{
    m_tester->m_stArchiveData.qSize = 1;
    ArchiveData stData = m_tester->archiveData();
    ASSERT_EQ(stData.qSize, 1);
}
