// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_LIBARCHIVE_PLUGIN_H
#define OWN_LIBARCHIVE_PLUGIN_H

#include "archive_backend.h"
#include "kpluginfactory.h"

class LibarchiveWriteFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libarchive.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit LibarchiveWriteFactory();
    ~LibarchiveWriteFactory() override;
};

#endif // OWN_LIBARCHIVE_PLUGIN_H
