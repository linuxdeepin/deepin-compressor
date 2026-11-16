// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACY_BACKEND_DESCRIPTORS_H
#define LEGACY_BACKEND_DESCRIPTORS_H

#include "archive_registry.h"

#include <QString>
#include <QStringList>

namespace archive {

// Build a read-only BackendDescriptor for formats that cannot be modified
// (RAR, ISO, AR, and single-file compressed formats).
BackendDescriptor makeReadOnlyLegacyDescriptor(const QString &id,
                                                const QStringList &mimeTypes,
                                                const QStringList &extensions,
                                                BackendFactory factory);

// Build a read-write BackendDescriptor for formats that support creation
// and optionally modification / multi-volume.
BackendDescriptor makeReadWriteLegacyDescriptor(const QString &id,
                                                 const QStringList &mimeTypes,
                                                 const QStringList &extensions,
                                                 bool supportsModify,
                                                 bool supportsMultiVolume,
                                                 BackendFactory factory);

} // namespace archive

#endif // LEGACY_BACKEND_DESCRIPTORS_H
