#ifndef XZPLUGIN_H
#define XZPLUGIN_H

#include "../singlefileplugin.h"
#include "kpluginfactory.h"


class LibXzInterfaceFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libxz.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibXzInterfaceFactory();
    ~LibXzInterfaceFactory();
};

class LibXzInterface : public LibSingleFileInterface
{
    Q_OBJECT

public:
    LibXzInterface(QObject *parent, const QVariantList & args);
    ~LibXzInterface() override;
};

#endif // XZPLUGIN_H
