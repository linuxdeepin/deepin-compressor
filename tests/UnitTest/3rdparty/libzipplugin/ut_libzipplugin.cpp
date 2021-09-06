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

#include "libzipplugin.h"
#include "gtest/src/stub.h"
#include "queries.h"
#include "datamanager.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QDebug>

Q_DECLARE_METATYPE(KPluginMetaData)

class UT_LibzipPluginFactory : public QObject, public ::testing::Test
{
public:
    UT_LibzipPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibzipPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibzipPluginFactory *m_tester;
};

class UT_LibzipPlugin : public QObject, public ::testing::Test
{
public:
    UT_LibzipPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/zip/extract/test.zip";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibzipPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibzipPlugin *m_tester;
};


TEST_F(UT_LibzipPluginFactory, initTest)
{

}

TEST_F(UT_LibzipPlugin, initTest)
{

}

TEST_F(UT_LibzipPlugin, test_list)
{
    PluginFinishType eFinishType = m_tester->list();
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_testArchive)
{
    PluginFinishType eFinishType = m_tester->testArchive();
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
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

TEST_F(UT_LibzipPlugin, test_extractFiles_001)
{
    QList<FileEntry> files;
    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/zip/extract/temp";

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    PluginFinishType eFinishType = m_tester->extractFiles(files, options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);

    QDir dir(options.strTargetPath);
    dir.removeRecursively();
}

TEST_F(UT_LibzipPlugin, test_extractFiles_002)
{
    m_tester->list();
    ArchiveData stData = DataManager::get_instance().archiveData();

    if (stData.listRootEntry.count() > 0) {
        QList<FileEntry> files;
        ExtractionOptions options;
        files << stData.listRootEntry[0];
        options.bAllExtract = false;
        options.strTargetPath = _UTSOURCEDIR;
        options.strTargetPath += "/test_sources/zip/extract/temp";

        Stub stub;
        stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
        stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
        stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
        stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
        stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

        PluginFinishType eFinishType = m_tester->extractFiles(files, options);
        bool bResult = (eFinishType == PFT_Nomral) ? true : false;
        EXPECT_EQ(bResult, true);

        QDir dir(options.strTargetPath);
        dir.removeRecursively();
    }
}

TEST_F(UT_LibzipPlugin, test_moveFiles)
{
    CompressOptions options;
    PluginFinishType eFinishType = m_tester->moveFiles(QList<FileEntry>(), options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_copyFiles)
{
    CompressOptions options;
    PluginFinishType eFinishType = m_tester->copyFiles(QList<FileEntry>(), options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_deleteFiles)
{
    QString str = _UTSOURCEDIR;
    QString strFile1 = str + "/test_sources/zip/delete/test.zip";
    QString strFile2 = str + "/test_sources/zip/delete/testDelete.zip";
    QFile::copy(strFile1, strFile2);
    bool bResult = true;
    if (QFileInfo(strFile2).exists()) {
        m_tester->m_strArchiveName = strFile2;
        m_tester->list();
        ArchiveData stData = DataManager::get_instance().archiveData();
        FileEntry entry = stData.listRootEntry[0];
        PluginFinishType eFinishType = m_tester->deleteFiles(QList<FileEntry>() << entry);
        bResult = (eFinishType == PFT_Nomral) ? true : false;
    }

    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_addComment)
{
    PluginFinishType eFinishType = m_tester->addComment("sssss");
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_updateArchiveData)
{
    UpdateOptions options;
    PluginFinishType eFinishType = m_tester->updateArchiveData(options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_pauseOperation)
{
    m_tester->m_bPause = false;
    m_tester->pauseOperation();
    bool bResult = (m_tester->m_bPause == true) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_continueOperation)
{
    m_tester->m_bPause = true;
    m_tester->continueOperation();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_doKill)
{
    m_tester->m_bPause = true;
    m_tester->m_bCancel = false;
    m_tester->doKill();
    bool bResult = (m_tester->m_bPause == false && m_tester->m_bCancel == true) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibzipPlugin, test_progressCallback)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    if (archive) {
        m_tester->progressCallback(archive, 0.6, m_tester);
    }

    EXPECT_NE(archive, nullptr);

    zip_close(archive);
}

TEST_F(UT_LibzipPlugin, test_cancelCallback)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    if (archive) {
        m_tester->cancelCallback(archive, m_tester);
    }

    EXPECT_NE(archive, nullptr);

    zip_close(archive);
}

TEST_F(UT_LibzipPlugin, test_handleArchiveData)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    bool bResult = false;
    if (archive) {
        bResult = m_tester->handleArchiveData(archive, 0);
    }

    EXPECT_EQ(bResult, true);

    zip_close(archive);
}

TEST_F(UT_LibzipPlugin, test_statBuffer2FileEntry)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名
    const auto nofEntries = zip_get_num_entries(archive, 0);
    qInfo() << "ssssssssssss" << nofEntries;
    bool bResult = false;
    if (archive && nofEntries > 0) {
        zip_stat_t statBuffer;
        zip_stat_index(archive, zip_uint64_t(0), ZIP_FL_ENC_RAW, &statBuffer);
        FileEntry entry;

        entry.strFullPath = statBuffer.name;
        m_tester->statBuffer2FileEntry(statBuffer, entry);
        bResult = !entry.strFileName.isEmpty();
        EXPECT_EQ(bResult, true);
    } else {
        EXPECT_EQ(bResult, false);
    }


    zip_close(archive);
}

