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

class TestLibzipPluginFactory : public QObject, public ::testing::Test
{
public:
    TestLibzipPluginFactory() {}

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

class TestLibzipPlugin : public QObject, public ::testing::Test
{
public:
    TestLibzipPlugin() {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
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


TEST_F(TestLibzipPluginFactory, initTest)
{

}

TEST_F(TestLibzipPlugin, initTest)
{

}

TEST_F(TestLibzipPlugin, testlist)
{
    qDebug() << "*gaoxiang*" << m_tester->m_strArchiveName;
    PluginFinishType eFinishType = m_tester->list();
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testtestArchive)
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

TEST_F(TestLibzipPlugin, testextractFiles_AllExtract)
{
    QList<FileEntry> files;
    ExtractionOptions options;
    options.bAllExtract = true;
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

TEST_F(TestLibzipPlugin, testextractFiles_PartExtract)
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

TEST_F(TestLibzipPlugin, testaddFiles)
{
    m_tester->m_strArchiveName = QFileInfo("../UnitTest/test_sources/zip/compress/test.zip").absoluteFilePath();
    QList<FileEntry> files;
    CompressOptions options;
    FileEntry entry;
    entry.strFullPath = QFileInfo("../UnitTest/test_sources/zip/compress/test.txt").absoluteFilePath();
    files << entry;
    entry.strFullPath = QFileInfo("../UnitTest/test_sources/zip/compress/test1.txt").absoluteFilePath();
    files << entry;

    options.strPassword = "11";
    options.iCompressionLevel = 3;
    options.bEncryption = true;


    PluginFinishType eFinishType = m_tester->addFiles(files, options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);

    QFile::remove(m_tester->m_strArchiveName);
}

TEST_F(TestLibzipPlugin, testmoveFiles)
{
    CompressOptions options;
    PluginFinishType eFinishType = m_tester->moveFiles(QList<FileEntry>(), options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testcopyFiles)
{
    CompressOptions options;
    PluginFinishType eFinishType = m_tester->copyFiles(QList<FileEntry>(), options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testdeleteFiles)
{
    QString strFile1 = QFileInfo("../UnitTest/test_sources/zip/delete/test.zip").absoluteFilePath();
    QString strFile2 = QFileInfo("../UnitTest/test_sources/zip/delete/testDelete.zip").absoluteFilePath();
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

    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testaddComment)
{
    PluginFinishType eFinishType = m_tester->addComment("sssss");
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testupdateArchiveData)
{
    UpdateOptions options;
    PluginFinishType eFinishType = m_tester->updateArchiveData(options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testpauseOperation)
{
    m_tester->m_bPause = false;
    m_tester->pauseOperation();
    bool bResult = (m_tester->m_bPause == true) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testcontinueOperation)
{
    m_tester->m_bPause = true;
    m_tester->continueOperation();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testdoKill)
{
    m_tester->m_bPause = true;
    m_tester->m_bCancel = false;
    m_tester->doKill();
    bool bResult = (m_tester->m_bPause == false && m_tester->m_bCancel == true) ? true : false;
    ASSERT_EQ(bResult, true);
}

TEST_F(TestLibzipPlugin, testwriteEntry)
{
    m_tester->m_strArchiveName = QFileInfo("../UnitTest/test_sources/zip/compress/temp/test1.zip").absoluteFilePath();
    QList<FileEntry> files;
    CompressOptions options;
    FileEntry entry;
    entry.strFullPath = QFileInfo("../UnitTest/test_sources/zip/compress/test.txt").absoluteFilePath();
    files << entry;

    options.iCompressionLevel = 3;

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    bool bResult = false;
    if (archive) {
        QString strPath = QFileInfo(entry.strFullPath).absolutePath() + QDir::separator();
        bResult = m_tester->writeEntry(archive, entry.strFullPath, options, false, strPath);
    }

    zip_close(archive);
    ASSERT_EQ(bResult, true);

    QFile::remove(m_tester->m_strArchiveName);
}

TEST_F(TestLibzipPlugin, testprogressCallback)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    if (archive) {
        m_tester->progressCallback(archive, 0.6, m_tester);
    }

    ASSERT_NE(archive, nullptr);

    zip_close(archive);
}

TEST_F(TestLibzipPlugin, testcancelCallback)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    if (archive) {
        m_tester->cancelCallback(archive, m_tester);
    }

    ASSERT_NE(archive, nullptr);

    zip_close(archive);
}

TEST_F(TestLibzipPlugin, testhandleArchiveData)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    bool bResult = false;
    if (archive) {
        bResult = m_tester->handleArchiveData(archive, 0);
    }

    ASSERT_EQ(bResult, true);

    zip_close(archive);
}

TEST_F(TestLibzipPlugin, teststatBuffer2FileEntry)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    bool bResult = false;
    if (archive) {
        zip_stat_t statBuffer;
        zip_stat_index(archive, zip_uint64_t(0), ZIP_FL_ENC_RAW, &statBuffer);
        FileEntry entry;
        entry.strFullPath = statBuffer.name;
        m_tester->statBuffer2FileEntry(statBuffer, entry);
        bResult = !entry.strFileName.isEmpty();
    }

    ASSERT_EQ(bResult, true);

    zip_close(archive);
}

TEST_F(TestLibzipPlugin, testextractEntry)
{
    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名

    bool bResult = false;
    if (archive) {
        ExtractionOptions options;
        options.bAllExtract = true;
        options.strTargetPath = QFileInfo("../UnitTest/test_sources/zip/extract/temp").absoluteFilePath();
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

    ASSERT_EQ(bResult, true);

    zip_close(archive);
}

TEST_F(TestLibzipPlugin, testemitProgress)
{
    m_tester->emitProgress(0.6);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestLibzipPlugin, testcancelResult001)
{
    m_tester->m_bCancel = true;
    int iResult = m_tester->cancelResult();
    ASSERT_EQ(iResult, 1);
}

TEST_F(TestLibzipPlugin, testcancelResult002)
{
    m_tester->m_bCancel = false;
    int iResult = m_tester->cancelResult();
    ASSERT_EQ(iResult, 0);
}

TEST_F(TestLibzipPlugin, testpasswordUnicode001)
{
    QString str = m_tester->passwordUnicode("哈哈", 0);
    ASSERT_EQ(str, "哈哈");
}

TEST_F(TestLibzipPlugin, testpasswordUnicode002)
{
    QString strPassword = "1";
    const char *commentstr = m_tester->passwordUnicode(strPassword, 0);
    ASSERT_EQ(commentstr, strPassword.toUtf8().constData());
}

TEST_F(TestLibzipPlugin, testdeleteEntry)
{
    QString strFile1 = QFileInfo("../UnitTest/test_sources/zip/delete/test.zip").absoluteFilePath();
    QString strFile2 = QFileInfo("../UnitTest/test_sources/zip/delete/testDelete.zip").absoluteFilePath();
    QFile::copy(strFile1, strFile2);
    m_tester->m_strArchiveName = strFile2;

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_tester->m_strArchiveName).constData(), 0, &errcode);

    bool bResult = false;
    if (archive) {
        bResult = m_tester->deleteEntry(0, archive);
    }
    ASSERT_EQ(bResult, true);

    zip_close(archive);
}

TEST_F(TestLibzipPlugin, testgetIndexBySelEntry)
{
    m_tester->list();
    ArchiveData stData = DataManager::get_instance().archiveData();

    FileEntry entry = stData.listRootEntry[0];
    m_tester->getIndexBySelEntry(QList<FileEntry>() << entry);

    bool bResult = false;
    if (m_tester->m_listCurIndex.count() > 0 && m_tester->m_listCurIndex[0] == 0) {
        bResult = true;
    }
    ASSERT_EQ(bResult, true);
}
