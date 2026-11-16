// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACY_BACKEND_REGISTRY_H
#define LEGACY_BACKEND_REGISTRY_H

#include <functional>
#include <QString>

class ReadOnlyArchiveInterface;

namespace archive {

class ArchiveBackend;

// Factory that creates a legacy KDE plugin interface for the given archive
// path.  bWrite=true requests a read-write interface; false requests read-only.
using LegacyInterfaceFactory = std::function<ReadOnlyArchiveInterface *(const QString &archivePath, bool bWrite)>;

// Returns the production factory (wraps UiTools::createInterface).
// In test builds a stub is linked instead so tests do not depend on KDE plugins.
LegacyInterfaceFactory defaultLegacyInterfaceFactory();

// Register all legacy KDE-backed archive formats (zip, 7z, tar, gz, bz2,
// xz, rar, iso, ar) into ArchiveRegistry.  Call once at application
// startup to make the registry functional.
//
// If factory is null, uses defaultLegacyInterfaceFactory().
void registerLegacyBackends(LegacyInterfaceFactory factory = nullptr);

} // namespace archive

#endif // LEGACY_BACKEND_REGISTRY_H
