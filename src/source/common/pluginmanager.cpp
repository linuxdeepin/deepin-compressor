// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanager.h"
#include "kpluginloader.h"
#include "common/ddlog.h"

#include <QPluginLoader>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QDebug>

#include <algorithm>

//静态成员变量初始化。
QMutex PluginManager::m_mutex;//一个线程可以多次锁同一个互斥量
QAtomicPointer<PluginManager> PluginManager::m_instance = nullptr;//原子指针，默认初始化是0

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "Initializing PluginManager";
    loadPlugins();
    qCDebug(appLog) << "PluginManager initialized with" << m_plugins.size() << "plugins";
}

PluginManager &PluginManager::get_instance()
{
    // qCDebug(appLog) << "Getting PluginManager instance";
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<PluginManager>::isTestAndSetNative()) //运行时检测
        qCInfo(appLog) << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (m_instance.testAndSetOrdered(nullptr, nullptr)) { //第一次检测
        qCDebug(appLog) << "Creating new PluginManager instance";
        QMutexLocker locker(&m_mutex);//加互斥锁。

        m_instance.testAndSetOrdered(nullptr, new PluginManager);//第二次检测。
    }

    return *m_instance;
}

QVector<Plugin *> PluginManager::installedPlugins() const
{
    qCDebug(appLog) << "Getting installed plugins, count:" << m_plugins.size();
    return m_plugins;
}

QVector<Plugin *> PluginManager::availablePlugins() const
{
    qCDebug(appLog) << "Getting available plugins";
    QVector<Plugin *> availablePlugins;
    for (Plugin *plugin : qAsConst(m_plugins)) {
        if (plugin->isValid()) {
            availablePlugins << plugin;
        }
    }

    qCDebug(appLog) << "Found" << availablePlugins.size() << "available plugins";
    return availablePlugins;
}

QVector<Plugin *> PluginManager::availableWritePlugins() const
{
    qCDebug(appLog) << "Getting available write plugins";
    QVector<Plugin *> availableWritePlugins;
    const auto plugins = availablePlugins();
    for (Plugin *plugin : plugins) {
        if (plugin->isReadWrite()) {
            availableWritePlugins << plugin;
        }
    }

    qCDebug(appLog) << "Found" << availableWritePlugins.size() << "available write plugins";
    return availableWritePlugins;
}

QVector<Plugin *> PluginManager::enabledPlugins() const
{
    qCDebug(appLog) << "Getting enabled plugins";
    QVector<Plugin *> enabledPlugins;
    for (Plugin *plugin : qAsConst(m_plugins)) {
        if (plugin->isEnabled()) {
            enabledPlugins << plugin;
        }
    }

    qCDebug(appLog) << "Found" << enabledPlugins.size() << "enabled plugins";
    return enabledPlugins;
}

QVector<Plugin *> PluginManager::preferredPluginsFor(const CustomMimeType &mimeType)
{
    qCDebug(appLog) << "Getting preferred plugins for mime type:" << mimeType.name();
    const auto mimeName = mimeType.name();
    if (m_preferredPluginsCache.contains(mimeName)) {
        qCDebug(appLog) << "Using cached preferred plugins for:" << mimeName;
        return m_preferredPluginsCache.value(mimeName);
    }

    const auto plugins = preferredPluginsFor(mimeType, false);
    m_preferredPluginsCache.insert(mimeName, plugins);
    qCDebug(appLog) << "Cached" << plugins.size() << "preferred plugins for:" << mimeName;
    return plugins;
}

QVector<Plugin *> PluginManager::preferredWritePluginsFor(const CustomMimeType &mimeType) const
{
    qCDebug(appLog) << "Getting preferred write plugins for mime type:" << mimeType.name();
    return preferredPluginsFor(mimeType, true);
}

Plugin *PluginManager::preferredPluginFor(const CustomMimeType &mimeType)
{
    qCDebug(appLog) << "Getting preferred plugin for mime type:" << mimeType.name();
    const QVector<Plugin *> preferredPlugins = preferredPluginsFor(mimeType);
    if (preferredPlugins.isEmpty()) {
        qCWarning(appLog) << "No preferred plugin found for mime type:" << mimeType.name();
        return new Plugin();
    }
    qCDebug(appLog) << "Using preferred plugin:" << preferredPlugins.first()->metaData().pluginId();
    return preferredPlugins.first();
}

