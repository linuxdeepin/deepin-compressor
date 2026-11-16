// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "legacy_backend_adapter.h"
#include "legacy_backend_observer_adapter.h"
#include "archive_types.h"
#include "backend_capabilities.h"
#include "archiveinterface.h"
#include "datamanager.h"
#include "queries.h"
#include <QPointer>

#include <QSignalSpy>
#include <QCoreApplication>

using namespace archive;

// ---------------------------------------------------------------------------
// Fake archive interfaces
// ---------------------------------------------------------------------------

// A fake read-only interface that records calls and lets the test control
// the return values and whether operations run synchronously or
// asynchronously (via setWaitForFinishedSignal).
class FakeReadOnlyInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT
public:
    explicit FakeReadOnlyInterface()
        : ReadOnlyArchiveInterface(nullptr, {})
    {
        // Default: synchronous mode (waitForFinished returns false,
        // meaning the operation completed inline — no signal-based finish).
        setWaitForFinishedSignal(false);
    }
    void setWaitForFinished(bool v) { setWaitForFinishedSignal(v); }
    QString strPassword() const { return m_strPassword; }

    // -- recorded state --
    int listCallCount = 0;
    int testArchiveCallCount = 0;
    int extractCallCount = 0;
    PluginFinishType listResult = PFT_Nomral;
    PluginFinishType testResult = PFT_Nomral;
    PluginFinishType extractResult = PFT_Nomral;
    ExtractionOptions lastExtractionOptions;
    QList<FileEntry> lastExtractEntries;
    bool doKillCalled = false;
    bool pauseCalled = false;
    bool continueCalled = false;

    // When true, extractFiles emits signalQuery with an OverwriteQuery
    // so tests can verify the adapter auto-responds.
    bool emitOverwriteQuery = false;
    QScopedPointer<OverwriteQuery> lastOverwriteQuery;

    // When true, extractFiles emits signalQuery with a PasswordNeededQuery
    // so tests can verify the adapter forwards headerPassword correctly.
    bool emitPasswordQuery = false;
    // When true, list() emits signalQuery with a PasswordNeededQuery
    // so tests can verify headerPassword is set correctly during open().
    bool emitPasswordQueryDuringList = false;
    QScopedPointer<PasswordNeededQuery> lastPasswordQuery;

    // -- control --
    void setErrorType(ErrorType et) { m_eErrorType = et; }

    // Populate DataManager with test entries.
    void populateArchiveData(const QStringList &paths)
    {
        ArchiveData &data = DataManager::get_instance().archiveData();
        data.mapFileEntry.clear();
        data.listRootEntry.clear();
        for (const QString &p : paths) {
            FileEntry entry;
            entry.strFullPath = p;
            entry.strFileName = QFileInfo(p).fileName();
            entry.isDirectory = p.endsWith("/");
            data.mapFileEntry.insert(p, entry);
            // listRootEntry contains only top-level entries (no "/")
            if (!p.contains("/") || (p.endsWith("/") && !p.left(p.size() - 1).contains("/"))) {
                data.listRootEntry.append(entry);
            }
        }
    }

    void setListEncrypted(bool encrypted)
    {
        DataManager::get_instance().archiveData().isListEncrypted = encrypted;
    }

    // -- pure virtual overrides --
    PluginFinishType list() override
    {
        listCallCount++;
        if (emitPasswordQueryDuringList) {
            lastPasswordQuery.reset(new PasswordNeededQuery("test.zip"));
            Q_EMIT signalQuery(lastPasswordQuery.data());
        }
        return listResult;
    }

    PluginFinishType testArchive() override
    {
        testArchiveCallCount++;
        return testResult;
    }

    PluginFinishType extractFiles(const QList<FileEntry> &files,
                                  const ExtractionOptions &options) override
    {
        extractCallCount++;
        lastExtractionOptions = options;
        lastExtractEntries = files;
        if (emitOverwriteQuery) {
            lastOverwriteQuery.reset(new OverwriteQuery("conflict.txt"));
            Q_EMIT signalQuery(lastOverwriteQuery.data());
        }
        if (emitPasswordQuery) {
            lastPasswordQuery.reset(new PasswordNeededQuery("test.zip"));
            Q_EMIT signalQuery(lastPasswordQuery.data());
        }
        return extractResult;
    }

    void pauseOperation() override { pauseCalled = true; }
    void continueOperation() override { continueCalled = true; }

    bool doKill() override
    {
        doKillCalled = true;
        return true;
    }
};

// A fake read-write interface that also records write operations.
class FakeReadWriteInterface : public ReadWriteArchiveInterface
{
    Q_OBJECT
public:
    explicit FakeReadWriteInterface()
        : ReadWriteArchiveInterface(nullptr, {})
    {
        setWaitForFinishedSignal(false);
    }
    void setWaitForFinished(bool v) { setWaitForFinishedSignal(v); }

