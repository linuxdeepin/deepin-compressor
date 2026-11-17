// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "registry_integration.h"
#include "archive_registry.h"
#include "legacy_backend_adapter.h"

#include <QByteArray>
#include <QScopeGuard>

namespace archive {

bool isRegistryFeatureEnabled()
{
    const QByteArray val = qgetenv("DEEPIN_COMPRESSOR_USE_REGISTRY");
    if (val.isEmpty())
        return false;
    // "1" or "true" (case-insensitive) enables; everything else disables.
    return val == "1" || val.compare("true", Qt::CaseInsensitive) == 0;
}

ReadOnlyArchiveInterface *tryRegistryBackend(const QString &fileName, bool bWrite)
{
    if (!isRegistryFeatureEnabled())
        return nullptr;

    // Thread-local recursion guard.
    //
    // In production, defaultLegacyInterfaceFactory() delegates to
    // UiTools::createInterface(), which in turn calls this function.
    // The re-entrant call must skip the Registry path and fall through
    // to the legacy PluginManager path to avoid infinite recursion.
    static thread_local bool inRegistryPath = false;
    if (inRegistryPath)
        return nullptr;

    inRegistryPath = true;
    auto guard = qScopeGuard([] { inRegistryPath = false; });

    auto backend = ArchiveRegistry::instance()
                       .createBackendByExtension(fileName, bWrite);
    if (backend) {
        auto *adapter = dynamic_cast<LegacyBackendAdapter *>(backend.get());
        if (adapter) {
            ReadOnlyArchiveInterface *iface = adapter->releaseInterface();
            if (iface)
                return iface;
        }
    }

    return nullptr;
}

} // namespace archive