Plugin *PluginManager::preferredWritePluginFor(const CustomMimeType &mimeType) const
{
    qCDebug(appLog) << "Getting preferred write plugin for mime type:" << mimeType.name();
    const QVector<Plugin *> preferredWritePlugins = preferredWritePluginsFor(mimeType);
    if (preferredWritePlugins.isEmpty()) {
        qCWarning(appLog) << "No preferred write plugin found for mime type:" << mimeType.name();
        return new Plugin();
    }
    qCDebug(appLog) << "Using preferred write plugin:" << preferredWritePlugins.first()->metaData().pluginId();
    return preferredWritePlugins.first();
}

QStringList PluginManager::supportedMimeTypes(MimeSortingMode mode) const
{
    qCDebug(appLog) << "Getting supported mime types with mode:" << mode;
    QSet<QString> supported;
    QMimeDatabase db;
    const auto plugins = availablePlugins();
    qCDebug(appLog) << "Processing" << plugins.size() << "available plugins";
    for (Plugin *plugin : plugins) {
        const auto mimeTypes = plugin->metaData().mimeTypes();
        for (const auto &mimeType : mimeTypes) {
            if (db.mimeTypeForName(mimeType).isValid()) {
                supported.insert(mimeType);
            }
        }
    }

    // Remove entry for lrzipped tar if lrzip executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lrzip")).isEmpty()) {
        qCDebug(appLog) << "Removing lrzip-compressed-tar support - executable not found";
        supported.remove(QStringLiteral("application/x-lrzip-compressed-tar"));
    }

    // Remove entry for lz4-compressed tar if lz4 executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lz4")).isEmpty()) {
        qCDebug(appLog) << "Removing lz4-compressed-tar support - executable not found";
        supported.remove(QStringLiteral("application/x-lz4-compressed-tar"));
    }

    // Remove entry for lzo-compressed tar if libarchive not linked against lzo and lzop executable not found in path.
    if (!libarchiveHasLzo() && QStandardPaths::findExecutable(QStringLiteral("lzop")).isEmpty()) {
        qCDebug(appLog) << "Removing tzo support - no lzo support and lzop not found";
        supported.remove(QStringLiteral("application/x-tzo"));
    }

    if (mode == SortByComment) {
        qCDebug(appLog) << "Sorting mime types by comment";
        return sortByComment(supported);
    }

    qCDebug(appLog) << "Found" << supported.size() << "supported mime types";
    return supported.values();
}

QStringList PluginManager::supportedWriteMimeTypes(MimeSortingMode mode) const
{
    qCDebug(appLog) << "Getting supported write mime types with mode:" << mode;
    QSet<QString> supported;
    QMimeDatabase db;
    const auto plugins = availableWritePlugins();
    qCDebug(appLog) << "Processing" << plugins.size() << "available write plugins";
    for (Plugin *plugin : plugins) {
        const auto mimeTypes = plugin->metaData().mimeTypes();
        for (const auto &mimeType : mimeTypes) {
            if (db.mimeTypeForName(mimeType).isValid()) {
                supported.insert(mimeType);
            }
        }
    }

    // Remove entry for lrzipped tar if lrzip executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lrzip")).isEmpty()) {
        qCDebug(appLog) << "Removing lrzip-compressed-tar write support - executable not found";
        supported.remove(QStringLiteral("application/x-lrzip-compressed-tar"));
    }

    // Remove entry for lz4-compressed tar if lz4 executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lz4")).isEmpty()) {
        qCDebug(appLog) << "Removing lz4-compressed-tar write support - executable not found";
        supported.remove(QStringLiteral("application/x-lz4-compressed-tar"));
    }

    // Remove entry for lzo-compressed tar if libarchive not linked against lzo and lzop executable not found in path.
    if (!libarchiveHasLzo() && QStandardPaths::findExecutable(QStringLiteral("lzop")).isEmpty()) {
        qCDebug(appLog) << "Removing tzo write support - no lzo support and lzop not found";
        supported.remove(QStringLiteral("application/x-tzo"));
    }

    qCDebug(appLog) << "Removing ISO image write support";
    supported.remove(QStringLiteral("application/x-iso9660-image"));

    qCDebug(appLog) << "Removing RAR write support";
    supported.remove(QStringLiteral("application/vnd.rar"));
    supported.remove(QStringLiteral("application/x-rar"));
