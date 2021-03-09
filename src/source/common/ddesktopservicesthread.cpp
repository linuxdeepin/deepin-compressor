/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
