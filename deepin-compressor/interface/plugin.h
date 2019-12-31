/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>

#include "kpluginmetadata.h"
#include <QJsonObject>


class Plugin : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int priority READ priority CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged MEMBER m_enabled)
    Q_PROPERTY(bool readWrite READ isReadWrite CONSTANT)
    Q_PROPERTY(QStringList readOnlyExecutables READ readOnlyExecutables CONSTANT)
    Q_PROPERTY(QStringList readWriteExecutables READ readWriteExecutables CONSTANT)
    Q_PROPERTY(KPluginMetaData metaData READ metaData MEMBER m_metaData CONSTANT)

public:
    explicit Plugin(QObject *parent = nullptr, const KPluginMetaData &metaData = KPluginMetaData());


    int priority() const;
    bool isEnabled() const;
    void setEnabled(bool enabled);
    bool isReadWrite() const;
    QStringList readOnlyExecutables() const;
    QStringList readWriteExecutables() const;
    KPluginMetaData metaData() const;
    bool hasRequiredExecutables() const;
    bool isValid() const;

Q_SIGNALS:
    void enabledChanged();

private:

    static bool findExecutables(const QStringList &executables);

    bool m_enabled;
    KPluginMetaData m_metaData;
};



#endif
