#include "bz2plugin.h"

#include <QString>

//#include "kpluginfactory.h"

//K_PLUGIN_CLASS_WITH_JSON(LibBzip2Interface, "kerfuffle_libbz2.json")

LibBzip2InterfaceFactory::LibBzip2InterfaceFactory()
{
    registerPlugin<LibBzip2Interface>();
}
LibBzip2InterfaceFactory::~LibBzip2InterfaceFactory()
{

}

LibBzip2Interface::LibBzip2Interface(QObject *parent, const QVariantList &args)
    : LibSingleFileInterface(parent, args)
{
    m_mimeType = QStringLiteral("application/x-bzip");
    m_possibleExtensions.append(QStringLiteral(".bz2"));
}

LibBzip2Interface::~LibBzip2Interface()
{
}

//#include "bz2plugin.moc"