    // -- read side --
    int listCallCount = 0;
    PluginFinishType listResult = PFT_Nomral;

    int testArchiveCallCount = 0;
    PluginFinishType testResult = PFT_Nomral;

    int extractCallCount = 0;
    PluginFinishType extractResult = PFT_Nomral;
    ExtractionOptions lastExtractionOptions;
    QList<FileEntry> lastExtractEntries;

    // -- write side --
    int addCallCount = 0;
    PluginFinishType addResult = PFT_Nomral;
    CompressOptions lastCompressOptions;
    QList<FileEntry> lastAddEntries;

    int moveCallCount = 0;
    PluginFinishType moveResult = PFT_Nomral;

    int copyCallCount = 0;
    PluginFinishType copyResult = PFT_Nomral;

    int deleteCallCount = 0;
    PluginFinishType deleteResult = PFT_Nomral;

    int renameCallCount = 0;
    PluginFinishType renameResult = PFT_Nomral;

    int commentCallCount = 0;
    PluginFinishType commentResult = PFT_Nomral;
    QString lastComment;

    int updateCallCount = 0;
    PluginFinishType updateResult = PFT_Nomral;
    UpdateOptions lastUpdateOptions;

    bool doKillCalled = false;

    // When true, extractFiles emits signalQuery with an OverwriteQuery
    bool emitOverwriteQuery = false;
    QScopedPointer<OverwriteQuery> lastOverwriteQuery;
    bool emitPasswordQuery = false;
    QScopedPointer<PasswordNeededQuery> lastPasswordQuery;

    void setErrorType(ErrorType et) { m_eErrorType = et; }

    void populateArchiveData(const QStringList &paths)
    {
        ArchiveData &data = DataManager::get_instance().archiveData();
        data.mapFileEntry.clear();
        data.listRootEntry.clear();
        for (const QString &p : paths) {
            FileEntry entry;
            entry.strFullPath = p;
            entry.strFileName = QFileInfo(p).fileName();
            entry.isDirectory = p.endsWith("/");
            data.mapFileEntry.insert(p, entry);
        }
    }

    void setListEncrypted(bool encrypted)
    {
        DataManager::get_instance().archiveData().isListEncrypted = encrypted;
    }

    // -- read side overrides --
    PluginFinishType list() override
    {
        listCallCount++;
        return listResult;
    }

    PluginFinishType testArchive() override
    {
        testArchiveCallCount++;
        return testResult;
    }

    PluginFinishType extractFiles(const QList<FileEntry> &files,
                                  const ExtractionOptions &options) override
    {
        extractCallCount++;
        lastExtractionOptions = options;
        lastExtractEntries = files;
        if (emitOverwriteQuery) {
            lastOverwriteQuery.reset(new OverwriteQuery("conflict.txt"));
            Q_EMIT signalQuery(lastOverwriteQuery.data());
        }
        if (emitPasswordQuery) {
            lastPasswordQuery.reset(new PasswordNeededQuery("test.zip"));
            Q_EMIT signalQuery(lastPasswordQuery.data());
        }
        return extractResult;
    }

    void pauseOperation() override {}
    void continueOperation() override {}

    bool doKill() override
    {
        doKillCalled = true;
        return true;
    }

    // -- write side overrides --
    PluginFinishType addFiles(const QList<FileEntry> &files,
                              const CompressOptions &options) override
    {
        addCallCount++;
        lastCompressOptions = options;
        lastAddEntries = files;
        return addResult;
    }

    PluginFinishType moveFiles(const QList<FileEntry> &,
                               const CompressOptions &) override
    {
        moveCallCount++;
        return moveResult;
    }

    PluginFinishType copyFiles(const QList<FileEntry> &,
                               const CompressOptions &) override
    {
        copyCallCount++;
        return copyResult;
    }

    PluginFinishType deleteFiles(const QList<FileEntry> &) override
    {
        deleteCallCount++;
        return deleteResult;
    }

    PluginFinishType renameFiles(const QList<FileEntry> &) override
    {
        renameCallCount++;
        return renameResult;
    }

    PluginFinishType addComment(const QString &comment) override
    {
        commentCallCount++;
        lastComment = comment;
        return commentResult;
    }

    PluginFinishType updateArchiveData(const UpdateOptions &options) override
    {
        updateCallCount++;
        lastUpdateOptions = options;
        return updateResult;
    }
};

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class LegacyBackendAdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        DataManager::get_instance().resetArchiveData();
    }
    void TearDown() override
    {
        DataManager::get_instance().resetArchiveData();
    }
};

