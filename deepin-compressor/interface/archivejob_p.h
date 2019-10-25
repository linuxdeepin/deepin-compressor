
#ifndef KJOB_P_H
#define KJOB_P_H

#include "archivejob.h"
#include <QMap>
#include <QEventLoopLocker>


class QTimer;
class QEventLoop;


class  KJobPrivate
{
public:
    KJobPrivate();
    virtual ~KJobPrivate();

    KJob *q_ptr;

    QString errorText;
    int error;
    KJob::Unit progressUnit;
    QMap<KJob::Unit, qulonglong> processedAmount;
    QMap<KJob::Unit, qulonglong> totalAmount;
    unsigned long percentage;
    QTimer *speedTimer;
    QEventLoop *eventLoop;
    // eventLoopLocker prevents QCoreApplication from exiting when the last
    // window is closed until the job has finished running
    QEventLoopLocker eventLoopLocker;
    KJob::Capabilities capabilities;
    bool suspended;
    bool isAutoDelete;

    void _k_speedTimeout();

    bool isFinished;

    Q_DECLARE_PUBLIC(KJob)
};

#endif
