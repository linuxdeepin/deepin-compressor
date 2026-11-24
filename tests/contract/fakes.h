// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTRACT_FAKES_H
#define CONTRACT_FAKES_H

#include "backend_observer.h"

#include <QStringList>
#include <QVector>

namespace dcarchive {

class FakeObserver : public BackendObserver
{
public:
    QVector<ProgressInfo> progressEvents;
    QStringList currentFiles;
    QVector<WarningInfo> warnings;
    QVector<FinishInfo> finishes;
    QVector<PasswordRequest> passwordRequests;
    QVector<FileConflictRequest> conflictRequests;

    PasswordRequest::Response passwordResponse = PasswordRequest::Response::Cancel;
    QString passwordValue;
    FileConflictRequest::Response conflictResponse = FileConflictRequest::Response::Cancel;

    void onProgress(const ProgressInfo &progress) override { progressEvents.append(progress); }
    void onCurrentFile(const QString &path) override { currentFiles.append(path); }
    void onPasswordRequired(PasswordRequest &request) override
    {
        passwordRequests.append(request);
        request.response = passwordResponse;
        request.password = passwordValue;
    }
    void onFileConflict(FileConflictRequest &request) override
    {
        conflictRequests.append(request);
        request.response = conflictResponse;
    }
    void onWarning(const WarningInfo &warning) override { warnings.append(warning); }
    void onFinished(const FinishInfo &finish) override { finishes.append(finish); }

    void reset()
    {
        progressEvents.clear();
        currentFiles.clear();
        warnings.clear();
        finishes.clear();
        passwordRequests.clear();
        conflictRequests.clear();
    }
};

} // namespace dcarchive

#endif // CONTRACT_FAKES_H