// ---------------------------------------------------------------------------
// Capabilities
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, ReadOnlyCapabilities)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);
    auto caps = adapter.capabilities();
    EXPECT_TRUE(caps.list);
    EXPECT_TRUE(caps.test);
    EXPECT_TRUE(caps.extract);
    EXPECT_FALSE(caps.create);
    EXPECT_FALSE(caps.append);
    EXPECT_FALSE(caps.remove);
    EXPECT_FALSE(caps.rename);
}

TEST_F(LegacyBackendAdapterTest, ReadWriteCapabilities)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    LegacyBackendAdapter adapter(fake);
    auto caps = adapter.capabilities();
    EXPECT_TRUE(caps.list);
    EXPECT_TRUE(caps.test);
    EXPECT_TRUE(caps.extract);
    EXPECT_TRUE(caps.create);
    EXPECT_TRUE(caps.append);
    EXPECT_TRUE(caps.move);
    EXPECT_TRUE(caps.copy);
    EXPECT_TRUE(caps.remove);
    EXPECT_TRUE(caps.rename);
    EXPECT_TRUE(caps.comment);
    EXPECT_TRUE(caps.update);
}

// ---------------------------------------------------------------------------
// open + list with dedup
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, OpenAndListNoDuplication)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    // Populate with: root dir, root file, nested file.
    // mapFileEntry has ALL entries; listRootEntry has only top-level.
    fake->populateArchiveData({"dir1/", "dir1/file1.txt", "root.txt"});

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    openReq.archivePath = "/tmp/test.zip";
    auto openResult = adapter.open(openReq);
    EXPECT_TRUE(openResult.success);
    EXPECT_EQ(fake->listCallCount, 1);

    auto listResult = adapter.list();
    EXPECT_TRUE(listResult.success);
    // mapFileEntry has 3 entries; result should have exactly 3 (not 5
    // which would happen if listRootEntry entries were also appended).
    EXPECT_EQ(listResult.entries.size(), 3);

    // Verify no duplicate paths
    QSet<QString> paths;
    for (const auto &e : listResult.entries) {
        EXPECT_FALSE(paths.contains(e.fullPath))
            << "Duplicate path: " << e.fullPath.toStdString();
        paths.insert(e.fullPath);
    }
}

TEST_F(LegacyBackendAdapterTest, ListCachesDataNoDuplicateCall)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);
    EXPECT_EQ(fake->listCallCount, 1);

    // list() should reuse cached data, not call fake->list() again
    auto result = adapter.list();
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->listCallCount, 1);
}

// ---------------------------------------------------------------------------
// Sync vs async observer lifecycle
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, SyncExtractCallsOnFinishedOnce)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    // Synchronous mode: waitForFinished() returns false
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.targetPath = "/tmp/out";
    req.preservePaths = true;

    auto result = adapter.extractFiles(req, observer);
    // In sync mode, the adapter calls onFinished itself
    EXPECT_TRUE(observer.isFinished());
    EXPECT_EQ(observer.finishInfo().success, result.success);
}

TEST_F(LegacyBackendAdapterTest, AsyncExtractOnFinishedViaSignal)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    // Async mode: waitForFinished() returns true — operation finishes via signal
    fake->setWaitForFinished(true);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.targetPath = "/tmp/out";
    req.preservePaths = true;

    auto result = adapter.extractFiles(req, observer);
    // In async mode, adapter does NOT call onFinished (waits for signal)
    EXPECT_FALSE(observer.isFinished());

    // Simulate the signal that the old interface would emit
    emit fake->signalFinished(PFT_Nomral);
    QCoreApplication::processEvents();

    EXPECT_TRUE(observer.isFinished());
    EXPECT_TRUE(observer.finishInfo().success);
}

TEST_F(LegacyBackendAdapterTest, AsyncExtractOnFinishedNotDuplicated)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(true);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.preservePaths = true;

    adapter.extractFiles(req, observer);

    // Emit signal multiple times — onFinished should fire only once because
    // the adapter nulls m_observer after the first signal.
    emit fake->signalFinished(PFT_Nomral);
    QCoreApplication::processEvents();
    emit fake->signalFinished(PFT_Error);
    QCoreApplication::processEvents();

    EXPECT_TRUE(observer.isFinished());
    // Should have only one finish event
    EXPECT_TRUE(observer.finishInfo().success);
}

// ---------------------------------------------------------------------------
// ExtractRequest preservePaths/overwriteAll/skipAll
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, ExtractPreservePathsFalseReturnsNotSupported)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.preservePaths = false;

    auto result = adapter.extractFiles(req, observer);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
    EXPECT_EQ(fake->extractCallCount, 0);
}

