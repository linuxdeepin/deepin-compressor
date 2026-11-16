// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_backend_registry.h"
#include "archive_registry.h"
#include "backend_capabilities.h"
#include "legacy_backend_adapter.h"
#include "legacy_backend_descriptors.h"

#include "archiveinterface.h"

#include <QFileInfo>
#include <QDebug>

namespace archive {

// ---------------------------------------------------------------------------
// registerLegacyBackends
//
// Registers BackendDescriptor entries for common archive formats, each
// backed by a LegacyBackendAdapter wrapping the existing KDE plugin loaded
// via the provided (or default) LegacyInterfaceFactory.  This is the
// production wiring that makes ArchiveRegistry usable — without this call
// the registry is isolated code.
//
// Priority is deliberately low (0) so that self-developed backends (when
// they arrive in later phases) take precedence.  isLegacy = true so the
// feature flag (useLegacyBackends) can exclude them when ready.
// ---------------------------------------------------------------------------

// Factory helper: creates a LegacyBackendAdapter for the given archive path
// using the provided interface factory.  bWrite controls whether a
// read-write or read-only interface is requested — matching the original
// createInterface(fileName, bWrite) semantics.  No write-to-read fallback:
// a write request that fails returns nullptr so the caller can try the
// next descriptor or fall back to the legacy PluginManager path.
static std::unique_ptr<ArchiveBackend> createLegacyBackend(const QString &archivePath,
                                                            bool bWrite,
                                                            const LegacyInterfaceFactory &factory,
                                                            const BackendCapabilities &caps)
{
    ReadOnlyArchiveInterface *iface = factory(archivePath, bWrite);
    if (!iface) {
        qWarning() << "Legacy backend: no plugin found for" << archivePath
                   << "(bWrite =" << bWrite << ")";
        return nullptr;
    }

    ReadWriteArchiveInterface *rw = qobject_cast<ReadWriteArchiveInterface *>(iface);
    if (rw) {
        return std::make_unique<LegacyBackendAdapter>(rw, caps);
    }
    return std::make_unique<LegacyBackendAdapter>(iface, caps);
}

void registerLegacyBackends(LegacyInterfaceFactory factory)
{
    if (!factory) {
        factory = defaultLegacyInterfaceFactory();
    }

    ArchiveRegistry &reg = ArchiveRegistry::instance();

    // Helper: build a BackendFactory that creates a LegacyBackendAdapter
    // with the same BackendCapabilities the descriptor declares, so the
    // adapter's runtime capabilities() always match the descriptor used
    // for operation routing.
    auto makeFactory = [&factory](const BackendCapabilities &caps) -> BackendFactory {
        return [factory, caps](const QString &path, bool bWrite) -> std::unique_ptr<ArchiveBackend> {
            return createLegacyBackend(path, bWrite, factory, caps);
        };
    };

    // ZIP: read + write (create, append, delete, rename), no multi-volume.
    {
        BackendCapabilities caps;
        caps.list = true; caps.test = true; caps.extract = true;
        caps.create = true; caps.append = true; caps.move = true; caps.copy = true;
        caps.remove = true; caps.rename = true; caps.comment = true; caps.update = true;
        caps.password = true; caps.headerPassword = true;
        caps.progress = true; caps.cancel = true; caps.pause = true;
        reg.registerBackend(makeReadWriteLegacyDescriptor(
            QStringLiteral("legacy-zip"),
            {QStringLiteral("application/zip")},
            {QStringLiteral("zip")},
            true,   // supportsModify
            false,  // supportsMultiVolume
            makeFactory(caps)));
    }

    // 7z: read + write + multi-volume.
    {
        BackendCapabilities caps;
        caps.list = true; caps.test = true; caps.extract = true;
        caps.create = true; caps.append = true; caps.move = true; caps.copy = true;
        caps.remove = true; caps.rename = true; caps.comment = true; caps.update = true;
        caps.password = true; caps.headerPassword = true; caps.multiVolume = true;
        caps.progress = true; caps.cancel = true; caps.pause = true;
        reg.registerBackend(makeReadWriteLegacyDescriptor(
            QStringLiteral("legacy-7z"),
            {QStringLiteral("application/x-7z-compressed")},
            {QStringLiteral("7z")},
            true,   // supportsModify
            true,   // supportsMultiVolume
            makeFactory(caps)));
    }

    // TAR and compressed variants: create only, no in-place modification.
    {
        BackendCapabilities caps;
        caps.list = true; caps.test = true; caps.extract = true;
        caps.create = true;
        caps.password = true;
        caps.progress = true; caps.cancel = true; caps.pause = true;
        reg.registerBackend(makeReadWriteLegacyDescriptor(
            QStringLiteral("legacy-tar"),
            {QStringLiteral("application/x-tar"),
             QStringLiteral("application/x-gzip"),
             QStringLiteral("application/x-bzip2"),
             QStringLiteral("application/x-xz")},
            {QStringLiteral("tar"),
             QStringLiteral("gz"),
             QStringLiteral("tgz"),
             QStringLiteral("bz2"),
             QStringLiteral("tbz2"),
             QStringLiteral("xz"),
             QStringLiteral("txz")},
            false,  // supportsModify
            false,  // supportsMultiVolume
            makeFactory(caps)));
    }

    // RAR: read-only (no creation or modification via legacy plugins).
    {
        BackendCapabilities caps;
        caps.list = true; caps.test = true; caps.extract = true;
        caps.password = true;
        caps.progress = true; caps.cancel = true; caps.pause = true;
        reg.registerBackend(makeReadOnlyLegacyDescriptor(
            QStringLiteral("legacy-rar"),
            {QStringLiteral("application/x-rar"),
             QStringLiteral("application/vnd.rar"),
             QStringLiteral("application/x-rar-compressed")},
            {QStringLiteral("rar")},
            makeFactory(caps)));
    }

    // ISO: read-only.
    {
        BackendCapabilities caps;
        caps.list = true; caps.test = true; caps.extract = true;
        caps.password = true;
        caps.progress = true; caps.cancel = true; caps.pause = true;
        reg.registerBackend(makeReadOnlyLegacyDescriptor(
            QStringLiteral("legacy-iso"),
            {QStringLiteral("application/x-iso9660-image")},
            {QStringLiteral("iso")},
            makeFactory(caps)));
    }

    // AR / DEB: read-only.
    {
        BackendCapabilities caps;
        caps.list = true; caps.test = true; caps.extract = true;
        caps.password = true;
        caps.progress = true; caps.cancel = true; caps.pause = true;
        reg.registerBackend(makeReadOnlyLegacyDescriptor(
            QStringLiteral("legacy-ar"),
            {QStringLiteral("application/x-archive")},
            {QStringLiteral("ar"),
             QStringLiteral("deb")},
            makeFactory(caps)));
    }

    qDebug() << "Legacy archive backends registered";
}

} // namespace archive
