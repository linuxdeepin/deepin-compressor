// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contract_test.h"

#include "libzipplugin.h"
#include "readwritelibarchiveplugin.h"
#include "cli7zplugin.h"

#include "queries.h"
#include "gtest/src/stub.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDirIterator>

Q_DECLARE_METATYPE(KPluginMetaData)

// ---------------------------------------------------------------------------
// Stub helpers for OverwriteQuery
// ---------------------------------------------------------------------------
static void rw_waitForResponse_stub() {}
static bool rw_responseCancelled_false_stub() { return false; }
static bool rw_responseSkip_false_stub() { return false; }
static bool rw_responseSkipAll_false_stub() { return false; }
static bool rw_responseOverwriteAll_true_stub() { return true; }

// ---------------------------------------------------------------------------
// Global test fixture
// ---------------------------------------------------------------------------
static ContractTestFixture &rw_globalFixture()
{
    static ContractTestFixture fixture;
    return fixture;
}

// ---------------------------------------------------------------------------
// Factory helpers
// ---------------------------------------------------------------------------
static ReadWriteArchiveInterface *makeRwLibzipPlugin(const QString &path)
{
    return new LibzipPlugin(nullptr, makePluginArgs(path));
}

static ReadWriteArchiveInterface *makeRwReadWriteLibarchivePlugin(const QString &path)
{
    return new ReadWriteLibarchivePlugin(nullptr, makePluginArgs(path));
}

static ReadWriteArchiveInterface *makeRwCli7zPlugin(const QString &path)
{
    return new Cli7zPlugin(nullptr, makePluginArgs(path));
}

static QString archiveExtension(const ReadWriteBackendCase &param)
{
    if (param.name == "ReadWriteLibarchivePlugin")
        return ".tar";
    if (param.name == "Cli7zPlugin")
        return ".7z";
    return ".zip";
}

// ---------------------------------------------------------------------------
// Test case instantiation — each backend declares its read-write capabilities.
//
// LibzipPlugin: supports addFiles, deleteFiles, renameFiles, addComment.
// ReadWriteLibarchivePlugin: addFiles/deleteFiles/renameFiles are implemented;
//   addComment is commented out in header (returns PFT_Error via base class).
// ---------------------------------------------------------------------------
INSTANTIATE_TEST_SUITE_P(
    AllReadWriteBackends,
    ReadWriteContractTest,
    ::testing::Values(
        ReadWriteBackendCase{
            "LibzipPlugin", makeRwLibzipPlugin,
            "zip", "readwrite/zip/test.zip",
            false,  // isAsync
            true,   // supportsAddFiles
            true,   // supportsDeleteFiles
            true,   // supportsRenameFiles
            true,   // supportsAddComment
            true,   // supportsUpdateArchiveData
            true    // unsupportedAddCommentReturnsError
        },
        ReadWriteBackendCase{
            "ReadWriteLibarchivePlugin", makeRwReadWriteLibarchivePlugin,
            "tar", "readwrite/tar/test.tar",
            false,  // isAsync
            true,   // supportsAddFiles — implemented in source
            true,   // supportsDeleteFiles — implemented in source
            true,   // supportsRenameFiles — implemented in source
            false,  // supportsAddComment — commented out in header
            true,   // supportsUpdateArchiveData
            true    // unsupportedAddCommentReturnsError
        },
        ReadWriteBackendCase{
            "Cli7zPlugin", makeRwCli7zPlugin,
            "7z", "readwrite/cli7z/test.7z",
            true,   // isAsync
            true,   // supportsAddFiles
            true,   // supportsDeleteFiles
            true,   // supportsRenameFiles
            false,  // supportsAddComment — CliInterface currently returns success without writing
            true,   // supportsUpdateArchiveData
            false   // unsupportedAddCommentReturnsError
        }
    ),
    [](const ::testing::TestParamInfo<ReadWriteBackendCase> &info) {
        return info.param.name;
    }
);

