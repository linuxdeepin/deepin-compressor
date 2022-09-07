// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
