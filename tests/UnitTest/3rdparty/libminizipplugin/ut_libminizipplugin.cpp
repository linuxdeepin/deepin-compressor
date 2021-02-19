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

#include "libminizipplugin.h"
#include "gtest/src/stub.h"
#include "queries.h"
#include "datamanager.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QDebug>

#define MAX_FILENAME 512

Q_DECLARE_METATYPE(KPluginMetaData)

class TestLibminizipPluginFactory : public QObject, public ::testing::Test
{
public:
    TestLibminizipPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibminizipPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibminizipPluginFactory *m_tester;
};

class TestLibminizipPlugin : public QObject, public ::testing::Test
{
public:
    TestLibminizipPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("../UnitTest/test_sources/crx/test.crx").absoluteFilePath();
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibminizipPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibminizipPlugin *m_tester;
};


TEST_F(TestLibminizipPluginFactory, initTest)
{

}

TEST_F(TestLibminizipPlugin, initTest)
{

}

TEST_F(TestLibminizipPlugin, testlist)
{
    PluginFinishType eFinishType = m_tester->list();
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibminizipPlugin, testtestArchive)
{
    PluginFinishType eFinishType = m_tester->testArchive();
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);
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

TEST_F(TestLibminizipPlugin, testextractFiles_AllExtract)
{
    QList<FileEntry> files;
    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = QFileInfo("../UnitTest/test_sources/crx/temp").absoluteFilePath();

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    PluginFinishType eFinishType = m_tester->extractFiles(files, options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);

    QDir dir(options.strTargetPath);
    dir.removeRecursively();
}

TEST_F(TestLibminizipPlugin, testextractFiles_PartExtract)
{
    m_tester->list();
    ArchiveData stData = DataManager::get_instance().archiveData();
    QList<FileEntry> files;
    ExtractionOptions options;
    files << stData.listRootEntry[0];
    options.bAllExtract = false;
    options.strTargetPath = QFileInfo("../UnitTest/test_sources/zip/extract/temp").absoluteFilePath();

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    PluginFinishType eFinishType = m_tester->extractFiles(files, options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);

    QDir dir(options.strTargetPath);
    dir.removeRecursively();
}

TEST_F(TestLibminizipPlugin, testpauseOperation)
{
    m_tester->m_bPause = false;
    m_tester->pauseOperation();
    bool bResult = (m_tester->m_bPause == true) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibminizipPlugin, testcontinueOperation)
{
    m_tester->m_bPause = true;
    m_tester->continueOperation();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibminizipPlugin, testdoKill)
{
    m_tester->m_bPause = true;
    m_tester->m_bCancel = false;
    m_tester->doKill();
    bool bResult = (m_tester->m_bPause == false && m_tester->m_bCancel == true) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibminizipPlugin, testhandleArchiveData)
{
    unzFile zipfile = unzOpen(QFile::encodeName(m_tester->m_strArchiveName).constData());
    bool bResult = m_tester->handleArchiveData(zipfile);
    ASSERT_EQ(bResult, true);
    unzClose(zipfile);
}

TEST_F(TestLibminizipPlugin, testextractEntry)
{
    unzFile zipfile = unzOpen(QFile::encodeName(m_tester->m_strArchiveName).constData());

    unz_file_info file_info;
    char filename[ MAX_FILENAME ];
    ErrorType eType = ET_NoError;
    if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
        unzClose(zipfile);
//        eType = ET_FileWriteError;
    }

    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = QFileInfo("../UnitTest/test_sources/crx/temp").absoluteFilePath();
    qlonglong qExtractSize = 0;
    QString strFileName = filename;
    eType = m_tester->extractEntry(zipfile, file_info, options, qExtractSize, strFileName);

    ASSERT_EQ(eType, ET_NoError);
    unzClose(zipfile);

    QDir dir(options.strTargetPath);
    dir.removeRecursively();
}

TEST_F(TestLibminizipPlugin, testgetSelFiles)
{
    m_tester->list();
    ArchiveData stData = DataManager::get_instance().archiveData();
    FileEntry entry;
    if (stData.listRootEntry.count() > 0)
        entry = stData.listRootEntry[0];

    QStringList listFiles = m_tester->getSelFiles(QList<FileEntry>() << entry);

    bool bResult = false;
    if (listFiles.count() > 0 && listFiles[0] == entry.strFullPath) {
        bResult = true;
    }

    ASSERT_EQ(bResult, true);
}
