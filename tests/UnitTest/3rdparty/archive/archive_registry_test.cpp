// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "archive_registry.h"
#include "archive_types.h"
#include "backend_capabilities.h"
#include "legacy_backend_adapter.h"
#include "legacy_backend_descriptors.h"
#include "legacy_backend_registry.h"
#include "archiveinterface.h"
#include "datamanager.h"

#include <QCoreApplication>
#include <memory>

using namespace archive;

// ---------------------------------------------------------------------------
// Minimal fake ReadWriteArchiveInterface for capability consistency tests.
//
// The fake implements every pure-virtual method inherited from
// ReadOnlyArchiveInterface and ReadWriteArchiveInterface so that
// registerLegacyBackends() can instantiate a LegacyBackendAdapter via
// qobject_cast<ReadWriteArchiveInterface*>.  All methods return success
// — the tests only inspect capabilities(), not operation results.
// ---------------------------------------------------------------------------
class RegistryTestRWInterface : public ReadWriteArchiveInterface
{
    Q_OBJECT
public:
    explicit RegistryTestRWInterface()
        : ReadWriteArchiveInterface(nullptr, {})
    {
        setWaitForFinishedSignal(false);
    }

    PluginFinishType list() override { return PFT_Nomral; }
    PluginFinishType testArchive() override { return PFT_Nomral; }
    PluginFinishType extractFiles(const QList<FileEntry> &, const ExtractionOptions &) override { return PFT_Nomral; }
    PluginFinishType addFiles(const QList<FileEntry> &, const CompressOptions &) override { return PFT_Nomral; }
    PluginFinishType moveFiles(const QList<FileEntry> &, const CompressOptions &) override { return PFT_Nomral; }
    PluginFinishType copyFiles(const QList<FileEntry> &, const CompressOptions &) override { return PFT_Nomral; }
    PluginFinishType deleteFiles(const QList<FileEntry> &) override { return PFT_Nomral; }
    PluginFinishType renameFiles(const QList<FileEntry> &) override { return PFT_Nomral; }
    PluginFinishType addComment(const QString &) override { return PFT_Nomral; }
    PluginFinishType updateArchiveData(const UpdateOptions &) override { return PFT_Nomral; }
    void pauseOperation() override {}
    void continueOperation() override {}
    bool doKill() override { return true; }
};

// A minimal read-only interface for read-only format tests.
class RegistryTestROInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT
public:
    explicit RegistryTestROInterface()
        : ReadOnlyArchiveInterface(nullptr, {})
    {
        setWaitForFinishedSignal(false);
    }

    PluginFinishType list() override { return PFT_Nomral; }
    PluginFinishType testArchive() override { return PFT_Nomral; }
    PluginFinishType extractFiles(const QList<FileEntry> &, const ExtractionOptions &) override { return PFT_Nomral; }
    void pauseOperation() override {}
    void continueOperation() override {}
    bool doKill() override { return true; }
};

namespace {

// A trivial mock backend for testing registry selection.
// Capabilities are set from the descriptor so that createBackendByExtension
// returns a backend whose capabilities match the descriptor.
class MockBackend : public ArchiveBackend
{
public:
    explicit MockBackend(BackendCapabilities caps = BackendCapabilities{})
        : m_caps(std::move(caps))
    {
        m_caps.list = true; // every backend can list
    }
    BackendCapabilities capabilities() const override { return m_caps; }
    VoidResult open(const OpenRequest &) override { return VoidResult::ok(); }
    EntryResult list() override { return EntryResult::ok(); }
    VoidResult testArchive() override { return VoidResult::ok(); }
    VoidResult extractFiles(const ExtractRequest &, BackendObserver &) override { return VoidResult::ok(); }
    VoidResult addFiles(const AddRequest &, BackendObserver &) override { return VoidResult::notSupported(); }
    VoidResult moveFiles(const MoveRequest &, BackendObserver &) override { return VoidResult::notSupported(); }
    VoidResult copyFiles(const CopyRequest &, BackendObserver &) override { return VoidResult::notSupported(); }
    VoidResult deleteFiles(const DeleteRequest &) override { return VoidResult::notSupported(); }
    VoidResult renameFiles(const RenameRequest &) override { return VoidResult::notSupported(); }
    VoidResult addComment(const QString &) override { return VoidResult::notSupported(); }
    VoidResult updateArchiveData(const UpdateRequest &) override { return VoidResult::notSupported(); }
    void pause() override {}
    void resume() override {}
    void cancel() override {}
private:
    BackendCapabilities m_caps;
};

// Factory that always returns a valid backend with default capabilities.
std::unique_ptr<ArchiveBackend> makeMockBackend(const QString &, bool)
{
    return std::make_unique<MockBackend>();
}

// Build a factory that creates a MockBackend with the given capabilities.
// This lets us verify that createBackendByExtension returns a backend whose
// capabilities match the registered descriptor.
BackendFactory makeMockBackendFactory(BackendCapabilities caps)
{
    return [caps](const QString &, bool) -> std::unique_ptr<ArchiveBackend> {
        return std::make_unique<MockBackend>(caps);
    };
}

// Factory that always fails (returns nullptr).
std::unique_ptr<ArchiveBackend> makeNullBackend(const QString &, bool)
{
    return nullptr;
}

BackendDescriptor makeDescriptor(const QString &id,
                                  const QStringList &mimes,
                                  const QStringList &exts,
                                  int priority,
                                  bool isLegacy,
                                  BackendFactory factory)
{
    BackendDescriptor d;
    d.id = id;
    d.mimeTypes = mimes;
    d.extensions = exts;
    d.priority = priority;
    d.isLegacy = isLegacy;
    d.factory = factory;
    d.capabilities.list = true;
    return d;
}

class ArchiveRegistryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clean registry state for each test
        auto &reg = ArchiveRegistry::instance();
        for (const auto &b : reg.backends()) {
            reg.unregisterBackend(b.id);
        }
        reg.setUseLegacyBackends(true);
    }
};

TEST_F(ArchiveRegistryTest, RegisterAndUnregister)
{
    auto &reg = ArchiveRegistry::instance();
    auto desc = makeDescriptor("test-zip", {"application/zip"}, {"zip"}, 10, false, makeMockBackend);
    reg.registerBackend(desc);

    EXPECT_EQ(reg.backends().size(), 1u);
    EXPECT_EQ(reg.backends().first().id, "test-zip");

    reg.unregisterBackend("test-zip");
    EXPECT_EQ(reg.backends().size(), 0u);
}

