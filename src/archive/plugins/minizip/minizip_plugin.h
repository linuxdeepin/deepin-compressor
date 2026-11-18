// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_MINIZIP_PLUGIN_H
#define OWN_MINIZIP_PLUGIN_H

#include "minizip_backend.h"
#include "kpluginfactory.h"

class MinizipFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libminizip.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit MinizipFactory();
    ~MinizipFactory() override;
};

#endif // OWN_MINIZIP_PLUGIN_H
