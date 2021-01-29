/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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

#include <gtest/gtest.h>
#include <gtest/src/stub.h>

#include "readonlylibarchiveplugin.h"
#include "queries.h"
#include "datamanager.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDebug>

Q_DECLARE_METATYPE(KPluginMetaData)

class TestReadOnlyLibarchivePluginFactory : public QObject, public ::testing::Test
{
public:
    TestReadOnlyLibarchivePluginFactory() {}

public:
    virtual void SetUp()
    {
        m_tester = new ReadOnlyLibarchivePluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ReadOnlyLibarchivePluginFactory *m_tester;
};

class TestReadOnlyLibarchivePlugin : public QObject, public ::testing::Test
{
public:
    TestReadOnlyLibarchivePlugin() {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("../UnitTest/test_sources/tar/extract/test.tar").absoluteFilePath();
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new ReadOnlyLibarchivePlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ReadOnlyLibarchivePlugin *m_tester;
};

TEST_F(TestReadOnlyLibarchivePluginFactory, initTest)
{

}

TEST_F(TestReadOnlyLibarchivePlugin, initTest)
{

}

TEST_F(TestReadOnlyLibarchivePlugin, testlist)
{
    ASSERT_EQ(m_tester->list(), PFT_Nomral);
}

TEST_F(TestReadOnlyLibarchivePlugin, testtestArchive)
{
    ASSERT_EQ(m_tester->testArchive(), PFT_Nomral);
}

bool initializeReader_stub()
{
    return false;
}

void waitForResponse_stub()
{
    return;
}

bool responseSkip_false_stub()
{
    return false;
}

bool responseSkipAll_false_stub()
{
    return false;
}

bool responseOverwriteAll_true_stub()
{
    return true;
}

bool responseCancelled_false_stub()
{
    return false;
}

TEST_F(TestReadOnlyLibarchivePlugin, testextractFiles)
{
    Stub stub;
    stub.set(ADDR(LibarchivePlugin, initializeReader), initializeReader_stub);

    QList<FileEntry> files;
    ExtractionOptions options;

    ASSERT_EQ(m_tester->extractFiles(files, options), PFT_Error);
}

TEST_F(TestReadOnlyLibarchivePlugin, testextractFiles1)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    QList<FileEntry> files;
    ExtractionOptions options;
    options.strTargetPath = QFileInfo("../UnitTest/test_sources/tar/extract").absoluteFilePath();

    ASSERT_EQ(m_tester->extractFiles(files, options), PFT_Nomral);
}

TEST_F(TestReadOnlyLibarchivePlugin, testextractFiles2)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    QList<FileEntry> files;
    ExtractionOptions options;
    options.strTargetPath = QFileInfo("../UnitTest/test_sources/tar/extract").absoluteFilePath();

    ASSERT_EQ(m_tester->extractFiles(files, options), PFT_Cancel);
}

bool mkpath_stub(const QString &dirPath)
{
    return false;
}
TEST_F(TestReadOnlyLibarchivePlugin, testextractFiles3)
{
    Stub stub;
    stub.set(ADDR(QDir, mkpath), mkpath_stub);
    QList<FileEntry> files;
    ExtractionOptions options;
    options.strTargetPath = QFileInfo("../UnitTest/test_sources/tar/extracterror").absoluteFilePath();

    ASSERT_EQ(m_tester->extractFiles(files, options), PFT_Error);
}

TEST_F(TestReadOnlyLibarchivePlugin, testextractFiles4)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    QList<FileEntry> files;
    FileEntry file1;
    file1.isDirectory = false;
    file1.strFullPath = QLatin1String("test.txt");
    files.push_back(file1);
    ExtractionOptions options;
    options.strTargetPath = QFileInfo("../UnitTest/test_sources/tar/extract").absoluteFilePath();
    m_tester->m_ArchiveEntryCount = 2;

    ASSERT_EQ(m_tester->extractFiles(files, options), PFT_Nomral);

    QFile::remove(options.strTargetPath + "/test.txt");
    QFile::remove(options.strTargetPath + "/test1.txt");
}

TEST_F(TestReadOnlyLibarchivePlugin, testaddFiles)
{
    QList<FileEntry> files;
    CompressOptions options;

    ASSERT_EQ(m_tester->addFiles(files, options), PFT_Error);
}

