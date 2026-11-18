// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pigz_backend.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>

#include <csignal>

namespace {

QString quote(const QString &value)
{
    QString escaped = value;
    escaped.replace(QLatin1Char('\''), QLatin1String("'\\''"));
    return QLatin1Char('\'') + escaped + QLatin1Char('\'');
}

} // namespace

PigzBackend::PigzBackend(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
}

PigzBackend::~PigzBackend()
{
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

PluginFinishType PigzBackend::list()
{
    return PFT_Nomral;
}

PluginFinishType PigzBackend::testArchive()
{
    return PFT_Nomral;
}

PluginFinishType PigzBackend::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PFT_Nomral;
}

PluginFinishType PigzBackend::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    m_totalSize = options.qTotalSize;
    m_pending.clear();
    m_killed = false;
    m_eErrorType = ET_NoError;

    QStringList tarArgs;
    for (const FileEntry &file : files) {
        QString path = file.strFullPath;
        while (path.endsWith(QLatin1Char('/')))
            path.chop(1);

        const int pos = path.lastIndexOf(QLatin1Char('/'));
        if (pos > 0)
            tarArgs << QStringLiteral("-C") << quote(path.left(pos + 1)) << quote(path.mid(pos + 1));
        else
            tarArgs << quote(path);
    }

    if (tarArgs.isEmpty())
        return PFT_Error;

    const QString pipeline = QStringLiteral("tar cvf - %1 | pigz -p %2 -%3 > %4")
            .arg(tarArgs.join(QLatin1Char(' ')))
            .arg(options.iCPUTheadNum)
            .arg(options.iCompressionLevel)
            .arg(quote(m_strArchiveName));

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &PigzBackend::reportProgress);
    }

    connect(m_process, &QProcess::readyReadStandardOutput, this, &PigzBackend::readStdout);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) { finishProcess(exitCode); });

    m_process->start(QStringLiteral("/bin/bash"), {QStringLiteral("-c"), pipeline});
    m_process->waitForStarted();
    m_timer->start(1000);

    return PFT_Nomral;
}

void PigzBackend::readStdout()
{
    if (m_killed || !m_process)
        return;

    m_pending += m_process->readAllStandardOutput();
    QList<QByteArray> lines = m_pending.split('\n');
    m_pending = lines.takeLast();

    for (const QByteArray &line : lines) {
        const QString text = QString::fromLocal8Bit(line);
        if (text.contains(QLatin1String("No space left on device"))) {
            m_eErrorType = ET_InsufficientDiskSpace;
            m_process->kill();
            return;
        }
        if (text.isEmpty())
            continue;

        // tar 的详细输出就是文件列表：既是进度来源，也是界面上的“当前文件”
        reportProgress();
        emit signalCurFileName(text);
    }
}

void PigzBackend::reportProgress()
{
    if (m_totalSize <= 0)
        return;

    const qint64 size = QFileInfo(m_strArchiveName).size();
    emit signalprogress(double(size) / m_totalSize * 100);
}

void PigzBackend::finishProcess(int exitCode)
{
    if (m_timer)
        m_timer->stop();

    const bool ok = (exitCode == 0) && m_eErrorType == ET_NoError;
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }

    emit signalprogress(100);
    emit signalFinished(ok ? PFT_Nomral : PFT_Error);
}

PluginFinishType PigzBackend::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PFT_Nomral;
}

PluginFinishType PigzBackend::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PFT_Nomral;
}

PluginFinishType PigzBackend::deleteFiles(const QList<FileEntry> &files)
{
    Q_UNUSED(files)
    return PFT_Nomral;
}

PluginFinishType PigzBackend::renameFiles(const QList<FileEntry> &files)
{
    Q_UNUSED(files)
    return PFT_Nomral;
}

PluginFinishType PigzBackend::addComment(const QString &comment)
{
    Q_UNUSED(comment)
    return PFT_Nomral;
}

PluginFinishType PigzBackend::updateArchiveData(const UpdateOptions &options)
{
    Q_UNUSED(options)
    return PFT_Nomral;
}

QList<qint64> PigzBackend::childPids() const
{
    QList<qint64> result;
    if (!m_process)
        return result;

    QList<qint64> pending{m_process->processId()};
    while (!pending.isEmpty()) {
        const qint64 pid = pending.takeFirst();
        result << pid;

        QDir taskDir(QStringLiteral("/proc/%1/task").arg(pid));
        for (const QString &task : taskDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFile children(QStringLiteral("/proc/%1/task/%2/children").arg(pid).arg(task));
            if (!children.open(QIODevice::ReadOnly))
                continue;
            for (const QByteArray &child : children.readAll().split(' ')) {
                bool ok = false;
                const qint64 childPid = child.trimmed().toLongLong(&ok);
                if (ok && childPid > 0)
                    pending << childPid;
            }
        }
    }
    return result;
}

void PigzBackend::pauseOperation()
{
    const QList<qint64> pids = childPids();
    for (int i = pids.size() - 1; i >= 0; --i)
        ::kill(static_cast<pid_t>(pids.at(i)), SIGSTOP);
}

void PigzBackend::continueOperation()
{
    const QList<qint64> pids = childPids();
    for (int i = pids.size() - 1; i >= 0; --i)
        ::kill(static_cast<pid_t>(pids.at(i)), SIGCONT);
}

bool PigzBackend::doKill()
{
    m_killed = true;
    if (m_timer)
        m_timer->stop();

    const QList<qint64> pids = childPids();
    for (int i = pids.size() - 1; i >= 0; --i)
        ::kill(static_cast<pid_t>(pids.at(i)), SIGKILL);

    return true;
}
