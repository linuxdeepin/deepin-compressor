// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "known_differences.h"

namespace archive {

// ---------------------------------------------------------------------------
// Baseline known-difference list.
//
// Each entry documents a specific difference between the old KDE-plugin
// interface (ReadOnlyArchiveInterface / ReadWriteArchiveInterface) and the
// new ArchiveBackend / BackendObserver interface introduced in V-8285.
//
// Entries marked "PendingFollowup" require a follow-up issue in a later
// migration phase: V-8286 (01a1cfaf-ecc6-78a2-bf18-2e4ce9bb3b0e),
// V-8287 (01a1cfaf-eda5-7e70-a14a-f62985fec689), V-8288 (01a1cfaf-ee93-7b95-a5eb-118ef5ed33f1).
// ---------------------------------------------------------------------------

static const KnownDifference s_differences[] = {
    {
        QStringLiteral("KD-001"),
        DifferenceCategory::OldDefect,
        QStringLiteral("UAF when interface is released while signals still connected"),
        QStringLiteral("Old code returned raw interface pointers without disconnecting "
                       "Qt signal connections; if the caller deleted the interface "
                       "while a queued signal was pending, callbacks hit dead memory."),
        QStringLiteral("LegacyBackendAdapter::releaseInterface() and the destructor "
                       "both call disconnectForwardingConnections() to sever all "
                       "forwarding connections before releasing ownership."),
        QStringLiteral("Prevents use-after-free in async signal delivery paths."),
        VerificationStatus::Verified,
        QStringLiteral("ASan UAF regression test passed; disconnectForwardingConnections "
                       "covers all 5 forwarding connections (progress, currentFile, "
                       "error, query, finished).")
    },
    {
        QStringLiteral("KD-002"),
        DifferenceCategory::BehaviourChange,
        QStringLiteral("Operation-type-aware backend routing"),
        QStringLiteral("Old createInterface(fileName, bWrite) only distinguished "
                       "read vs. write; a backend supporting Add but not Move/Copy "
                       "would still be selected for any write operation."),
        QStringLiteral("ArchiveRegistry::createBackend(path, mime, ArchiveOperation, "
                       "errorCode*) checks BackendCapabilities::supports(op) and "
                       "returns ErrorCode::NotSupported when the format matches "
                       "but the operation is not supported."),
        QStringLiteral("Business layer can distinguish 'unknown format' from "
                       "'operation not supported' and present appropriate error."),
        VerificationStatus::Verified,
        QStringLiteral("OperationRouting_* tests in archive_registry_test.cpp "
                       "(8 tests covering List, Add, Move, Delete, Comment, "
                       "Extract, unknown format, factory fallthrough).")
    },
    {
        QStringLiteral("KD-003"),
        DifferenceCategory::BehaviourChange,
        QStringLiteral("Staged compatibility bridge — business layer not yet switched"),
        QStringLiteral("Old business layer calls UiTools::createInterface() which "
                       "returns ReadOnlyArchiveInterface* directly."),
        QStringLiteral("tryRegistryBackend() creates an ArchiveBackend via the "
                       "registry, then releases the old interface back to the caller. "
                       "Full business-layer migration to ArchiveBackend operations "
                       "is a later phase; the current phase provides the bridge "
                       "and proves the ArchiveBackend path is functional via e2e tests."),
        QStringLiteral("Business layer temporarily depends on both old and new "
                       "interfaces; feature flag DEEPIN_COMPRESSOR_USE_REGISTRY "
                       "controls whether the registry path is used."),
        VerificationStatus::AcceptedRisk,
        QStringLiteral("ArchiveBackendE2ETest.* (7 tests) prove ArchiveBackend "
                       "operations (open/list/test/extract/pause/resume/cancel) "
                       "work end-to-end with observer event sequences. "
                       "Full switch is a follow-up phase.")
    },
    {
        QStringLiteral("KD-004"),
        DifferenceCategory::Unsupported,
        QStringLiteral("Write operations not exercised through ArchiveBackend in e2e"),
        QStringLiteral("Old write operations (add/move/copy/delete/rename/comment/"
                       "update) are implemented in LegacyBackendAdapter but not "
                       "covered by e2e tests with real plugin instances."),
        QStringLiteral("ArchiveBackend e2e tests cover read operations (open/list/"
                       "test/extract) and control operations (pause/resume/cancel). "
                       "Write-operation e2e tests require a ReadWriteArchiveInterface "
                       "fake, planned for the CLI backend migration phase."),
        QStringLiteral("Write paths are unit-tested via LegacyBackendAdapter tests "
                       "(move/copy/rename/update success + error paths, comment success path). "
                       "End-to-end through the registry with real plugin instances "
                       "is deferred to V-8286 (libarchive/libzip/pigz/single-file "
                       "backend migration, issue 01a1cfaf-ecc6-78a2-bf18-2e4ce9bb3b0e) "
                       "and V-8287 (CLI backend migration for 7z/RAR, issue "
                       "01a1cfaf-eda5-7e70-a14a-f62985fec689)."),
        VerificationStatus::PendingFollowup,
        QStringLiteral("Follow-up: V-8286 (issue 01a1cfaf-ecc6-78a2-bf18-2e4ce9bb3b0e) "
                       "and V-8287 (issue 01a1cfaf-eda5-7e70-a14a-f62985fec689) will "
                       "add write-operation e2e tests with real backends.")
    },
    {
        QStringLiteral("KD-005"),
        DifferenceCategory::Unsupported,
        QStringLiteral("Progress event lacks processed/total size"),
        QStringLiteral("Old signalprogress(double) only emits a percentage; "
                       "processed and total byte counts are not available."),
        QStringLiteral("BackendObserver::onProgress receives ProgressInfo with "
                       "percentage, processedSize, and totalSize fields, but "
                       "LegacyBackendAdapter only fills percentage (processedSize "
                       "and totalSize remain 0)."),
        QStringLiteral("Observers that need byte-level progress see 0 for size "
                       "fields when using the legacy adapter."),
        VerificationStatus::AcceptedRisk,
        QStringLiteral("Non-breaking: new fields default to 0. Real backends "
                       "migrated in V-8286/V-8287 will populate them.")
    },
    {
        QStringLiteral("KD-006"),
        DifferenceCategory::Unsupported,
        QStringLiteral("File conflict response not fully forwarded"),
        QStringLiteral("Old OverwriteQuery uses a synchronous response model "
                       "via setResponse()."),
        QStringLiteral("LegacyBackendAdapter forwards OverwriteQuery to "
                       "BackendObserver::onFileConflict and reads the response. "
                       "However, the 'Readonly' action and multi-file conflict "
                       "batching are not yet fully mapped."),
        QStringLiteral("The 'Readonly' conflict action may not be correctly "
                       "forwarded in all edge cases."),
        VerificationStatus::PendingFollowup,
        QStringLiteral("Follow-up: test with real conflict scenarios in "
                       "V-8286 (issue 01a1cfaf-ecc6-78a2-bf18-2e4ce9bb3b0e) and "
                       "V-8287 (issue 01a1cfaf-eda5-7e70-a14a-f62985fec689). Current "
                       "basic overwrite/skip/overwrite-all paths are covered by "
                       "unit tests.")
    },
    {
        QStringLiteral("KD-007"),
        DifferenceCategory::OldDefect,
        QStringLiteral("No explicit error-code mapping for all plugin error types"),
        QStringLiteral("Old plugins use a broad PluginFinishType enum; some error "
                       "conditions (missing volume, tool missing, disk full) are "
                       "not consistently distinguished."),
        QStringLiteral("LegacyBackendAdapter::toErrorCode() maps common cases "
                       "(Cancelled, WrongPassword, NeedPassword, etc.) but some "
                       "fall through to ErrorCode::Unknown."),
        QStringLiteral("Callers may see ErrorCode::Unknown for uncommon error "
                       "conditions that the old interface represented ambiguously."),
        VerificationStatus::AcceptedRisk,
        QStringLiteral("Mapping covers all PFT_* values; unmapped cases are "
                       "inherently ambiguous in the old interface. Future "
                       "self-developed backends will emit precise error codes.")
    },
    {
        QStringLiteral("KD-008"),
        DifferenceCategory::BehaviourChange,
        QStringLiteral("Feature flag controls registry participation"),
        QStringLiteral("Old code always used PluginManager to load backends; "
                       "there was no opt-in/opt-out mechanism."),
        QStringLiteral("DEEPIN_COMPRESSOR_USE_REGISTRY environment variable "
                       "(\"1\"/\"true\") enables the registry path; default is off. "
                       "tryRegistryBackend() returns nullptr when disabled, "
                       "falling back to the legacy PluginManager path."),
        QStringLiteral("Rollout is gradual and reversible; disabling the flag "
                       "restores the original code path immediately."),
        VerificationStatus::Verified,
        QStringLiteral("registry_integration_test.cpp tests both enabled and "
                       "disabled states, including the re-entrancy guard.")
    },
};

static const int s_count = sizeof(s_differences) / sizeof(s_differences[0]);

QList<KnownDifference> knownDifferences()
{
    QList<KnownDifference> result;
    result.reserve(s_count);
    for (int i = 0; i < s_count; ++i) {
        result.append(s_differences[i]);
    }
    return result;
}

const KnownDifference *findDifference(const QString &id)
{
    for (int i = 0; i < s_count; ++i) {
        if (s_differences[i].id == id) {
            return &s_differences[i];
        }
    }
    return nullptr;
}

} // namespace archive
