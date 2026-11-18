// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_LIBZIP_PLUGIN_H
#define OWN_LIBZIP_PLUGIN_H

#include "libzip_backend.h"
#include "kpluginfactory.h"

class LibzipFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libzip.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit LibzipFactory();
    ~LibzipFactory() override;
};

#endif // OWN_LIBZIP_PLUGIN_H
