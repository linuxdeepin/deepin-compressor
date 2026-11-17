// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archive_registry.h"

#include <QDebug>
#include <QFileInfo>
#include <algorithm>

namespace archive {

ArchiveRegistry &ArchiveRegistry::instance()
{
    static ArchiveRegistry registry;
    return registry;
}

ArchiveRegistry::ArchiveRegistry()
    : QObject(nullptr)
{
}

ArchiveRegistry::~ArchiveRegistry()
{
}

void ArchiveRegistry::registerBackend(const BackendDescriptor &descriptor)
{
    // Remove any prior entry with the same id.
    unregisterBackend(descriptor.id);

    m_all.append(descriptor);

    for (const QString &mime : descriptor.mimeTypes) {
        m_byMime[mime].append(descriptor);
    }
    for (const QString &ext : descriptor.extensions) {
        m_byExt[ext.toLower()].append(descriptor);
    }
}

void ArchiveRegistry::unregisterBackend(const QString &id)
{
    QMutableVectorIterator<BackendDescriptor> it(m_all);
    while (it.hasNext()) {
        if (it.next().id == id) {
            it.remove();
        }
    }
    // Clean index maps.
    QMutableHashIterator<QString, QVector<BackendDescriptor>> mit(m_byMime);
    while (mit.hasNext()) {
        mit.next();
        QMutableVectorIterator<BackendDescriptor> vit(mit.value());
        while (vit.hasNext()) {
            if (vit.next().id == id) {
                vit.remove();
            }
        }
        if (mit.value().isEmpty()) {
            mit.remove();
        }
    }
    QMutableHashIterator<QString, QVector<BackendDescriptor>> eit(m_byExt);
    while (eit.hasNext()) {
        eit.next();
        QMutableVectorIterator<BackendDescriptor> vit(eit.value());
        while (vit.hasNext()) {
            if (vit.next().id == id) {
                vit.remove();
            }
        }
        if (eit.value().isEmpty()) {
            eit.remove();
        }
    }
}

QVector<BackendDescriptor> ArchiveRegistry::backends() const
{
    return m_all;
}

QVector<BackendDescriptor> ArchiveRegistry::findBackends(const QString &mimeType) const
{
    return filterAndSort(m_byMime.value(mimeType));
}

QVector<BackendDescriptor> ArchiveRegistry::findBackendsByExtension(const QString &suffix) const
{
    return filterAndSort(m_byExt.value(suffix.toLower()));
}

std::unique_ptr<ArchiveBackend> ArchiveRegistry::createBackend(const QString &archivePath,
                                                                 const QString &mimeType,
                                                                 bool bWrite) const
{
    QVector<BackendDescriptor> candidates = findBackends(mimeType);
    if (candidates.isEmpty()) {
        // Fall back to extension-based lookup.
        QString ext = QFileInfo(archivePath).suffix();
        candidates = findBackendsByExtension(ext);
    }

    for (const BackendDescriptor &desc : candidates) {
        if (!m_useLegacy && desc.isLegacy) {
            continue;
        }
        // When a write interface is requested, skip descriptors that lack
        // write capability (e.g. RAR/ISO/AR read-only backends).  This
        // matches the original createInterface(fileName, bWrite=true)
        // behaviour of only consulting write-capable plugins.
        if (bWrite && !desc.capabilities.create && !desc.capabilities.append) {
            continue;
        }
        std::unique_ptr<ArchiveBackend> backend = desc.factory(archivePath, bWrite);
        if (backend) {
            return backend;
        }
        qWarning() << "Backend factory returned nullptr, trying next:" << desc.id;
    }

    qWarning() << "No backend available for" << archivePath << "(" << mimeType << ")";
    return nullptr;
}

std::unique_ptr<ArchiveBackend> ArchiveRegistry::createBackendByExtension(const QString &archivePath,
                                                                                   bool bWrite) const
{
    QString ext = QFileInfo(archivePath).suffix();
    QString mime = mimeTypeForExtension(ext);
    return createBackend(archivePath, mime, bWrite);
}

std::unique_ptr<ArchiveBackend> ArchiveRegistry::createBackend(const QString &archivePath,
                                                                const QString &mimeType,
                                                                ArchiveOperation op,
                                                                ErrorCode *errorCode) const
{
    if (errorCode)
        *errorCode = ErrorCode::NoError;

    QVector<BackendDescriptor> candidates = findBackends(mimeType);
    if (candidates.isEmpty()) {
        // Fall back to extension-based lookup.
        QString ext = QFileInfo(archivePath).suffix();
        candidates = findBackendsByExtension(ext);
    }

    bool formatMatched = false;

    for (const BackendDescriptor &desc : candidates) {
        if (!m_useLegacy && desc.isLegacy) {
            continue;
        }
        formatMatched = true;

        // Only consider backends that support the requested operation.
        if (!desc.capabilities.supports(op)) {
            continue;
        }

        // Determine bWrite from the operation: write-class operations
        // need a read-write interface; read-class operations need read-only.
        bool bWrite = (op == ArchiveOperation::Add ||
                       op == ArchiveOperation::Move ||
                       op == ArchiveOperation::Copy ||
                       op == ArchiveOperation::Delete ||
                       op == ArchiveOperation::Rename ||
                       op == ArchiveOperation::Comment ||
                       op == ArchiveOperation::Update);

        std::unique_ptr<ArchiveBackend> backend = desc.factory(archivePath, bWrite);
        if (backend) {
            return backend;
        }
        qWarning() << "Backend factory returned nullptr, trying next:" << desc.id;
    }

    // Format matched but no backend supported the operation.
    if (formatMatched && errorCode) {
        *errorCode = ErrorCode::NotSupported;
    }

    qWarning() << "No backend available for" << archivePath << "(" << mimeType
               << ") operation" << static_cast<int>(op);
    return nullptr;
}

std::unique_ptr<ArchiveBackend> ArchiveRegistry::createBackendByExtension(const QString &archivePath,
                                                                          ArchiveOperation op,
                                                                          ErrorCode *errorCode) const
{
    QString ext = QFileInfo(archivePath).suffix();
    QString mime = mimeTypeForExtension(ext);
    return createBackend(archivePath, mime, op, errorCode);
}

bool ArchiveRegistry::useLegacyBackends() const
{
    return m_useLegacy;
}

void ArchiveRegistry::setUseLegacyBackends(bool enabled)
{
    m_useLegacy = enabled;
}

QString ArchiveRegistry::mimeTypeForExtension(const QString &suffix) const
{
    QString lower = suffix.toLower();
    QVector<BackendDescriptor> candidates = filterAndSort(m_byExt.value(lower));
    if (!candidates.isEmpty()) {
        // Return the first non-empty MIME type from the highest-priority backend.
        for (const BackendDescriptor &desc : candidates) {
            if (!desc.mimeTypes.isEmpty()) {
                return desc.mimeTypes.first();
            }
        }
    }
    return QString();
}

QVector<BackendDescriptor> ArchiveRegistry::filterAndSort(const QVector<BackendDescriptor> &candidates) const
{
    QVector<BackendDescriptor> result;
    for (const BackendDescriptor &desc : candidates) {
        if (!m_useLegacy && desc.isLegacy) {
            continue;
        }
        result.append(desc);
    }
    std::sort(result.begin(), result.end(), [](const BackendDescriptor &a, const BackendDescriptor &b) {
        return a.priority > b.priority;
    });
    return result;
}

} // namespace archive