//    supported.remove(QStringLiteral("application/octet-stream"));

    if (mode == SortByComment) {
        qCDebug(appLog) << "Sorting write mime types by comment";
        return sortByComment(supported);
    }

    qCDebug(appLog) << "Found" << supported.size() << "supported write mime types";
    return supported.values();
}

QVector<Plugin *> PluginManager::filterBy(const QVector<Plugin *> &plugins, const CustomMimeType &mimeType) const
{
    qCDebug(appLog) << "Filtering plugins for mime type:" << mimeType.name();
    const bool supportedMime = supportedMimeTypes().contains(mimeType.name());
    QVector<Plugin *> filteredPlugins;
    for (Plugin *plugin : plugins) {
        if (!supportedMime) {
            // Check whether the mimetype inherits from a supported mimetype.
            const QStringList mimeTypes = plugin->metaData().mimeTypes();
            for (const QString &mime : mimeTypes) {
                if (mimeType.inherits(mime)/* && (mime != "application/octet-stream")*/) {
                    qCDebug(appLog) << "Found plugin" << plugin->metaData().pluginId() << "supporting parent mime type:" << mime;
                    filteredPlugins << plugin;
                }
            }
        } else if (plugin->metaData().mimeTypes().contains(mimeType.name())) {
            qCDebug(appLog) << "Found plugin" << plugin->metaData().pluginId() << "directly supporting mime type:" << mimeType.name();
            
            if (mimeType.name() == QString("application/x-tzo") && plugin->metaData().pluginId() == QString("kerfuffle_cli7z")) {
                qCDebug(appLog) << "Adding special case for x-tzo mime type";
                filteredPlugins << plugin;
                continue;
            }

            filteredPlugins << plugin;
        }
    }

    qCDebug(appLog) << "Found" << filteredPlugins.count() << "matching plugins";
    return filteredPlugins;
}

void PluginManager::setFileSize(qint64 size)
{
    qCDebug(appLog) << "Setting file size to:" << size;
    m_filesize = size;
}

void PluginManager::loadPlugins()
{
    qCDebug(appLog) << "Loading plugins from /usr/lib/";
    QCoreApplication::addLibraryPath("/usr/lib/");
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("deepin-compressor/plugins"));
    qCDebug(appLog) << "Found" << plugins.size() << "potential plugins";
    
    QSet<QString> addedPlugins;
    for (const KPluginMetaData &metaData : plugins) {
        const auto pluginId = metaData.pluginId();
        // Filter out duplicate plugins.
        if (addedPlugins.contains(pluginId)) {
            qCDebug(appLog) << "Skipping duplicate plugin:" << pluginId;
            continue;
        }

        Plugin *plugin = new Plugin(this, metaData);
        plugin->setEnabled(true);
        addedPlugins << pluginId;
        m_plugins << plugin;
        qCDebug(appLog) << "Loaded plugin:" << pluginId;
    }
    qCDebug(appLog) << "Successfully loaded" << addedPlugins.size() << "unique plugins";
}

