#ifndef CLI7ZPLUGIN_GLOBAL_H
#define CLI7ZPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CLI7ZPLUGIN_LIBRARY)
#  define CLI7ZPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CLI7ZPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CLI7ZPLUGIN_GLOBAL_H