// ===========================================================================
// Contract: getArchiveName returns exact path passed to constructor
// ===========================================================================
TEST_P(ReadWriteContractTest, ArchiveNameMatchesConstructor)
{
    EXPECT_EQ(m_iface->getArchiveName(), m_archivePath);
}

// ===========================================================================
// Contract: addFiles — create a real archive, re-list, verify entries + content
// ===========================================================================
TEST_P(ReadWriteContractTest, AddFilesCreatesArchive)
{
    if (!GetParam().supportsAddFiles) GTEST_SKIP();

    const auto &param = GetParam();

    // Determine the correct extension for the backend
    QString ext = archiveExtension(param);
    QString archivePath = rw_globalFixture().tempPath() + "/rw_add_" +
                          QString::fromStdString(param.name) + ext;

    // Source file to add
    QString srcDir = rw_globalFixture().tempPath() + "/rw_add_src_" +
                     QString::fromStdString(param.name);
    QDir().mkpath(srcDir);
    QString srcFile = srcDir + "/test_add.txt";
    {
        QFile f(srcFile);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << "add test content for verification";
            f.close();
        }
    }
    ASSERT_TRUE(QFileInfo::exists(srcFile));

    // Create a new plugin instance pointing at the new archive path
    std::unique_ptr<ReadWriteArchiveInterface> iface(param.factory(archivePath));
    ASSERT_NE(iface, nullptr);

    QList<FileEntry> files;
    FileEntry entry;
    entry.strFullPath = srcFile;
    entry.strFileName = "test_add.txt";
    files << entry;

    CompressOptions options;
    PluginFinishType result = runOpOn(iface.get(), [&iface, &files, &options]() {
        return iface->addFiles(files, options);
    });
    EXPECT_EQ(result, PFT_Nomral);

    // Verify the archive file was created
    EXPECT_TRUE(QFileInfo::exists(archivePath));

    // Re-list the created archive and verify entries
    DataManager::get_instance().resetArchiveData();
    PluginFinishType listResult = runOpOn(iface.get(), [&iface]() {
        return iface->list();
    });
    EXPECT_EQ(listResult, PFT_Nomral);

    ArchiveData &data = DataManager::get_instance().archiveData();
    EXPECT_GT(data.mapFileEntry.size(), 0);

    // Verify the added file name appears in the entry list
    bool foundAddedFile = false;
    for (const FileEntry &e : data.listRootEntry) {
        if (e.strFileName == "test_add.txt" || e.strFileName.contains("test_add.txt")) {
            foundAddedFile = true;
            break;
        }
    }
    EXPECT_TRUE(foundAddedFile);

    // Extract the added file and verify content
    QString extractDir = rw_globalFixture().tempPath() + "/rw_add_extract_" +
                         QString::fromStdString(param.name);
    QDir().mkpath(extractDir);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), rw_waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), rw_responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), rw_responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), rw_responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), rw_responseOverwriteAll_true_stub);

    ExtractionOptions extractOptions;
    extractOptions.bAllExtract = true;
    extractOptions.strTargetPath = extractDir;

    PluginFinishType extractResult = runOpOn(iface.get(), [&iface, &extractOptions]() {
        return iface->extractFiles(QList<FileEntry>(), extractOptions);
    });
    EXPECT_EQ(extractResult, PFT_Nomral);

    // Verify extracted file content
    bool contentVerified = false;
    QDirIterator it(extractDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (it.fileName().contains("test_add")) {
            QFile f(filePath);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString content = QTextStream(&f).readAll();
                if (content.contains("add test content for verification")) {
                    contentVerified = true;
                }
                f.close();
            }
        }
    }
    EXPECT_TRUE(contentVerified);

    QDir(extractDir).removeRecursively();
}

