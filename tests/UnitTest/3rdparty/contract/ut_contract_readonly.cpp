// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contract_test.h"

#include "libzipplugin.h"
#include "libminizipplugin.h"
#include "readonlylibarchiveplugin.h"
#include "readwritelibarchiveplugin.h"
#include "gzplugin.h"
#include "bz2plugin.h"
#include "xzplugin.h"
#include "cli7zplugin.h"
#include "clirarplugin.h"
#include "libpigzplugin.h"

#include "queries.h"
#include "gtest/src/stub.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDirIterator>

Q_DECLARE_METATYPE(KPluginMetaData)

// ---------------------------------------------------------------------------
// Stub helpers for OverwriteQuery (used by extract tests)
// ---------------------------------------------------------------------------
static void waitForResponse_stub() {}
static bool responseCancelled_false_stub() { return false; }
static bool responseCancelled_true_stub() { return true; }
static bool responseSkip_false_stub() { return false; }
static bool responseSkipAll_false_stub() { return false; }
static bool responseOverwriteAll_true_stub() { return true; }

// ---------------------------------------------------------------------------
// Stub helpers for PasswordNeededQuery (used by password-protected extract tests)
// ---------------------------------------------------------------------------
static void pw_waitForResponse_stub() {}
static bool pw_responseCancelled_true_stub() { return true; }
static bool pw_responseCancelled_false_stub() { return false; }
static QString pw_password_secret_stub() { return "secret"; }
static QString pw_password_wrong_stub() { return "wrongpassword"; }

// ---------------------------------------------------------------------------
// Global test fixture — creates real archives on disk for all backends
// ---------------------------------------------------------------------------
static ContractTestFixture &globalFixture()
{
    static ContractTestFixture fixture;
    return fixture;
}

// ---------------------------------------------------------------------------
// Factory helpers — each creates a plugin instance using real archive paths.
// ---------------------------------------------------------------------------

static ReadOnlyArchiveInterface *makeLibzipPlugin(const QString &path)
{
    return new LibzipPlugin(nullptr, makePluginArgs(path));
}

static ReadOnlyArchiveInterface *makeLibminizipPlugin(const QString &path)
{
    return new LibminizipPlugin(nullptr, makePluginArgs(path));
}

static ReadOnlyArchiveInterface *makeReadOnlyLibarchivePlugin(const QString &path)
{
    return new ReadOnlyLibarchivePlugin(nullptr, makePluginArgs(path));
}

static ReadOnlyArchiveInterface *makeGzPlugin(const QString &path)
{
    return new LibGzipInterface(nullptr, makePluginArgs(path));
}

static ReadOnlyArchiveInterface *makeBz2Plugin(const QString &path)
{
    return new LibBzip2Interface(nullptr, makePluginArgs(path));
}

static ReadOnlyArchiveInterface *makeXzPlugin(const QString &path)
{
    return new LibXzInterface(nullptr, makePluginArgs(path));
}

static ReadOnlyArchiveInterface *makeCli7zPlugin(const QString &path)
{
    return new Cli7zPlugin(nullptr, makePluginArgs(path));
}

static ReadOnlyArchiveInterface *makeCliRarPlugin(const QString &path)
{
    return new CliRarPlugin(nullptr, makePluginArgs(path));
}

