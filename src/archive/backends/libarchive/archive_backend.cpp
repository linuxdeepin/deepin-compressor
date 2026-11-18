// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archive_backend.h"

#include "datamanager.h"
#include "queries.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThread>

#include <archive_entry.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

constexpr int kBlockSize = 10240;

// libarchive 给出的路径形如 "./a.txt"、"sub/"，统一成界面用的形式
QString normalizePath(const char *raw)
{
    QString path = QString::fromUtf8(raw ? raw : "");
    while (path.startsWith(QLatin1String("./")))
        path.remove(0, 2);
    path.remove(QRegularExpression(QStringLiteral("/{2,}")));
    return path;
}

bool isUnsafePath(const QString &path)
{
    if (path.startsWith(QLatin1Char('/')) || path.contains(QLatin1String("../")))
        return true;

    const QStringList parts = path.split(QLatin1Char('/'));
    return parts.contains(QLatin1String(".."));
}

void setMtime(const QString &path, qint64 seconds, bool isLink)
{
    struct timespec times[2];
    times[0].tv_sec = seconds;
    times[0].tv_nsec = 0;
    times[1].tv_sec = seconds;
    times[1].tv_nsec = 0;
    utimensat(AT_FDCWD, QFile::encodeName(path).constData(), times,
              isLink ? AT_SYMLINK_NOFOLLOW : 0);
}

} // namespace

ArchiveBackend::ArchiveBackend(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
}

ArchiveBackend::~ArchiveBackend()
{
    closeArchive();
}

bool ArchiveBackend::openArchive()
{
    closeArchive();

    m_reader = archive_read_new();
    if (!m_reader)
        return false;

    archive_read_support_filter_all(m_reader);
    archive_read_support_format_all(m_reader);

    const QByteArray path = QFile::encodeName(m_strArchiveName);
    if (archive_read_open_filename(m_reader, path.constData(), kBlockSize) != ARCHIVE_OK) {
        closeArchive();
        m_eErrorType = ET_ArchiveDamaged;
        return false;
    }

    return true;
}

void ArchiveBackend::closeArchive()
{
    if (m_reader) {
        archive_read_close(m_reader);
        archive_read_free(m_reader);
        m_reader = nullptr;
    }
}