TEST_F(ArchiveRegistryTest, FindByMimeType)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("test-zip", {"application/zip"}, {"zip"}, 10, false, makeMockBackend));
    reg.registerBackend(makeDescriptor("test-7z", {"application/x-7z-compressed"}, {"7z"}, 5, false, makeMockBackend));

    auto zipBackends = reg.findBackends("application/zip");
    EXPECT_EQ(zipBackends.size(), 1u);
    EXPECT_EQ(zipBackends.first().id, "test-zip");

    auto sevenzBackends = reg.findBackends("application/x-7z-compressed");
    EXPECT_EQ(sevenzBackends.size(), 1u);
    EXPECT_EQ(sevenzBackends.first().id, "test-7z");
}

TEST_F(ArchiveRegistryTest, FindByExtension)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("test-zip", {"application/zip"}, {"zip"}, 10, false, makeMockBackend));

    auto extBackends = reg.findBackendsByExtension("zip");
    EXPECT_EQ(extBackends.size(), 1u);
    EXPECT_EQ(extBackends.first().id, "test-zip");
}

TEST_F(ArchiveRegistryTest, PrioritySort)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("low", {"application/zip"}, {"zip"}, 1, false, makeMockBackend));
    reg.registerBackend(makeDescriptor("high", {"application/zip"}, {"zip"}, 100, false, makeMockBackend));
    reg.registerBackend(makeDescriptor("mid", {"application/zip"}, {"zip"}, 50, false, makeMockBackend));

    auto backends = reg.findBackends("application/zip");
    ASSERT_EQ(backends.size(), 3u);
    EXPECT_EQ(backends[0].id, "high");
    EXPECT_EQ(backends[1].id, "mid");
    EXPECT_EQ(backends[2].id, "low");
}

TEST_F(ArchiveRegistryTest, UseLegacyFlagFiltersLegacy)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("legacy", {"application/zip"}, {"zip"}, 100, true, makeMockBackend));
    reg.registerBackend(makeDescriptor("native", {"application/zip"}, {"zip"}, 10, false, makeMockBackend));

    // With legacy enabled, both visible, legacy first (higher priority)
    reg.setUseLegacyBackends(true);
    auto all = reg.findBackends("application/zip");
    EXPECT_EQ(all.size(), 2u);

    // With legacy disabled, only native visible
    reg.setUseLegacyBackends(false);
    auto filtered = reg.findBackends("application/zip");
    EXPECT_EQ(filtered.size(), 1u);
    EXPECT_EQ(filtered.first().id, "native");
}

TEST_F(ArchiveRegistryTest, CreateBackendSelectsHighestPriority)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("low", {"application/zip"}, {"zip"}, 1, false, makeMockBackend));
    reg.registerBackend(makeDescriptor("high", {"application/zip"}, {"zip"}, 100, false, makeMockBackend));

    auto backend = reg.createBackend("/tmp/test.zip", "application/zip", false);
    ASSERT_NE(backend, nullptr);
    // Both factories succeed; highest priority is selected first
    EXPECT_TRUE(backend->capabilities().list);
}

TEST_F(ArchiveRegistryTest, CreateBackendFallbackOnNullFactory)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("null", {"application/zip"}, {"zip"}, 100, false, makeNullBackend));
    reg.registerBackend(makeDescriptor("good", {"application/zip"}, {"zip"}, 1, false, makeMockBackend));

    auto backend = reg.createBackend("/tmp/test.zip", "application/zip", false);
    ASSERT_NE(backend, nullptr);
    // Null factory failed, should fall back to "good"
    EXPECT_TRUE(backend->capabilities().list);
}

TEST_F(ArchiveRegistryTest, CreateBackendReturnsNullWhenNoMatch)
{
    auto &reg = ArchiveRegistry::instance();
    auto backend = reg.createBackend("/tmp/test.unknown", "application/unknown", false);
    EXPECT_EQ(backend, nullptr);
}

TEST_F(ArchiveRegistryTest, MimeTypeForExtension)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("test-zip", {"application/zip"}, {"zip"}, 10, false, makeMockBackend));

    EXPECT_EQ(reg.mimeTypeForExtension("zip"), "application/zip");
    EXPECT_TRUE(reg.mimeTypeForExtension("unknown").isEmpty());
}

TEST_F(ArchiveRegistryTest, CreateBackendByExtension)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("test-zip", {"application/zip"}, {"zip"}, 10, false, makeMockBackend));

    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    ASSERT_NE(backend, nullptr);
}

TEST_F(ArchiveRegistryTest, DuplicateIdReplaces)
{
    auto &reg = ArchiveRegistry::instance();
    reg.registerBackend(makeDescriptor("dup", {"application/zip"}, {"zip"}, 10, false, makeMockBackend));
    reg.registerBackend(makeDescriptor("dup", {"application/x-7z-compressed"}, {"7z"}, 20, false, makeMockBackend));

    EXPECT_EQ(reg.backends().size(), 1u);
    auto zipBackends = reg.findBackends("application/zip");
    EXPECT_EQ(zipBackends.size(), 0u);
    auto sevenzBackends = reg.findBackends("application/x-7z-compressed");
    EXPECT_EQ(sevenzBackends.size(), 1u);
}

// ---------------------------------------------------------------------------
// Legacy backend descriptor integration tests
//
// These tests verify per-format capability declarations and extension/MIME
// routing using the same descriptor factory functions that
// registerLegacyBackends() uses in production.  A mock factory is passed
// instead of createLegacyBackend so the tests do not depend on UiTools
// or the KDE plugin system.
// ---------------------------------------------------------------------------

class LegacyDescriptorTest : public ::testing::Test
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

    void registerAllLegacyDescriptors()
    {
        auto &reg = ArchiveRegistry::instance();
        // Build descriptors first so we can pass their capabilities to the
        // mock factory — this lets createBackendByExtension return a backend
        // whose capabilities match the descriptor.
        {
            auto desc = makeReadWriteLegacyDescriptor(
                "legacy-zip", {"application/zip"}, {"zip"}, true, false, {});
            desc.factory = makeMockBackendFactory(desc.capabilities);
            reg.registerBackend(desc);
        }
        {
            auto desc = makeReadWriteLegacyDescriptor(
                "legacy-7z", {"application/x-7z-compressed"}, {"7z"}, true, true, {});
            desc.factory = makeMockBackendFactory(desc.capabilities);
            reg.registerBackend(desc);
        }
        {
            auto desc = makeReadWriteLegacyDescriptor(
                "legacy-tar",
                {"application/x-tar", "application/x-gzip", "application/x-bzip2", "application/x-xz"},
                {"tar", "gz", "tgz", "bz2", "tbz2", "xz", "txz"},
                false, false, {});
            desc.factory = makeMockBackendFactory(desc.capabilities);
            reg.registerBackend(desc);
        }
        {
            auto desc = makeReadOnlyLegacyDescriptor(
                "legacy-rar",
                {"application/x-rar", "application/vnd.rar", "application/x-rar-compressed"},
                {"rar"}, {});
            desc.factory = makeMockBackendFactory(desc.capabilities);
            reg.registerBackend(desc);
        }
        {
            auto desc = makeReadOnlyLegacyDescriptor(
                "legacy-iso", {"application/x-iso9660-image"}, {"iso"}, {});
            desc.factory = makeMockBackendFactory(desc.capabilities);
            reg.registerBackend(desc);
        }
        {
            auto desc = makeReadOnlyLegacyDescriptor(
                "legacy-ar", {"application/x-archive"}, {"ar", "deb"}, {});
            desc.factory = makeMockBackendFactory(desc.capabilities);
            reg.registerBackend(desc);
        }
    }
};