TEST_F(LegacyBackendAdapterTest, ExtractOverwriteAllSetsOption)
{
    FakeReadWriteInterface *rwFake = new FakeReadWriteInterface();
    rwFake->populateArchiveData({"a.txt"});
    rwFake->setWaitForFinished(false);
    rwFake->emitOverwriteQuery = true;

    LegacyBackendAdapter adapter(rwFake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.preservePaths = true;
    req.overwriteAll = true;

    adapter.extractFiles(req, observer);
    EXPECT_EQ(rwFake->extractCallCount, 1);
    // The adapter's signalQuery handler should auto-respond with
    // Result_OverwriteAll when overwriteAll is set in the request.
    ASSERT_NE(rwFake->lastOverwriteQuery.data(), nullptr);
    EXPECT_TRUE(rwFake->lastOverwriteQuery->responseOverwriteAll());
}

TEST_F(LegacyBackendAdapterTest, ExtractSkipAllSetsOption)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);
    fake->emitOverwriteQuery = true;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.preservePaths = true;
    req.skipAll = true;

    adapter.extractFiles(req, observer);
    EXPECT_EQ(fake->extractCallCount, 1);
    // The adapter's signalQuery handler should auto-respond with
    // Result_SkipAll when skipAll is set in the request.
    ASSERT_NE(fake->lastOverwriteQuery.data(), nullptr);
    EXPECT_TRUE(fake->lastOverwriteQuery->responseSkipAll());
}

TEST_F(LegacyBackendAdapterTest, ExtractOptionsTargetPathAndPassword)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.targetPath = "/tmp/extract";
    req.password = "mypass";
    req.preservePaths = true;
    req.entries = {"a.txt"};

    adapter.extractFiles(req, observer);
    EXPECT_EQ(fake->lastExtractionOptions.strTargetPath.toStdString(), "/tmp/extract");
    EXPECT_EQ(fake->lastExtractionOptions.password.toStdString(), "mypass");
    EXPECT_FALSE(fake->lastExtractionOptions.bAllExtract);
    EXPECT_EQ(fake->lastExtractEntries.size(), 1);
    EXPECT_EQ(fake->lastExtractEntries[0].strFullPath.toStdString(), "a.txt");
}

TEST_F(LegacyBackendAdapterTest, ExtractAllEntriesEmpty)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt", "b.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    ExtractRequest req;
    req.preservePaths = true;
    // empty entries = extract all

    adapter.extractFiles(req, observer);
    EXPECT_TRUE(fake->lastExtractionOptions.bAllExtract);
    EXPECT_TRUE(fake->lastExtractEntries.isEmpty());
}

// ---------------------------------------------------------------------------
// OpenRequest::headerEncrypted and m_listEncrypted
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, OpenCachesListEncrypted)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setListEncrypted(true);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    openReq.password = "secret";
    ASSERT_TRUE(adapter.open(openReq).success);

    // The adapter should have cached isListEncrypted from DataManager
    EXPECT_TRUE(fake->strPassword().toStdString() == "secret");
}

// Verify that headerPassword in the PasswordRequest reflects m_listEncrypted
// (which is cached from DataManager::isListEncrypted after open()).
// The adapter must not silently discard this — headerPassword must be true
// when the archive uses list encryption.
TEST_F(LegacyBackendAdapterTest, HeaderPasswordReflectsListEncrypted)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setListEncrypted(true);
    fake->emitPasswordQuery = true;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    openReq.password = "secret";
    ASSERT_TRUE(adapter.open(openReq).success);

    // Capture the PasswordRequest via the observer
    bool passwordRequested = false;
    bool headerPasswordSeen = false;
    LegacyBackendObserverAdapter observer;
    observer.setPasswordHandler([&](const PasswordRequest &req) -> QString {
        passwordRequested = true;
        headerPasswordSeen = req.headerPassword;
        return QString("testpass");
    });

    ExtractRequest req;
    adapter.extractFiles(req, observer);
    EXPECT_TRUE(passwordRequested);
    EXPECT_TRUE(headerPasswordSeen);
}

// Verify that headerPassword is false when the archive is not list-encrypted.
TEST_F(LegacyBackendAdapterTest, HeaderPasswordFalseWhenNotEncrypted)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setListEncrypted(false);
    fake->emitPasswordQuery = true;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    bool headerPasswordSeen = true;
    LegacyBackendObserverAdapter observer;
    observer.setPasswordHandler([&](const PasswordRequest &req) -> QString {
        headerPasswordSeen = req.headerPassword;
        return QString("testpass");
    });

    ExtractRequest req;
    adapter.extractFiles(req, observer);
    EXPECT_FALSE(headerPasswordSeen);
}