// ---------------------------------------------------------------------------
// Test case instantiation — each backend declares its capabilities.
// A future self-developed backend only needs to add its case here.
// ---------------------------------------------------------------------------
INSTANTIATE_TEST_SUITE_P(
    AllReadOnlyBackends,
    ReadOnlyContractTest,
    ::testing::Values(
        // LibzipPlugin — full read-only support, supports corrupt + password tests
        ReadOnlyBackendCase{
            "LibzipPlugin", makeLibzipPlugin,
            "zip", "readonly/zip/test.zip",
            false,  // isAsync
            true, true, true,   // list, testArchive, extractFiles
            true,   // canTestCorrupt
            true,   // canTestPasswordArchive
            true    // canExtractWrongPassword
        },
        ReadOnlyBackendCase{
            "LibminizipPlugin", makeLibminizipPlugin,
            "zip", "readonly/minizip/test.zip",
            false, true, true, true, true, false, false
        },
        ReadOnlyBackendCase{
            "ReadOnlyLibarchivePlugin", makeReadOnlyLibarchivePlugin,
            "tar", "readonly/tar/test.tar",
            false, true, true, true, true, false, false
        },
        ReadOnlyBackendCase{
            "LibGzipInterface", makeGzPlugin,
            "gz", "readonly/gz/test.gz",
            false, true, true, true, false, false, false
        },
        ReadOnlyBackendCase{
            "LibBzip2Interface", makeBz2Plugin,
            "bz2", "readonly/bz2/test.txt.bz2",
            false, true, true, true, false, false, false
        },
        ReadOnlyBackendCase{
            "LibXzInterface", makeXzPlugin,
            "xz", "readonly/xz/test.xz",
            false, true, true, true, false, false, false
        },
        // Cli7zPlugin — async, uses real 7z binary for list/extract;
        // testArchive() is a sync no-op returning PFT_Nomral
        ReadOnlyBackendCase{
            "Cli7zPlugin", makeCli7zPlugin,
            "7z", "readonly/cli7z/test.7z",
            true,   // isAsync — list/extract spawn real process
            true,   // supportsList
            true,   // supportsTestArchive (no-op, returns PFT_Nomral)
            true,   // supportsExtractFiles
            false, false, false
        },
        // CliRarPlugin — async, uses real unrar binary for list/extract;
        // testArchive() is a sync no-op
        ReadOnlyBackendCase{
            "CliRarPlugin", makeCliRarPlugin,
            "rar", "readonly/cliRar/test.rar",
            true,   // isAsync
            true, true, true,
            false, false, false
        }
    ),
    [](const ::testing::TestParamInfo<ReadOnlyBackendCase> &info) {
        return info.param.name;
    }
);

// ===========================================================================
// Contract: list() — load archive and verify entry data
// Verifies: PluginFinishType, entry count, entry names, directory flag
// ===========================================================================
TEST_P(ReadOnlyContractTest, ListLoadsArchive)
{
    if (!GetParam().supportsList) GTEST_SKIP();

    PluginFinishType result = runOp([this]() { return m_iface->list(); });
    EXPECT_EQ(result, PFT_Nomral);
}

TEST_P(ReadOnlyContractTest, ListEntriesNotEmpty)
{
    if (!GetParam().supportsList) GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    ArchiveData &data = DataManager::get_instance().archiveData();
    EXPECT_FALSE(data.listRootEntry.isEmpty());
}

TEST_P(ReadOnlyContractTest, ListEntriesHaveCorrectCount)
{
    if (!GetParam().supportsList) GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    ArchiveData &data = DataManager::get_instance().archiveData();
    // All test archives contain at least 1 file entry
    EXPECT_GT(data.mapFileEntry.size(), 0);
}

TEST_P(ReadOnlyContractTest, ListEntriesHaveNames)
{
    if (!GetParam().supportsList) GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    ArchiveData &data = DataManager::get_instance().archiveData();
    for (const FileEntry &entry : data.listRootEntry) {
        EXPECT_FALSE(entry.strFileName.isEmpty());
    }
}

TEST_P(ReadOnlyContractTest, ListEntriesHaveDirectoryFlag)
{
    if (!GetParam().supportsList) GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    ArchiveData &data = DataManager::get_instance().archiveData();
    // At least one non-directory entry should exist (files were added)
    bool hasFileEntry = false;
    for (const FileEntry &entry : data.listRootEntry) {
        if (!entry.isDirectory) {
            hasFileEntry = true;
            break;
        }
    }
    EXPECT_TRUE(hasFileEntry);
}

// ===========================================================================
// Contract: testArchive() — verify archive integrity
// ===========================================================================
TEST_P(ReadOnlyContractTest, TestArchiveNormalSucceeds)
{
    if (!GetParam().supportsTestArchive) GTEST_SKIP();

    PluginFinishType result = m_iface->testArchive();
    EXPECT_EQ(result, PFT_Nomral);
}

