// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kpluginloader.h"
#include "kpluginfactory.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QPluginLoader>

namespace {

// 逐个候选路径交给回调：相对目录按 Qt 库搜索路径展开，绝对路径直接用
void forEachPluginPath(const QString &directory, const std::function<void(const QString &)> &callback)
{
    QStringList dirs;
    if (QDir::isAbsolutePath(directory)) {
        dirs << directory;
    } else {
        const QStringList libraryPaths = QCoreApplication::libraryPaths();
        for (const QString &libraryPath : libraryPaths)
            dirs << libraryPath + QLatin1Char('/') + directory;
    }

    for (const QString &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;

        const QStringList entries = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            const QString file = dir.absoluteFilePath(entry);
            if (!QLibrary::isLibrary(file))
                continue;
            callback(file);
        }
    }
}

} // namespace

KPluginLoader::KPluginLoader(const QString &plugin, QObject *parent)
    : QObject(parent)
    , m_name(plugin)
    , m_loader(new QPluginLoader(plugin, this))
{
}

KPluginLoader::KPluginLoader(const KPluginMetaData &metaData, QObject *parent)
    : KPluginLoader(metaData.fileName(), parent)
{
}

KPluginLoader::~KPluginLoader() = default;

QObject *KPluginLoader::instance()
{
    if (!m_loader)
        return nullptr;
    return m_loader->instance();
}

KPluginFactory *KPluginLoader::factory()
{
    QObject *object = instance();
    if (!object)
        return nullptr;

    KPluginFactory *pluginFactory = qobject_cast<KPluginFactory *>(object);
    if (!pluginFactory)
        qWarning() << "The library" << m_name << "does not offer a KPluginFactory.";
    return pluginFactory;
}

KPluginMetaData KPluginLoader::metaData() const
{
    return KPluginMetaData(m_name);
}

QString KPluginLoader::fileName() const
{
    return m_loader ? m_loader->fileName() : QString();
}

QString KPluginLoader::errorString() const
{
    return m_loader ? m_loader->errorString() : QString();
}

QString KPluginLoader::findPlugin(const QString &name)
{
    QPluginLoader loader(name);
    return loader.fileName();
}

QVector<KPluginMetaData> KPluginLoader::findPlugins(const QString &directory)
{
    return findPlugins(directory, std::function<bool(const KPluginMetaData &)>());
}

QVector<KPluginMetaData> KPluginLoader::findPlugins(const QString &directory,
                                                    const std::function<bool(const KPluginMetaData &)> &filter)
{
    QVector<KPluginMetaData> result;

    forEachPluginPath(directory, [&](const QString &pluginPath) {
        const KPluginMetaData metadata(pluginPath);
        if (!metadata.isValid())
            return;
        if (filter && !filter(metadata))
            return;
        result.append(metadata);
    });

    return result;
}

QVector<KPluginMetaData> KPluginLoader::findPluginsById(const QString &directory, const QString &pluginId)
{
    return findPlugins(directory, [&pluginId](const KPluginMetaData &metadata) {
        return metadata.pluginId() == pluginId;
    });
}
