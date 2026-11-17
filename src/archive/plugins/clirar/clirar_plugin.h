// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_CLIRAR_PLUGIN_H
#define OWN_CLIRAR_PLUGIN_H

#include "clirar_backend.h"
#include "kpluginfactory.h"

class CliRarFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_clirar.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit CliRarFactory();
    ~CliRarFactory() override;
};

#endif // OWN_CLIRAR_PLUGIN_H