QVector<Plugin *> PluginManager::preferredPluginsFor(const CustomMimeType &mimeType, bool readWrite) const
{
    qCDebug(appLog) << "Getting preferred plugins for mime type:" << mimeType.name() << "readWrite:" << readWrite;
    QVector<Plugin *> preferredPlugins = filterBy((readWrite ? availableWritePlugins() : availablePlugins()), mimeType);

    qCDebug(appLog) << "Sorting" << preferredPlugins.size() << "plugins by priority";
    std::sort(preferredPlugins.begin(), preferredPlugins.end(), [](Plugin * p1, Plugin * p2) {
        return p1->priority() > p2->priority();
    });

    //arm64 use libzip for comptess *.zip cause libzip is better under huawei
    //#ifdef __arm64__
    //    if( entrySize == 1 && readWrite &&  mimeType.name() == QString("application/zip"))
    //    {
    //        foreach(Plugin* plugin, preferredPlugins)
    //        {
    //            if(plugin->metaData().name().contains("7zip"))
    //            {
    //                preferredPlugins.removeOne(plugin);
    //                break;
    //            }
    //        }
    //    }
    //#else
    //    Q_UNUSED(entrySize)
    //#endif

//    if ((!readWrite) && (mimeType.name() == QString("application/zip") || mimeType.name() == QString("application/x-tar"))) {
//        foreach (Plugin *plugin, preferredPlugins) {
//            if (plugin->metaData().name().contains("7zip")) {
//                preferredPlugins.removeOne(plugin);
//                break;
//            }
//        }
//    }

    qCDebug(appLog) << "Returning" << preferredPlugins.size() << "preferred plugins";
    return preferredPlugins;
}

QStringList PluginManager::sortByComment(const QSet<QString> &mimeTypes)
{
    qCDebug(appLog) << "Sorting" << mimeTypes.size() << "mime types by comment";
    QMap<QString, QString> map;

    // Initialize the QMap to sort by comment.
    for (const QString &mimeType : mimeTypes) {
        QMimeType mime(QMimeDatabase().mimeTypeForName(mimeType));
        map[mime.preferredSuffix().toLower()] = mime.name();
    }

    // Convert to sorted QStringList.
    QStringList sortedMimeTypes;
    for (const QString &value : qAsConst(map)) {
        sortedMimeTypes << value;
    }

    qCDebug(appLog) << "Sorted mime types, result count:" << sortedMimeTypes.size();
    return sortedMimeTypes;
}

bool PluginManager::libarchiveHasLzo()
{
    qCDebug(appLog) << "Checking if libarchive has LZO support";
    // Step 1: look for the libarchive plugin, which is built against libarchive.
    const QString strPluginPath = []() {
        const QStringList paths = QCoreApplication::libraryPaths();
        for (const QString &path : paths) {
            const QString pluginPath = QStringLiteral("%1/kerfuffle/kerfuffle_libarchive.so").arg(path);
            if (QFileInfo::exists(pluginPath)) {
                qCDebug(appLog) << "Found libarchive plugin at:" << pluginPath;
                return pluginPath;
            }
        }
        qCWarning(appLog) << "Libarchive plugin not found";
        return QString();
    }();

    // Step 2: process the libarchive plugin dependencies to figure out the absolute libarchive path.
    QProcess dependencyTool;
    QStringList args;
#ifdef DEPENDENCY_TOOL_ARGS
    args << QStringLiteral(DEPENDENCY_TOOL_ARGS);
#endif
    dependencyTool.setProgram(QStringLiteral("ldd"));
    dependencyTool.setArguments(args + QStringList(strPluginPath));
    dependencyTool.start();
    dependencyTool.waitForFinished();
    const QString output = QString::fromUtf8(dependencyTool.readAllStandardOutput());
    QRegularExpression regex(QStringLiteral("/.*/libarchive.so|/.*/libarchive.*.dylib"));
    if (!regex.match(output).hasMatch()) {
        qCWarning(appLog) << "libarchive.so not found in dependencies";
        return false;
    }

    // Step 3: check whether libarchive links against liblzo.
    const QStringList libarchivePath(regex.match(output).captured(0));
    qCDebug(appLog) << "Found libarchive at:" << libarchivePath;
    dependencyTool.setArguments(args + libarchivePath);
    dependencyTool.start();
    dependencyTool.waitForFinished();
    const bool hasLzo = dependencyTool.readAllStandardOutput().contains(QByteArrayLiteral("lzo"));
    qCDebug(appLog) << "Libarchive LZO support:" << hasLzo;
    return hasLzo;
}