TEST_P(ReadOnlyContractTest, ListCorruptArchiveReturnsError)
{
    if (!GetParam().canTestCorrupt) GTEST_SKIP();

    // Create a corrupt archive by truncating
    QString corruptPath = globalFixture().makeCorruptArchive(
        "corrupt/" + QString::fromStdString(GetParam().name) + "/test_corrupt",
        m_archivePath);

    std::unique_ptr<ReadOnlyArchiveInterface> corruptIface(GetParam().factory(corruptPath));
    ASSERT_NE(corruptIface, nullptr);

    // testArchive() is a no-op stub in current backends, so verify corruption
    // detection through list() which actually opens the archive.
    PluginFinishType result = corruptIface->list();
    if (GetParam().isAsync) {
        waitForFinishedSignal(corruptIface.get());
    }
    EXPECT_NE(result, PFT_Nomral);
}

TEST_P(ReadOnlyContractTest, ExtractPasswordProtectedArchive)
{
    if (!GetParam().canTestPasswordArchive) GTEST_SKIP();

    // Create a password-protected zip archive
    QString pwPath = globalFixture().makePasswordZip(
        "pw_test/" + QString::fromStdString(GetParam().name) + "/test_pw.zip", "secret");

    // 1. list() should succeed — listing doesn't require password for zip
    std::unique_ptr<ReadOnlyArchiveInterface> pwIface(GetParam().factory(pwPath));
    ASSERT_NE(pwIface, nullptr);
    PluginFinishType listResult = pwIface->list();
    if (GetParam().isAsync) {
        waitForFinishedSignal(pwIface.get());
    }
    EXPECT_EQ(listResult, PFT_Nomral);

    // 2. extractFiles() without password (cancelled) should not return PFT_Nomral
    std::unique_ptr<ReadOnlyArchiveInterface> pwIface2(GetParam().factory(pwPath));
    ASSERT_NE(pwIface2, nullptr);
    pwIface2->list();
    if (GetParam().isAsync) {
        waitForFinishedSignal(pwIface2.get());
    }

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    stub.set(ADDR(PasswordNeededQuery, waitForResponse), pw_waitForResponse_stub);
    stub.set(ADDR(PasswordNeededQuery, responseCancelled), pw_responseCancelled_true_stub);

    QString extractDir = globalFixture().tempPath() + "/pw_cancel_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);
    ExtractionOptions options;
    options.bAllExtract = true;
    options.bExistList = true;
    options.qSize = DataManager::get_instance().archiveData().qSize;
    options.strTargetPath = extractDir;
    PluginFinishType resultNoPw = pwIface2->extractFiles(QList<FileEntry>(), options);
    EXPECT_NE(resultNoPw, PFT_Nomral);
    QDir(extractDir).removeRecursively();

    // 3. extractFiles() with correct password should succeed
    std::unique_ptr<ReadOnlyArchiveInterface> pwIface3(GetParam().factory(pwPath));
    ASSERT_NE(pwIface3, nullptr);
    pwIface3->list();
    if (GetParam().isAsync) {
        waitForFinishedSignal(pwIface3.get());
    }

    Stub stub2;
    stub2.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub2.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub2.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub2.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub2.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    stub2.set(ADDR(PasswordNeededQuery, waitForResponse), pw_waitForResponse_stub);
    stub2.set(ADDR(PasswordNeededQuery, responseCancelled), pw_responseCancelled_false_stub);
    stub2.set(ADDR(PasswordNeededQuery, password), pw_password_secret_stub);

    QString extractDir2 = globalFixture().tempPath() + "/pw_correct_" +
                          QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir2);
    ExtractionOptions options2;
    options2.bAllExtract = true;
    options2.strTargetPath = extractDir2;
    PluginFinishType resultWithPw = pwIface3->extractFiles(QList<FileEntry>(), options2);
    EXPECT_EQ(resultWithPw, PFT_Nomral);

    // Verify extracted file has correct content
    bool foundContent = false;
    QDirIterator it(extractDir2, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFile f(it.next());
        if (f.open(QIODevice::ReadOnly)) {
            if (f.readAll().contains("password protected content")) {
                foundContent = true;
            }
            f.close();
        }
    }
    EXPECT_TRUE(foundContent);

    QDir(extractDir2).removeRecursively();
}

