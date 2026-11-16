// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_REGISTRY_H
#define ARCHIVE_REGISTRY_H

#include "archive_backend.h"
#include "archive_types.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>
#include <memory>

namespace archive {

// ---------------------------------------------------------------------------
// Backend factory — produces a fresh ArchiveBackend instance for a given
// archive path.  Each concrete backend (legacy adapter, libarchive, CLI,
// etc.) registers one or more factories keyed by MIME type and/or file
// extension, together with a priority value for deterministic selection.
// ---------------------------------------------------------------------------
using BackendFactory = std::function<std::unique_ptr<ArchiveBackend>(const QString &archivePath, bool bWrite)>;

struct BackendDescriptor {
    QString id;                 // unique backend identifier (e.g. "legacy-7z", "libarchive")
    BackendCapabilities capabilities;
    BackendFactory factory;
    QStringList mimeTypes;      // MIME types this backend handles
    QStringList extensions;     // file extensions (without dot), e.g. "zip", "7z"
    int priority = 0;           // higher = preferred
    bool isLegacy = false;      // true for KDE-plugin-backed adapters
};

// ---------------------------------------------------------------------------
// ArchiveRegistry — replaces KPluginFactory/KPluginLoader for backend
// selection.  Maps MIME types and file extensions to backend factories,
// selects the highest-priority available backend, and falls back to the
// next candidate when a factory fails to load.
//
// A feature flag (useLegacyBackends) controls whether legacy KDE-backed
// adapters participate in selection.  When false, only self-developed
// backends are considered; when true (the default during migration),
// legacy adapters act as the behavior baseline.
// ---------------------------------------------------------------------------
class ArchiveRegistry : public QObject
{
    Q_OBJECT

public:
    static ArchiveRegistry &instance();

    // Register a backend descriptor.  Duplicate IDs replace the prior entry.
    void registerBackend(const BackendDescriptor &descriptor);

    // Remove a backend by ID.
    void unregisterBackend(const QString &id);

    // All registered descriptors (copy, safe to iterate).
    QVector<BackendDescriptor> backends() const;

    // Find backends matching the given MIME type, sorted by descending priority.
    QVector<BackendDescriptor> findBackends(const QString &mimeType) const;

    // Find backends matching the given file extension, sorted by priority.
    QVector<BackendDescriptor> findBackendsByExtension(const QString &suffix) const;

    // Select and instantiate the best backend for mimeType.  Tries each
    // candidate in priority order; on factory failure (returns nullptr)
    // falls back to the next.  Returns nullptr if no backend is available.
    //
    // When bWrite is true, only descriptors with write capability (create
    // or append) are considered, and the factory is asked for a read-write
    // interface.  When false, all descriptors are considered but the
    // factory is asked for read-only — this matches the original
    // createInterface(fileName, bWrite) semantics.
    std::unique_ptr<ArchiveBackend> createBackend(const QString &archivePath,
                                                  const QString &mimeType,
                                                  bool bWrite) const;

    // Select and instantiate the best backend for mimeType that supports
    // the requested ArchiveOperation.  This is the operation-type-aware
    // routing required by V-8285: a backend may match the format but lack
    // the specific operation capability (e.g. a RAR backend supports List
    // and Extract but not Add/Move/Delete).  Only descriptors whose
    // BackendCapabilities::supports(op) returns true are tried.
    //
    // Returns nullptr (and sets *errorCode when non-null) if no backend
    // matches the format, or if all matching backends lack the operation.
    // When a format matches but no backend supports the operation,
    // errorCode is set to ErrorCode::NotSupported so callers can
    // distinguish "unknown format" from "operation not supported".
    std::unique_ptr<ArchiveBackend> createBackend(const QString &archivePath,
                                                  const QString &mimeType,
                                                  ArchiveOperation op,
                                                  ErrorCode *errorCode = nullptr) const;

    // Same as the operation-based createBackend but derives MIME type
    // from the file extension.
    std::unique_ptr<ArchiveBackend> createBackendByExtension(const QString &archivePath,
                                                              ArchiveOperation op,
                                                              ErrorCode *errorCode = nullptr) const;

    // Same as createBackend but derives the MIME type from the file extension.
    std::unique_ptr<ArchiveBackend> createBackendByExtension(const QString &archivePath,
                                                              bool bWrite) const;

    // Feature flag: when true, legacy KDE-backed adapters participate.
    // Default is true (migration period — legacy is the behavior baseline).
    bool useLegacyBackends() const;
    void setUseLegacyBackends(bool enabled);

    // Convenience: map a file suffix (without dot) to a MIME type using
    // the registered descriptors' mimeType/extension pairs.
    QString mimeTypeForExtension(const QString &suffix) const;

private:
    ArchiveRegistry();
    ~ArchiveRegistry() override;
    ArchiveRegistry(const ArchiveRegistry &) = delete;
    ArchiveRegistry &operator=(const ArchiveRegistry &) = delete;

    QVector<BackendDescriptor> filterAndSort(const QVector<BackendDescriptor> &candidates) const;

    QHash<QString, QVector<BackendDescriptor>> m_byMime;
    QHash<QString, QVector<BackendDescriptor>> m_byExt;
    QVector<BackendDescriptor> m_all;
    bool m_useLegacy = true;
};

} // namespace archive

#endif // ARCHIVE_REGISTRY_H
