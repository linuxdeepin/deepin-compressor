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

#include "bz2plugin.h"
#include "gtest/src/stub.h"
#include "queries.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>

#define MAX_FILENAME 512

Q_DECLARE_METATYPE(KPluginMetaData)

class TestLibBzip2InterfaceFactory : public QObject, public ::testing::Test
{
public:
    TestLibBzip2InterfaceFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibBzip2InterfaceFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibBzip2InterfaceFactory *m_tester;
};

class TestLibBzip2Interface : public QObject, public ::testing::Test
{
public:
    TestLibBzip2Interface(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/bz2/test.txt.bz2";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibBzip2Interface(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibBzip2Interface *m_tester;
};


TEST_F(TestLibBzip2InterfaceFactory, initTest)
{

}

TEST_F(TestLibBzip2Interface, initTest)
{

}

TEST_F(TestLibBzip2Interface, testlist)
{
    PluginFinishType eType = m_tester->list();
    ASSERT_EQ(eType, PFT_Nomral);
}

TEST_F(TestLibBzip2Interface, testinittestArchive)
{
    PluginFinishType eType = m_tester->testArchive();
    ASSERT_EQ(eType, PFT_Nomral);
}



void waitForResponse_stub()
{
    return;
}

bool responseCancelled_true_stub()
{
    return true;
}

bool responseSkip_true_stub()
{
    return true;
}

bool responseSkipAll_true_stub()
{
    return true;
}

bool responseOverwriteAll_true_stub()
{
    return true;
}

bool responseCancelled_false_stub()
{
    return false;
}

bool responseSkip_false_stub()
{
    return false;
}

bool responseSkipAll_false_stub()
{
    return false;
}

bool responseOverwriteAll_false_stub()
{
    return false;
}

TEST_F(TestLibBzip2Interface, testextractFiles)
{
    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/bz2/temp";

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    PluginFinishType eType = m_tester->extractFiles(QList<FileEntry>(), options);

    ASSERT_EQ(eType, PFT_Nomral);

    QDir dir(options.strTargetPath);
    dir.removeRecursively();
}

TEST_F(TestLibBzip2Interface, testpauseOperation)
{
    m_tester->m_bPause = false;
    m_tester->pauseOperation();
    bool bResult = (m_tester->m_bPause == true) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibBzip2Interface, testcontinueOperation)
{
    m_tester->m_bPause = true;
    m_tester->continueOperation();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibBzip2Interface, testdoKill)
{
    m_tester->m_bPause = true;
    m_tester->doKill();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibBzip2Interface, testuncompressedFileName)
{
    QString str = m_tester->uncompressedFileName();
    ASSERT_EQ(str, "test.txt");
}
