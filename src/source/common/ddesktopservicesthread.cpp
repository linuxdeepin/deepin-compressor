// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddesktopservicesthread.h"
#include <QDebug>

#include <DDesktopServices>
#include <dtkwidget_global.h>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
DDesktopServicesThread::DDesktopServicesThread(QObject *parent)
    : QThread(parent)
{
    qDebug() << "DDesktopServicesThread initialized";
}

void DDesktopServicesThread::setOpenFiles(const QStringList &listFiles)
{
    qInfo() << "Set open files list, count:" << listFiles.size();
    m_listFiles = listFiles;
}

bool DDesktopServicesThread::hasFiles()
{
    return !m_listFiles.isEmpty();
}

void DDesktopServicesThread::run()
{
    qDebug() << "Start opening" << m_listFiles.size() << "files/folders";
    for (int i = 0; i < m_listFiles.count(); ++i) {
        QFileInfo info(m_listFiles[i]);
        if (info.isDir()) {
            qDebug() << "Opening folder:" << m_listFiles[i];
            DDesktopServices::showFolder(m_listFiles[i]);
        } else {
            qDebug() << "Opening file:" << m_listFiles[i];
            DDesktopServices::showFileItem(m_listFiles[i]);
        }
    }
    qDebug() << "Finished opening all files/folders";
}
