#ifndef LIBZIPPLUGIN_GLOBAL_H
#define LIBZIPPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBZIPPLUGIN_LIBRARY)
#define LIBZIPPLUGINSHARED_EXPORT Q_DECL_EXPORT // 必须添加到符号声明中（共享库项目）
#else
#define LIBZIPPLUGINSHARED_EXPORT Q_DECL_IMPORT // 必须添加到符号声明中（使用共享库的客户项目）
#endif

#endif // LIBZIPPLUGIN_GLOBAL_H
