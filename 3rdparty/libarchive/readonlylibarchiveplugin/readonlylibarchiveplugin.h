/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
