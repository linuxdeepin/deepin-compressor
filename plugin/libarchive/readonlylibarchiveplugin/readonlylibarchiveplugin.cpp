
#include "readonlylibarchiveplugin.h"

//#include "kpluginfactory.h"

//K_PLUGIN_CLASS_WITH_JSON(ReadOnlyLibarchivePlugin, "kerfuffle_libarchive_readonly.json")
ReadOnlyLibarchivePluginFactory::ReadOnlyLibarchivePluginFactory()
{
    registerPlugin<ReadOnlyLibarchivePlugin>();
}
ReadOnlyLibarchivePluginFactory::~ReadOnlyLibarchivePluginFactory()
{

}

ReadOnlyLibarchivePlugin::ReadOnlyLibarchivePlugin(QObject *parent, const QVariantList & args)
    : LibarchivePlugin(parent, args)
{
}

ReadOnlyLibarchivePlugin::~ReadOnlyLibarchivePlugin()
{
}



//#include "readonlylibarchiveplugin.moc"
