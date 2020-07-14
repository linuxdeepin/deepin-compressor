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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "plugin.h"

#include <QMimeType>
#include <QVector>

class PluginManager : public QObject
{
    Q_OBJECT

public:
    enum MimeSortingMode {
        Unsorted,
        SortByComment
    };

    explicit PluginManager(QObject *parent = nullptr);

    /**
    * @brief installedPlugins 获取已安装插件
    */
    QVector<Plugin *> installedPlugins() const;

    /**
    * @brief availablePlugins 获取有效插件
    */
    QVector<Plugin *> availablePlugins() const;

    /**
    * @brief availableWritePlugins 获取可写插件
    */
    QVector<Plugin *> availableWritePlugins() const;
    QVector<Plugin *> enabledPlugins() const;

    /**
    * @brief preferredPluginsFor 根据类型获取使用的插件
    */
    QVector<Plugin *> preferredPluginsFor(const QMimeType &mimeType);

    /**
    * @brief preferredWritePluginsFor 根据压缩类型获取可写插件
    */
    QVector<Plugin *> preferredWritePluginsFor(const QMimeType &mimeType) const;
    Plugin *preferredPluginFor(const QMimeType &mimeType);
    Plugin *preferredWritePluginFor(const QMimeType &mimeType) const;
    QStringList supportedMimeTypes(MimeSortingMode mode = Unsorted) const;
    QStringList supportedWriteMimeTypes(MimeSortingMode mode = Unsorted) const;
    QVector<Plugin *> filterBy(const QVector<Plugin *> &plugins, const QMimeType &mimeType) const;
    void setFileSize(qint64 size);

private:

    void loadPlugins();
    QVector<Plugin *> preferredPluginsFor(const QMimeType &mimeType, bool readWrite) const;
    static QStringList sortByComment(const QSet<QString> &mimeTypes);
    static bool libarchiveHasLzo();

    QVector<Plugin *> m_plugins;
    QHash<QString, QVector<Plugin *>> m_preferredPluginsCache;
    qint64  m_filesize = 0;
};


#endif
