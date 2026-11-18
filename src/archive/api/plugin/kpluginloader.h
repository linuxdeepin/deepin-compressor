// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OWN_KPLUGINLOADER_H
#define OWN_KPLUGINLOADER_H

#include "kpluginmetadata.h"

#include <QObject>
#include <QString>
#include <QVector>

#include <functional>

class KPluginFactory;
class QPluginLoader;

/**
 * @brief 插件加载器：扫目录拿元数据，按路径载入 .so 取工厂。
 * 与 3rdparty/interface/plugin 的实现等价，底层仍是 QPluginLoader。
 */
class KPluginLoader : public QObject
{
    Q_OBJECT

public:
    explicit KPluginLoader(const QString &plugin, QObject *parent = nullptr);
    explicit KPluginLoader(const KPluginMetaData &metaData, QObject *parent = nullptr);
    ~KPluginLoader() override;

    KPluginFactory *factory();
    QObject *instance();
    KPluginMetaData metaData() const;
    QString fileName() const;
    QString errorString() const;

    /// 在 Qt 库搜索路径下按名字找插件，找不到返回空
    static QString findPlugin(const QString &name);
    /// 扫描 "库搜索路径/<directory>"（directory 也允许是绝对路径）
    static QVector<KPluginMetaData> findPlugins(const QString &directory);
    static QVector<KPluginMetaData> findPlugins(const QString &directory,
                                               const std::function<bool(const KPluginMetaData &)> &filter);
    static QVector<KPluginMetaData> findPluginsById(const QString &directory, const QString &pluginId);

private:
    QString m_name;
    QPluginLoader *m_loader = nullptr;
};

#endif // OWN_KPLUGINLOADER_H
