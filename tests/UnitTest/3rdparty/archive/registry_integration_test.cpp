// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "registry_integration.h"
#include "archive_registry.h"
#include "archive_types.h"
#include "backend_capabilities.h"
#include "legacy_backend_adapter.h"
#include "legacy_backend_descriptors.h"
#include "legacy_backend_registry.h"
#include "archiveinterface.h"

#include <QCoreApplication>
#include <QByteArray>
#include <memory>

using namespace archive;

// ---------------------------------------------------------------------------
// Mock ReadOnlyArchiveInterface — minimal concrete implementation so we can
// verify that tryRegistryBackend returns the interface wrapped inside a
// LegacyBackendAdapter.
// ---------------------------------------------------------------------------
class MockReadOnlyInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT
public:
    explicit MockReadOnlyInterface(QObject *parent = nullptr)
        : ReadOnlyArchiveInterface(parent, QVariantList())
    {
    }

    PluginFinishType list() override { return PFT_Nomral; }
    PluginFinishType testArchive() override { return PFT_Nomral; }
    PluginFinishType extractFiles(const QList<FileEntry> &, const ExtractionOptions &) override
    {
        return PFT_Nomral;
    }
    void pauseOperation() override {}
    void continueOperation() override {}
};

// ---------------------------------------------------------------------------
// Test fixture — restores env var and registry state after each test.
// ---------------------------------------------------------------------------
class RegistryIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Save and clear the env var so tests start from a known state.
        m_envWasSet = qEnvironmentVariableIsSet("DEEPIN_COMPRESSOR_USE_REGISTRY");
        if (m_envWasSet) {
            m_savedValue = qgetenv("DEEPIN_COMPRESSOR_USE_REGISTRY");
        }
        qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "0"); // disabled by default

        // Clean registry state for each test.
        auto &reg = ArchiveRegistry::instance();
        for (const auto &b : reg.backends()) {
            reg.unregisterBackend(b.id);
        }
        reg.setUseLegacyBackends(true);
    }

    void TearDown() override
    {
        // Restore env var state — never leave residue for other tests.
        if (m_envWasSet) {
            qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", m_savedValue.constData());
        } else {
            qunsetenv("DEEPIN_COMPRESSOR_USE_REGISTRY");
        }

        // Clean registry.
        auto &reg = ArchiveRegistry::instance();
        for (const auto &b : reg.backends()) {
            reg.unregisterBackend(b.id);
        }
    }

    // Helper: register a legacy backend that wraps the given interface.
    // The factory captures the interface pointer and hands it to a
    // LegacyBackendAdapter, matching the production createLegacyBackend() flow.
    static BackendDescriptor makeLegacyDescriptor(
        const QString &id,
        const QStringList &mimes,
        const QStringList &exts,
        int priority,
        ReadOnlyArchiveInterface *iface,
        bool *factoryCalled = nullptr,
        bool *bWriteReceived = nullptr)
    {
        BackendDescriptor d;
        d.id = id;
        d.mimeTypes = mimes;
        d.extensions = exts;
        d.priority = priority;
        d.isLegacy = true;
        d.capabilities.list = true;
        d.capabilities.create = true;
        d.capabilities.append = true;
        d.factory = [iface, factoryCalled, bWriteReceived](
                        const QString &, bool bWrite) -> std::unique_ptr<ArchiveBackend> {
            if (factoryCalled)
                *factoryCalled = true;
            if (bWriteReceived)
                *bWriteReceived = bWrite;
            if (!iface)
                return nullptr;
            return std::make_unique<LegacyBackendAdapter>(iface);
        };
        return d;
    }

    // Helper: register a non-legacy (self-developed) backend that returns
    // a plain MockBackend, not a LegacyBackendAdapter.  tryRegistryBackend
    // should NOT extract an interface from this (dynamic_cast fails).
    static BackendDescriptor makeNonLegacyDescriptor(
        const QString &id,
        const QStringList &mimes,
        const QStringList &exts,
        int priority)
    {
        BackendDescriptor d;
        d.id = id;
        d.mimeTypes = mimes;
        d.extensions = exts;
        d.priority = priority;
        d.isLegacy = false;
        d.capabilities.list = true;
        d.factory = [](const QString &, bool) -> std::unique_ptr<ArchiveBackend> {
            // Return a backend that is NOT a LegacyBackendAdapter.
            class PlainBackend : public ArchiveBackend
            {
            public:
                BackendCapabilities capabilities() const override
                {
                    BackendCapabilities c;
                    c.list = true;
                    return c;
                }
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
            };
            return std::make_unique<PlainBackend>();
        };
        return d;
    }