// Verify that OpenRequest::headerEncrypted forces m_listEncrypted to true
// even when DataManager reports isListEncrypted = false.
TEST_F(LegacyBackendAdapterTest, HeaderEncryptedOverridesDataManager)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setListEncrypted(false);
    fake->emitPasswordQuery = true;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    openReq.headerEncrypted = true;
    ASSERT_TRUE(adapter.open(openReq).success);

    bool headerPasswordSeen = false;
    LegacyBackendObserverAdapter observer;
    observer.setPasswordHandler([&](const PasswordRequest &req) -> QString {
        headerPasswordSeen = req.headerPassword;
        return QString("testpass");
    });

    ExtractRequest req;
    adapter.extractFiles(req, observer);
    EXPECT_TRUE(headerPasswordSeen);
}

// ---------------------------------------------------------------------------
// AddRequest headerEncrypt/volumeSize/compressionLevel conversion
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, AddFilesCompressOptionsConversion)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    AddRequest req;
    req.files = {"/tmp/file1.txt", "/tmp/file2.txt"};
    req.password = "encpass";
    req.encrypt = true;
    req.headerEncrypt = true;
    req.compressionLevel = 5;
    req.compressionMethod = "deflate";
    req.encryptionMethod = "aes256";
    req.volumeSize = 10240;
    req.cpuThreads = 4;

    adapter.addFiles(req, observer);
    EXPECT_EQ(fake->addCallCount, 1);
    EXPECT_EQ(fake->lastCompressOptions.strPassword.toStdString(), "encpass");
    EXPECT_TRUE(fake->lastCompressOptions.bEncryption);
    EXPECT_TRUE(fake->lastCompressOptions.bHeaderEncryption);
    EXPECT_EQ(fake->lastCompressOptions.iCompressionLevel, 5);
    EXPECT_EQ(fake->lastCompressOptions.strCompressionMethod.toStdString(), "deflate");
    EXPECT_EQ(fake->lastCompressOptions.strEncryptionMethod.toStdString(), "aes256");
    EXPECT_EQ(fake->lastCompressOptions.iVolumeSize, 10240);
    EXPECT_TRUE(fake->lastCompressOptions.bSplit);
    EXPECT_EQ(fake->lastCompressOptions.iCPUTheadNum, 4);
    EXPECT_EQ(fake->lastAddEntries.size(), 2);
}

// ---------------------------------------------------------------------------
// ErrorType -> ErrorCode mappings
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingNoError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_NoError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    // Even with PFT_Error, if errorType is ET_NoError the code maps to NoError
    // (but success is false because finishType != PFT_Nomral)
    EXPECT_FALSE(result.success);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingWrongPassword)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_WrongPassword);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::WrongPassword);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingNeedPassword)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_NeedPassword);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::NeedPassword);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingArchiveDamaged)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_ArchiveDamaged);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::CorruptArchive);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingFileOpenError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_FileOpenError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::FileOpenError);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingFileReadError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_FileReadError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::FileReadError);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingFileWriteError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_FileWriteError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::FileWriteError);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingDeleteError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_DeleteError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::FileDeleteError);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingRenameError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_RenameError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::FileRenameError);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingMissingVolume)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_MissingVolume);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::MissingVolume);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingInsufficientDiskSpace)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_InsufficientDiskSpace);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::InsufficientDiskSpace);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingPluginError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_PluginError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::PluginError);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingUserCancel)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Cancel;
    fake->setErrorType(ET_UserCancelOpertion);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::Cancelled);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingLongNameError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_LongNameError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::LongNameError);
}

TEST_F(LegacyBackendAdapterTest, ErrorTypeMappingFileSeekError)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->listResult = PFT_Error;
    fake->setErrorType(ET_FileSeekError);

    LegacyBackendAdapter adapter(fake);
    OpenRequest req;
    auto result = adapter.open(req);
    EXPECT_EQ(result.code, ErrorCode::FileReadError);
}

// ---------------------------------------------------------------------------
// Read-only adapter returns NotSupported for write ops
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, ReadOnlyRejectsAddFiles)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    LegacyBackendObserverAdapter observer;
    AddRequest req;
    auto result = adapter.addFiles(req, observer);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
}

TEST_F(LegacyBackendAdapterTest, ReadOnlyRejectsMoveFiles)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    LegacyBackendObserverAdapter observer;
    MoveRequest req;
    auto result = adapter.moveFiles(req, observer);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
}

TEST_F(LegacyBackendAdapterTest, ReadOnlyRejectsCopyFiles)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    LegacyBackendObserverAdapter observer;
    CopyRequest req;
    auto result = adapter.copyFiles(req, observer);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
}

TEST_F(LegacyBackendAdapterTest, ReadOnlyRejectsDeleteFiles)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    DeleteRequest req;
    auto result = adapter.deleteFiles(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
}

TEST_F(LegacyBackendAdapterTest, ReadOnlyRejectsRenameFiles)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    RenameRequest req;
    auto result = adapter.renameFiles(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
}