// --- ZIP: read + write, modify, no multi-volume ---

TEST_F(LegacyDescriptorTest, ZipDescriptorCapabilities)
{
    auto desc = makeReadWriteLegacyDescriptor(
        "legacy-zip", {"application/zip"}, {"zip"}, true, false, makeMockBackend);

    EXPECT_TRUE(desc.capabilities.list);
    EXPECT_TRUE(desc.capabilities.test);
    EXPECT_TRUE(desc.capabilities.extract);
    EXPECT_TRUE(desc.capabilities.create);
    EXPECT_TRUE(desc.capabilities.append);
    EXPECT_TRUE(desc.capabilities.remove);
    EXPECT_TRUE(desc.capabilities.rename);
    EXPECT_TRUE(desc.capabilities.headerPassword);
    EXPECT_FALSE(desc.capabilities.multiVolume);
    EXPECT_TRUE(desc.isLegacy);
}

// --- 7z: read + write, modify, multi-volume ---

TEST_F(LegacyDescriptorTest, SevenZipDescriptorCapabilities)
{
    auto desc = makeReadWriteLegacyDescriptor(
        "legacy-7z", {"application/x-7z-compressed"}, {"7z"}, true, true, makeMockBackend);

    EXPECT_TRUE(desc.capabilities.create);
    EXPECT_TRUE(desc.capabilities.append);
    EXPECT_TRUE(desc.capabilities.multiVolume);
    EXPECT_TRUE(desc.capabilities.headerPassword);
}

// --- TAR: create but no modify ---

TEST_F(LegacyDescriptorTest, TarDescriptorCapabilities)
{
    auto desc = makeReadWriteLegacyDescriptor(
        "legacy-tar", {"application/x-tar"}, {"tar"}, false, false, makeMockBackend);

    EXPECT_TRUE(desc.capabilities.create);
    EXPECT_FALSE(desc.capabilities.append);
    EXPECT_FALSE(desc.capabilities.remove);
    EXPECT_FALSE(desc.capabilities.rename);
    EXPECT_FALSE(desc.capabilities.multiVolume);
}

// --- RAR: read-only ---

TEST_F(LegacyDescriptorTest, RarDescriptorCapabilities)
{
    auto desc = makeReadOnlyLegacyDescriptor(
        "legacy-rar", {"application/x-rar"}, {"rar"}, makeMockBackend);

    EXPECT_TRUE(desc.capabilities.list);
    EXPECT_TRUE(desc.capabilities.test);
    EXPECT_TRUE(desc.capabilities.extract);
    EXPECT_FALSE(desc.capabilities.create);
    EXPECT_FALSE(desc.capabilities.append);
    EXPECT_FALSE(desc.capabilities.remove);
    EXPECT_FALSE(desc.capabilities.rename);
    EXPECT_FALSE(desc.capabilities.multiVolume);
}

// --- ISO: read-only ---

TEST_F(LegacyDescriptorTest, IsoDescriptorCapabilities)
{
    auto desc = makeReadOnlyLegacyDescriptor(
        "legacy-iso", {"application/x-iso9660-image"}, {"iso"}, makeMockBackend);

    EXPECT_TRUE(desc.capabilities.extract);
    EXPECT_FALSE(desc.capabilities.create);
    EXPECT_FALSE(desc.capabilities.append);
}

// --- Extension routing ---

TEST_F(LegacyDescriptorTest, ExtensionRoutingZip)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    auto backends = reg.findBackendsByExtension("zip");
    ASSERT_EQ(backends.size(), 1u);
    EXPECT_EQ(backends.first().id, "legacy-zip");
}

TEST_F(LegacyDescriptorTest, ExtensionRouting7z)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    auto backends = reg.findBackendsByExtension("7z");
    ASSERT_EQ(backends.size(), 1u);
    EXPECT_EQ(backends.first().id, "legacy-7z");
}

TEST_F(LegacyDescriptorTest, ExtensionRoutingRar)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    auto backends = reg.findBackendsByExtension("rar");
    ASSERT_EQ(backends.size(), 1u);
    EXPECT_EQ(backends.first().id, "legacy-rar");
}

TEST_F(LegacyDescriptorTest, ExtensionRoutingTar)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    auto backends = reg.findBackendsByExtension("tar");
    ASSERT_EQ(backends.size(), 1u);
    EXPECT_EQ(backends.first().id, "legacy-tar");
}

// --- MIME routing ---

TEST_F(LegacyDescriptorTest, MimeRoutingZip)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    auto backends = reg.findBackends("application/zip");
    ASSERT_EQ(backends.size(), 1u);
    EXPECT_EQ(backends.first().id, "legacy-zip");
}

// --- Feature flag: legacy disabled filters out all legacy descriptors ---

TEST_F(LegacyDescriptorTest, FeatureFlagFiltersLegacy)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();

    reg.setUseLegacyBackends(true);
    EXPECT_EQ(reg.findBackendsByExtension("zip").size(), 1u);

    reg.setUseLegacyBackends(false);
    EXPECT_EQ(reg.findBackendsByExtension("zip").size(), 0u);
    EXPECT_EQ(reg.findBackendsByExtension("rar").size(), 0u);
    EXPECT_EQ(reg.findBackendsByExtension("7z").size(), 0u);
}

// --- createBackendByExtension works for legacy formats ---

TEST_F(LegacyDescriptorTest, CreateBackendByExtensionZip)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    ASSERT_NE(backend, nullptr);
    EXPECT_TRUE(backend->capabilities().list);
}

TEST_F(LegacyDescriptorTest, CreateBackendByExtensionRar)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    auto backend = reg.createBackendByExtension("/tmp/test.rar", false);
    ASSERT_NE(backend, nullptr);
    // RAR is read-only
    EXPECT_TRUE(backend->capabilities().extract);
    EXPECT_FALSE(backend->capabilities().create);
}

// --- All descriptors registered with correct count ---

