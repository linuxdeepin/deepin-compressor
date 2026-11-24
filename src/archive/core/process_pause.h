#ifndef ARCHIVE_PROCESS_PAUSE_H
#define ARCHIVE_PROCESS_PAUSE_H

#include <QProcess>

#include <csignal>
#include <sys/types.h>

namespace dcarchive {

inline void syncProcessPause(const QProcess &process, bool requested, bool &paused)
{
    if (requested == paused || process.state() != QProcess::Running)
        return;

    const qint64 pid = process.processId();
    if (pid > 0 && ::kill(static_cast<pid_t>(pid), requested ? SIGSTOP : SIGCONT) == 0)
        paused = requested;
}

} // namespace dcarchive

#endif