bool ArchiveBackend::nextEntry(Entry &entry)
{
    struct archive_entry *aentry = nullptr;
    const int rc = archive_read_next_header(m_reader, &aentry);
    if (rc != ARCHIVE_OK && rc != ARCHIVE_WARN)
        return false;

    QString path = normalizePath(archive_entry_pathname(aentry));
    const mode_t type = archive_entry_filetype(aentry);

    entry = Entry();
    entry.file.isDirectory = (type == AE_IFDIR);
    if (entry.file.isDirectory && !path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');

    entry.file.strFullPath = path;
    entry.file.strFileName = QFileInfo(path.endsWith(QLatin1Char('/')) ? path.chopped(1) : path).fileName();
    entry.file.qSize = entry.file.isDirectory ? 0 : archive_entry_size(aentry);
    entry.file.uLastModifiedTime = static_cast<uint>(archive_entry_mtime(aentry));

    entry.mode = archive_entry_perm(aentry);
    entry.isLink = (type == AE_IFLNK);
    if (entry.isLink)
        entry.linkTarget = QString::fromUtf8(archive_entry_symlink(aentry));

    return true;
}

qint64 ArchiveBackend::uncompressedStreamSize()
{
    struct archive *reader = archive_read_new();
    if (!reader)
        return -1;

    archive_read_support_filter_all(reader);
    archive_read_support_format_raw(reader);

    if (archive_read_open_filename(reader, QFile::encodeName(m_strArchiveName).constData(), kBlockSize) != ARCHIVE_OK) {
        archive_read_free(reader);
        return -1;
    }

    qint64 total = 0;
    struct archive_entry *entry = nullptr;
    if (archive_read_next_header(reader, &entry) == ARCHIVE_OK) {
        QByteArray buffer(kBlockSize * 6, '\0');
        la_ssize_t read = 0;
        while ((read = archive_read_data(reader, buffer.data(), buffer.size())) > 0)
            total += read;
    }

    archive_read_close(reader);
    archive_read_free(reader);
    return total;
}

PluginFinishType ArchiveBackend::list()
{
    DataManager::get_instance().resetArchiveData();
    ArchiveData &data = DataManager::get_instance().archiveData();

    if (!openArchive())
        return PFT_Error;

    data.qComressSize = QFileInfo(m_strArchiveName).size();

    // 旧实现先把 tar.bz2 / tar.lzma / tar.Z 用 7z 解出来再 list，
    // 因此这三个后缀的“压缩大小”报的是解压后 tar 的大小，这里保持一致。
    const QString lowerName = m_strArchiveName.toLower();
    if (lowerName.endsWith(QLatin1String(".tar.bz2")) || lowerName.endsWith(QLatin1String(".tar.lzma"))
            || lowerName.endsWith(QLatin1String(".tar.z"))) {
        const qint64 streamSize = uncompressedStreamSize();
        if (streamSize > 0)
            data.qComressSize = streamSize;
    }

    QSet<QString> seenRoots;
    qint64 totalSize = 0;

    Entry entry;
    while (nextEntry(entry)) {
        data.mapFileEntry.insert(entry.file.strFullPath, entry.file);

        const QString keyPath = entry.file.strFullPath.chopped(entry.file.isDirectory ? 1 : 0);
        const QString rootKey = entry.file.strFullPath.section(QLatin1Char('/'), 0, 0)
                + (entry.file.isDirectory ? QStringLiteral("/") : QString());
        if (!keyPath.contains(QLatin1Char('/')) && !seenRoots.contains(rootKey)) {
            seenRoots.insert(rootKey);
            data.listRootEntry << entry.file;
        }

        totalSize += entry.file.qSize;
        archive_read_data_skip(m_reader);
    }

    data.qSize = totalSize;
    closeArchive();
    return PFT_Nomral;
}

PluginFinishType ArchiveBackend::testArchive()
{
    m_workStatus = WT_Test;

    if (!openArchive())
        return PFT_Error;

    QByteArray buffer(kBlockSize, '\0');
    Entry entry;
    while (nextEntry(entry)) {
        while (archive_read_data(m_reader, buffer.data(), buffer.size()) > 0) { }
    }

    closeArchive();
    return PFT_Nomral;
}

PluginFinishType ArchiveBackend::extractFiles(const QList<FileEntry> &files,
                                              const ExtractionOptions &options)
{
    if (!openArchive())
        return PFT_Error;

    if (!options.bExistList) {
        FileEntry root;
        root.strFullPath = QFileInfo(m_strArchiveName).fileName();
        root.isDirectory = true;
        DataManager::get_instance().archiveData().listRootEntry << root;
    }

    QSet<QString> selected;
    for (const FileEntry &file : files)
        selected.insert(file.strFullPath);

    const QString basePath = options.strTargetPath.endsWith(QLatin1Char('/'))
            ? options.strTargetPath
            : options.strTargetPath + QLatin1Char('/');
    if (!QDir().exists(basePath))
        QDir().mkpath(basePath);

    qint64 extracted = 0;
    QList<QPair<QString, qint64>> dirTimes;
    bool userCancelled = false;

    Entry entry;
    while (nextEntry(entry)) {
        if (!options.bAllExtract && !selected.contains(entry.file.strFullPath)) {
            archive_read_data_skip(m_reader);
            continue;
        }
        if (isUnsafePath(entry.file.strFullPath)) {
            archive_read_data_skip(m_reader);
            continue;
        }
        if (QThread::currentThread()->isInterruptionRequested())
            break;

        while (m_bPause)
            QThread::msleep(100);

        const QString target = basePath + entry.file.strFullPath;

        emit signalCurFileName(entry.file.strFullPath);

        if (entry.file.isDirectory) {
            if (!QDir().exists(target))
                QDir().mkpath(target);
            dirTimes << qMakePair(target, static_cast<qint64>(entry.file.uLastModifiedTime));
            archive_read_data_skip(m_reader);
            continue;
        }

        const QFileInfo targetInfo(target);
        if (!QDir().exists(targetInfo.absolutePath()))
            QDir().mkpath(targetInfo.absolutePath());

        if (targetInfo.exists()) {
            OverwriteQuery query(target);
            emit signalQuery(&query);
            query.waitForResponse();

            if (query.responseCancelled()) {
                emit signalCancel();
                userCancelled = true;
                break;
            }
            if (query.responseSkip() || query.responseSkipAll()) {
                archive_read_data_skip(m_reader);
                continue;
            }
            if (query.responseOverwriteAll())
                m_bOverwriteAll = true;
            QFile::remove(target);
        }

        if (entry.isLink) {
            if (!QFile::link(entry.linkTarget, target)) {
                emit signalFileWriteErrorName(targetInfo.fileName());
                m_eErrorType = ET_FileWriteError;
                archive_read_data_skip(m_reader);
                continue;
            }
            setMtime(target, static_cast<qint64>(entry.file.uLastModifiedTime), true);
            continue;
        }

        QFile out(target);
        if (!out.open(QIODevice::WriteOnly)) {
            emit signalFileWriteErrorName(targetInfo.fileName());
            m_eErrorType = ET_FileWriteError;
            archive_read_data_skip(m_reader);
            continue;
        }

        QByteArray buffer(kBlockSize, '\0');
        la_ssize_t read = 0;
        while ((read = archive_read_data(m_reader, buffer.data(), buffer.size())) > 0) {
            if (out.write(buffer.constData(), read) != read) {
                m_eErrorType = ET_InsufficientDiskSpace;
                break;
            }
            extracted += read;
            if (options.qSize > 0) {
                const qint64 percent = extracted * 100 / options.qSize;
                if (percent != m_lastPercent) {
                    m_lastPercent = percent;
                    emit signalprogress(static_cast<double>(percent));
                }
            }
        }
        out.close();

        if (read < 0 && m_eErrorType == ET_NoError) {
            m_eErrorType = ET_ArchiveDamaged;
            break;
        }

        chmod(QFile::encodeName(target).constData(), entry.mode & 07777);
        setMtime(target, static_cast<qint64>(entry.file.uLastModifiedTime), false);
    }

    for (const auto &item : dirTimes)
        setMtime(item.first, item.second, false);

    closeArchive();

    if (userCancelled)
        return PFT_Cancel;

    return m_eErrorType == ET_NoError ? PFT_Nomral : PFT_Error;
}

void ArchiveBackend::pauseOperation()
{
    m_bPause = true;
}

void ArchiveBackend::continueOperation()
{
    m_bPause = false;
}

bool ArchiveBackend::doKill()
{
    m_bPause = false;
    m_bCancel = true;
    return true;
}
