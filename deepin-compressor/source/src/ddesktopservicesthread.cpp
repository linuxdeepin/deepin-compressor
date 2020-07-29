#include "ddesktopservicesthread.h"
#include <DDesktopServices>

DDesktopServicesThread::DDesktopServicesThread(QObject *parent): QThread(parent)
{

}

bool DDesktopServicesThread::event(QEvent *event)
{

}

void DDesktopServicesThread::run()
{
    qDebug() <<"m_path"<<m_path;
     DDesktopServices::showFileItem(m_path);
}
