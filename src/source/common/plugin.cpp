// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include <QDebug>
#include "common/ddlog.h"

#include <QJsonArray>
#include <QStandardPaths>

Plugin::Plugin(QObject *parent, const KPluginMetaData &metaData)
    : QObject(parent)
    , m_enabled(true)
    , m_metaData(metaData)
{
    qCDebug(appLog) << "Initializing plugin:" << metaData.pluginId();
}

int Plugin::priority() const
{
    qCDebug(appLog) << "Getting priority for plugin:" << m_metaData.pluginId();
    const int priority = m_metaData.rawData()[QStringLiteral("X-KDE-Priority")].toInt();
    qCDebug(appLog) << "Plugin priority:" << (priority > 0 ? priority : 0);
    return (priority > 0 ? priority : 0);
}

bool Plugin::isEnabled() const
{
    qCDebug(appLog) << "Checking if plugin is enabled:" << m_metaData.pluginId() << "enabled:" << m_enabled;
    return m_enabled;
}

void Plugin::setEnabled(bool enabled)
{
    qCDebug(appLog) << "Setting plugin enabled state:" << m_metaData.pluginId() << "to" << enabled;
    m_enabled = enabled;
    emit enabledChanged();
}

bool Plugin::isReadWrite() const
{
    qCDebug(appLog) << "Checking if plugin is read-write:" << m_metaData.pluginId();
    const bool isDeclaredReadWrite = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadWrite")].toBool();
    bool hasExecutables = findExecutables(readWriteExecutables());
    bool result = isDeclaredReadWrite && hasExecutables;
    qCDebug(appLog) << "Plugin read-write check - declared:" << isDeclaredReadWrite << "has executables:" << hasExecutables << "result:" << result;
    return result;
}

QStringList Plugin::readOnlyExecutables() const
{
    qCDebug(appLog) << "Getting read-only executables for plugin:" << m_metaData.pluginId();
    QStringList readOnlyExecutables;

    const QJsonArray array = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadOnlyExecutables")].toArray();
    for (const QJsonValue &value : array) {
        readOnlyExecutables << value.toString();
    }

    qCDebug(appLog) << "Read-only executables:" << readOnlyExecutables;
    return readOnlyExecutables;
}

QStringList Plugin::readWriteExecutables() const
{
    qCDebug(appLog) << "Getting read-write executables for plugin:" << m_metaData.pluginId();
    QStringList readWriteExecutables;

    const QJsonArray array = m_metaData.rawData()[QStringLiteral("X-KDE-Kerfuffle-ReadWriteExecutables")].toArray();
    for (const QJsonValue &value : array) {
        readWriteExecutables << value.toString();
    }

    qCDebug(appLog) << "Read-write executables:" << readWriteExecutables;
    return readWriteExecutables;
}

KPluginMetaData Plugin::metaData() const
{
    qCDebug(appLog) << "Getting metadata for plugin:" << m_metaData.pluginId();
    return m_metaData;
}

bool Plugin::hasRequiredExecutables() const
{
    qCDebug(appLog) << "Checking required executables for plugin:" << m_metaData.pluginId();
    bool result = findExecutables(readOnlyExecutables());
    qCDebug(appLog) << "Required executables check result:" << result;
    return result;
}

bool Plugin::isValid() const
{
    bool valid = isEnabled() && m_metaData.isValid() && hasRequiredExecutables();
    qCDebug(appLog) << "Plugin validation:" << m_metaData.pluginId() << "is" << (valid ? "valid" : "invalid");
    return valid;
}

bool Plugin::findExecutables(const QStringList &executables)
{
    qCDebug(appLog) << "Searching for required executables:" << executables;
    for (const QString &executable : executables) {
        if (executable.isEmpty()) {
            qCDebug(appLog) << "Skipping empty executable";
            continue;
        }

        const QString path = QStandardPaths::findExecutable(executable);
        if (path.isEmpty()) {
            qCWarning(appLog) << "Executable not found:" << executable;
            return false;
        }
        qCDebug(appLog) << "Found executable:" << executable << "at" << path;
    }

    qCDebug(appLog) << "All required executables found";
    return true;
}
