// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_BZ2_PLUGIN_H
#define OWN_BZ2_PLUGIN_H

#include "single_file_backend.h"
#include "kpluginfactory.h"

class Bz2PluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libbz2.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit Bz2PluginFactory();
    ~Bz2PluginFactory() override;
};

#endif // OWN_BZ2_PLUGIN_H