TEST_F(LegacyDescriptorTest, AllFormatsRegistered)
{
    registerAllLegacyDescriptors();
    auto &reg = ArchiveRegistry::instance();
    EXPECT_EQ(reg.backends().size(), 6u);
}


// ---------------------------------------------------------------------------
// registerLegacyBackends() integration tests
//
// These tests call the real registerLegacyBackends() function (not a helper)
// to verify the production registration path: 6 backends registered, correct
// format routing, capability declarations, feature-flag filtering, and
// fallback behavior when the interface factory yields nullptr.
// ---------------------------------------------------------------------------

class RegisterLegacyBackendsTest : public ::testing::Test
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
};

// Calling registerLegacyBackends() with no factory uses the default factory
// (stub returns nullptr in test build).  Registration still registers all 6
// descriptors — the factory is only invoked during backend instantiation.
TEST_F(RegisterLegacyBackendsTest, RegistersSixBackendsWithDefaultFactory)
{
    registerLegacyBackends();
    auto &reg = ArchiveRegistry::instance();
    EXPECT_EQ(reg.backends().size(), 6u);
}

// Verify extension routing through the real registration path.
TEST_F(RegisterLegacyBackendsTest, ExtensionRoutingAfterRegistration)
{
    registerLegacyBackends();
    auto &reg = ArchiveRegistry::instance();

    EXPECT_EQ(reg.findBackendsByExtension("zip").first().id, "legacy-zip");
    EXPECT_EQ(reg.findBackendsByExtension("7z").first().id, "legacy-7z");
    EXPECT_EQ(reg.findBackendsByExtension("rar").first().id, "legacy-rar");
    EXPECT_EQ(reg.findBackendsByExtension("iso").first().id, "legacy-iso");
    EXPECT_EQ(reg.findBackendsByExtension("tar").first().id, "legacy-tar");
    EXPECT_EQ(reg.findBackendsByExtension("ar").first().id, "legacy-ar");
    EXPECT_EQ(reg.findBackendsByExtension("deb").first().id, "legacy-ar");
}

// Verify MIME routing through the real registration path.
TEST_F(RegisterLegacyBackendsTest, MimeRoutingAfterRegistration)
{
    registerLegacyBackends();
    auto &reg = ArchiveRegistry::instance();

    EXPECT_EQ(reg.findBackends("application/zip").first().id, "legacy-zip");
    EXPECT_EQ(reg.findBackends("application/x-7z-compressed").first().id, "legacy-7z");
    EXPECT_EQ(reg.findBackends("application/x-rar").first().id, "legacy-rar");
    EXPECT_EQ(reg.findBackends("application/x-iso9660-image").first().id, "legacy-iso");
    EXPECT_EQ(reg.findBackends("application/x-archive").first().id, "legacy-ar");
}

// Verify per-format capability declarations through the real path.
TEST_F(RegisterLegacyBackendsTest, CapabilityDeclarationsAfterRegistration)
{
    registerLegacyBackends();
    auto &reg = ArchiveRegistry::instance();

    // ZIP: read + write + modify, no multi-volume
    auto zip = reg.findBackendsByExtension("zip").first();
    EXPECT_TRUE(zip.capabilities.create);
    EXPECT_TRUE(zip.capabilities.append);
    EXPECT_FALSE(zip.capabilities.multiVolume);

    // 7z: read + write + modify + multi-volume
    auto sevenz = reg.findBackendsByExtension("7z").first();
    EXPECT_TRUE(sevenz.capabilities.create);
    EXPECT_TRUE(sevenz.capabilities.multiVolume);

    // TAR: create but no modify
    auto tar = reg.findBackendsByExtension("tar").first();
    EXPECT_TRUE(tar.capabilities.create);
    EXPECT_FALSE(tar.capabilities.append);

    // RAR: read-only
    auto rar = reg.findBackendsByExtension("rar").first();
    EXPECT_FALSE(rar.capabilities.create);
    EXPECT_TRUE(rar.capabilities.extract);
}

// Verify feature flag filtering through the real path.
TEST_F(RegisterLegacyBackendsTest, FeatureFlagFiltersLegacyAfterRegistration)
{
    registerLegacyBackends();
    auto &reg = ArchiveRegistry::instance();

    reg.setUseLegacyBackends(true);
    EXPECT_EQ(reg.findBackendsByExtension("zip").size(), 1u);

    reg.setUseLegacyBackends(false);
    EXPECT_EQ(reg.findBackendsByExtension("zip").size(), 0u);
    EXPECT_EQ(reg.findBackendsByExtension("rar").size(), 0u);
    EXPECT_EQ(reg.findBackendsByExtension("7z").size(), 0u);
}

// Verify fallback: when the factory returns nullptr (stub in test build),
// createBackendByExtension returns nullptr — the selection chain tried the
// factory and fell through.
TEST_F(RegisterLegacyBackendsTest, FallbackWhenFactoryReturnsNullForNonZip)
{
    registerLegacyBackends();
    auto &reg = ArchiveRegistry::instance();

    // Default factory (stub) returns nullptr for non-zip formats.
    auto backend = reg.createBackendByExtension("/tmp/test.7z", false);
    EXPECT_EQ(backend, nullptr);
}

// Verify that passing an explicit mock factory to registerLegacyBackends()
// is used during backend instantiation.
TEST_F(RegisterLegacyBackendsTest, CustomFactoryUsedForInstantiation)
{
    bool factoryCalled = false;
    LegacyInterfaceFactory mockFactory = [&factoryCalled](const QString &, bool) -> ReadOnlyArchiveInterface * {
        factoryCalled = true;
        return nullptr; // no plugin available
    };

    registerLegacyBackends(mockFactory);
    auto &reg = ArchiveRegistry::instance();

    EXPECT_EQ(reg.backends().size(), 6u);

    // Factory is not called during registration.
    EXPECT_FALSE(factoryCalled);

    // Factory is called during backend instantiation.
    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_EQ(backend, nullptr);
    EXPECT_TRUE(factoryCalled);
}


