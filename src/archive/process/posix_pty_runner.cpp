// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "posix_pty_runner.h"

#include <QDebug>
#include <QCoreApplication>

#ifdef Q_OS_UNIX
#include <pty.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#endif

namespace archive {

PosixPtyRunner::PosixPtyRunner(QObject *parent)
    : PtyRunner(parent)
{
}

PosixPtyRunner::~PosixPtyRunner()
{
    cleanup();
}

bool PosixPtyRunner::start(const QString &program,
                           const QStringList &args,
                           const QString &workDir)
{
#ifdef Q_OS_UNIX
    m_cancelled = false;
    m_exitCode = -1;
    m_outputBuffer.clear();

    int masterFd = -1;
    pid_t pid = forkpty(&masterFd, nullptr, nullptr, nullptr);
    if (pid < 0) {
        qWarning() << "forkpty failed:" << strerror(errno);
        emit errorOccurred(QStringLiteral("forkpty failed: %1").arg(strerror(errno)));
        return false;
    }

    if (pid == 0) {
        // Child process
        if (!workDir.isEmpty()) {
            QByteArray wd = workDir.toUtf8();
            if (chdir(wd.constData()) != 0) {
                // Non-fatal: continue in current directory
            }
        }

        // Create independent process group so we can kill the entire
        // process tree (CLI backends may spawn child processes).
        setsid();

        // Build argv array
        QByteArray progBytes = program.toUtf8();
        QList<QByteArray> argBytes;
        argBytes << progBytes;
        for (const QString &a : args) {
            argBytes << a.toUtf8();
        }

        // Use heap-allocated char* array since execvp needs char* const[]
        char **argv = new char *[argBytes.size() + 1];
        for (int i = 0; i < argBytes.size(); ++i) {
            argv[i] = const_cast<char *>(argBytes[i].constData());
        }
        argv[argBytes.size()] = nullptr;

        execvp(progBytes.constData(), argv);
        // If execvp returns, it failed
        qFatal("execvp failed: %s", strerror(errno));
    }

    // Parent process
    m_masterFd = masterFd;
    m_childPid = pid;
    m_running = true;

    // Set non-blocking read
    int flags = fcntl(m_masterFd, F_GETFL, 0);
    fcntl(m_masterFd, F_SETFL, flags | O_NONBLOCK);

    return true;
#else
    Q_UNUSED(program)
    Q_UNUSED(args)
    Q_UNUSED(workDir)
    emit errorOccurred(QStringLiteral("PTY not supported on this platform"));
    return false;
#endif
}

bool PosixPtyRunner::waitForFinished(int timeoutMs)
{
#ifdef Q_OS_UNIX
    if (!m_running || m_childPid <= 0) {
        return true;
    }

    int effective = (timeoutMs < 0) ? m_timeoutMs : timeoutMs;

    int elapsed = 0;
    const int interval = 100; // 100ms

    while (m_running && elapsed < effective) {
        // Try to read output
        char buf[4096];
        ssize_t n;
        while ((n = read(m_masterFd, buf, sizeof(buf))) > 0) {
            QByteArray chunk(buf, n);
            m_outputBuffer.append(chunk);
            if (m_outputCb) {
                m_outputCb(chunk);
            }
            emit outputReady(chunk);

            // Detect password prompts in the PTY output and auto-respond
            // via the registered callback.
            if (m_passwordCb && isPasswordPrompt(chunk)) {
                QByteArray password = m_passwordCb();
                writeInput(password);
            }
        }

        // Check if child exited
        int status = 0;
        pid_t ret = waitpid(static_cast<pid_t>(m_childPid), &status, WNOHANG);
        if (ret == static_cast<pid_t>(m_childPid)) {
            if (WIFEXITED(status)) {
                m_exitCode = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                m_exitCode = -1;
            }
            m_running = false;
            emit finished(m_exitCode, m_cancelled);
            break;
        }

        QCoreApplication::processEvents();
        usleep(interval * 1000);
        elapsed += interval;
    }

    return !m_running;
#else
    Q_UNUSED(timeoutMs)
    return true;
#endif
}

QByteArray PosixPtyRunner::readAllStandardOutput()
{
#ifdef Q_OS_UNIX
    if (m_masterFd >= 0) {
        char buf[4096];
        ssize_t n;
        while ((n = read(m_masterFd, buf, sizeof(buf))) > 0) {
            m_outputBuffer.append(buf, n);
        }
    }
#endif
    QByteArray result = m_outputBuffer;
    m_outputBuffer.clear();
    return result;
}

QByteArray PosixPtyRunner::readAllStandardError()
{
    // PTY merges stdout and stderr
    return readAllStandardOutput();
}

void PosixPtyRunner::cancel()
{
#ifdef Q_OS_UNIX
    if (!m_running || m_childPid <= 0) {
        return;
    }

    m_cancelled = true;

    // Send SIGTERM to the entire process group (negative PID).
    // The child called setsid() in start(), so it is its own group leader.
    ::kill(-static_cast<pid_t>(m_childPid), SIGTERM);

    // Wait up to 5 seconds
    for (int i = 0; i < 50; ++i) {
        int status = 0;
        if (waitpid(static_cast<pid_t>(m_childPid), &status, WNOHANG) != 0) {
            m_running = false;
            if (WIFEXITED(status)) {
                m_exitCode = WEXITSTATUS(status);
            }
            emit finished(m_exitCode, m_cancelled);
            return;
        }
        usleep(100000); // 100ms
    }

    // SIGKILL the entire process group
    ::kill(-static_cast<pid_t>(m_childPid), SIGKILL);
    int status = 0;
    waitpid(static_cast<pid_t>(m_childPid), &status, 0);
    m_running = false;
    m_exitCode = -1;
    emit finished(m_exitCode, m_cancelled);
#endif
}

void PosixPtyRunner::pause()
{
#ifdef Q_OS_UNIX
    if (m_childPid > 0) {
        // Send SIGSTOP to the entire process group (negative PID),
        // matching cancel() behaviour.  The child called setsid() in
        // start(), so it is its own group leader and descendant
        // processes are also stopped.
        ::kill(-static_cast<pid_t>(m_childPid), SIGSTOP);
    }
#endif
}

void PosixPtyRunner::resume()
{
#ifdef Q_OS_UNIX
    if (m_childPid > 0) {
        // Resume the entire process group (negative PID), matching
        // the pause() and cancel() behaviour.
        ::kill(-static_cast<pid_t>(m_childPid), SIGCONT);
    }
#endif
}

void PosixPtyRunner::writeInput(const QByteArray &data)
{
#ifdef Q_OS_UNIX
    if (m_masterFd >= 0) {
        ::write(m_masterFd, data.constData(), data.size());
    }
#endif
}

QByteArray PosixPtyRunner::readOutput()
{
    return readAllStandardOutput();
}

bool PosixPtyRunner::isPasswordPrompt(const QByteArray &line) const
{
    // Common password prompt patterns from 7z, rar, unzip, etc.
    QString str = QString::fromUtf8(line);
    if (str.contains("password", Qt::CaseInsensitive) ||
        str.contains("passphrase", Qt::CaseInsensitive)) {
        return true;
    }
    // 7z uses "Enter password" or "Verify password"
    if (str.contains("Enter password") || str.contains("Verify password")) {
        return true;
    }
    // rar uses "Enter password (will not be echoed)"
    if (str.contains("Enter password")) {
        return true;
    }
    return false;
}

void PosixPtyRunner::cleanup()
{
    if (m_running && m_childPid > 0) {
        cancel();
    }
    closeFds();
}

void PosixPtyRunner::closeFds()
{
#ifdef Q_OS_UNIX
    if (m_masterFd >= 0) {
        ::close(m_masterFd);
        m_masterFd = -1;
    }
#endif
}

} // namespace archive
