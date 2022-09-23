// Copyright (c) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
// Copyright (c) 2008-2009 Harald Hvaal <haraldhv@stud.ntnu.no>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef READONLYLIBARCHIVEPLUGIN_H
#define READONLYLIBARCHIVEPLUGIN_H

#include "libarchiveplugin.h"
#include "kpluginfactory.h"

class ReadOnlyLibarchivePluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libarchive_readonly.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit ReadOnlyLibarchivePluginFactory();
    ~ReadOnlyLibarchivePluginFactory();
};

class ReadOnlyLibarchivePlugin : public LibarchivePlugin
{
    Q_OBJECT

public:
    explicit ReadOnlyLibarchivePlugin(QObject *parent, const QVariantList &args);
    ~ReadOnlyLibarchivePlugin() override;
};

#endif // READONLYLIBARCHIVEPLUGIN_H
