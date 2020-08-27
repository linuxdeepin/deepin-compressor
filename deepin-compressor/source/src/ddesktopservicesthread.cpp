#include "ddesktopservicesthread.h"

#include <DDesktopServices>

DDesktopServicesThread::DDesktopServicesThread(QObject *parent)
    : QThread(parent)
{

}

bool DDesktopServicesThread::event(QEvent *event)
{
    return true;
}

void DDesktopServicesThread::run()
{
    QFileInfo fileinfo(m_path);
    qDebug() << "m_path" << m_path;
    if (fileinfo.isDir()) {
        DDesktopServices::showFolder(m_path); // 如果是文件夹
    } else if (fileinfo.isFile()) {
        DDesktopServices::showFileItem(m_path);
    }

    return;
}
