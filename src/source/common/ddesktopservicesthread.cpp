#include "ddesktopservicesthread.h"

#include <DDesktopServices>

DDesktopServicesThread::DDesktopServicesThread(QObject *parent)
    : QThread(parent)
{

}

void DDesktopServicesThread::setOpenFiles(const QStringList &listFiles)
{
    m_listFiles = listFiles;
}

bool DDesktopServicesThread::hasFiles()
{
    return !m_listFiles.isEmpty();
}

void DDesktopServicesThread::run()
{
    for (int i = 0; i < m_listFiles.count(); ++i) {
        QFileInfo info(m_listFiles[i]);
        if (info.isDir()) {
            // 如果是文件夹
            DDesktopServices::showFolder(m_listFiles[i]);
        } else {
            // 如果是文件
            DDesktopServices::showFileItem(m_listFiles[i]);
        }
    }
}
