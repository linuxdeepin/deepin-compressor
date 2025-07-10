// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include <QDebug>

#include <QJsonArray>
#include <QStandardPaths>

Plugin::Plugin(QObject *parent, const KPluginMetaData &metaData)
    : QObject(parent)
    , m_enabled(true)
    , m_metaData(metaData)
{
    qDebug() << "Initializing plugin:" << metaData.pluginId();
}

int Plugin::priority() const
{
    qDebug() << "Getting priority for plugin:" << m_metaData.pluginId();
    const int priority = m_metaData.rawData()[QStringLiteral("X-KDE-Priority")].toInt();
    qDebug() << "Plugin priority:" << (priority > 0 ? priority : 0);
    return (priority > 0 ? priority : 0);
}

bool Plugin::isEnabled() const
{
    qDebug() << "Checking if plugin is enabled:" << m_metaData.pluginId() << "enabled:" << m_enabled;
    return m_enabled;
}

void Plugin::setEnabled(bool enabled)
{
    qDebug() << "Setting plugin enabled state:" << m_metaData.pluginId() << "to" << enabled;
    m_enabled = enabled;
    emit enabledChanged();
}

bool Plugin::isReadWrite() const
{
    qDebug() << "Checking if plugin is read-write:" << m_metaData.pluginId();
    const bool isDeclaredReadWrite = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadWrite")].toBool();
    bool hasExecutables = findExecutables(readWriteExecutables());
    bool result = isDeclaredReadWrite && hasExecutables;
    qDebug() << "Plugin read-write check - declared:" << isDeclaredReadWrite << "has executables:" << hasExecutables << "result:" << result;
    return result;
}

QStringList Plugin::readOnlyExecutables() const
{
    qDebug() << "Getting read-only executables for plugin:" << m_metaData.pluginId();
    QStringList readOnlyExecutables;

    const QJsonArray array = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadOnlyExecutables")].toArray();
    for (const QJsonValue &value : array) {
        readOnlyExecutables << value.toString();
    }

    qDebug() << "Read-only executables:" << readOnlyExecutables;
    return readOnlyExecutables;
}

QStringList Plugin::readWriteExecutables() const
{
    qDebug() << "Getting read-write executables for plugin:" << m_metaData.pluginId();
    QStringList readWriteExecutables;

    const QJsonArray array = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadWriteExecutables")].toArray();
    for (const QJsonValue &value : array) {
        readWriteExecutables << value.toString();
    }

    qDebug() << "Read-write executables:" << readWriteExecutables;
    return readWriteExecutables;
}

KPluginMetaData Plugin::metaData() const
{
    qDebug() << "Getting metadata for plugin:" << m_metaData.pluginId();
    return m_metaData;
}

bool Plugin::hasRequiredExecutables() const
{
    qDebug() << "Checking required executables for plugin:" << m_metaData.pluginId();
    bool result = findExecutables(readOnlyExecutables());
    qDebug() << "Required executables check result:" << result;
    return result;
}

bool Plugin::isValid() const
{
    bool valid = isEnabled() && m_metaData.isValid() && hasRequiredExecutables();
    qDebug() << "Plugin validation:" << m_metaData.pluginId() << "is" << (valid ? "valid" : "invalid");
    return valid;
}

bool Plugin::findExecutables(const QStringList &executables)
{
    qDebug() << "Searching for required executables:" << executables;
    for (const QString &executable : executables) {
        if (executable.isEmpty()) {
            qDebug() << "Skipping empty executable";
            continue;
        }

        const QString path = QStandardPaths::findExecutable(executable);
        if (path.isEmpty()) {
            qWarning() << "Executable not found:" << executable;
            return false;
        }
        qDebug() << "Found executable:" << executable << "at" << path;
    }

    qDebug() << "All required executables found";
    return true;
}
