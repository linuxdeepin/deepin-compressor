// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_PIGZ_PLUGIN_H
#define OWN_PIGZ_PLUGIN_H

#include "pigz_backend.h"
#include "kpluginfactory.h"

class PigzFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libpigz.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit PigzFactory();
    ~PigzFactory() override;
};

#endif // OWN_PIGZ_PLUGIN_H