private:
    bool m_envWasSet = false;
    QByteArray m_savedValue;
};

// ===========================================================================
// Environment variable semantics
// ===========================================================================

TEST_F(RegistryIntegrationTest, EnvVarUnsetMeansDisabled)
{
    qunsetenv("DEEPIN_COMPRESSOR_USE_REGISTRY");
    EXPECT_FALSE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarZeroMeansDisabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "0");
    EXPECT_FALSE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarFalseMeansDisabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "false");
    EXPECT_FALSE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarFalseCaseInsensitiveMeansDisabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "FALSE");
    EXPECT_FALSE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarOneMeansEnabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");
    EXPECT_TRUE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarTrueMeansEnabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "true");
    EXPECT_TRUE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarTrueCaseInsensitiveMeansEnabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "TRUE");
    EXPECT_TRUE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarTrueMixedCaseMeansEnabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "True");
    EXPECT_TRUE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarEmptyStringMeansDisabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "");
    EXPECT_FALSE(isRegistryFeatureEnabled());
}

TEST_F(RegistryIntegrationTest, EnvVarGarbageMeansDisabled)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "yes");
    EXPECT_FALSE(isRegistryFeatureEnabled());
}

// ===========================================================================
// tryRegistryBackend — flag disabled
// ===========================================================================

TEST_F(RegistryIntegrationTest, FlagDisabledReturnsNullptr)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "0");

    // Register a backend so we can prove the flag gate, not the empty
    // registry, is the reason for nullptr.
    MockReadOnlyInterface iface;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("test-zip", {"application/zip"}, {"zip"}, 10, &iface));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);
}

TEST_F(RegistryIntegrationTest, FlagUnsetReturnsNullptr)
{
    qunsetenv("DEEPIN_COMPRESSOR_USE_REGISTRY");

    MockReadOnlyInterface iface;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("test-zip", {"application/zip"}, {"zip"}, 10, &iface));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);
}

// ===========================================================================
// tryRegistryBackend — flag enabled, Registry hit
// ===========================================================================

TEST_F(RegistryIntegrationTest, FlagEnabledRegistryHitReturnsInterface)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    MockReadOnlyInterface iface;
    bool factoryCalled = false;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("test-zip", {"application/zip"}, {"zip"}, 10,
                             &iface, &factoryCalled));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &iface);
    EXPECT_TRUE(factoryCalled);
}

// ===========================================================================
// tryRegistryBackend — flag enabled, Registry miss
// ===========================================================================

TEST_F(RegistryIntegrationTest, FlagEnabledRegistryMissReturnsNullptr)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    // No backends registered — Registry has nothing.
    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);
}

// ===========================================================================
// tryRegistryBackend — factory failure falls through
// ===========================================================================

TEST_F(RegistryIntegrationTest, FlagEnabledFactoryFailsReturnsNullptr)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    // Register a backend whose factory returns nullptr (simulating plugin
    // load failure).
    bool factoryCalled = false;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("fail-zip", {"application/zip"}, {"zip"}, 10,
                             nullptr, &factoryCalled));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);
    EXPECT_TRUE(factoryCalled);
}

TEST_F(RegistryIntegrationTest, FlagEnabledFactoryFailureFallsThroughToNext)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    // High-priority backend that fails.
    bool failFactoryCalled = false;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("fail-zip", {"application/zip"}, {"zip"}, 20,
                             nullptr, &failFactoryCalled));

    // Low-priority backend that succeeds.
    MockReadOnlyInterface iface;
    bool okFactoryCalled = false;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("ok-zip", {"application/zip"}, {"zip"}, 10,
                             &iface, &okFactoryCalled));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &iface);
    EXPECT_TRUE(failFactoryCalled);
    EXPECT_TRUE(okFactoryCalled);
}

// ===========================================================================
// bWrite propagation
// ===========================================================================

TEST_F(RegistryIntegrationTest, BWriteFalsePropagatedToFactory)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    MockReadOnlyInterface iface;
    bool bWriteReceived = true; // expect false
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("test-zip", {"application/zip"}, {"zip"}, 10,
                             &iface, nullptr, &bWriteReceived));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_NE(result, nullptr);
    EXPECT_FALSE(bWriteReceived);
}

TEST_F(RegistryIntegrationTest, BWriteTruePropagatedToFactory)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    MockReadOnlyInterface iface;
    bool bWriteReceived = false; // expect true
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("test-zip", {"application/zip"}, {"zip"}, 10,
                             &iface, nullptr, &bWriteReceived));

    auto *result = tryRegistryBackend("/tmp/test.zip", true);
    EXPECT_NE(result, nullptr);
    EXPECT_TRUE(bWriteReceived);
}

