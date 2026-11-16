// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "archive_registry.h"
#include "archive_types.h"
#include "backend_capabilities.h"
#include "backend_observer.h"
#include "legacy_backend_adapter.h"
#include "legacy_backend_descriptors.h"
#include "legacy_backend_registry.h"
#include "registry_integration.h"
#include "legacy_backend_observer_adapter.h"
#include "archiveinterface.h"
#include "datamanager.h"
#include "queries.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QMimeDatabase>
#include <QScopeGuard>
#include <memory>

#include "libzipplugin.h"

Q_DECLARE_METATYPE(KPluginMetaData)

using namespace archive;

// ---------------------------------------------------------------------------
// E2E fake interface — emits the full signal sequence that a real plugin
// would emit during an extract operation: progress, current file, password
// query, and finished.
// ---------------------------------------------------------------------------
class E2EFakeInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT
public:
    explicit E2EFakeInterface()
        : ReadOnlyArchiveInterface(nullptr, {})
    {
        setWaitForFinishedSignal(false);
    }

    void enableWaitForFinishedSignal() { setWaitForFinishedSignal(true); }

    // Control flags
    bool emitProgressAndFile = false;
    bool emitPasswordQuery = false;
    bool emitFinishSignal = false;

    // Recorded state
    int listCallCount = 0;
    int testCallCount = 0;
    int extractCallCount = 0;
    PluginFinishType listResult = PFT_Nomral;
    PluginFinishType testResult = PFT_Nomral;
    PluginFinishType extractResult = PFT_Nomral;

    // Pause/resume/cancel tracking
    bool pauseCalled = false;
    bool continueCalled = false;
    bool doKillCalled = false;

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
            data.listRootEntry.append(entry);
        }
    }

    PluginFinishType list() override
    {
        listCallCount++;
        return listResult;
    }

    PluginFinishType testArchive() override
    {
        testCallCount++;
        return testResult;
    }

    PluginFinishType extractFiles(const QList<FileEntry> &,
                                  const ExtractionOptions &) override
    {
        extractCallCount++;
        if (emitProgressAndFile) {
            Q_EMIT signalprogress(25.0);
            Q_EMIT signalCurFileName(QStringLiteral("dir/file1.txt"));
            Q_EMIT signalprogress(50.0);
            Q_EMIT signalCurFileName(QStringLiteral("dir/file2.txt"));
            Q_EMIT signalprogress(100.0);
        }
        if (emitPasswordQuery) {
            PasswordNeededQuery query(QStringLiteral("secret.zip"));
            Q_EMIT signalQuery(&query);
        }
        if (emitFinishSignal) {
            Q_EMIT signalFinished(PFT_Nomral);
        }
        return extractResult;
    }

    void pauseOperation() override { pauseCalled = true; }
    void continueOperation() override { continueCalled = true; }
    bool doKill() override { doKillCalled = true; return true; }
};

// ---------------------------------------------------------------------------
// Recording observer — captures all events for verification.
// ---------------------------------------------------------------------------
class RecordingObserver : public BackendObserver
{
public:
    void onProgress(const ProgressInfo &progress) override
    {
        progressEvents.append(progress.percentage);
    }

    void onCurrentFile(const QString &path) override
    {
        currentFileEvents.append(path);
    }

    void onPasswordRequired(PasswordRequest &request) override
    {
        passwordRequested = true;
        passwordHeaderPassword = request.headerPassword;
        request.password = QStringLiteral("testpass");
    }

    void onFileConflict(FileConflictRequest &request) override
    {
        fileConflictRequested = true;
        request.action = FileConflictRequest::Overwrite;
    }

    void onWarning(const WarningInfo &warning) override
    {
        warnings.append(warning.message);
    }

    void onFinished(const FinishInfo &finish) override
    {
        finishedCalled = true;
        finishSuccess = finish.success;
        finishCode = finish.code;
    }

