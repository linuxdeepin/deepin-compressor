// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACY_BACKEND_OBSERVER_ADAPTER_H
#define LEGACY_BACKEND_OBSERVER_ADAPTER_H

#include "backend_observer.h"
#include "archive_types.h"

#include <QObject>
#include <QString>
#include <QList>

namespace archive {

// ---------------------------------------------------------------------------
// LegacyBackendObserverAdapter
//
// A concrete BackendObserver implementation that bridges new observer events
// to the old UI layer during the migration period.  It records all events
// and can forward password/conflict requests to the old Query-based dialog
// system when the legacy UI is still active.
//
// This adapter lets the existing UI (which expects Query objects and
// signal/slot connections) continue to work when a backend emits events
// through the new BackendObserver interface.  When the new UI is ready,
// it can implement BackendObserver directly and this adapter becomes
// unnecessary.
// ---------------------------------------------------------------------------
class LegacyBackendObserverAdapter : public QObject, public BackendObserver
{
    Q_OBJECT

public:
    explicit LegacyBackendObserverAdapter(QObject *parent = nullptr);
    ~LegacyBackendObserverAdapter() override;

    // --- BackendObserver interface ---

    void onProgress(const ProgressInfo &progress) override;
    void onCurrentFile(const QString &path) override;
    void onPasswordRequired(PasswordRequest &request) override;
    void onFileConflict(FileConflictRequest &request) override;
    void onWarning(const WarningInfo &warning) override;
    void onFinished(const FinishInfo &finish) override;

    // --- Recorded event accessors (for testing and legacy UI bridge) ---

    double lastProgress() const { return m_lastProgress; }
    QString lastCurrentFile() const { return m_lastCurrentFile; }
    bool wasPasswordRequested() const { return m_passwordRequested; }
    bool wasFileConflict() const { return m_fileConflict; }
    bool isFinished() const { return m_finished; }
    FinishInfo finishInfo() const { return m_finishInfo; }

    QList<WarningInfo> warnings() const { return m_warnings; }

    // Set a callback to handle password requests using the old Query system.
    // When set, onPasswordRequired delegates to this callback instead of
    // leaving the password empty.
    using PasswordHandler = std::function<QString(const PasswordRequest &)>;
    void setPasswordHandler(PasswordHandler handler) { m_passwordHandler = std::move(handler); }

    // Set a callback to handle file conflicts using the old Query system.
    using ConflictHandler = std::function<FileConflictRequest::Action(const FileConflictRequest &)>;
    void setConflictHandler(ConflictHandler handler) { m_conflictHandler = std::move(handler); }

signals:
    void progressChanged(double percentage);
    void currentFileChanged(const QString &path);
    void passwordRequested(PasswordRequest &request);
    void fileConflict(FileConflictRequest &request);
    void warningOccurred(const WarningInfo &warning);
    void finished(const FinishInfo &finish);

private:
    double m_lastProgress = 0.0;
    QString m_lastCurrentFile;
    bool m_passwordRequested = false;
    bool m_fileConflict = false;
    bool m_finished = false;
    FinishInfo m_finishInfo;
    QList<WarningInfo> m_warnings;

    PasswordHandler m_passwordHandler;
    ConflictHandler m_conflictHandler;
};

} // namespace archive

#endif // LEGACY_BACKEND_OBSERVER_ADAPTER_H