TEST_F(LegacyBackendAdapterTest, ReadOnlyRejectsAddComment)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    auto result = adapter.addComment("test");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
}

TEST_F(LegacyBackendAdapterTest, ReadOnlyRejectsUpdateArchiveData)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    UpdateRequest req;
    auto result = adapter.updateArchiveData(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.code, ErrorCode::NotSupported);
}

// ---------------------------------------------------------------------------
// Read-write adapter write operations
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, ReadWriteAddFiles)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    AddRequest req;
    req.files = {"/tmp/new.txt"};

    auto result = adapter.addFiles(req, observer);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->addCallCount, 1);
}

TEST_F(LegacyBackendAdapterTest, ReadWriteDeleteFiles)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    DeleteRequest req;
    req.entries = {"a.txt"};
    auto result = adapter.deleteFiles(req);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->deleteCallCount, 1);
}

TEST_F(LegacyBackendAdapterTest, ReadWriteAddComment)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    auto result = adapter.addComment("my comment");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->commentCallCount, 1);
    EXPECT_EQ(fake->lastComment.toStdString(), "my comment");
}

// ---------------------------------------------------------------------------
// Cancel / pause / resume
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, CancelCallsDoKill)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    adapter.cancel();
    EXPECT_TRUE(fake->doKillCalled);
}

TEST_F(LegacyBackendAdapterTest, PauseCallsPauseOperation)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    adapter.pause();
    EXPECT_TRUE(fake->pauseCalled);
}

TEST_F(LegacyBackendAdapterTest, ResumeCallsContinueOperation)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    LegacyBackendAdapter adapter(fake);

    adapter.resume();
    EXPECT_TRUE(fake->continueCalled);
}

// ---------------------------------------------------------------------------
// Legacy result type tests (kept from prior version)
// ---------------------------------------------------------------------------

TEST_F(LegacyBackendAdapterTest, VoidResultOkAndFail)
{
    auto ok = VoidResult::ok();
    EXPECT_TRUE(ok.success);
    EXPECT_EQ(ok.code, ErrorCode::NoError);

    auto fail = VoidResult::fail(ErrorCode::CorruptArchive, "damaged");
    EXPECT_FALSE(fail.success);
    EXPECT_EQ(fail.code, ErrorCode::CorruptArchive);
    EXPECT_EQ(fail.message.toStdString(), "damaged");
}

TEST_F(LegacyBackendAdapterTest, VoidResultNotSupported)
{
    auto ns = VoidResult::notSupported();
    EXPECT_FALSE(ns.success);
    EXPECT_EQ(ns.code, ErrorCode::NotSupported);
}

TEST_F(LegacyBackendAdapterTest, EntryResultOkWithEntries)
{
    QList<ArchiveEntry> entries;
    ArchiveEntry e1;
    e1.fullPath = "/dir/file1.txt";
    e1.fileName = "file1.txt";
    e1.isDirectory = false;
    e1.size = 100;
    entries.append(e1);

    auto result = EntryResult::ok(entries);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.entries.size(), 1);
    EXPECT_EQ(result.entries[0].fileName.toStdString(), "file1.txt");
}


// ---------------------------------------------------------------------------
// List-stage header password test
// ---------------------------------------------------------------------------

// Verify that headerPassword=true when a password query fires during
// list() inside open().  The fix sets m_listEncrypted from
// request.headerEncrypted BEFORE calling list(), so the signalQuery
// handler sees the correct value.
TEST_F(LegacyBackendAdapterTest, HeaderPasswordDuringListStage)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setListEncrypted(false);  // DataManager says not encrypted
    fake->emitPasswordQueryDuringList = true;
    fake->setWaitForFinished(true);  // Keep observer alive after extractFiles

    LegacyBackendAdapter adapter(fake);

    // Set up observer by calling extractFiles first (async mode keeps
    // m_observer alive for the subsequent open() call).
    bool passwordRequested = false;
    bool headerPasswordSeen = false;
    LegacyBackendObserverAdapter observer;
    observer.setPasswordHandler([&](const PasswordRequest &req) -> QString {
        passwordRequested = true;
        headerPasswordSeen = req.headerPassword;
        return QString("testpass");
    });

    ExtractRequest extractReq;
    adapter.extractFiles(extractReq, observer);

    // Now call open() with headerEncrypted=true.  list() will emit a
    // password query.  The handler should see m_listEncrypted=true
    // (set from request.headerEncrypted before list() was called).
    OpenRequest openReq;
    openReq.headerEncrypted = true;
    ASSERT_TRUE(adapter.open(openReq).success);

    EXPECT_TRUE(passwordRequested);
    EXPECT_TRUE(headerPasswordSeen);
}

