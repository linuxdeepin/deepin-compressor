// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processopenthread.h"
#include "kprocess.h"
#include <QDebug>
#include "common/ddlog.h"

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
    // 启动外部进程
    KProcess *cmdprocess = new KProcess;
    cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
    cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    cmdprocess->setProgram(m_strProgramPath, m_listArguments);
    cmdprocess->start();
    cmdprocess->waitForFinished(-1);
    qCDebug(appLog) << "Process finished with exit code:" << cmdprocess->exitCode();
    delete cmdprocess;
}