TEST_F(TestReadOnlyLibarchivePlugin, testcopyFiles)
{
    QList<FileEntry> files;
    CompressOptions options;

    ASSERT_EQ(m_tester->copyFiles(files, options), PFT_Error);
}

TEST_F(TestReadOnlyLibarchivePlugin, testdeleteFiles)
{
    QList<FileEntry> files;
    CompressOptions options;

    ASSERT_EQ(m_tester->deleteFiles(files), PFT_Error);
}

TEST_F(TestReadOnlyLibarchivePlugin, testaddComment)
{
    ASSERT_EQ(m_tester->addComment("comment"), PFT_Error);
}

TEST_F(TestReadOnlyLibarchivePlugin, testpauseOperation)
{
    m_tester->pauseOperation();
    ASSERT_EQ(m_tester->m_bPause, true);
}

TEST_F(TestReadOnlyLibarchivePlugin, testcontinueOperation)
{
    m_tester->continueOperation();
    ASSERT_EQ(m_tester->m_bPause, false);
}

TEST_F(TestReadOnlyLibarchivePlugin, testdoKill)
{
    bool ret = m_tester->doKill();
    ASSERT_EQ(m_tester->m_bPause, false);
    ASSERT_EQ(ret, false);
}

TEST_F(TestReadOnlyLibarchivePlugin, testinitializeReader)
{
    ASSERT_EQ(m_tester->initializeReader(), true);
}

int archive_read_support_filter_all_stub(struct archive *)
{
    return ARCHIVE_FAILED;
}
TEST_F(TestReadOnlyLibarchivePlugin, testinitializeReader1)
{
    Stub stub;
    stub.set(archive_read_support_filter_all, archive_read_support_filter_all_stub);
    ASSERT_EQ(m_tester->initializeReader(), false);
}

int archive_read_support_format_all_stub(struct archive *)
{
    return ARCHIVE_FAILED;
}

TEST_F(TestReadOnlyLibarchivePlugin, testinitializeReader2)
{
    Stub stub;
    stub.set(archive_read_support_format_all, archive_read_support_format_all_stub);
    ASSERT_EQ(m_tester->initializeReader(), false);
}

int archive_read_open_filename_stub(struct archive *)
{
    return ARCHIVE_FAILED;
}

TEST_F(TestReadOnlyLibarchivePlugin, testinitializeReader3)
{
    Stub stub;
    stub.set(archive_read_open_filename, archive_read_open_filename_stub);
    ASSERT_EQ(m_tester->initializeReader(), false);
}

TEST_F(TestReadOnlyLibarchivePlugin, testconvertCompressionName)
{
//    Stub stub;
//    stub.set(archive_read_support_format_all, archive_read_support_format_all_stub);

    const QString method = "gzip_error";
    ASSERT_EQ(m_tester->convertCompressionName(method).isEmpty(), true);
}

TEST_F(TestReadOnlyLibarchivePlugin, testconvertCompressionName1)
{
//    Stub stub;
//    stub.set(archive_read_support_format_all, archive_read_support_format_all_stub);

    const QString method = "zstd";
    ASSERT_EQ(m_tester->convertCompressionName(method).toStdString(), "Zstandard");
}

TEST_F(TestReadOnlyLibarchivePlugin, testupdateArchiveData)
{
    UpdateOptions options;
    options.eType = UpdateOptions::Delete;
    FileEntry file1;
    file1.isDirectory = false;
    file1.strFullPath = QLatin1String("test.txt");
    options.listEntry.push_back(file1);
    m_tester->list();
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    int mapFileEntrysizeold = stArchiveData.mapFileEntry.size();
    int listRootEntrysizeold = stArchiveData.listRootEntry.size();
    ASSERT_EQ(m_tester->updateArchiveData(options), PFT_Nomral);
    int mapFileEntrysizenew = stArchiveData.mapFileEntry.size();
    int listRootEntrysizenew = stArchiveData.listRootEntry.size();
    ASSERT_EQ(mapFileEntrysizeold, mapFileEntrysizenew + 1);
}

TEST_F(TestReadOnlyLibarchivePlugin, testextractionFlags)
{
    ASSERT_EQ(m_tester->extractionFlags(), 0x0204);
}
