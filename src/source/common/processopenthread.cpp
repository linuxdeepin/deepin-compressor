// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processopenthread.h"
#include <QDebug>
#include <QDesktopServices>
#include <QProcess>
#include <QUrl>

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

    // 默认应用场景（xdg-open 或程序路径为空）：
    // 使用 QDesktopServices::openUrl 由 Qt 平台插件正确处理 file:// URI 和 D-Bus 激活，
    // 避免文件路径在异步 D-Bus 握手过程中丢失
    if (m_strProgramPath.isEmpty() || m_strProgramPath.endsWith(QLatin1String("xdg-open"))) {
        if (m_listArguments.isEmpty()) {
            qWarning() << "No file to open, arguments is empty";
            return;
        }
        const QString &file = m_listArguments.first();
        bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(file));
        if (!ok) {
            qWarning() << "QDesktopServices::openUrl failed for file:" << file;
        }
        return;
    }

    // 指定程序场景：使用 QProcess::startDetached 非阻塞启动
    bool ok = QProcess::startDetached(m_strProgramPath, m_listArguments);
    if (!ok) {
        qWarning() << "QProcess::startDetached failed for program:" << m_strProgramPath
                   << "arguments:" << m_listArguments;
    }
}
