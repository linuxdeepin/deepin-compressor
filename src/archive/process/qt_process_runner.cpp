// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qt_process_runner.h"

#include <QTimer>
#include <QDebug>
#include <QCoreApplication>

#ifdef Q_OS_UNIX
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace archive {

QtProcessRunner::QtProcessRunner(QObject *parent)
    : ProcessRunner(parent)
{
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &QtProcessRunner::onFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &QtProcessRunner::onErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &QtProcessRunner::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &QtProcessRunner::onReadyReadStandardError);
}

QtProcessRunner::~QtProcessRunner()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        cancel();
        m_process->waitForFinished(5000);
    }
}

bool QtProcessRunner::start(const QString &program,
                            const QStringList &args,
                            const QString &workDir)
{
    m_cancelled = false;
    m_exitCode = -1;
    m_stdoutBuffer.clear();
    m_stderrBuffer.clear();

    if (!workDir.isEmpty()) {
        m_process->setWorkingDirectory(workDir);
    }

    m_process->start(program, args);

    if (!m_process->waitForStarted(10000)) {
        emit errorOccurred(m_process->errorString());
        return false;
    }

    return true;
}

bool QtProcessRunner::waitForFinished(int timeoutMs)
{
    if (!m_process || m_process->state() == QProcess::NotRunning) {
        return true;
    }

    int effective = (timeoutMs < 0) ? m_timeoutMs : timeoutMs;
    return m_process->waitForFinished(effective);
}

QByteArray QtProcessRunner::readAllStandardOutput()
{
    m_stdoutBuffer += m_process->readAllStandardOutput();
    return m_stdoutBuffer;
}

QByteArray QtProcessRunner::readAllStandardError()
{
    m_stderrBuffer += m_process->readAllStandardError();
    return m_stderrBuffer;
}

int QtProcessRunner::exitCode() const
{
    if (m_process->state() != QProcess::NotRunning) {
        return -1;
    }
    return m_exitCode;
}

bool QtProcessRunner::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

void QtProcessRunner::cancel()
{
    if (!m_process || m_process->state() == QProcess::NotRunning) {
        return;
    }

    m_cancelled = true;

    // Use only QProcess API for process management — no manual waitpid,
    // which would conflict with QProcess's own child-reaping.
    m_process->terminate();
    if (!m_process->waitForFinished(5000)) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

void QtProcessRunner::pause()
{
    m_paused = true;
#ifdef Q_OS_UNIX
    qint64 pid = m_process->processId();
    if (pid > 0) {
        ::kill(static_cast<pid_t>(pid), SIGSTOP);
    }
#endif
}

void QtProcessRunner::resume()
{
    m_paused = false;
#ifdef Q_OS_UNIX
    qint64 pid = m_process->processId();
    if (pid > 0) {
        ::kill(static_cast<pid_t>(pid), SIGCONT);
    }
#endif
}

void QtProcessRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    m_exitCode = exitCode;
    emit finished(exitCode, m_cancelled);
}

void QtProcessRunner::onErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    emit errorOccurred(m_process->errorString());
}

void QtProcessRunner::onReadyReadStandardOutput()
{
    QByteArray data = m_process->readAllStandardOutput();
    m_stdoutBuffer += data;
    if (m_outputCb) {
        m_outputCb(data);
    }
    emit outputReady(data);
}

void QtProcessRunner::onReadyReadStandardError()
{
    QByteArray data = m_process->readAllStandardError();
    m_stderrBuffer += data;
    if (m_outputCb) {
        m_outputCb(data);
    }
    emit outputReady(data);
}

} // namespace archive
