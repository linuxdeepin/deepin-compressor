// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_backend_descriptors.h"
#include "backend_capabilities.h"

namespace archive {

BackendDescriptor makeReadOnlyLegacyDescriptor(const QString &id,
                                                const QStringList &mimeTypes,
                                                const QStringList &extensions,
                                                BackendFactory factory)
{
    BackendDescriptor desc;
    desc.id = id;
    desc.mimeTypes = mimeTypes;
    desc.extensions = extensions;
    desc.priority = 0;
    desc.isLegacy = true;
    desc.factory = factory;

    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.password = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;
    desc.capabilities = caps;

    return desc;
}

BackendDescriptor makeReadWriteLegacyDescriptor(const QString &id,
                                                 const QStringList &mimeTypes,
                                                 const QStringList &extensions,
                                                 bool supportsModify,
                                                 bool supportsMultiVolume,
                                                 BackendFactory factory)
{
    BackendDescriptor desc;
    desc.id = id;
    desc.mimeTypes = mimeTypes;
    desc.extensions = extensions;
    desc.priority = 0;
    desc.isLegacy = true;
    desc.factory = factory;

    BackendCapabilities caps;
    caps.list = true;
    caps.test = true;
    caps.extract = true;
    caps.create = true;
    caps.password = true;
    caps.progress = true;
    caps.cancel = true;
    caps.pause = true;

    if (supportsModify) {
        caps.append = true;
        caps.move = true;
        caps.copy = true;
        caps.remove = true;
        caps.rename = true;
        caps.comment = true;
        caps.update = true;
        caps.headerPassword = true;
    }

    if (supportsMultiVolume) {
        caps.multiVolume = true;
    }

    desc.capabilities = caps;

    return desc;
}

} // namespace archive
