

#ifndef KPLUGINFACTORY_P_H
#define KPLUGINFACTORY_P_H

#include "kpluginfactory.h"

#include <QHash>

class KPluginFactoryPrivate
{
    Q_DECLARE_PUBLIC(KPluginFactory)
protected:
    typedef QPair<const QMetaObject *, KPluginFactory::CreateInstanceFunction> Plugin;

    KPluginFactoryPrivate() : catalogInitialized(false) {}
    ~KPluginFactoryPrivate()
    {
    }

    QHash<QString, Plugin> createInstanceHash;
    QString catalogName;
    bool catalogInitialized;

    KPluginFactory *q_ptr;
};

#endif // KPLUGINFACTORY_P_H
