
#ifndef KEXPORTPLUGIN_H
#define KEXPORTPLUGIN_H

#include <QPluginLoader>
#include <QtPlugin>


/**
 * \relates KPluginLoader
 * Use this macro if you want to give your plugin a version number.
 * You can later access the version number with KPluginLoader::pluginVersion()
 */
#define K_EXPORT_PLUGIN_VERSION(version) \
    Q_EXTERN_C Q_DECL_EXPORT const quint32 kde_plugin_version = version;

/**
 * \relates KPluginLoader
 * This macro exports the main object of the plugin. Most times, this will be a KPluginFactory
 * or derived class, but any QObject derived class can be used.
 * Take a look at the documentation of Q_EXPORT_PLUGIN2 for some details.
 */

#if defined (Q_OS_WIN32) && defined(Q_CC_BOR)
#define Q_STANDARD_CALL __stdcall
#else
#define Q_STANDARD_CALL

#ifndef KCOREADDONS_NO_DEPRECATED
class  K_EXPORT_PLUGIN_is_deprecated_see_KDE5PORTING
{
};

#define K_EXPORT_PLUGIN(factory) \
    K_EXPORT_PLUGIN_is_deprecated_see_KDE5PORTING dummy;
#endif

#endif

#endif // KEXPORTPLUGIN_H

