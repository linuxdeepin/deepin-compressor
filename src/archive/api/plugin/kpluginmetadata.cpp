// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kpluginmetadata.h"

#include <QFile>
#include <QLocale>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPluginLoader>

KPluginMetaData::KPluginMetaData() = default;

KPluginMetaData::KPluginMetaData(const QString &file)
{
    loadFromFile(file);
}

KPluginMetaData::KPluginMetaData(const QJsonObject &metaData, const QString &file)
    : m_root(metaData)
    , m_file(file)
{
    // 两种形态都要接受：{ "KPlugin": {...} } 和直接 {...}
    m_plugin = m_root.value(QStringLiteral("KPlugin")).toObject();
    if (m_plugin.isEmpty())
        m_plugin = m_root;
    m_valid = !m_plugin.value(QStringLiteral("Id")).toString().isEmpty();
}

KPluginMetaData::KPluginMetaData(const KPluginMetaData &other) = default;
KPluginMetaData &KPluginMetaData::operator=(const KPluginMetaData &other) = default;
KPluginMetaData::~KPluginMetaData() = default;

void KPluginMetaData::loadFromFile(const QString &file)
{
    m_file = file;
    m_root = QJsonObject();
    m_plugin = QJsonObject();
    m_valid = false;

    if (file.isEmpty())
        return;

    if (file.endsWith(QLatin1String(".json"))) {
        QFile json(file);
        if (!json.open(QIODevice::ReadOnly))
            return;
        const QJsonDocument doc = QJsonDocument::fromJson(json.readAll());
        m_root = doc.object();
    } else {
        // 插件二进制：Qt 把 Q_PLUGIN_METADATA 的 json 放在 "MetaData" 键下
        QPluginLoader loader(file);
        m_root = loader.metaData();
        const QJsonObject embedded = m_root.value(QStringLiteral("MetaData")).toObject();
        if (!embedded.isEmpty())
            m_root = embedded;
    }

    m_plugin = m_root.value(QStringLiteral("KPlugin")).toObject();
    if (m_plugin.isEmpty())
        m_plugin = m_root;
    m_valid = !m_plugin.value(QStringLiteral("Id")).toString().isEmpty();
}

bool KPluginMetaData::isValid() const
{
    return m_valid;
}

QString KPluginMetaData::fileName() const
{
    return m_file;
}

QString KPluginMetaData::name() const
{
    // 旧实现取本地化名字：json 里是 "Name[zh_CN]" 这种带下划线的键，
    // 而 QLocale::uiLanguages() 给的是 "zh-Hans-CN"，两种写法都要试。
    QStringList candidates;
    const QLocale system = QLocale::system();
    candidates << system.name();
    candidates << system.uiLanguages();
    for (const QString &uiLanguage : candidates) {
        QStringList keys;
        keys << uiLanguage << QString(uiLanguage).replace(QLatin1Char('-'), QLatin1Char('_'))
             << uiLanguage.section(QLatin1Char('-'), 0, 0)
             << QString(uiLanguage).replace(QLatin1Char('-'), QLatin1Char('_')).section(QLatin1Char('_'), 0, 1);

        for (const QString &key : keys) {
            if (key.isEmpty())
                continue;
            const QString localized = m_plugin.value(QStringLiteral("Name[%1]").arg(key)).toString();
            if (!localized.isEmpty())
                return localized;
        }
    }
    return m_plugin.value(QStringLiteral("Name")).toString();
}

QString KPluginMetaData::pluginId() const
{
    return m_plugin.value(QStringLiteral("Id")).toString();
}

QString KPluginMetaData::version() const
{
    return m_plugin.value(QStringLiteral("Version")).toString();
}

QStringList KPluginMetaData::mimeTypes() const
{
    QStringList types;
    const QJsonArray array = m_plugin.value(QStringLiteral("MimeTypes")).toArray();
    types.reserve(array.size());
    for (const QJsonValue &value : array)
        types.append(value.toString());
    return types;
}

QJsonObject KPluginMetaData::rawData() const
{
    return m_root;
}

QVariantMap KPluginMetaData::toVariantMap() const
{
    return m_root.toVariantMap();
}