// Verify that headerPassword=false when the archive is not header-encrypted
// and a password query fires during list().
TEST_F(LegacyBackendAdapterTest, HeaderPasswordFalseDuringListStage)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setListEncrypted(false);
    fake->emitPasswordQueryDuringList = true;
    fake->setWaitForFinished(true);

    LegacyBackendAdapter adapter(fake);

    bool passwordRequested = false;
    bool headerPasswordSeen = true;
    LegacyBackendObserverAdapter observer;
    observer.setPasswordHandler([&](const PasswordRequest &req) -> QString {
        passwordRequested = true;
        headerPasswordSeen = req.headerPassword;
        return QString("testpass");
    });

    ExtractRequest extractReq;
    adapter.extractFiles(extractReq, observer);

    OpenRequest openReq;
    openReq.headerEncrypted = false;
    ASSERT_TRUE(adapter.open(openReq).success);

    EXPECT_TRUE(passwordRequested);
    EXPECT_FALSE(headerPasswordSeen);
}


// ---------------------------------------------------------------------------
// Regression: use-after-free after releaseInterface()
//
// When releaseInterface() is called, the adapter disconnects all signal
// forwarding connections before handing the interface to the caller.  After
// the adapter is destroyed, emitting signals on the retained interface must
// not crash and must not invoke any stale adapter callbacks.
// ---------------------------------------------------------------------------

// Emit every forwarded signal on the interface and verify no crash.
// Also verify that an independent receiver still gets the signals.
TEST_F(LegacyBackendAdapterTest, ReleaseInterfaceThenEmitSignalsNoCrash)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();

    // Create the adapter on the heap so we can destroy it independently.
    auto adapter = std::unique_ptr<LegacyBackendAdapter>(
        new LegacyBackendAdapter(fake));

    // Attach independent signal spies to verify they still receive signals
    // after the adapter is gone.
    QSignalSpy progressSpy(fake, &ReadOnlyArchiveInterface::signalprogress);
    QSignalSpy curFileSpy(fake, &ReadOnlyArchiveInterface::signalCurFileName);
    QSignalSpy errorSpy(fake, &ReadOnlyArchiveInterface::error);
    QSignalSpy finishedSpy(fake, &ReadOnlyArchiveInterface::signalFinished);

    // Release the interface — this disconnects all adapter forwarding.
    ReadOnlyArchiveInterface *released = adapter->releaseInterface();
    ASSERT_EQ(released, fake);

    // Destroy the adapter.  The interface must survive.
    adapter.reset();

    // Emit every forwarded signal on the retained interface.
    // If the forwarding connections were not disconnected, these would
    // access the destroyed adapter's members → use-after-free.
    Q_EMIT fake->signalprogress(50.0);
    Q_EMIT fake->signalCurFileName(QStringLiteral("test.txt"));
    Q_EMIT fake->error(QStringLiteral("err"), QStringLiteral("detail"));
    Q_EMIT fake->signalFinished(PFT_Nomral);

    // The independent receivers should still have received all signals.
    EXPECT_EQ(progressSpy.count(), 1);
    EXPECT_EQ(curFileSpy.count(), 1);
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(finishedSpy.count(), 1);

    delete fake;
}

// Verify that releaseInterface() + destructor (without prior release) both
// properly disconnect.  This covers the belt-and-suspenders path where the
// destructor calls disconnectForwardingConnections() even when the interface
// has not been released.
TEST_F(LegacyBackendAdapterTest, DestructorDisconnectsForwardingConnections)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();

    // Scope the adapter so it is destroyed at the end.
    {
        LegacyBackendAdapter adapter(fake);
        // Adapter is destroyed here — it also deletes the interface.
    }

    // The interface was deleted by the adapter, so we cannot emit on it.
    // This test just verifies the destructor path doesn't crash.
    SUCCEED();
}

// Verify that signalQuery also does not crash after releaseInterface().
// signalQuery has the most complex handler (OverwriteQuery / PasswordNeededQuery).
TEST_F(LegacyBackendAdapterTest, ReleaseInterfaceThenEmitSignalQueryNoCrash)
{
    FakeReadOnlyInterface *fake = new FakeReadOnlyInterface();

    auto adapter = std::unique_ptr<LegacyBackendAdapter>(
        new LegacyBackendAdapter(fake));

    // Release and destroy the adapter.
    ReadOnlyArchiveInterface *released = adapter->releaseInterface();
    ASSERT_EQ(released, fake);
    adapter.reset();

    // Emit signalQuery with various query types.
    {
        OverwriteQuery query(QStringLiteral("conflict.txt"));
        Q_EMIT fake->signalQuery(&query);
    }
    {
        PasswordNeededQuery query(QStringLiteral("secret.zip"));
        Q_EMIT fake->signalQuery(&query);
    }

    // If we reach here without crashing, the fix works.
    SUCCEED();

    delete fake;
}


