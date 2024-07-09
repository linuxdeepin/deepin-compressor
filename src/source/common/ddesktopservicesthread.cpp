// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddesktopservicesthread.h"

#include <DDesktopServices>
#include <dtkwidget_global.h>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
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
