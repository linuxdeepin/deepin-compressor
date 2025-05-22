// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>
#include <QDebug>

#include "eventlogutils.h"

EventLogUtils *EventLogUtils::mInstance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    if (mInstance == nullptr) {
        qDebug() << "Creating new EventLogUtils instance";
        mInstance = new EventLogUtils;
    } else {
        qDebug() << "Returning existing EventLogUtils instance";
    }
    return *mInstance;
}

EventLogUtils::EventLogUtils()
{
    qDebug() << "Initializing EventLogUtils";
    QLibrary library("libdeepin-event-log.so");

    init =reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("SendEventLog"));

    if (init == nullptr) {
        qWarning() << "Failed to resolve Initialize function";
        return;
    }

    init("deepin-compressor", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    qDebug() << "Writing event log";
    if (writeEventLog == nullptr) {
        qWarning() << "writeEventLog function not initialized";
        return;
    }

    QJsonObject eventObj;
    QJsonObject policyObj;
    policyObj.insert("reportMode", REPORT_AND_BROADCAST);
    eventObj.insert("policy", policyObj);
    eventObj.insert("info", data);
    //std::string str = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    writeEventLog(QJsonDocument(eventObj).toJson(QJsonDocument::Compact).toStdString());
}
