// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archivejob.h"
#include <QDebug>
#include "common/ddlog.h"

ArchiveJob::ArchiveJob(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "ArchiveJob object created";
}

ArchiveJob::~ArchiveJob()
{
    qCDebug(appLog) << "ArchiveJob object destroyed";
}

void ArchiveJob::kill()
{
    if (doKill()) {
        // 设置结束标志
        m_eFinishedType = PFT_Cancel;
        m_eErrorType = ET_UserCancelOpertion;
        qCWarning(appLog) << "Archive job cancelled by user";
        finishJob();
    }
}

bool ArchiveJob::status()
{
    return true;
}

void ArchiveJob::finishJob()
{
    qCDebug(appLog) << "Archive job finished with status:" << m_eFinishedType;
    emit signalJobFinshed();
    deleteLater();
}

bool ArchiveJob::doKill()
{
    return false;
}
