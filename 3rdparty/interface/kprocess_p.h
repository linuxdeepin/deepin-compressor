#ifndef KPROCESS_P_H
#define KPROCESS_P_H

#include "kprocess.h"

class KProcessPrivate
{
    Q_DECLARE_PUBLIC(KProcess)
protected:
    KProcessPrivate(KProcess *q)
        : openMode(QIODevice::ReadWrite)
        , q_ptr(q)
    {
    }

    QString prog;
    QStringList args;
    QIODevice::OpenMode openMode;

    KProcess *q_ptr;
};

#endif // KPROCESS_P_H