// ===========================================================================
// Contract: extractFiles() — full extraction with content verification
// ===========================================================================
TEST_P(ReadOnlyContractTest, ExtractAllFilesSucceeds)
{
    if (!GetParam().supportsExtractFiles) GTEST_SKIP();

    // List first to populate archive data
    runOp([this]() { return m_iface->list(); });

    QString extractDir = globalFixture().tempPath() + "/extract_all_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = extractDir;

    PluginFinishType result = runOp([this, &options]() {
        return m_iface->extractFiles(QList<FileEntry>(), options);
    });

    EXPECT_EQ(result, PFT_Nomral);

    // Verify extraction produced files
    QDir dir(extractDir);
    EXPECT_FALSE(dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot).isEmpty());

    QDir(extractDir).removeRecursively();
}

TEST_P(ReadOnlyContractTest, ExtractAllFilesContentVerified)
{
    if (!GetParam().supportsExtractFiles) GTEST_SKIP();
    // Skip backends whose list() is a stub (no real entries to extract)
    if (GetParam().name == "LibPigzPlugin") GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    ArchiveData &data = DataManager::get_instance().archiveData();
    ASSERT_FALSE(data.listRootEntry.isEmpty());

    QString extractDir = globalFixture().tempPath() + "/extract_content_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = extractDir;

    PluginFinishType result = runOp([this, &options]() {
        return m_iface->extractFiles(QList<FileEntry>(), options);
    });
    EXPECT_EQ(result, PFT_Nomral);

    // Verify at least one extracted file has non-empty content
    bool foundFileWithContent = false;
    QStringList extractedFiles;
    QDirIterator it(extractDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        extractedFiles << it.fileName();
        QFile f(filePath);
        if (f.open(QIODevice::ReadOnly)) {
            if (f.size() > 0) {
                foundFileWithContent = true;
            }
            f.close();
        }
    }
    EXPECT_FALSE(extractedFiles.isEmpty());
    EXPECT_TRUE(foundFileWithContent);

    QDir(extractDir).removeRecursively();
}

TEST_P(ReadOnlyContractTest, ExtractPartialFiles)
{
    if (!GetParam().supportsExtractFiles) GTEST_SKIP();
    if (GetParam().name == "LibPigzPlugin") GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    ArchiveData &data = DataManager::get_instance().archiveData();
    ASSERT_FALSE(data.listRootEntry.isEmpty());

    // Pick the first file entry (non-directory)
    FileEntry target;
    bool found = false;
    for (const FileEntry &e : data.listRootEntry) {
        if (!e.isDirectory) {
            target = e;
            found = true;
            break;
        }
    }
    if (!found) {
        target = data.listRootEntry.first();
        found = true;
    }
    ASSERT_TRUE(found);

    QString extractDir = globalFixture().tempPath() + "/extract_partial_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    QList<FileEntry> files;
    files << target;
    ExtractionOptions options;
    options.bAllExtract = false;
    options.strTargetPath = extractDir;

    PluginFinishType result = runOp([this, &files, &options]() {
        return m_iface->extractFiles(files, options);
    });
    EXPECT_EQ(result, PFT_Nomral);

    QDir(extractDir).removeRecursively();
}

TEST_P(ReadOnlyContractTest, ExtractWrongPasswordReturnsError)
{
    if (!GetParam().canExtractWrongPassword) GTEST_SKIP();

    QString pwPath = globalFixture().makePasswordZip(
        "pw_extract/" + QString::fromStdString(GetParam().name) + "/test.zip", "secret");

    std::unique_ptr<ReadOnlyArchiveInterface> pwIface(GetParam().factory(pwPath));
    ASSERT_NE(pwIface, nullptr);

    // List first (required before extract)
    PluginFinishType listResult = pwIface->list();
    if (GetParam().isAsync) {
        waitForFinishedSignal(pwIface.get());
    }

    // extractFiles() clears any preset password via setPassword(QString()),
    // so the password must be provided via PasswordNeededQuery stub.
    // Stub PasswordNeededQuery to return a wrong password.
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    stub.set(ADDR(PasswordNeededQuery, waitForResponse), pw_waitForResponse_stub);
    stub.set(ADDR(PasswordNeededQuery, responseCancelled), pw_responseCancelled_false_stub);
    stub.set(ADDR(PasswordNeededQuery, password), pw_password_wrong_stub);

    QString extractDir = globalFixture().tempPath() + "/extract_wrongpw_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = extractDir;

    PluginFinishType result = pwIface->extractFiles(QList<FileEntry>(), options);
    // Wrong password should not produce normal completion
    EXPECT_NE(result, PFT_Nomral);

    QDir(extractDir).removeRecursively();
}