    QList<double> progressEvents;
    QStringList currentFileEvents;
    bool passwordRequested = false;
    bool passwordHeaderPassword = false;
    bool fileConflictRequested = false;
    bool finishedCalled = false;
    bool finishSuccess = false;
    ErrorCode finishCode = ErrorCode::NoError;
    QStringList warnings;
};

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class ArchiveBackendE2ETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto &reg = ArchiveRegistry::instance();
        for (const auto &b : reg.backends()) {
            reg.unregisterBackend(b.id);
        }
        reg.setUseLegacyBackends(true);
    }

    void TearDown() override
    {
        auto &reg = ArchiveRegistry::instance();
        for (const auto &b : reg.backends()) {
            reg.unregisterBackend(b.id);
        }
    }

    // Register a legacy backend wrapping the given fake interface.
    void registerLegacyBackend(const QString &id,
                               const QStringList &mimes,
                               const QStringList &exts,
                               ReadOnlyArchiveInterface *iface,
                               BackendCapabilities caps = {})
    {
        BackendDescriptor d;
        d.id = id;
        d.mimeTypes = mimes;
        d.extensions = exts;
        d.priority = 0;
        d.isLegacy = true;
        d.capabilities = caps;
        if (caps.list == false && caps.extract == false && caps.create == false) {
            // Default to read-only caps
            d.capabilities.list = true;
            d.capabilities.test = true;
            d.capabilities.extract = true;
            d.capabilities.password = true;
            d.capabilities.progress = true;
            d.capabilities.cancel = true;
            d.capabilities.pause = true;
        }
        d.factory = [iface](const QString &, bool) -> std::unique_ptr<ArchiveBackend> {
            if (!iface)
                return nullptr;
            return std::make_unique<LegacyBackendAdapter>(iface);
        };
        ArchiveRegistry::instance().registerBackend(d);
    }
};

// ===========================================================================
// E2E: ArchiveBackend::open + list through the registry
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, OpenAndListThroughRegistry)
{
    E2EFakeInterface *fake = new E2EFakeInterface();
    fake->populateArchiveData({"a.txt", "b.txt"});

    registerLegacyBackend("e2e-zip", {"application/zip"}, {"zip"}, fake);

    // Get a backend through the registry — NOT through tryRegistryBackend
    // which releases the interface. This proves the business layer can use
    // ArchiveBackend directly.
    auto backend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::List);
    ASSERT_NE(backend, nullptr);

    // Open through ArchiveBackend
    OpenRequest openReq;
    openReq.archivePath = "/tmp/test.zip";
    ASSERT_TRUE(backend->open(openReq).success);

    // List through ArchiveBackend
    EntryResult result = backend->list();
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->listCallCount, 1);
    EXPECT_EQ(result.entries.size(), 2);


}

// ===========================================================================
// E2E: ArchiveBackend::testArchive through the registry
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, TestArchiveThroughRegistry)
{
    E2EFakeInterface *fake = new E2EFakeInterface();
    fake->populateArchiveData({"a.txt"});

    registerLegacyBackend("e2e-zip", {"application/zip"}, {"zip"}, fake);

    auto backend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::Test);
    ASSERT_NE(backend, nullptr);

    OpenRequest openReq;
    openReq.archivePath = "/tmp/test.zip";
    ASSERT_TRUE(backend->open(openReq).success);

    VoidResult result = backend->testArchive();
    EXPECT_TRUE(result.success);
    EXPECT_EQ(fake->testCallCount, 1);


}

// ===========================================================================
// E2E: ArchiveBackend::extractFiles with full observer event sequence
//
// This test proves the ArchiveBackend business boundary: the observer
// receives progress, current file, and finish events in the correct order.
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, ExtractFilesWithObserverEventSequence)
{
    E2EFakeInterface *fake = new E2EFakeInterface();
    fake->populateArchiveData({"dir/file1.txt", "dir/file2.txt"});
    fake->emitProgressAndFile = true;

    registerLegacyBackend("e2e-zip", {"application/zip"}, {"zip"}, fake);

    auto backend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::Extract);
    ASSERT_NE(backend, nullptr);

    OpenRequest openReq;
    openReq.archivePath = "/tmp/test.zip";
    ASSERT_TRUE(backend->open(openReq).success);

    RecordingObserver observer;
    ExtractRequest extractReq;
    extractReq.targetPath = "/tmp/extract";
    VoidResult result = backend->extractFiles(extractReq, observer);

    // Verify observer received the full progress + current file event sequence
    ASSERT_EQ(observer.progressEvents.size(), 3);
    EXPECT_DOUBLE_EQ(observer.progressEvents[0], 25.0);
    EXPECT_DOUBLE_EQ(observer.progressEvents[1], 50.0);
    EXPECT_DOUBLE_EQ(observer.progressEvents[2], 100.0);

    ASSERT_EQ(observer.currentFileEvents.size(), 2);
    EXPECT_EQ(observer.currentFileEvents[0], "dir/file1.txt");
    EXPECT_EQ(observer.currentFileEvents[1], "dir/file2.txt");

    // Verify finish event
    EXPECT_TRUE(observer.finishedCalled);
    EXPECT_TRUE(observer.finishSuccess);


}

