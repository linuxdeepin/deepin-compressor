// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processopenthread.h"
#include "common/ddlog.h"

#include <QProcess>

ProcessOpenThread::ProcessOpenThread(QObject *parent)
    : QThread(parent)
{
    connect(this, &ProcessOpenThread::finished, this, &QObject::deleteLater);
}

void ProcessOpenThread::setProgramPath(const QString &strProgramPath)
{
    qCDebug(appLog) << "Setting program path:" << strProgramPath;
    m_strProgramPath = strProgramPath;
}

void ProcessOpenThread::setArguments(const QStringList &listArguments)
{
    qCDebug(appLog) << "Setting arguments, count:" << listArguments.size();
    m_listArguments = listArguments;
}

void ProcessOpenThread::run()
{
    qCDebug(appLog) << "Starting external process:" << m_strProgramPath;

    // 启动外部进程。子进程可能运行很久（外部程序窗口），这里阻塞到它退出为止。
    QProcess process;
    process.setProgram(m_strProgramPath);
    process.setArguments(m_listArguments);
    process.setProcessChannelMode(QProcess::MergedChannels);
    // 子进程的输出没人读，直接丢弃，避免管道写满把子进程卡住
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start();
    if (!process.waitForStarted(-1)) {
        qCWarning(appLog) << "Failed to start process:" << m_strProgramPath
                          << process.errorString();
        return;
    }

    process.waitForFinished(-1);
    qCDebug(appLog) << "Process finished with exit code:" << process.exitCode();
}
