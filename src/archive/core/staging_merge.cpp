// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "staging_merge.h"

#include "path_guard.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <unistd.h>

namespace dcarchive {
namespace {

bool copyFile(const QString &sourcePath, const QString &targetPath, ArchiveResult &result)
{
    QFile source(sourcePath);
    if (!source.open(QIODevice::ReadOnly)) {
        result.error = ArchiveError::NotFound;
        result.message = sourcePath;
        return false;
    }
    QSaveFile target(targetPath);
    if (!target.open(QIODevice::WriteOnly)) {
        result.error = target.error() == QFileDevice::ResourceError
                ? ArchiveError::DiskFull : ArchiveError::PermissionDenied;
        result.message = targetPath;
        return false;
    }
    while (!source.atEnd()) {
        const QByteArray chunk = source.read(64 * 1024);
        if (chunk.isEmpty() && source.error() != QFileDevice::NoError) {
            result.error = ArchiveError::NotFound;
            result.message = sourcePath;
            target.cancelWriting();
            return false;
        }
        if (target.write(chunk) != chunk.size()) {
            result.error = target.error() == QFileDevice::ResourceError
                    ? ArchiveError::DiskFull : ArchiveError::PermissionDenied;
            result.message = targetPath;
            target.cancelWriting();
            return false;
        }
    }
    const QFileInfo sourceInfo(sourcePath);
    if (!target.commit()) {
        result.error = target.error() == QFileDevice::ResourceError
                ? ArchiveError::DiskFull : ArchiveError::PermissionDenied;
        result.message = targetPath;
        return false;
    }
    QFile::setPermissions(targetPath, sourceInfo.permissions());
    return true;
}

bool mergeSymlink(const QString &sourcePath, const QString &targetPath,
                  const QString &targetRoot, ArchiveResult &result)
{
    const QString linkTarget = QFile::symLinkTarget(sourcePath);
    if (!symlinkTargetWithinRoot(targetRoot, targetPath, linkTarget)) {
        result.error = ArchiveError::PermissionDenied;
        result.message = targetPath;
        return false;
    }
    QFile::remove(targetPath);
    if (::symlink(QFile::encodeName(linkTarget).constData(),
                  QFile::encodeName(targetPath).constData()) != 0) {
        result.error = ArchiveError::PermissionDenied;
        result.message = targetPath;
        return false;
    }
    return true;
}

bool copyLocalEntry(const QString &sourcePath, const QString &targetPath, ArchiveResult &result)
{
    const QFileInfo sourceInfo(sourcePath);
    if (sourceInfo.isSymLink()) {
        const QString linkTarget = QFile::symLinkTarget(sourcePath);
        if (linkTarget.isEmpty() || ::symlink(QFile::encodeName(linkTarget).constData(),
                                              QFile::encodeName(targetPath).constData()) != 0) {
            result.error = ArchiveError::PermissionDenied;
            result.message = targetPath;
            return false;
        }
        return true;
    }
    if (sourceInfo.isDir()) {
        if (!QDir().mkpath(targetPath)) {
            result.error = ArchiveError::PermissionDenied;
            result.message = targetPath;
            return false;
        }
        QDirIterator it(sourcePath,
                        QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                        QDirIterator::NoIteratorFlags);
        const QDir sourceRoot(sourcePath);
        while (it.hasNext()) {
            const QString childSource = it.next();
            const QString childTarget = QDir(targetPath).filePath(
                    sourceRoot.relativeFilePath(childSource));
            if (!copyLocalEntry(childSource, childTarget, result))
                return false;
        }
        return true;
    }
    if (!QDir().mkpath(QFileInfo(targetPath).path()) || !QFile::copy(sourcePath, targetPath)) {
        result.error = ArchiveError::PermissionDenied;
        result.message = targetPath;
        return false;
    }
    return true;
}

} // namespace

bool mergeStagedEntries(const QString &stagingRoot, const QString &targetRoot,
                        const QVector<ArchiveEntry> &entries,
                        const QString &destinationPrefix, ArchiveResult &result)
{
    for (const ArchiveEntry &entry : entries) {
        const QString relativePath = archivePathRelativeTo(entry.path, destinationPrefix);
        const QString sourcePath = safeOutputPath(stagingRoot, entry.path);
        const QString targetPath = safeOutputPath(targetRoot, relativePath);
        if (sourcePath.isEmpty() || targetPath.isEmpty()) {
            result.error = ArchiveError::PermissionDenied;
            result.message = entry.path;
            return false;
        }
        if (entry.type == EntryType::Directory) {
            if (!ensureDirectoryWithinRoot(targetRoot, targetPath)) {
                result.error = ArchiveError::PermissionDenied;
                result.message = entry.path;
                return false;
            }
            continue;
        }
        if (!ensureParentWithinRoot(targetRoot, targetPath)) {
            result.error = ArchiveError::PermissionDenied;
            result.message = entry.path;
            return false;
        }
        const QFileInfo sourceInfo(sourcePath);
        if (sourceInfo.isSymLink()) {
            if (!mergeSymlink(sourcePath, targetPath, targetRoot, result))
                return false;
        } else if (!copyFile(sourcePath, targetPath, result)) {
            return false;
        }
    }
    return true;
}

bool stageLocalEntries(const QVector<ArchiveEntry> &entries, const QString &destinationPath,
                       const QString &stagingRoot, QStringList *stagedPaths,
                       ArchiveResult &result)
{
    const QString destinationRoot = QDir(stagingRoot).filePath(
            normalizedArchivePath(destinationPath));
    if (!QDir().mkpath(destinationRoot)) {
        result.error = ArchiveError::PermissionDenied;
        result.message = destinationRoot;
        return false;
    }
    for (const ArchiveEntry &entry : entries) {
        const QString name = entry.name.isEmpty()
                ? QFileInfo(entry.path).fileName() : entry.name;
        if (name.isEmpty()) {
            result.error = ArchiveError::NotFound;
            result.message = entry.path;
            return false;
        }
        const QString stagedPath = QDir(destinationRoot).filePath(name);
        if (!copyLocalEntry(entry.path, stagedPath, result))
            return false;
        if (stagedPaths)
            stagedPaths->append(name);
    }
    return true;
}

} // namespace dcarchive