// ===========================================================================
// E2E: ArchiveBackend::extractFiles with password event
//
// Verifies the observer receives a password request event when the
// underlying plugin emits a PasswordNeededQuery.
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, ExtractFilesWithPasswordEvent)
{
    E2EFakeInterface *fake = new E2EFakeInterface();
    fake->populateArchiveData({"secret.txt"});
    fake->emitPasswordQuery = true;
    fake->enableWaitForFinishedSignal();

    registerLegacyBackend("e2e-zip", {"application/zip"}, {"zip"}, fake);

    auto backend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::Extract);
    ASSERT_NE(backend, nullptr);

    // Set list encrypted so the adapter knows it's a header password
    DataManager::get_instance().archiveData().isListEncrypted = true;

    OpenRequest openReq;
    openReq.archivePath = "/tmp/test.zip";
    openReq.headerEncrypted = true;
    ASSERT_TRUE(backend->open(openReq).success);

    RecordingObserver observer;
    observer.passwordHeaderPassword = true; // will be overwritten by event

    ExtractRequest extractReq;
    VoidResult result = backend->extractFiles(extractReq, observer);

    EXPECT_TRUE(observer.passwordRequested);
    EXPECT_TRUE(observer.passwordHeaderPassword);


}

// ===========================================================================
// E2E: ArchiveBackend capabilities match descriptor
//
// Verifies that the backend returned by the registry reports capabilities
// matching the descriptor, proving the business layer can query
// capabilities before attempting an operation.
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, CapabilitiesMatchDescriptor)
{
    E2EFakeInterface *fake = new E2EFakeInterface();

    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.password = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;

    registerLegacyBackend("e2e-zip", {"application/zip"}, {"zip"}, fake, caps);

    auto backend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::Extract);
    ASSERT_NE(backend, nullptr);

    BackendCapabilities reported = backend->capabilities();
    EXPECT_TRUE(reported.list);
    EXPECT_TRUE(reported.test);
    EXPECT_TRUE(reported.extract);
    EXPECT_TRUE(reported.password);
    EXPECT_FALSE(reported.create);
    EXPECT_FALSE(reported.move);


}

// ===========================================================================
// E2E: Operation routing rejects unsupported operation through registry
//
// A read-only backend should not be selectable for Add operation, even
// through the registry path. This proves the business layer gets
// NotSupported when the operation is not available.
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, UnsupportedOperationRejectedThroughRegistry)
{
    E2EFakeInterface *fake = new E2EFakeInterface();

    // Read-only backend
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;

    registerLegacyBackend("e2e-zip", {"application/zip"}, {"zip"}, fake, caps);

    ErrorCode ec = ErrorCode::NoError;
    auto backend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::Add, &ec);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(ec, ErrorCode::NotSupported);

    // List should still work
    auto listBackend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::List);
    EXPECT_NE(listBackend, nullptr);


}

// ===========================================================================
// E2E: ArchiveBackend pause/resume/cancel through registry
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, PauseResumeCancelThroughRegistry)
{
    E2EFakeInterface *fake = new E2EFakeInterface();
    fake->populateArchiveData({"a.txt"});

    registerLegacyBackend("e2e-zip", {"application/zip"}, {"zip"}, fake);

    auto backend = ArchiveRegistry::instance().createBackendByExtension(
        "/tmp/test.zip", ArchiveOperation::List);
    ASSERT_NE(backend, nullptr);

    backend->pause();
    EXPECT_TRUE(fake->pauseCalled);

    backend->resume();
    EXPECT_TRUE(fake->continueCalled);

    backend->cancel();
    EXPECT_TRUE(fake->doKillCalled);


}

