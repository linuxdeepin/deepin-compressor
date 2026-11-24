// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pigz_backend.h"
#include "path_guard.h"
#include "process_pause.h"
#include "write_progress.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStringList>
#include <QTemporaryDir>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <limits>

namespace dcarchive {
namespace {

bool succeeded(const QProcess &process)
{
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

} // namespace

PigzBackend::PigzBackend(const QString &path, const QString &mimeType)
    : m_path(path)
    , m_mimeType(mimeType)
{
}

PigzBackend::~PigzBackend() = default;

BackendCapabilities PigzBackend::capabilities() const
{
    BackendCapabilities caps;
    caps.create = true;
    caps.pause = true;
    caps.progress = true;
    caps.cancel = true;
    return caps;
}

VoidResult PigzBackend::open(const OpenRequest &request)
{
    if (!request.path.isEmpty())
        m_path = request.path;
    if (!request.mimeType.isEmpty())
        m_mimeType = request.mimeType;
    m_cancelled = false;
    m_paused = false;
    return {};
}

EntryResult PigzBackend::list()
{
    // 与迁移前行为保持一致：pigz 只负责创建，不提供列表。
    return {};
}

VoidResult PigzBackend::testArchive()
{
    return {};
}

VoidResult PigzBackend::extractFiles(const ExtractRequest &, BackendObserver &)
{
    return {};
}

VoidResult PigzBackend::addFiles(const AddRequest &request, BackendObserver &observer)
{
    ArchiveResult result;
    if (request.entries.isEmpty()) {
        result.error = ArchiveError::NotFound;
        notifyFinished(observer, result);
        return {result};
    }

    const QFileInfo destination(m_path);
    QTemporaryDir staging(QDir(destination.absolutePath()).filePath(QStringLiteral(".dc-pigz-XXXXXX")));
    if (!staging.isValid()) {
        result.error = ArchiveError::PermissionDenied;
        result.message = staging.errorString();
        notifyFinished(observer, result);
        return {result};
    }
    const QString temporaryArchive = staging.filePath(QStringLiteral("archive.tar.gz"));

    QStringList tarArgs{QStringLiteral("-cf"), QStringLiteral("-"),
                        QStringLiteral("--anchored"), QStringLiteral("--no-wildcards")};
    tarArgs << QStringLiteral("--blocking-factor=20")
            << QStringLiteral("--checkpoint=64")
            << QStringLiteral("--checkpoint-action=echo=DC_TAR %u");
    QStringList sources;
    for (const ArchiveEntry &entry : request.entries)
        sources << entry.path;
    QStringList excluded = request.excludedSourcePaths;
    excluded << staging.path() << m_path;
    for (const QString &path : relativeSourceExclusions(sources, excluded))
        tarArgs << QStringLiteral("--exclude=") + path;
    for (const ArchiveEntry &entry : request.entries) {
        const QFileInfo info(entry.path);
        const QString name = info.fileName();
        tarArgs << QStringLiteral("-C") << info.absolutePath()
                << (name.startsWith(QLatin1Char('-')) ? QStringLiteral("./") + name : name);
    }

    WriteProgress progress(observer, compressionSourceBytes(request, excluded, m_cancelled));

    const int threads = request.cpuThreads > 0 ? request.cpuThreads : 1;
    const int level = request.compressionLevel >= 0 ? request.compressionLevel : 6;
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    QProcess tar;
    QProcess pigz;
    tar.setProcessEnvironment(environment);
    pigz.setProcessEnvironment(environment);
    tar.setStandardOutputProcess(&pigz);
    pigz.setStandardOutputFile(temporaryArchive);

    pigz.start(QStringLiteral("pigz"), {QStringLiteral("-p"), QString::number(threads),
                                       QStringLiteral("-%1").arg(level)});
    if (!pigz.waitForStarted()) {
        result.error = ArchiveError::ToolMissing;
        result.message = pigz.errorString();
        notifyFinished(observer, result);
        return {result};
    }
    tar.start(QStringLiteral("tar"), tarArgs);
    if (!tar.waitForStarted()) {
        pigz.kill();
        pigz.waitForFinished();
        result.error = ArchiveError::ToolMissing;
        result.message = tar.errorString();
        notifyFinished(observer, result);
        return {result};
    }
    QByteArray tarErrors;
    QByteArray pending;
    const QByteArray checkpointPrefix(": DC_TAR ");
    const auto readCheckpoints = [&]() {
        pending += tar.readAllStandardError();
        qsizetype start = 0;
        for (qsizetype end = pending.indexOf('\n'); end >= 0; end = pending.indexOf('\n', start)) {
            const QByteArray line = pending.mid(start, end - start);
            start = end + 1;
            const qsizetype marker = line.indexOf(checkpointPrefix);
            if (marker >= 0) {
                bool valid = false;
                const qint64 record = line.mid(marker + checkpointPrefix.size()).toLongLong(&valid);
                if (valid && record > 1 && record <= std::numeric_limits<qint64>::max() / (20 * 512))
                    progress.setBytes((record - 1) * (20 * 512));
            } else if (tarErrors.size() < 8192) {
                tarErrors += line + '\n';
            }
        }
        pending.remove(0, start);
    };
    bool tarPaused = false;
    bool pigzPaused = false;
    while (tar.state() != QProcess::NotRunning || pigz.state() != QProcess::NotRunning) {
        if (m_cancelled) {
            tar.kill();
            pigz.kill();
        } else {
            const bool requested = m_paused;
            syncProcessPause(tar, requested, tarPaused);
            syncProcessPause(pigz, requested, pigzPaused);
        }
        if (tar.state() != QProcess::NotRunning)
            tar.waitForFinished(50);
        if (pigz.state() != QProcess::NotRunning)
            pigz.waitForFinished(50);
        readCheckpoints();
    }
    readCheckpoints();
    if (m_cancelled) {
        result.error = ArchiveError::Cancelled;
        result.cancelled = true;
    } else if (!succeeded(tar) || !succeeded(pigz)) {
        result.message = QString::fromLocal8Bit(tarErrors + pending + pigz.readAllStandardError()).trimmed();
        result.error = result.message.contains(QLatin1String("No space left on device"))
                ? ArchiveError::DiskFull : ArchiveError::Unknown;
    } else if (destination.exists() && !QFile::setPermissions(temporaryArchive, destination.permissions())) {
        result.error = ArchiveError::PermissionDenied;
        result.message = m_path;
    } else if (::rename(QFile::encodeName(temporaryArchive).constData(),
                        QFile::encodeName(destination.absoluteFilePath()).constData()) != 0) {
        const int error = errno;
        result.error = error == ENOSPC ? ArchiveError::DiskFull : ArchiveError::PermissionDenied;
        result.message = QString::fromLocal8Bit(std::strerror(error));
    }

    notifyFinished(observer, result);
    return {result};
}

VoidResult PigzBackend::moveFiles(const MoveRequest &, BackendObserver &)
{
    return {};
}

VoidResult PigzBackend::copyFiles(const CopyRequest &, BackendObserver &)
{
    return {};
}

VoidResult PigzBackend::deleteFiles(const DeleteRequest &)
{
    return {};
}

VoidResult PigzBackend::renameFiles(const RenameRequest &)
{
    return {};
}

VoidResult PigzBackend::addComment(const QString &)
{
    return {};
}

VoidResult PigzBackend::updateArchiveData(const UpdateRequest &)
{
    return {};
}

void PigzBackend::pause()
{
    m_paused = true;
}

void PigzBackend::resume()
{
    m_paused = false;
}

void PigzBackend::cancel()
{
    m_cancelled = true;
}

void PigzBackend::notifyFinished(BackendObserver &observer, const ArchiveResult &result)
{
    FinishInfo info;
    info.result = result;
    observer.onFinished(info);
}

} // namespace dcarchive
