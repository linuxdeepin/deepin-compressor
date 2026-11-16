// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POSIX_PTY_RUNNER_H
#define POSIX_PTY_RUNNER_H

#include "pty_runner.h"

#include <QByteArray>

namespace archive {

class PosixPtyRunner : public PtyRunner
{
    Q_OBJECT

public:
    explicit PosixPtyRunner(QObject *parent = nullptr);
    ~PosixPtyRunner() override;

    bool start(const QString &program,
               const QStringList &args,
               const QString &workDir = QString()) override;

    bool waitForFinished(int timeoutMs = -1) override;
    QByteArray readAllStandardOutput() override;
    QByteArray readAllStandardError() override;
    int exitCode() const override { return m_exitCode; }
    bool isRunning() const override { return m_running; }

    void setTimeout(int timeoutMs) override { m_timeoutMs = timeoutMs; }
    int timeout() const override { return m_timeoutMs; }

    void cancel() override;
    void pause() override;
    void resume() override;

    void writeInput(const QByteArray &data) override;
    QByteArray readOutput() override;
    bool isPasswordPrompt(const QByteArray &line) const override;

    int masterFd() const { return m_masterFd; }

private:
    void cleanup();
    void closeFds();

    int m_masterFd = -1;
    qint64 m_childPid = -1;
    int m_exitCode = -1;
    bool m_running = false;
    bool m_cancelled = false;
    int m_timeoutMs = 30000;
    QByteArray m_outputBuffer;
};

} // namespace archive

#endif // POSIX_PTY_RUNNER_H