// ===========================================================================
// Contract: signal tests — verify progress, cancel, finished signals
// ===========================================================================
TEST_P(ReadOnlyContractTest, SignalFinishedOnList)
{
    if (!GetParam().supportsList) GTEST_SKIP();

    SignalTracker tracker;
    tracker.connect(m_iface.get());

    PluginFinishType result = runOp([this]() { return m_iface->list(); });

    // For async backends, signalFinished is emitted after process completes
    // For sync backends that don't emit signalFinished, we still verify the
    // operation completed (result == PFT_Nomral)
    if (GetParam().isAsync) {
        EXPECT_GT(tracker.finishedCount, 0);
        EXPECT_EQ(tracker.lastFinishType, PFT_Nomral);
    }
    EXPECT_EQ(result, PFT_Nomral);
}

TEST_P(ReadOnlyContractTest, SignalFinishedOnExtract)
{
    if (!GetParam().supportsExtractFiles) GTEST_SKIP();
    if (GetParam().name == "LibPigzPlugin") GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    SignalTracker tracker;
    tracker.connect(m_iface.get());

    QString extractDir = globalFixture().tempPath() + "/sig_extract_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = extractDir;

    PluginFinishType result = runOp([this, &options]() {
        return m_iface->extractFiles(QList<FileEntry>(), options);
    });

    EXPECT_EQ(result, PFT_Nomral);

    // Async backends should emit signalFinished
    if (GetParam().isAsync) {
        EXPECT_GT(tracker.finishedCount, 0);
        EXPECT_EQ(tracker.lastFinishType, PFT_Nomral);
    }

    QDir(extractDir).removeRecursively();
}

TEST_P(ReadOnlyContractTest, SignalProgressOnExtract)
{
    if (!GetParam().supportsExtractFiles) GTEST_SKIP();
    if (GetParam().name == "LibPigzPlugin") GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    SignalTracker tracker;
    tracker.connect(m_iface.get());

    QString extractDir = globalFixture().tempPath() + "/sig_progress_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    ExtractionOptions options;
    options.bAllExtract = true;
    options.bExistList = true;
    options.qSize = DataManager::get_instance().archiveData().qSize;
    options.strTargetPath = extractDir;

    runOp([this, &options]() {
        return m_iface->extractFiles(QList<FileEntry>(), options);
    });

    // LibzipPlugin is a synchronous backend that emits progress while
    // extracting. This backend must expose observable progress to src.
    if (GetParam().name == "LibzipPlugin") {
        EXPECT_GT(tracker.progressCount, 0);
        EXPECT_GE(tracker.lastProgress, 0.0);
        EXPECT_LE(tracker.lastProgress, 100.0);
    }

    QDir(extractDir).removeRecursively();
}

TEST_P(ReadOnlyContractTest, SignalCurFileNameOnExtract)
{
    if (!GetParam().supportsExtractFiles) GTEST_SKIP();
    if (GetParam().name == "LibPigzPlugin") GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });

    SignalTracker tracker;
    tracker.connect(m_iface.get());

    QString extractDir = globalFixture().tempPath() + "/sig_curname_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = extractDir;

    runOp([this, &options]() {
        return m_iface->extractFiles(QList<FileEntry>(), options);
    });

    // signalCurFileName should be emitted during extraction for backends that
    // process individual files
    // LibzipPlugin emits signalCurFileName; verify at least one emission
    if (GetParam().name == "LibzipPlugin" || GetParam().name == "LibminizipPlugin" ||
        GetParam().name == "ReadOnlyLibarchivePlugin") {
        EXPECT_GT(tracker.curFileNameCount, 0);
    }

    QDir(extractDir).removeRecursively();
}

