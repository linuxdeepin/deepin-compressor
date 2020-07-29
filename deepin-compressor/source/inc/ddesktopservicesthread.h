#ifndef DDESKTOPSERVICESTHREAD_H
#define DDESKTOPSERVICESTHREAD_H

#include <QThread>

#include <DDesktopServices>
#include <QDebug>
DWIDGET_USE_NAMESPACE

class DDesktopServicesThread : public QThread
{
public:
    explicit DDesktopServicesThread(QObject *parent = nullptr);
    QString m_path;
    // QObject interface
public:
    bool event(QEvent *event);

    // QThread interface
protected:
    void run();
};

#endif // DDESKTOPSERVICESTHREAD_H
