// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_GZ_PLUGIN_H
#define OWN_GZ_PLUGIN_H

#include "single_file_backend.h"
#include "kpluginfactory.h"

class GzPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libgz.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit GzPluginFactory();
    ~GzPluginFactory() override;
};

#endif // OWN_GZ_PLUGIN_H