// ===========================================================================
// Contract: cancel via doKill — verify signalCancel or error on cancel flow
// ===========================================================================
TEST_P(ReadOnlyContractTest, DoKillReturnsFalseWhenIdle)
{
    // doKill when no process is running should return false
    EXPECT_FALSE(m_iface->doKill());
}

TEST_P(ReadOnlyContractTest, OverwriteCancelEmitsCancelSignal)
{
    if (GetParam().name != "LibzipPlugin") GTEST_SKIP();

    runOp([this]() { return m_iface->list(); });
    ArchiveData &data = DataManager::get_instance().archiveData();
    ASSERT_FALSE(data.listRootEntry.isEmpty());

    FileEntry target;
    bool found = false;
    for (const FileEntry &entry : data.listRootEntry) {
        if (!entry.isDirectory) {
            target = entry;
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);

    QString extractDir = globalFixture().tempPath() + "/cancel_overwrite_" +
                         QString::fromStdString(GetParam().name);
    QDir().mkpath(extractDir);

    QString destination = extractDir + "/" + target.strFileName;
    QDir().mkpath(QFileInfo(destination).absolutePath());
    QFile existing(destination);
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    existing.write("existing destination");
    existing.close();

    SignalTracker tracker;
    tracker.connect(m_iface.get());

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_true_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = extractDir;

    PluginFinishType result = m_iface->extractFiles(QList<FileEntry>(), options);
    EXPECT_NE(result, PFT_Nomral);
    EXPECT_GT(tracker.queryCount, 0);
    EXPECT_GT(tracker.cancelCount, 0);

    QDir(extractDir).removeRecursively();
}

// ===========================================================================
// Contract: password operations
// ===========================================================================
TEST_P(ReadOnlyContractTest, PasswordRoundTrip)
{
    m_iface->setPassword("test123");
    EXPECT_EQ(m_iface->getPassword().toStdString(), "test123");
}

TEST_P(ReadOnlyContractTest, PasswordRoundTripEmpty)
{
    m_iface->setPassword("");
    EXPECT_EQ(m_iface->getPassword().toStdString(), "");
}

TEST_P(ReadOnlyContractTest, PasswordRoundTripUnicode)
{
    m_iface->setPassword(QString::fromUtf8("密码123"));
    EXPECT_EQ(m_iface->getPassword(), QString::fromUtf8("密码123"));
}

// ===========================================================================
// Contract: errorType defaults to ET_NoError
// ===========================================================================
TEST_P(ReadOnlyContractTest, DefaultErrorTypeIsNoError)
{
    EXPECT_EQ(m_iface->errorType(), ET_NoError);
}

// ===========================================================================
// Contract: status / pause / continue operations
// ===========================================================================
TEST_P(ReadOnlyContractTest, DefaultStatusIsNotPaused)
{
    EXPECT_FALSE(m_iface->status());
}

TEST_P(ReadOnlyContractTest, PauseOperationSetsPauseFlag)
{
    if (GetParam().name == "LibPigzPlugin") GTEST_SKIP();
    m_iface->pauseOperation();
    EXPECT_TRUE(m_iface->status());
}

TEST_P(ReadOnlyContractTest, ContinueOperationClearsPauseFlag)
{
    if (GetParam().name == "LibPigzPlugin") GTEST_SKIP();
    m_iface->pauseOperation();
    EXPECT_TRUE(m_iface->status());
    m_iface->continueOperation();
    EXPECT_FALSE(m_iface->status());
}

// ===========================================================================
// Contract: isUserCancel defaults to false
// ===========================================================================
TEST_P(ReadOnlyContractTest, DefaultIsUserCancelIsFalse)
{
    EXPECT_FALSE(m_iface->isUserCancel());
}

// ===========================================================================
// Contract: waitForFinished is deterministic when idle
// ===========================================================================
TEST_P(ReadOnlyContractTest, WaitForFinishedIsStableWhenIdle)
{
    bool first = m_iface->waitForFinished();
    EXPECT_EQ(m_iface->waitForFinished(), first);
}