// ===========================================================================
// Non-legacy backend (not a LegacyBackendAdapter) — dynamic_cast fails
// ===========================================================================

TEST_F(RegistryIntegrationTest, NonLegacyBackendReturnsNullptr)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    // A non-legacy backend returns a plain ArchiveBackend, not a
    // LegacyBackendAdapter.  tryRegistryBackend should return nullptr
    // because dynamic_cast<LegacyBackendAdapter*> fails.
    ArchiveRegistry::instance().registerBackend(
        makeNonLegacyDescriptor("self-zip", {"application/zip"}, {"zip"}, 10));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);
}

// ===========================================================================
// Recursion guard
// ===========================================================================

TEST_F(RegistryIntegrationTest, RecursionGuardPreventsReentry)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    // Register a backend whose factory calls tryRegistryBackend recursively.
    // The recursive call must return nullptr (guard active) and not
    // infinite-loop.
    int outerCallCount = 0;
    int innerCallResult = -1; // 0 = nullptr, 1 = non-null

    BackendDescriptor d;
    d.id = "recursive-zip";
    d.mimeTypes = {"application/zip"};
    d.extensions = {"zip"};
    d.priority = 10;
    d.isLegacy = true;
    d.capabilities.list = true;
    d.capabilities.create = true;
    d.capabilities.append = true;
    d.factory = [&outerCallCount, &innerCallResult](
                    const QString &path, bool bWrite) -> std::unique_ptr<ArchiveBackend> {
        ++outerCallCount;
        // Recursive call — should be blocked by the guard.
        auto *inner = tryRegistryBackend(path, bWrite);
        innerCallResult = (inner != nullptr) ? 1 : 0;
        // Return nullptr so the outer call also fails.
        return nullptr;
    };
    ArchiveRegistry::instance().registerBackend(d);

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(outerCallCount, 1);        // outer factory called once
    EXPECT_EQ(innerCallResult, 0);       // inner call returned nullptr
}

TEST_F(RegistryIntegrationTest, RecursionGuardResetsAfterCall)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    MockReadOnlyInterface iface1;
    MockReadOnlyInterface iface2;

    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("test-zip-1", {"application/zip"}, {"zip"}, 20, &iface1));

    // First call should succeed.
    auto *result1 = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_NE(result1, nullptr);
    EXPECT_EQ(result1, &iface1);

    // Clean up and register a different backend for the second call.
    ArchiveRegistry::instance().unregisterBackend("test-zip-1");
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("test-zip-2", {"application/zip"}, {"zip"}, 20, &iface2));

    // Second call should also succeed — guard was reset.
    auto *result2 = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(result2, &iface2);
}

// ===========================================================================
// Smoke: Registry failure does not return dangling pointer
// ===========================================================================

TEST_F(RegistryIntegrationTest, RegistryFailureReturnsNullptrNotDangling)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    // Factory returns nullptr → no backend created → tryRegistryBackend
    // must return nullptr, never a dangling pointer.
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("fail-zip", {"application/zip"}, {"zip"}, 10, nullptr));

    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);
}

// ===========================================================================
// End-to-end: flag on + real backend → returns usable interface
// ===========================================================================

TEST_F(RegistryIntegrationTest, EndToEndReturnsUsableInterface)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    MockReadOnlyInterface iface;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("e2e-zip", {"application/zip"}, {"zip"}, 10, &iface));

    auto *result = tryRegistryBackend("/tmp/archive.zip", false);

    // Verify the returned interface is the one we registered and is usable.
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result, &iface);
    EXPECT_EQ(result->list(), PFT_Nomral);
    EXPECT_EQ(result->testArchive(), PFT_Nomral);
}

// ===========================================================================
// useLegacyBackends flag interaction
// ===========================================================================

TEST_F(RegistryIntegrationTest, LegacyDisabledExcludesLegacyBackend)
{
    qputenv("DEEPIN_COMPRESSOR_USE_REGISTRY", "1");

    MockReadOnlyInterface iface;
    ArchiveRegistry::instance().registerBackend(
        makeLegacyDescriptor("legacy-zip", {"application/zip"}, {"zip"}, 10, &iface));

    // When useLegacyBackends is false, the legacy descriptor is excluded.
    ArchiveRegistry::instance().setUseLegacyBackends(false);
    auto *result = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_EQ(result, nullptr);

    // Re-enable and verify it works.
    ArchiveRegistry::instance().setUseLegacyBackends(true);
    auto *result2 = tryRegistryBackend("/tmp/test.zip", false);
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(result2, &iface);
}

#include "registry_integration_test.moc"