// Verify that the factory captured by registerLegacyBackends() remains
// valid after the function returns.  This catches the use-after-scope bug
// where the factory parameter was captured by reference ([&factory]) instead
// of by value ([factory]).  When captured by reference, the parameter is
// destroyed when registerLegacyBackends() returns, leaving the registered
// lambda with a dangling reference.  Delayed creation then invokes the
// dangling reference — a use-after-scope that ASan/LSan would flag.
TEST_F(RegisterLegacyBackendsTest, FactorySurvivesAfterRegisterReturns)
{
    auto alive = std::make_shared<bool>(true);
    int callCount = 0;

    {
        LegacyInterfaceFactory mockFactory =
            [alive, &callCount](const QString &, bool) -> ReadOnlyArchiveInterface * {
            // If the factory was captured by reference and the parameter
            // is already destroyed, accessing the captured factory (and
            // thus this lambda) would be UB.  The shared_ptr check adds
            // a concrete signal that the capture is still valid.
            EXPECT_TRUE(*alive);
            ++callCount;
            return nullptr;
        };

        registerLegacyBackends(mockFactory);
        // mockFactory and the factory parameter are both destroyed here.
        // The registered lambda must have its own copy (by-value capture).
    }

    auto &reg = ArchiveRegistry::instance();
    EXPECT_EQ(reg.backends().size(), 6u);

    // Delayed creation: the factory lambda is invoked AFTER
    // registerLegacyBackends() has returned and its parameter destroyed.
    // createLegacyBackend() now respects bWrite, so with bWrite=false the
    // mock factory is called once per createBackendByExtension().
    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(callCount, 1);

    // Verify with a second extension to ensure multiple calls work.
    auto backend2 = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_EQ(backend2, nullptr);
    EXPECT_EQ(callCount, 2);

    *alive = false; // mark for cleanup
}


// ---------------------------------------------------------------------------
// bWrite semantics tests
// ---------------------------------------------------------------------------

// Factory that records the bWrite value it was called with.
struct BWriteRecorder {
    bool lastBWrite = false;
    int callCount = 0;
    LegacyInterfaceFactory factory() {
        return [this](const QString &, bool bWrite) -> ReadOnlyArchiveInterface * {
            lastBWrite = bWrite;
            ++callCount;
            return nullptr;
        };
    }
};

// Verify that bWrite=false passes false to the factory.
TEST_F(RegisterLegacyBackendsTest, ReadOnlyRequestPassesBWriteFalse)
{
    BWriteRecorder rec;
    registerLegacyBackends(rec.factory());
    auto &reg = ArchiveRegistry::instance();

    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(rec.callCount, 1);
    EXPECT_FALSE(rec.lastBWrite);
}

// Verify that bWrite=true passes true to the factory for write-capable
// descriptors (zip, 7z, tar).
TEST_F(RegisterLegacyBackendsTest, WriteRequestPassesBWriteTrue)
{
    BWriteRecorder rec;
    registerLegacyBackends(rec.factory());
    auto &reg = ArchiveRegistry::instance();

    auto backend = reg.createBackendByExtension("/tmp/test.zip", true);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(rec.callCount, 1);
    EXPECT_TRUE(rec.lastBWrite);
}

// Verify that bWrite=true skips read-only descriptors (RAR, ISO, AR).
// The factory should not be called at all for these formats when a write
// interface is requested.
TEST_F(RegisterLegacyBackendsTest, WriteRequestSkipsReadOnlyDescriptors)
{
    BWriteRecorder rec;
    registerLegacyBackends(rec.factory());
    auto &reg = ArchiveRegistry::instance();

    // RAR is read-only — factory must not be called for bWrite=true.
    auto rarBackend = reg.createBackendByExtension("/tmp/test.rar", true);
    EXPECT_EQ(rarBackend, nullptr);
    EXPECT_EQ(rec.callCount, 0);

    // ISO is read-only.
    auto isoBackend = reg.createBackendByExtension("/tmp/test.iso", true);
    EXPECT_EQ(isoBackend, nullptr);
    EXPECT_EQ(rec.callCount, 0);

    // AR/DEB is read-only.
    auto arBackend = reg.createBackendByExtension("/tmp/test.ar", true);
    EXPECT_EQ(arBackend, nullptr);
    EXPECT_EQ(rec.callCount, 0);
}

// Verify that bWrite=false still works for read-only formats (factory
// is called with false).
TEST_F(RegisterLegacyBackendsTest, ReadOnlyRequestWorksForReadOnlyFormats)
{
    BWriteRecorder rec;
    registerLegacyBackends(rec.factory());
    auto &reg = ArchiveRegistry::instance();

    auto backend = reg.createBackendByExtension("/tmp/test.rar", false);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(rec.callCount, 1);
    EXPECT_FALSE(rec.lastBWrite);
}

// Verify that a write request does NOT fall back to a read-only factory
// call.  With the old code, createLegacyBackend tried factory(path, true)
// then factory(path, false).  Now it only calls factory(path, bWrite).
TEST_F(RegisterLegacyBackendsTest, WriteRequestDoesNotFallBackToReadOnly)
{
    int callCount = 0;
    bool lastBWrite = false;
    LegacyInterfaceFactory mockFactory = [&callCount, &lastBWrite](const QString &, bool bWrite) -> ReadOnlyArchiveInterface * {
        ++callCount;
        lastBWrite = bWrite;
        return nullptr;
    };

    registerLegacyBackends(mockFactory);
    auto &reg = ArchiveRegistry::instance();

    // Write request for zip (write-capable) — factory called once with true.
    auto backend = reg.createBackendByExtension("/tmp/test.zip", true);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(callCount, 1);
    EXPECT_TRUE(lastBWrite);
}

// ---------------------------------------------------------------------------
// Registry integration tests: flag on/off, read/write selection, fallback
// ---------------------------------------------------------------------------

// Verify that useLegacyBackends=false excludes legacy descriptors from
// createBackendByExtension — simulating the "flag off → old path" behaviour
// where the registry returns nullptr and UiTools falls back to PluginManager.
TEST_F(ArchiveRegistryTest, FeatureFlagOffExcludesLegacyFromCreateBackend)
{
    auto &reg = ArchiveRegistry::instance();
    BackendCapabilities caps;
    caps.list = true;
    caps.create = true;
    caps.append = true;

    auto desc = makeDescriptor("legacy-zip", {"application/zip"}, {"zip"}, 10, true,
                               makeMockBackendFactory(caps));
    reg.registerBackend(desc);

    // Flag on → legacy descriptor participates.
    reg.setUseLegacyBackends(true);
    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_NE(backend, nullptr);

    // Flag off → legacy descriptor excluded, returns nullptr.
    reg.setUseLegacyBackends(false);
    auto backend2 = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_EQ(backend2, nullptr);
}

// Verify that a write request with no write-capable descriptor returns
// nullptr (no fallback to read-only).
TEST_F(ArchiveRegistryTest, WriteRequestReturnsNullWhenNoWriteCapability)
{
    auto &reg = ArchiveRegistry::instance();
    BackendCapabilities readOnlyCaps;
    readOnlyCaps.list = true;
    readOnlyCaps.extract = true;

    auto desc = makeDescriptor("ro-zip", {"application/zip"}, {"zip"}, 10, false,
                               makeMockBackendFactory(readOnlyCaps));
    desc.capabilities = readOnlyCaps;
    reg.registerBackend(desc);

    // Write request should find no suitable backend.
    auto backend = reg.createBackendByExtension("/tmp/test.zip", true);
    EXPECT_EQ(backend, nullptr);

    // Read request should succeed.
    auto roBackend = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_NE(roBackend, nullptr);
}

