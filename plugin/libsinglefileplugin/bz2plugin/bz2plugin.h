#ifndef BZ2PLUGIN_H
#define BZ2PLUGIN_H

#include "../singlefileplugin.h"
#include "kpluginfactory.h"


class LibBzip2InterfaceFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libbz2.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibBzip2InterfaceFactory();
    ~LibBzip2InterfaceFactory();
};

class LibBzip2Interface : public LibSingleFileInterface
{
    Q_OBJECT

public:
    LibBzip2Interface(QObject *parent, const QVariantList & args);
    ~LibBzip2Interface() override;
};

#endif // BZ2PLUGIN_H