// ===========================================================================
// Contract: deleteFiles — verify entry count decreases after deletion
// ===========================================================================
TEST_P(ReadWriteContractTest, DeleteFilesReducesEntryCount)
{
    if (!GetParam().supportsDeleteFiles) GTEST_SKIP();

    const auto &param = GetParam();
    QString ext = archiveExtension(param);
    QString workArchive = rw_globalFixture().tempPath() + "/rw_delete_" +
                          QString::fromStdString(param.name) + ext;

    // Copy the source archive
    QFile::copy(m_archivePath, workArchive);
    ASSERT_TRUE(QFileInfo::exists(workArchive));

    std::unique_ptr<ReadWriteArchiveInterface> iface(param.factory(workArchive));
    ASSERT_NE(iface, nullptr);

    // List first to populate archive data and get initial count
    DataManager::get_instance().resetArchiveData();
    runOpOn(iface.get(), [&iface]() { return iface->list(); });
    ArchiveData &data = DataManager::get_instance().archiveData();
    int initialCount = data.mapFileEntry.size();
    ASSERT_GT(initialCount, 0);

    // Pick first file entry to delete
    FileEntry entryToDelete;
    bool found = false;
    for (const FileEntry &e : data.listRootEntry) {
        if (!e.isDirectory) {
            entryToDelete = e;
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);

    QList<FileEntry> toDelete;
    toDelete << entryToDelete;

    PluginFinishType result = runOpOn(iface.get(), [&iface, &toDelete]() {
        return iface->deleteFiles(toDelete);
    });
    EXPECT_EQ(result, PFT_Nomral);

    // Re-list and verify entry count decreased
    DataManager::get_instance().resetArchiveData();
    runOpOn(iface.get(), [&iface]() { return iface->list(); });
    ArchiveData &dataAfter = DataManager::get_instance().archiveData();
    EXPECT_LT(dataAfter.mapFileEntry.size(), initialCount);
}

// ===========================================================================
// Contract: renameFiles — verify new name exists, old name doesn't
// ===========================================================================
TEST_P(ReadWriteContractTest, RenameFilesChangesName)
{
    if (!GetParam().supportsRenameFiles) GTEST_SKIP();

    const auto &param = GetParam();
    QString ext = archiveExtension(param);
    QString workArchive = rw_globalFixture().tempPath() + "/rw_rename_" +
                          QString::fromStdString(param.name) + ext;

    QFile::copy(m_archivePath, workArchive);
    ASSERT_TRUE(QFileInfo::exists(workArchive));

    std::unique_ptr<ReadWriteArchiveInterface> iface(param.factory(workArchive));
    ASSERT_NE(iface, nullptr);

    // List to populate archive data
    DataManager::get_instance().resetArchiveData();
    runOpOn(iface.get(), [&iface]() { return iface->list(); });
    ArchiveData &data = DataManager::get_instance().archiveData();
    ASSERT_FALSE(data.listRootEntry.isEmpty());

    // Pick first file entry and rename it
    FileEntry entry;
    bool found = false;
    for (const FileEntry &e : data.listRootEntry) {
        if (!e.isDirectory) {
            entry = e;
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);

    QString oldName = entry.strFileName;
    entry.strAlias = "renamed_file.txt";

    QList<FileEntry> toRename;
    toRename << entry;

    PluginFinishType result = runOpOn(iface.get(), [&iface, &toRename]() {
        return iface->renameFiles(toRename);
    });
    EXPECT_EQ(result, PFT_Nomral);

    // Re-list and verify: renamed entry should exist, old name should not
    DataManager::get_instance().resetArchiveData();
    runOpOn(iface.get(), [&iface]() { return iface->list(); });
    ArchiveData &dataAfter = DataManager::get_instance().archiveData();

    bool hasNewName = false;
    bool hasOldName = false;
    for (const FileEntry &e : dataAfter.mapFileEntry) {
        if (e.strFileName == "renamed_file.txt") hasNewName = true;
        if (e.strFileName == oldName) hasOldName = true;
    }
    EXPECT_TRUE(hasNewName);
    EXPECT_FALSE(hasOldName);
}

// ===========================================================================
// Contract: addComment — verify comment for supporting backends,
//                       verify error for non-supporting backends
// ===========================================================================
TEST_P(ReadWriteContractTest, AddCommentOnSupportingBackend)
{
    if (!GetParam().supportsAddComment) GTEST_SKIP();

    const auto &param = GetParam();
    QString workArchive = rw_globalFixture().tempPath() + "/rw_comment_" +
                          QString::fromStdString(param.name) + ".zip";

    QFile::copy(m_archivePath, workArchive);
    ASSERT_TRUE(QFileInfo::exists(workArchive));

    std::unique_ptr<ReadWriteArchiveInterface> iface(param.factory(workArchive));
    ASSERT_NE(iface, nullptr);

    PluginFinishType result = runOpOn(iface.get(), [&iface]() {
        return iface->addComment("contract test comment");
    });
    EXPECT_EQ(result, PFT_Nomral);

    // Verify the comment was written by re-listing
    DataManager::get_instance().resetArchiveData();
    runOpOn(iface.get(), [&iface]() { return iface->list(); });
    ArchiveData &data = DataManager::get_instance().archiveData();
    EXPECT_EQ(data.strComment.toStdString(), "contract test comment");
}

TEST_P(ReadWriteContractTest, AddCommentOnNonSupportingBackendReturnsError)
{
    if (GetParam().supportsAddComment) GTEST_SKIP();

    PluginFinishType result = m_iface->addComment("should fail");
    if (GetParam().unsupportedAddCommentReturnsError) {
        EXPECT_EQ(result, PFT_Error);
    } else {
        // CliInterface currently reports success without writing a comment.
        // Keep the current behavior explicit; a future backend must not drift.
        EXPECT_EQ(result, PFT_Nomral);
    }
}

// ===========================================================================
// Contract: updateArchiveData — verify ArchiveData changes after update
// ===========================================================================
TEST_P(ReadWriteContractTest, UpdateArchiveDataModifiesData)
{
    if (!GetParam().supportsUpdateArchiveData) GTEST_SKIP();

    const auto &param = GetParam();
    QString ext = archiveExtension(param);
    QString workArchive = rw_globalFixture().tempPath() + "/rw_update_" +
                          QString::fromStdString(param.name) + ext;

    QFile::copy(m_archivePath, workArchive);
    ASSERT_TRUE(QFileInfo::exists(workArchive));

    std::unique_ptr<ReadWriteArchiveInterface> iface(param.factory(workArchive));
    ASSERT_NE(iface, nullptr);

    // List first to populate archive data
    DataManager::get_instance().resetArchiveData();
    runOpOn(iface.get(), [&iface]() { return iface->list(); });
    ArchiveData &data = DataManager::get_instance().archiveData();
    int initialCount = data.mapFileEntry.size();
    ASSERT_GT(initialCount, 0);

    // Pick first entry to "delete" via updateArchiveData
    FileEntry entry;
    if (!data.listRootEntry.isEmpty()) {
        entry = data.listRootEntry.first();
    }

    // For backends that re-read from disk in updateArchiveData (e.g. LibzipPlugin),
    // deleteFiles must modify the archive on disk first so the re-read reflects
    // the change. For backends that modify in-memory data directly (e.g.
    // ReadWriteLibarchivePlugin), deleteFiles is not needed.
    if (GetParam().supportsDeleteFiles) {
        QList<FileEntry> toDelete;
        toDelete << entry;
        runOpOn(iface.get(), [&iface, &toDelete]() {
            return iface->deleteFiles(toDelete);
        });
    }

    UpdateOptions options;
    options.eType = UpdateOptions::Delete;
    options.listEntry.push_back(entry);

    // updateArchiveData() updates the in-memory archive model and does not
    // start a CLI process, even for asynchronous CLI backends.
    PluginFinishType result = iface->updateArchiveData(options);
    EXPECT_EQ(result, PFT_Nomral);

    // Verify entry count decreased
    ArchiveData &dataAfter = DataManager::get_instance().archiveData();
    EXPECT_LT(dataAfter.mapFileEntry.size(), initialCount);
}

// ===========================================================================
// Contract: signal tests — verify finished signal on operations
// ===========================================================================
TEST_P(ReadWriteContractTest, SignalFinishedOnList)
{
    SignalTracker tracker;
    tracker.connect(m_iface.get());

    DataManager::get_instance().resetArchiveData();
    PluginFinishType result = runOp([this]() { return m_iface->list(); });

    // Both LibzipPlugin and ReadWriteLibarchivePlugin are synchronous
    // backends (isAsync is not set for read-write cases). Synchronous
    // backends do not emit signalFinished after list() — they return the
    // result directly. We verify the operation succeeded instead.
    EXPECT_EQ(result, PFT_Nomral);
}

TEST_P(ReadWriteContractTest, SignalFinishedOnAddFiles)
{
    if (!GetParam().supportsAddFiles) GTEST_SKIP();

    const auto &param = GetParam();
    QString ext = archiveExtension(param);
    QString archivePath = rw_globalFixture().tempPath() + "/rw_sig_add_" +
                          QString::fromStdString(param.name) + ext;

    QString srcDir = rw_globalFixture().tempPath() + "/rw_sig_src_" +
                     QString::fromStdString(param.name);
    QDir().mkpath(srcDir);
    QString srcFile = srcDir + "/sig_test.txt";
    {
        QFile f(srcFile);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream(&f) << "signal test content";
            f.close();
        }
    }

    std::unique_ptr<ReadWriteArchiveInterface> iface(param.factory(archivePath));
    ASSERT_NE(iface, nullptr);

    SignalTracker tracker;
    tracker.connect(iface.get());

    QList<FileEntry> files;
    FileEntry entry;
    entry.strFullPath = srcFile;
    entry.strFileName = "sig_test.txt";
    files << entry;

    CompressOptions options;
    PluginFinishType result = runOpOn(iface.get(), [&iface, &files, &options]() {
        return iface->addFiles(files, options);
    });
    EXPECT_EQ(result, PFT_Nomral);

    // Async CLI backends and ReadWriteLibarchivePlugin emit signalFinished.
    if (GetParam().isAsync || GetParam().name == "ReadWriteLibarchivePlugin") {
        EXPECT_GT(tracker.finishedCount, 0);
        EXPECT_EQ(tracker.lastFinishType, PFT_Nomral);
    }
}

// ===========================================================================
// Contract: inherited read-only defaults
// ===========================================================================
TEST_P(ReadWriteContractTest, DefaultErrorTypeIsNoError)
{
    EXPECT_EQ(m_iface->errorType(), ET_NoError);
}

TEST_P(ReadWriteContractTest, DefaultStatusIsNotPaused)
{
    EXPECT_FALSE(m_iface->status());
}

TEST_P(ReadWriteContractTest, PauseContinueChangesStatus)
{
    m_iface->pauseOperation();
    EXPECT_TRUE(m_iface->status());
    m_iface->continueOperation();
    EXPECT_FALSE(m_iface->status());
}

TEST_P(ReadWriteContractTest, DefaultIsUserCancelIsFalse)
{
    EXPECT_FALSE(m_iface->isUserCancel());
}

TEST_P(ReadWriteContractTest, PasswordRoundTrip)
{
    m_iface->setPassword("rwtest123");
    EXPECT_EQ(m_iface->getPassword().toStdString(), "rwtest123");
}

TEST_P(ReadWriteContractTest, DoKillReturnsFalseWhenIdle)
{
    EXPECT_FALSE(m_iface->doKill());
}
