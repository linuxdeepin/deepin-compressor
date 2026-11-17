// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QT_PROCESS_RUNNER_H
#define QT_PROCESS_RUNNER_H

#include "process_runner.h"

#include <QProcess>

namespace archive {

class QtProcessRunner : public ProcessRunner
{
    Q_OBJECT

public:
    explicit QtProcessRunner(QObject *parent = nullptr);
    ~QtProcessRunner() override;

    bool start(const QString &program,
               const QStringList &args,
               const QString &workDir = QString()) override;

    bool waitForFinished(int timeoutMs = -1) override;
    QByteArray readAllStandardOutput() override;
    QByteArray readAllStandardError() override;
    int exitCode() const override;
    bool isRunning() const override;

    void setTimeout(int timeoutMs) override { m_timeoutMs = timeoutMs; }
    int timeout() const override { return m_timeoutMs; }

    void cancel() override;
    void pause() override;
    void resume() override;

private slots:
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    QProcess *m_process = nullptr;
    QByteArray m_stdoutBuffer;
    QByteArray m_stderrBuffer;
    int m_timeoutMs = 30000;
    int m_exitCode = -1;
    bool m_cancelled = false;
    bool m_paused = false;
};

} // namespace archive

#endif // QT_PROCESS_RUNNER_H