// ===========================================================================
// E2E: Real legacy plugin integration test — default factory + feature flag
//
// This test exercises the PRODUCTION default-factory and feature-flag path:
//
//   1. DEEPIN_COMPRESSOR_USE_REGISTRY=1 is set (production feature flag).
//   2. registerLegacyBackends() is called with NO argument, so it uses
//      defaultLegacyInterfaceFactory() — the stub in
//      legacy_backend_factory_stub.cpp which creates a real LibzipPlugin
//      for .zip files.
//   3. tryRegistryBackend(zipPath, false) is called — this is the production
//      entry point in UiTools::createInterface(). It checks the feature flag,
//      selects a backend via ArchiveRegistry, and releases the interface.
//      We assert the returned interface is non-null, proving the default
//      factory was called and returned a real plugin.
//   4. The ArchiveBackend path (open/list/extract) is then verified through
//      ArchiveRegistry::createBackendByExtension(), which also calls the
//      default factory.
//
// Unlike earlier rounds, no test-defined lambda is substituted for the
// default factory. The default factory is the code under test.
// ===========================================================================
TEST_F(ArchiveBackendE2ETest, RealLibzipPluginOpenListExtract)
{
    // --- Set up the production feature flag ---
    const bool envWasSet = qEnvironmentVariableIsSet("DEEPIN_COMPRESSOR_USE_REGISTRY");
    const QByteArray savedVal = qgetenv("DEEPIN_COMPRESSOR_USE_REGISTRY");
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");
    auto envGuard = qScopeGuard([&] {
        if (envWasSet)
            qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", savedVal.constData());
        else
            qunsetenv("DEEPIN_COMPRESSOR_USE_REGISTRY");
    });

    // Path to the real test.zip shipped with the test sources.
    const QString zipPath = QStringLiteral(_UTSOURCEDIR "/test_sources/zip/extract/test.zip");
    ASSERT_TRUE(QFileInfo::exists(zipPath)) << "Test zip not found: " << zipPath.toStdString();

    // Register all legacy backends with NO factory argument.
    // This calls defaultLegacyInterfaceFactory() internally — the stub
    // that creates a real LibzipPlugin for .zip files.
    registerLegacyBackends();

    auto &reg = ArchiveRegistry::instance();

    // --- Verify the production feature-flag path via tryRegistryBackend ---
    // tryRegistryBackend() checks DEEPIN_COMPRESSOR_USE_REGISTRY, selects a
    // backend through ArchiveRegistry (which calls the default factory),
    // and releases the wrapped interface back to the caller.
    ReadOnlyArchiveInterface *registryIface = tryRegistryBackend(zipPath, false);
    ASSERT_NE(registryIface, nullptr)
        << "tryRegistryBackend returned nullptr — default factory or feature flag not working";

    // Verify it is a real LibzipPlugin (not a mock or nullptr).
    auto *libzipIface = qobject_cast<LibzipPlugin *>(registryIface);
    EXPECT_NE(libzipIface, nullptr)
        << "tryRegistryBackend did not return a LibzipPlugin";

    // Clean up the interface released by tryRegistryBackend.
    delete registryIface;

    // --- Verify the ArchiveBackend path through the registry ---
    // Create a backend through the registry — this calls the default
    // factory again, creating a new real LibzipPlugin instance.
    auto backend = reg.createBackendByExtension(zipPath, ArchiveOperation::Extract);
    ASSERT_NE(backend, nullptr);

    // Open through ArchiveBackend — this calls LibzipPlugin::list()
    // internally, which opens the zip file and populates DataManager.
    OpenRequest openReq;
    openReq.archivePath = zipPath;
    ASSERT_TRUE(backend->open(openReq).success);

    // List through ArchiveBackend — reads entries from DataManager
    // populated by the real LibzipPlugin.
    EntryResult listResult = backend->list();
    EXPECT_TRUE(listResult.success);
    // The test.zip contains test.txt and test1.txt.
    EXPECT_GE(listResult.entries.size(), 2);

    // Extract files to a temporary directory.
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    RecordingObserver observer;
    ExtractRequest extractReq;
    extractReq.targetPath = tempDir.path();
    VoidResult extractResult = backend->extractFiles(extractReq, observer);

    // The real LibzipPlugin emits progress and currentFile signals
    // during extraction. Verify the observer received them.
    EXPECT_TRUE(extractResult.success);

    // Verify the observer received progress events (LibzipPlugin emits
    // signalprogress during extraction).
    EXPECT_FALSE(observer.progressEvents.isEmpty());

    // Verify the observer received currentFile events (LibzipPlugin emits
    // signalCurFileName for each file being extracted).
    EXPECT_FALSE(observer.currentFileEvents.isEmpty());

    // Verify the observer received a finished event (the adapter emits
    // onFinished when waitForFinished() returns false, which is the
    // default for LibzipPlugin).
    EXPECT_TRUE(observer.finishedCalled);
    EXPECT_TRUE(observer.finishSuccess);
}

#include "archive_backend_e2e_test.moc"