// Verify that a write-capable descriptor is preferred over a read-only
// one when bWrite=true, and the read-only one is skipped entirely.
TEST_F(ArchiveRegistryTest, WriteRequestPrefersWriteCapableDescriptor)
{
    auto &reg = ArchiveRegistry::instance();
    BackendCapabilities readOnlyCaps;
    readOnlyCaps.list = true;

    BackendCapabilities writeCaps;
    writeCaps.list = true;
    writeCaps.create = true;
    writeCaps.append = true;

    int roCallCount = 0;
    int rwCallCount = 0;

    auto roFactory = [&roCallCount](const QString &, bool) -> std::unique_ptr<ArchiveBackend> {
        ++roCallCount;
        return nullptr;
    };
    auto rwFactory = [&rwCallCount, writeCaps](const QString &, bool) -> std::unique_ptr<ArchiveBackend> {
        ++rwCallCount;
        return std::make_unique<MockBackend>(writeCaps);
    };

    // Register read-only with higher priority — it should still be skipped
    // for write requests.
    auto roDesc = makeDescriptor("ro-zip", {"application/zip"}, {"zip"}, 20, false, roFactory);
    roDesc.capabilities = readOnlyCaps;
    reg.registerBackend(roDesc);

    auto rwDesc = makeDescriptor("rw-zip", {"application/zip"}, {"zip"}, 10, false, rwFactory);
    rwDesc.capabilities = writeCaps;
    reg.registerBackend(rwDesc);

    auto backend = reg.createBackendByExtension("/tmp/test.zip", true);
    EXPECT_NE(backend, nullptr);
    EXPECT_EQ(roCallCount, 0);  // read-only skipped
    EXPECT_EQ(rwCallCount, 1);  // write-capable used
}

// Verify that factory failure (nullptr) falls through to the next descriptor
// in priority order — simulating the "registry failure → fallback" behaviour.
TEST_F(ArchiveRegistryTest, FactoryFailureFallsThroughToNextDescriptor)
{
    auto &reg = ArchiveRegistry::instance();
    BackendCapabilities caps;
    caps.list = true;
    caps.create = true;
    caps.append = true;

    // High-priority descriptor that always fails.
    auto failDesc = makeDescriptor("fail-zip", {"application/zip"}, {"zip"}, 20, false,
                                   makeNullBackend);
    failDesc.capabilities = caps;
    reg.registerBackend(failDesc);

    // Low-priority descriptor that succeeds.
    auto okDesc = makeDescriptor("ok-zip", {"application/zip"}, {"zip"}, 10, false,
                                 makeMockBackendFactory(caps));
    okDesc.capabilities = caps;
    reg.registerBackend(okDesc);

    auto backend = reg.createBackendByExtension("/tmp/test.zip", true);
    EXPECT_NE(backend, nullptr);
}

// Verify that a write-capable descriptor returns a valid backend for
// bWrite=true.
TEST_F(ArchiveRegistryTest, WriteRequestReturnsValidBackend)
{
    auto &reg = ArchiveRegistry::instance();
    BackendCapabilities writeCaps;
    writeCaps.list = true;
    writeCaps.create = true;
    writeCaps.append = true;

    auto desc = makeDescriptor("test-write-zip", {"application/zip"}, {"zip"}, 10, false,
                               makeMockBackendFactory(writeCaps));
    desc.capabilities = writeCaps;
    reg.registerBackend(desc);

    auto backend = reg.createBackendByExtension("/tmp/test.zip", true);
    EXPECT_NE(backend, nullptr);
    EXPECT_TRUE(backend->capabilities().create);
}

// Verify that read request also works for write-capable descriptors.
TEST_F(ArchiveRegistryTest, ReadRequestWorksForWriteCapableDescriptor)
{
    auto &reg = ArchiveRegistry::instance();
    BackendCapabilities writeCaps;
    writeCaps.list = true;
    writeCaps.create = true;
    writeCaps.append = true;

    auto desc = makeDescriptor("test-write-zip", {"application/zip"}, {"zip"}, 10, false,
                               makeMockBackendFactory(writeCaps));
    desc.capabilities = writeCaps;
    reg.registerBackend(desc);

    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    EXPECT_NE(backend, nullptr);
}


// ===========================================================================
// Operation-type routing tests (V-8285 Gap 1)
// ===========================================================================

// Verify that createBackend with ArchiveOperation::List selects a backend
// that supports list, even when it doesn't support write operations.
TEST_F(ArchiveRegistryTest, OperationRouting_ListSelectsListCapableBackend)
{
    auto &reg = ArchiveRegistry::instance();
    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;

    auto desc = makeDescriptor("ro-zip", {"application/zip"}, {"zip"}, 10, false,
                               makeMockBackendFactory(caps));
    desc.capabilities = caps;
    reg.registerBackend(desc);

    ErrorCode ec = ErrorCode::Unknown;
    auto backend = reg.createBackendByExtension("/tmp/test.zip",
                                                 ArchiveOperation::List, &ec);
    EXPECT_NE(backend, nullptr);
    EXPECT_EQ(ec, ErrorCode::NoError);
}

// Verify that a backend matching the format but NOT supporting the requested
// operation is skipped, and ErrorCode::NotSupported is returned.
TEST_F(ArchiveRegistryTest, OperationRouting_UnsupportedOperationReturnsNotSupported)
{
    auto &reg = ArchiveRegistry::instance();
    // Read-only backend: supports List/Test/Extract but not Add/Move/Delete.
    BackendCapabilities readOnlyCaps;
    readOnlyCaps.list = true;
    readOnlyCaps.test = true;
    readOnlyCaps.extract = true;

    auto desc = makeDescriptor("ro-zip", {"application/zip"}, {"zip"}, 10, false,
                               makeMockBackendFactory(readOnlyCaps));
    desc.capabilities = readOnlyCaps;
    reg.registerBackend(desc);

    // Request Add — format matches but operation not supported.
    ErrorCode ec = ErrorCode::Unknown;
    auto backend = reg.createBackendByExtension("/tmp/test.zip",
                                                 ArchiveOperation::Add, &ec);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(ec, ErrorCode::NotSupported);
}

