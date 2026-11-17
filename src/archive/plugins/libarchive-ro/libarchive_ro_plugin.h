// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_LIBARCHIVE_RO_PLUGIN_H
#define OWN_LIBARCHIVE_RO_PLUGIN_H

#include "archive_backend.h"
#include "kpluginfactory.h"

class LibarchiveReadOnlyFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libarchive_readonly.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit LibarchiveReadOnlyFactory();
    ~LibarchiveReadOnlyFactory() override;
};

#endif // OWN_LIBARCHIVE_RO_PLUGIN_H
