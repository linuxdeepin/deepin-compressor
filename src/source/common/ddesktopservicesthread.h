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

#ifndef DDESKTOPSERVICESTHREAD_H
#define DDESKTOPSERVICESTHREAD_H

#include <QThread>

#include <DDesktopServices>
#include <QDebug>
#include <QFileInfo>
DWIDGET_USE_NAMESPACE

// 文管打开文件目录线程
class DDesktopServicesThread : public QThread
{
public:
    explicit DDesktopServicesThread(QObject *parent = nullptr);

    /**
     * @brief setOpenFiles   设置打开路径或文件
     * @param listFiles     打开的文件目录
     */
    void setOpenFiles(const QStringList &listFiles);

    /**
     * @brief hasFiles      判断是否有文件
     * @return
     */
    bool hasFiles();

protected:
    /**
     * @brief run   运行
     */
    void run();

private:
    QStringList m_listFiles;    // 需要打开的文件路径
};

#endif // DDESKTOPSERVICESTHREAD_H
