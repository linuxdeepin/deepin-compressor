/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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

#ifndef PROCESSOPENTHREAD_H
#define PROCESSOPENTHREAD_H

#include "archiveinterface.h"
#include "plugin.h"

#include <QThread>

class ProcessOpenThread : public QThread
{
    Q_OBJECT
public:
    explicit ProcessOpenThread(QObject *parent = nullptr);

    /**
     * @brief setProgramPath    设置应用程序
     * @param strProgramPath    应用程序路径
     */
    void setProgramPath(const QString &strProgramPath);

    /**
     * @brief setArguments  设置参数
     * @param listArguments 启动参数
     */
    void setArguments(const QStringList &listArguments);

protected:
    void run() override;

private:
    QString m_strProgramPath;
    QStringList m_listArguments;
};


#endif  // UITOOLS_H
