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
        QString strFile = QFileInfo("test.txt.bz2").absoluteFilePath();
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


static bool qfile_exists_stub()
{
    return true;
}

TEST_F(TestLibBzip2Interface, testextractFiles)
{
    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = QFileInfo("/home/Desktop").absoluteFilePath();

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);

    typedef bool (QFile::*fptr)()const;
    fptr A_foo = (fptr)(&QFile::exists);   //获取虚函数地址
    stub.set(A_foo, qfile_exists_stub);

    Stub stub1;
    stub1.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_true_stub);
    stub1.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub1.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub1.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_false_stub);
    m_tester->extractFiles(QList<FileEntry>(), options);

    Stub stub2;
    stub2.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub2.set(ADDR(OverwriteQuery, responseSkip), responseSkip_true_stub);
    stub2.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub2.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_false_stub);
    m_tester->extractFiles(QList<FileEntry>(), options);

    Stub stub3;
    stub3.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub3.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub3.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_true_stub);
    stub3.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_false_stub);
    m_tester->extractFiles(QList<FileEntry>(), options);

    Stub stub4;
    stub4.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub4.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub4.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub4.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    m_tester->extractFiles(QList<FileEntry>(), options);

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
