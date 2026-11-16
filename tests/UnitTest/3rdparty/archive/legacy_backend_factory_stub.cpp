// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_backend_registry.h"
#include "archiveinterface.h"
#include "libzipplugin.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QFileInfo>
#include <QVariant>

Q_DECLARE_METATYPE(KPluginMetaData)

namespace archive {

// Test stub for defaultLegacyInterfaceFactory().
// Returns a factory that creates a real LibzipPlugin for .zip files and
// nullptr for other formats.  This lets E2E tests exercise the production
// path: registerLegacyBackends() (no-arg) -> defaultLegacyInterfaceFactory()
// -> real plugin creation -> tryRegistryBackend() -> real interface.
//
// Only LibzipPlugin is available in the test build; other format plugins
// (Cli7zPlugin, CliRarPlugin, etc.) are not linked, so non-zip paths
// return nullptr and the registry falls through.
LegacyInterfaceFactory defaultLegacyInterfaceFactory()
{
    return [](const QString &path, bool /*bWrite*/) -> ReadOnlyArchiveInterface * {
        if (!path.endsWith(QStringLiteral(".zip"), Qt::CaseInsensitive)) {
            return nullptr;
        }
        KPluginMetaData meta;
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(path, QMimeDatabase::MatchContent);
        const QVariantList args = {
            QVariant(path),
            QVariant::fromValue(meta),
            QVariant::fromValue(mime)
        };
        return new LibzipPlugin(nullptr, args);
    };
}

} // namespace archive
