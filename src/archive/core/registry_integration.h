// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REGISTRY_INTEGRATION_H
#define REGISTRY_INTEGRATION_H

#include <QString>

class ReadOnlyArchiveInterface;

namespace archive {

// ---------------------------------------------------------------------------
// Staged migration — compatibility bridge (V-8285 phase 2)
//
// The current phase provides a COMPATIBILITY BRIDGE, not a full business-layer
// switch.  tryRegistryBackend() routes backend selection through ArchiveRegistry
// and wraps the result in a LegacyBackendAdapter, but then releases the old
// ReadOnlyArchiveInterface back to the caller (UiTools::createInterface).
//
// This means:
//   - ArchiveRegistry, ArchiveBackend, BackendObserver, and LegacyBackendAdapter
//     are fully implemented and tested (including e2e tests proving the
//     ArchiveBackend operation path works with observer event sequences).
//   - The production business layer still operates on ReadOnlyArchiveInterface;
//     it does NOT yet call ArchiveBackend::open/list/extractFiles/... directly.
//   - Full business-layer migration to ArchiveBackend is a LATER phase
//     (after V-8286/V-8287 CLI backend migration provides self-developed
//     backends that are not KDE-plugin-based).
//
// The feature flag (DEEPIN_COMPRESSOR_USE_REGISTRY) controls whether the
// registry bridge path is active.  When disabled, the original PluginManager
// path is used unchanged.
//
// See KnownDifference KD-003 for the formal documentation of this staged
// approach.
// ---------------------------------------------------------------------------

// Returns true when the DEEPIN_COMPRESSOR_USE_REGISTRY environment variable
// is set to "1" or "true" (case-insensitive).  Any other value — including
// "0", "false", an empty string, or the variable being unset — returns
// false.
//
// This replaces the earlier qEnvironmentVariableIsSet() check, which treated
// any presence of the variable (even "0" or "false") as enabled.
bool isRegistryFeatureEnabled();

// Try to obtain a ReadOnlyArchiveInterface through the ArchiveRegistry
// selection path.
//
// This is the compatibility bridge: the registry selects a backend and wraps
// it in a LegacyBackendAdapter, but the adapter's wrapped interface is then
// released back to the caller so the existing business layer can operate on
// it unchanged.  The ArchiveBackend path (open/list/extract through
// ArchiveBackend + BackendObserver) is proven by e2e tests but not yet used
// in production code — that switch is a later phase.
//
// Returns nullptr when:
//   - the feature flag is disabled (see isRegistryFeatureEnabled)
//   - a re-entrant call is detected (thread-local recursion guard)
//   - no matching backend is found or the factory returns nullptr
//
// On success the caller takes ownership of the returned pointer.
//
// bWrite is forwarded to ArchiveRegistry::createBackendByExtension so that
// write-capable backends are selected for write requests.
ReadOnlyArchiveInterface *tryRegistryBackend(const QString &fileName, bool bWrite);

} // namespace archive

#endif // REGISTRY_INTEGRATION_H
