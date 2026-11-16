// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKEND_OBSERVER_H
#define BACKEND_OBSERVER_H

#include "archive_types.h"

namespace archive {

class BackendObserver
{
public:
    virtual ~BackendObserver() = default;

    virtual void onProgress(const ProgressInfo &progress) = 0;
    virtual void onCurrentFile(const QString &path) = 0;
    virtual void onPasswordRequired(PasswordRequest &request) = 0;
    virtual void onFileConflict(FileConflictRequest &request) = 0;
    virtual void onWarning(const WarningInfo &warning) = 0;
    virtual void onFinished(const FinishInfo &finish) = 0;
};

} // namespace archive

#endif // BACKEND_OBSERVER_H
