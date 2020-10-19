#ifndef CLI7ZPLUGIN_H
#define CLI7ZPLUGIN_H

#include "cliinterface.h"
#include "kpluginfactory.h"

class Cli7zPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_cli7z.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit Cli7zPluginFactory();
    ~Cli7zPluginFactory();
};

class Cli7zPlugin : public CliInterface
{
    Q_OBJECT

public:
    explicit Cli7zPlugin(QObject *parent, const QVariantList &args);
    ~Cli7zPlugin() override;

private:
    void setupCliProperties();
};

#endif // CLI7ZPLUGIN_H
