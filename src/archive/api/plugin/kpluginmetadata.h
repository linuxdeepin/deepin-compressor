// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_KPLUGINMETADATA_H
#define OWN_KPLUGINMETADATA_H

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

/**
 * @brief 插件元数据，对应 kerfuffle_*.json 的内容。
 *
 * 只提供本工程实际用到的查询：pluginId/name/mimeTypes/fileName 以及 rawData()。
 * rawData() 返回整份 json，Plugin 和 CustomMimeType 从里面读自定义字段
 * （X-KDE-Priority、X-KDE-Kerfuffle-ReadWrite、各 mime 的压缩方法等）。
 */
class KPluginMetaData
{
public:
    KPluginMetaData();
    /// @param file 插件 .so 路径，或已经落盘的 kerfuffle_*.json 路径
    explicit KPluginMetaData(const QString &file);
    KPluginMetaData(const QJsonObject &metaData, const QString &file);
    KPluginMetaData(const KPluginMetaData &other);
    KPluginMetaData &operator=(const KPluginMetaData &other);
    ~KPluginMetaData();

    bool isValid() const;
    QString fileName() const;
    QString name() const;
    QString pluginId() const;
    QString version() const;
    QStringList mimeTypes() const;
    QJsonObject rawData() const;
    QVariantMap toVariantMap() const;

private:
    void loadFromFile(const QString &file);

    QJsonObject m_root;     // 整份 json
    QJsonObject m_plugin;   // KPlugin 段
    QString m_file;
    bool m_valid = false;
};

// 与旧实现一致：Q_DECLARE_METATYPE 放在使用处的 .cpp 里，避免重复定义
#endif // OWN_KPLUGINMETADATA_H
