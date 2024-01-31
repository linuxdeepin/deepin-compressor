// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTLOGUTILS_H
#define EVENTLOGUTILS_H

#include <QJsonObject>
#include <string>

class EventLogUtils
{
public:
    enum EventTID {
        OpeningTime     = 1000000000,
        ClosingTime     = 1000000001,
        Start           = 1000000003,
        Quit            = 1000000004
    };
    enum CompressTID {
        LoadCompressFile        = 1000200040,
        OpenCompressFile        = 1000200041,
        ExtractCompressFile     = 1000200042,
        ExtractSingleFile       = 1000200043,
        closeCompressWnd        = 1000200044,
        AddCompressFile         = 1000200045,
        DelCompressFile         = 1000200046,
        RenameCompressFile      = 1000200047
    };
    enum REPORTMODE
    {
        BROADCAST               = 1,
        REPORT                  = 2,
        REPORT_AND_BROADCAST    = BROADCAST | REPORT
    };
    static EventLogUtils &get();
    void writeLogs(QJsonObject &data);

private:
    bool (*init)(const std::string &packagename, bool enable_sig) = nullptr;
    void (*writeEventLog)(const std::string &eventdata) = nullptr;

    static EventLogUtils *mInstance;

    EventLogUtils();
};

#endif // EVENTLOGUTILS_H