TEST_F(UT_LibzipPlugin, test_extractEntry)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    bool bResult = false;
    if (archive) {
        ExtractionOptions options;
        options.bAllExtract = true;
        options.strTargetPath = _UTSOURCEDIR;
        options.strTargetPath += "/test_sources/zip/extract/temp";
        qlonglong qExtractSize = 0;
        QString strFileName;

        Stub stub;
        stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
        stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
        stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
        stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
        stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

        ErrorType eType = m_tester->extractEntry(archive, 0, options, qExtractSize, strFileName);
        bResult = (eType == ET_NoError) ? true : false;

        QDir dir(options.strTargetPath);
        dir.removeRecursively();
    }

    EXPECT_EQ(bResult, true);

    zip_close(archive);
}

TEST_F(UT_LibzipPlugin, test_emitProgress)
{
    m_tester->emitProgress(0.6);
    EXPECT_NE(m_tester, nullptr);
}

TEST_F(UT_LibzipPlugin, test_cancelResult001)
{
    m_tester->m_bCancel = true;
    int iResult = m_tester->cancelResult();
    EXPECT_EQ(iResult, 1);
}

TEST_F(UT_LibzipPlugin, test_cancelResult002)
{
    m_tester->m_bCancel = false;
    int iResult = m_tester->cancelResult();
    EXPECT_EQ(iResult, 0);
}

TEST_F(UT_LibzipPlugin, test_passwordUnicode001)
{
    EXPECT_EQ(m_tester->passwordUnicode("hh", 0), QString("hh"));
}

TEST_F(UT_LibzipPlugin, test_deleteEntry)
{
    QString str = _UTSOURCEDIR;
    QString strFile1 = str + "/test_sources/zip/delete/test.zip";
    QString strFile2 = str + "/test_sources/zip/delete/testDelete.zip";
    QFile::copy(strFile1, strFile2);
    m_tester->m_strArchiveName = strFile2;

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), 0, &errcode);

    bool bResult = false;
    if (archive) {
        bResult = m_tester->deleteEntry(0, archive);
    }
    EXPECT_EQ(bResult, true);

    zip_close(archive);
}

TEST_F(UT_LibzipPlugin, test_getIndexBySelEntry)
{
    m_tester->list();
    ArchiveData stData = DataManager::get_instance().archiveData();

    if (stData.listRootEntry.count() > 0) {
        FileEntry entry = stData.listRootEntry[0];
        m_tester->getIndexBySelEntry(QList<FileEntry>() << entry);

        bool bResult = false;
        if (m_tester->m_listCurIndex.count() > 0 && m_tester->m_listCurIndex[0] == 0) {
            bResult = true;
        }
        EXPECT_EQ(bResult, true);
    }

}
