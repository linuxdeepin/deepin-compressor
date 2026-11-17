// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_CLI7Z_PLUGIN_H
#define OWN_CLI7Z_PLUGIN_H

#include "cli7z_backend.h"
#include "kpluginfactory.h"

class Cli7zFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_cli7z.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit Cli7zFactory();
    ~Cli7zFactory() override;
};

#endif // OWN_CLI7Z_PLUGIN_H
