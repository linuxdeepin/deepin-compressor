// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DebugTimeManager.h"
#include <QDateTime>
#include <QDebug>
#include "common/ddlog.h"

#include <sys/time.h>

DebugTimeManager    *DebugTimeManager::s_Instance = nullptr;

DebugTimeManager::DebugTimeManager()
{
    qCDebug(appLog) << "DebugTimeManager instance created";
}

void DebugTimeManager::clear()
{
    qCDebug(appLog) << "Clearing all debug points";
    m_MapPoint.clear();
    qCDebug(appLog) << "Debug points cleared";
}

void DebugTimeManager::beginPointQt(const QString &point, const QString &status)
{
    qCDebug(appLog) << "Begin Qt debug point:" << point << "status:" << status;
    PointInfo info;
    info.desc = status;
    info.time = QDateTime::currentMSecsSinceEpoch();
    m_MapPoint.insert(point, info);
    qCDebug(appLog) << "Qt debug point started";
}

void DebugTimeManager::endPointQt(const QString &point)
{
    qCDebug(appLog) << "End Qt debug point:" << point;
    if (m_MapPoint.find(point) != m_MapPoint.end()) {
        m_MapPoint[point].time = QDateTime::currentMSecsSinceEpoch() - m_MapPoint[point].time;
        qCInfo(appLog) << QString("[GRABPOINT] %1 %2 time=%3ms").arg(point).arg(m_MapPoint[point].desc).arg(m_MapPoint[point].time);
        m_MapPoint.remove(point);
        qCDebug(appLog) << "Qt debug point ended successfully";
    } else {
        qCWarning(appLog) << "Qt debug point not found:" << point;
    }
}

void DebugTimeManager::beginPointLinux(const QString &point, const QString &status)
{
    qCDebug(appLog) << "Begin Linux debug point:" << point << "status:" << status;
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    PointInfo info;
    info.desc = status;
    info.time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    m_MapPoint.insert(point, info);
    qCDebug(appLog) << "Linux debug point started";
}

void DebugTimeManager::endPointLinux(const QString &point)
{
    qCDebug(appLog) << "End Linux debug point:" << point;
    if (m_MapPoint.find(point) != m_MapPoint.end()) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        m_MapPoint[point].time = tv.tv_sec * 1000 + tv.tv_usec / 1000 - m_MapPoint[point].time;
        qCInfo(appLog) << QString("[GRABPOINT] %1 %2 time=%3ms").arg(point).arg(m_MapPoint[point].desc).arg(m_MapPoint[point].time);
        m_MapPoint.remove(point);
        qCDebug(appLog) << "Linux debug point ended successfully";
    } else {
        qCWarning(appLog) << "Linux debug point not found:" << point;
    }
}
