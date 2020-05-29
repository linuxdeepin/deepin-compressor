#include "xzplugin.h"

#include <QString>

//#include "kpluginfactory.h"

//K_PLUGIN_CLASS_WITH_JSON(LibXzInterface, "kerfuffle_libxz.json")

LibXzInterfaceFactory::LibXzInterfaceFactory()
{
    registerPlugin<LibXzInterface>();
}
LibXzInterfaceFactory::~LibXzInterfaceFactory()
{

}

LibXzInterface::LibXzInterface(QObject *parent, const QVariantList &args)
    : LibSingleFileInterface(parent, args)
{
    m_mimeType = QStringLiteral("application/x-lzma");
    m_possibleExtensions.append(QStringLiteral(".lzma"));
    m_possibleExtensions.append(QStringLiteral(".xz"));
}

LibXzInterface::~LibXzInterface()
{
}

//#include "xzplugin.moc"
