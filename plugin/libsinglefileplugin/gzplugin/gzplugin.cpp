#include "gzplugin.h"

#include <QString>

//#include "kpluginfactory.h"

//K_PLUGIN_CLASS_WITH_JSON(LibGzipInterface, "kerfuffle_libgz.json")
LibGzipInterfaceFactory::LibGzipInterfaceFactory()
{
    registerPlugin<LibGzipInterface>();
}
LibGzipInterfaceFactory::~LibGzipInterfaceFactory()
{

}

LibGzipInterface::LibGzipInterface(QObject *parent, const QVariantList &args)
    : LibSingleFileInterface(parent, args)
{
    m_mimeType = QStringLiteral("application/x-gzip");
    m_possibleExtensions.append(QStringLiteral(".gz"));
}

LibGzipInterface::~LibGzipInterface()
{
}

//#include "gzplugin.moc"
