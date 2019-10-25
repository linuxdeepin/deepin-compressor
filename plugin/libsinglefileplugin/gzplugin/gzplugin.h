#ifndef GZPLUGIN_H
#define GZPLUGIN_H

#include "../singlefileplugin.h"
#include "kpluginfactory.h"


class LibGzipInterfaceFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libgz.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibGzipInterfaceFactory();
    ~LibGzipInterfaceFactory();
};

class LibGzipInterface : public LibSingleFileInterface
{
    Q_OBJECT

public:
    LibGzipInterface(QObject *parent, const QVariantList & args);
    ~LibGzipInterface() override;
};

#endif // GZPLUGIN_H
