// Copyright (c) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
// Copyright (c) 2008-2009 Harald Hvaal <haraldhv@stud.ntnu.no>
// Copyright (c) 2010 Raphael Kubo da Costa <rakuco@FreeBSD.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "readonlylibarchiveplugin.h"

ReadOnlyLibarchivePluginFactory::ReadOnlyLibarchivePluginFactory()
{
    registerPlugin<ReadOnlyLibarchivePlugin>();
}
ReadOnlyLibarchivePluginFactory::~ReadOnlyLibarchivePluginFactory()
{

}

ReadOnlyLibarchivePlugin::ReadOnlyLibarchivePlugin(QObject *parent, const QVariantList &args)
    : LibarchivePlugin(parent, args)
{
}

ReadOnlyLibarchivePlugin::~ReadOnlyLibarchivePlugin()
{
}
