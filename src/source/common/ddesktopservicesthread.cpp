#include "ddesktopservicesthread.h"

#include <DDesktopServices>

DDesktopServicesThread::DDesktopServicesThread(QObject *parent)
    : QThread(parent)
{

}

void DDesktopServicesThread::setOpenFile(const QString &strFullPath)
{
    m_strFullPath = strFullPath;
}

void DDesktopServicesThread::run()
{
    QFileInfo info(m_strFullPath);
    if (info.isDir()) {
        // 如果是文件夹
        DDesktopServices::showFolder(m_strFullPath);
    } else {
        // 如果是文件
        DDesktopServices::showFileItem(m_strFullPath);
    }
}
