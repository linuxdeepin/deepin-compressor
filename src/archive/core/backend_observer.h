// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKEND_OBSERVER_H
#define BACKEND_OBSERVER_H

#include "archive_types.h"

#include <QString>

namespace dcarchive {

class BackendObserver
{
public:
    virtual ~BackendObserver() = default;

    virtual void onProgress(const ProgressInfo &progress) { Q_UNUSED(progress); }
    virtual void onCurrentFile(const QString &path) { Q_UNUSED(path); }
    virtual void onPasswordRequired(PasswordRequest &request) { request.response = PasswordRequest::Response::Cancel; }
    virtual void onFileConflict(FileConflictRequest &request) { request.response = FileConflictRequest::Response::Cancel; }
    virtual void onWarning(const WarningInfo &warning) { Q_UNUSED(warning); }
    virtual void onFinished(const FinishInfo &finish) { Q_UNUSED(finish); }
};

} // namespace dcarchive

#endif // BACKEND_OBSERVER_H
