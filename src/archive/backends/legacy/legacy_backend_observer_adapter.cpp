// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_backend_observer_adapter.h"

namespace archive {

LegacyBackendObserverAdapter::LegacyBackendObserverAdapter(QObject *parent)
    : QObject(parent)
{
}

LegacyBackendObserverAdapter::~LegacyBackendObserverAdapter()
{
}

void LegacyBackendObserverAdapter::onProgress(const ProgressInfo &progress)
{
    m_lastProgress = progress.percentage;
    emit progressChanged(progress.percentage);
}

void LegacyBackendObserverAdapter::onCurrentFile(const QString &path)
{
    m_lastCurrentFile = path;
    emit currentFileChanged(path);
}

void LegacyBackendObserverAdapter::onPasswordRequired(PasswordRequest &request)
{
    m_passwordRequested = true;

    if (m_passwordHandler) {
        QString password = m_passwordHandler(request);
        if (password.isEmpty()) {
            request.cancelled = true;
        } else {
            request.password = password;
        }
    }

    emit passwordRequested(request);
}

void LegacyBackendObserverAdapter::onFileConflict(FileConflictRequest &request)
{
    m_fileConflict = true;

    if (m_conflictHandler) {
        request.action = m_conflictHandler(request);
    }

    emit fileConflict(request);
}

void LegacyBackendObserverAdapter::onWarning(const WarningInfo &warning)
{
    m_warnings.append(warning);
    emit warningOccurred(warning);
}

void LegacyBackendObserverAdapter::onFinished(const FinishInfo &finish)
{
    m_finished = true;
    m_finishInfo = finish;
    emit finished(finish);
}

} // namespace archive
