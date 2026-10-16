// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processopenthread.h"
#include <QDesktopServices>
#include <QProcess>
#include <QUrl>
#include <QDebug>

ProcessOpenThread::ProcessOpenThread(QObject *parent)
    : QThread(parent)
{
    connect(this, &ProcessOpenThread::finished, this, &QObject::deleteLater);
}

void ProcessOpenThread::setProgramPath(const QString &strProgramPath)
{
    qDebug() << "Setting program path:" << strProgramPath;
    m_strProgramPath = strProgramPath;
}

void ProcessOpenThread::setArguments(const QStringList &listArguments)
{
    qDebug() << "Setting arguments, count:" << listArguments.size();
    m_listArguments = listArguments;
}

void ProcessOpenThread::run()
{
    qDebug() << "Starting external process:" << m_strProgramPath;

    // 当程序路径为空或为 xdg-open 时，使用 QDesktopServices::openUrl 打开文件
    // 避免 xdg-open -> D-Bus 异步激活链路中文件路径丢失的问题
    if (m_strProgramPath.isEmpty() || m_strProgramPath.endsWith(QLatin1String("xdg-open"))) {
        if (!m_listArguments.isEmpty()) {
            bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(m_listArguments.first()));
            if (!ok) {
                qWarning() << "QDesktopServices::openUrl failed for file:" << m_listArguments.first();
            }
        }
    } else {
        bool ok = QProcess::startDetached(m_strProgramPath, m_listArguments);
        if (!ok) {
            qWarning() << "QProcess::startDetached failed for program:" << m_strProgramPath;
        }
    }
}