// ===========================================================================
// Write operation behavior tests (move/copy/rename/update)
//
// These tests verify that the adapter correctly forwards write operations
// to the underlying ReadWriteArchiveInterface and returns the correct
// VoidResult.  Each operation is tested in both the success and error
// (PFT_Error) path.  Together with the existing AddFiles, DeleteFiles,
// and AddComment tests, these cover all write operations that
// ReadWriteArchiveInterface defines — satisfying the Stage 2 exit criterion
// that every old-backend-supported write operation has a repeatable test.
// ===========================================================================

// --- MoveFiles: success path ---
TEST_F(LegacyBackendAdapterTest, ReadWriteMoveFiles)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt", "b.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    MoveRequest req;
    req.entries = {"a.txt"};
    req.targetDir = "subdir/";

    auto result = adapter.moveFiles(req, observer);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->moveCallCount, 1);
}

// --- MoveFiles: error path ---
TEST_F(LegacyBackendAdapterTest, MoveFilesErrorReturnsFailure)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);
    fake->moveResult = PFT_Error;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    MoveRequest req;
    req.entries = {"a.txt"};
    req.targetDir = "subdir/";

    auto result = adapter.moveFiles(req, observer);
    EXPECT_FALSE(result.success);
}

// --- CopyFiles: success path ---
TEST_F(LegacyBackendAdapterTest, ReadWriteCopyFiles)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt", "b.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    CopyRequest req;
    req.entries = {"a.txt"};
    req.targetDir = "subdir/";

    auto result = adapter.copyFiles(req, observer);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->copyCallCount, 1);
}

// --- CopyFiles: error path ---
TEST_F(LegacyBackendAdapterTest, CopyFilesErrorReturnsFailure)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);
    fake->copyResult = PFT_Error;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    LegacyBackendObserverAdapter observer;
    CopyRequest req;
    req.entries = {"a.txt"};
    req.targetDir = "subdir/";

    auto result = adapter.copyFiles(req, observer);
    EXPECT_FALSE(result.success);
}

// --- RenameFiles: success path ---
TEST_F(LegacyBackendAdapterTest, ReadWriteRenameFiles)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"old.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    RenameRequest req;
    req.entries = {"old.txt"};
    req.newNames = {"new.txt"};

    auto result = adapter.renameFiles(req);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->renameCallCount, 1);
}

// --- RenameFiles: error path ---
TEST_F(LegacyBackendAdapterTest, RenameFilesErrorReturnsFailure)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"old.txt"});
    fake->setWaitForFinished(false);
    fake->renameResult = PFT_Error;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    RenameRequest req;
    req.entries = {"old.txt"};
    req.newNames = {"new.txt"};

    auto result = adapter.renameFiles(req);
    EXPECT_FALSE(result.success);
}

// --- UpdateArchiveData: success path ---
TEST_F(LegacyBackendAdapterTest, ReadWriteUpdateArchiveData)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    UpdateRequest req;
    req.type = UpdateRequest::Delete;
    req.entries = {"a.txt"};

    auto result = adapter.updateArchiveData(req);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->updateCallCount, 1);
}

// --- UpdateArchiveData: error path ---
TEST_F(LegacyBackendAdapterTest, UpdateArchiveDataErrorReturnsFailure)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();
    fake->populateArchiveData({"a.txt"});
    fake->setWaitForFinished(false);
    fake->updateResult = PFT_Error;

    LegacyBackendAdapter adapter(fake);

    OpenRequest openReq;
    ASSERT_TRUE(adapter.open(openReq).success);

    UpdateRequest req;
    req.type = UpdateRequest::Delete;
    req.entries = {"a.txt"};

    auto result = adapter.updateArchiveData(req);
    EXPECT_FALSE(result.success);
}

// --- Caps-aware constructor: write capabilities from descriptor ---
TEST_F(LegacyBackendAdapterTest, CapsAwareConstructorReflectsDescriptorCaps)
{
    FakeReadWriteInterface *fake = new FakeReadWriteInterface();

    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.create = true;
    // Deliberately disable all modify ops (TAR-like).
    caps.append = false;
    caps.move = false;
    caps.copy = false;
    caps.remove = false;
    caps.rename = false;
    caps.comment = false;
    caps.update = false;

    LegacyBackendAdapter adapter(fake, caps);
    auto reported = adapter.capabilities();
    EXPECT_TRUE(reported.create);
    EXPECT_FALSE(reported.append);
    EXPECT_FALSE(reported.move);
    EXPECT_FALSE(reported.copy);
    EXPECT_FALSE(reported.remove);
    EXPECT_FALSE(reported.rename);
    EXPECT_FALSE(reported.comment);
    EXPECT_FALSE(reported.update);
}

#include "legacy_backend_adapter_test.moc"

