#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "plugin.h"

#include <QMimeType>
#include <QVector>


class PluginManager : public QObject
{
    Q_OBJECT

public:
    enum MimeSortingMode
    {
        Unsorted,
        SortByComment
    };

    explicit PluginManager(QObject *parent = nullptr);
    QVector<Plugin*> installedPlugins() const;
    QVector<Plugin*> availablePlugins() const;
    QVector<Plugin*> availableWritePlugins() const;
    QVector<Plugin*> enabledPlugins() const;
    QVector<Plugin*> preferredPluginsFor(const QMimeType &mimeType);
    QVector<Plugin*> preferredWritePluginsFor(const QMimeType &mimeType) const;
    Plugin *preferredPluginFor(const QMimeType &mimeType);
    Plugin *preferredWritePluginFor(const QMimeType &mimeType) const;
    QStringList supportedMimeTypes(MimeSortingMode mode = Unsorted) const;
    QStringList supportedWriteMimeTypes(MimeSortingMode mode = Unsorted) const;
    QVector<Plugin*> filterBy(const QVector<Plugin*> &plugins, const QMimeType &mimeType) const;

private:

    void loadPlugins();
    QVector<Plugin*> preferredPluginsFor(const QMimeType &mimeType, bool readWrite) const;
    static QStringList sortByComment(const QSet<QString> &mimeTypes);
    static bool libarchiveHasLzo();

    QVector<Plugin*> m_plugins;
    QHash<QString, QVector<Plugin*>> m_preferredPluginsCache;
};


#endif