// Verify that when multiple backends match the format, the one supporting
// the requested operation is selected even if it has lower priority.
TEST_F(ArchiveRegistryTest, OperationRouting_SkipsHighPriorityForOperation)
{
    auto &reg = ArchiveRegistry::instance();

    // High-priority read-only backend (supports List but not Move).
    BackendCapabilities roCaps;
    roCaps.list = true;
    roCaps.test = true;
    roCaps.extract = true;

    int roFactoryCalled = 0;
    auto roFactory = [&roFactoryCalled, roCaps](const QString &, bool) -> std::unique_ptr<ArchiveBackend> {
        ++roFactoryCalled;
        return std::make_unique<MockBackend>(roCaps);
    };

    auto roDesc = makeDescriptor("ro-zip", {"application/zip"}, {"zip"}, 20, false, roFactory);
    roDesc.capabilities = roCaps;
    reg.registerBackend(roDesc);

    // Lower-priority read-write backend (supports Move).
    BackendCapabilities rwCaps;
    rwCaps.list = true;
    rwCaps.test = true;
    rwCaps.extract = true;
    rwCaps.create = true;
    rwCaps.append = true;
    rwCaps.move = true;
    rwCaps.copy = true;
    rwCaps.remove = true;
    rwCaps.rename = true;

    int rwFactoryCalled = 0;
    auto rwFactory = [&rwFactoryCalled, rwCaps](const QString &, bool) -> std::unique_ptr<ArchiveBackend> {
        ++rwFactoryCalled;
        return std::make_unique<MockBackend>(rwCaps);
    };

    auto rwDesc = makeDescriptor("rw-zip", {"application/zip"}, {"zip"}, 10, false, rwFactory);
    rwDesc.capabilities = rwCaps;
    reg.registerBackend(rwDesc);

    // Request Move — high-priority RO is skipped, lower-priority RW is selected.
    auto backend = reg.createBackendByExtension("/tmp/test.zip",
                                                 ArchiveOperation::Move);
    EXPECT_NE(backend, nullptr);
    EXPECT_EQ(roFactoryCalled, 0);
    EXPECT_EQ(rwFactoryCalled, 1);
}

// Verify that Delete operation maps to the 'remove' capability.
TEST_F(ArchiveRegistryTest, OperationRouting_DeleteMapsToRemoveCapability)
{
    auto &reg = ArchiveRegistry::instance();

    // Backend with create+append but NOT remove.
    BackendCapabilities capsNoRemove;
    capsNoRemove.list = true;
    capsNoRemove.create = true;
    capsNoRemove.append = true;
    capsNoRemove.move = true;
    capsNoRemove.copy = true;
    capsNoRemove.rename = true;

    auto desc = makeDescriptor("zip-no-remove", {"application/zip"}, {"zip"}, 10, false,
                               makeMockBackendFactory(capsNoRemove));
    desc.capabilities = capsNoRemove;
    reg.registerBackend(desc);

    ErrorCode ec = ErrorCode::NoError;
    auto backend = reg.createBackendByExtension("/tmp/test.zip",
                                                 ArchiveOperation::Delete, &ec);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(ec, ErrorCode::NotSupported);
}

// Verify that Comment operation maps to the 'comment' capability.
TEST_F(ArchiveRegistryTest, OperationRouting_CommentMapsToCommentCapability)
{
    auto &reg = ArchiveRegistry::instance();

    BackendCapabilities caps;
    caps.list = true;
    caps.create = true;
    caps.append = true;
    caps.comment = true;

    auto desc = makeDescriptor("zip-comment", {"application/zip"}, {"zip"}, 10, false,
                               makeMockBackendFactory(caps));
    desc.capabilities = caps;
    reg.registerBackend(desc);

    auto backend = reg.createBackendByExtension("/tmp/test.zip",
                                                 ArchiveOperation::Comment);
    EXPECT_NE(backend, nullptr);
    EXPECT_TRUE(backend->capabilities().comment);
}

// Verify that unknown format returns nullptr with NoError (not NotSupported).
TEST_F(ArchiveRegistryTest, OperationRouting_UnknownFormatReturnsNullNoError)
{
    auto &reg = ArchiveRegistry::instance();

    ErrorCode ec = ErrorCode::NoError;
    auto backend = reg.createBackendByExtension("/tmp/test.unknownformat",
                                                 ArchiveOperation::List, &ec);
    EXPECT_EQ(backend, nullptr);
    EXPECT_EQ(ec, ErrorCode::NoError);
}

// Verify that Extract operation selects extract-capable backend.
TEST_F(ArchiveRegistryTest, OperationRouting_ExtractSelectsExtractCapable)
{
    auto &reg = ArchiveRegistry::instance();

    // Backend that supports list but not extract.
    BackendCapabilities listOnly;
    listOnly.list = true;

    auto roDesc = makeDescriptor("list-only", {"application/zip"}, {"zip"}, 20, false,
                                 makeMockBackendFactory(listOnly));
    roDesc.capabilities = listOnly;
    reg.registerBackend(roDesc);

    // Backend that supports extract.
    BackendCapabilities extractCaps;
    extractCaps.list = true;
    extractCaps.extract = true;

    auto exDesc = makeDescriptor("extract-capable", {"application/zip"}, {"zip"}, 10, false,
                                 makeMockBackendFactory(extractCaps));
    exDesc.capabilities = extractCaps;
    reg.registerBackend(exDesc);

    auto backend = reg.createBackendByExtension("/tmp/test.zip",
                                                 ArchiveOperation::Extract);
    EXPECT_NE(backend, nullptr);
    EXPECT_TRUE(backend->capabilities().extract);
}

// Verify factory failure falls through to next operation-capable backend.
TEST_F(ArchiveRegistryTest, OperationRouting_FactoryFailureFallsThrough)
{
    auto &reg = ArchiveRegistry::instance();

    BackendCapabilities caps;
    caps.list = true;
    caps.extract = true;

    // High-priority that fails.
    auto failDesc = makeDescriptor("fail-zip", {"application/zip"}, {"zip"}, 20, false,
                                   makeNullBackend);
    failDesc.capabilities = caps;
    reg.registerBackend(failDesc);

    // Low-priority that succeeds.
    auto okDesc = makeDescriptor("ok-zip", {"application/zip"}, {"zip"}, 10, false,
                                 makeMockBackendFactory(caps));
    okDesc.capabilities = caps;
    reg.registerBackend(okDesc);

    auto backend = reg.createBackendByExtension("/tmp/test.zip",
                                                 ArchiveOperation::Extract);
    EXPECT_NE(backend, nullptr);
}


// ===========================================================================
// Capability consistency: adapter-reported capabilities match descriptor
//
// These tests verify that backends created through registerLegacyBackends()
// report capabilities() that match the descriptor used for operation
// routing.  This catches the regression where the old
// LegacyBackendAdapter(ReadWriteArchiveInterface*) constructor hardcoded
// all write capabilities to true, even when the descriptor declared them
// false (e.g. TAR with supportsModify=false).
// ===========================================================================

// Factory that returns a read-write interface for write-capable formats
// (ZIP, 7z, TAR) and a read-only interface for read-only formats (RAR).
static LegacyInterfaceFactory makeConsistencyTestFactory()
{
    return [](const QString &path, bool bWrite) -> ReadOnlyArchiveInterface * {
        // For write-capable formats, return a RW interface.
        if (path.endsWith(".zip") || path.endsWith(".7z") || path.endsWith(".tar")) {
            return new RegistryTestRWInterface();
        }
        // For read-only formats (rar, iso, ar), return a RO interface.
        return new RegistryTestROInterface();
    };
}

// Verify ZIP backend capabilities match the ZIP descriptor.
TEST_F(RegisterLegacyBackendsTest, AdapterCapsMatchDescriptor_ZIP)
{
    registerLegacyBackends(makeConsistencyTestFactory());
    auto &reg = ArchiveRegistry::instance();

    auto desc = reg.findBackendsByExtension("zip").first();
    auto backend = reg.createBackendByExtension("/tmp/test.zip", false);
    ASSERT_NE(backend, nullptr);

    auto caps = backend->capabilities();
    EXPECT_EQ(caps.create, desc.capabilities.create);
    EXPECT_EQ(caps.append, desc.capabilities.append);
    EXPECT_EQ(caps.move, desc.capabilities.move);
    EXPECT_EQ(caps.copy, desc.capabilities.copy);
    EXPECT_EQ(caps.remove, desc.capabilities.remove);
    EXPECT_EQ(caps.rename, desc.capabilities.rename);
    EXPECT_EQ(caps.comment, desc.capabilities.comment);
    EXPECT_EQ(caps.update, desc.capabilities.update);
    EXPECT_EQ(caps.multiVolume, desc.capabilities.multiVolume);
    // ZIP has all write caps enabled.
    EXPECT_TRUE(caps.create);
    EXPECT_TRUE(caps.append);
    EXPECT_TRUE(caps.move);
    EXPECT_TRUE(caps.copy);
    EXPECT_TRUE(caps.remove);
    EXPECT_TRUE(caps.rename);
    EXPECT_TRUE(caps.comment);
    EXPECT_TRUE(caps.update);
    EXPECT_FALSE(caps.multiVolume);
}

// Verify 7z backend capabilities match the 7z descriptor.
TEST_F(RegisterLegacyBackendsTest, AdapterCapsMatchDescriptor_7z)
{
    registerLegacyBackends(makeConsistencyTestFactory());
    auto &reg = ArchiveRegistry::instance();

    auto desc = reg.findBackendsByExtension("7z").first();
    auto backend = reg.createBackendByExtension("/tmp/test.7z", false);
    ASSERT_NE(backend, nullptr);

    auto caps = backend->capabilities();
    EXPECT_EQ(caps.create, desc.capabilities.create);
    EXPECT_EQ(caps.append, desc.capabilities.append);
    EXPECT_EQ(caps.move, desc.capabilities.move);
    EXPECT_EQ(caps.copy, desc.capabilities.copy);
    EXPECT_EQ(caps.remove, desc.capabilities.remove);
    EXPECT_EQ(caps.rename, desc.capabilities.rename);
    EXPECT_EQ(caps.comment, desc.capabilities.comment);
    EXPECT_EQ(caps.update, desc.capabilities.update);
    EXPECT_EQ(caps.multiVolume, desc.capabilities.multiVolume);
    // 7z has all write caps + multi-volume.
    EXPECT_TRUE(caps.multiVolume);
}

// Verify TAR backend capabilities match the TAR descriptor — this is the
// key regression test: TAR descriptor has supportsModify=false, so
// append/move/copy/remove/rename/comment/update must all be false in
// the adapter-reported capabilities, even though the adapter wraps a
// ReadWriteArchiveInterface.
TEST_F(RegisterLegacyBackendsTest, AdapterCapsMatchDescriptor_TAR)
{
    registerLegacyBackends(makeConsistencyTestFactory());
    auto &reg = ArchiveRegistry::instance();

    auto desc = reg.findBackendsByExtension("tar").first();
    auto backend = reg.createBackendByExtension("/tmp/test.tar", false);
    ASSERT_NE(backend, nullptr);

    auto caps = backend->capabilities();
    EXPECT_EQ(caps.create, desc.capabilities.create);
    EXPECT_EQ(caps.append, desc.capabilities.append);
    EXPECT_EQ(caps.move, desc.capabilities.move);
    EXPECT_EQ(caps.copy, desc.capabilities.copy);
    EXPECT_EQ(caps.remove, desc.capabilities.remove);
    EXPECT_EQ(caps.rename, desc.capabilities.rename);
    EXPECT_EQ(caps.comment, desc.capabilities.comment);
    EXPECT_EQ(caps.update, desc.capabilities.update);
    // TAR has create=true but all modify caps=false.
    EXPECT_TRUE(caps.create);
    EXPECT_FALSE(caps.append);
    EXPECT_FALSE(caps.move);
    EXPECT_FALSE(caps.copy);
    EXPECT_FALSE(caps.remove);
    EXPECT_FALSE(caps.rename);
    EXPECT_FALSE(caps.comment);
    EXPECT_FALSE(caps.update);
}

// Verify RAR backend capabilities match the RAR descriptor (read-only).
TEST_F(RegisterLegacyBackendsTest, AdapterCapsMatchDescriptor_RAR)
{
    registerLegacyBackends(makeConsistencyTestFactory());
    auto &reg = ArchiveRegistry::instance();

    auto desc = reg.findBackendsByExtension("rar").first();
    auto backend = reg.createBackendByExtension("/tmp/test.rar", false);
    ASSERT_NE(backend, nullptr);

    auto caps = backend->capabilities();
    EXPECT_EQ(caps.create, desc.capabilities.create);
    EXPECT_EQ(caps.append, desc.capabilities.append);
    EXPECT_EQ(caps.move, desc.capabilities.move);
    EXPECT_EQ(caps.copy, desc.capabilities.copy);
    EXPECT_EQ(caps.remove, desc.capabilities.remove);
    EXPECT_EQ(caps.rename, desc.capabilities.rename);
    EXPECT_EQ(caps.comment, desc.capabilities.comment);
    EXPECT_EQ(caps.update, desc.capabilities.update);
    // RAR is read-only.
    EXPECT_FALSE(caps.create);
    EXPECT_FALSE(caps.append);
    EXPECT_FALSE(caps.move);
    EXPECT_FALSE(caps.copy);
    EXPECT_FALSE(caps.remove);
    EXPECT_FALSE(caps.rename);
    EXPECT_FALSE(caps.comment);
    EXPECT_FALSE(caps.update);
}


} // namespace

#include "archive_registry_test.moc"
