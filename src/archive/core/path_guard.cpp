// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "path_guard.h"

#include <QDir>
#include <QFileInfo>

namespace dcarchive {
namespace {

bool isWithin(const QString &canonicalRoot, const QString &canonicalPath)
{
    return canonicalPath == canonicalRoot
            || canonicalPath.startsWith(canonicalRoot + QLatin1Char('/'));
}

} // namespace

QString safeOutputPath(const QString &targetDir, const QString &entryPath)
{
    QString clean = QDir::cleanPath(entryPath);
    if (clean.startsWith(QLatin1Char('/')) || clean == QLatin1String("..")
            || clean.startsWith(QLatin1String("../")))
        return {};
    return QDir(targetDir).filePath(clean);
}

bool ensureDirectoryWithinRoot(const QString &root, const QString &directory)
{
    const QFileInfo rootInfo(root);
    if (!rootInfo.isDir())
        return false;

    const QDir rootDir(root);
    const QString relative = rootDir.relativeFilePath(directory);
    if (relative == QLatin1String("."))
        return true;
    if (relative.startsWith(QLatin1String("../")) || relative == QLatin1String(".."))
        return false;

    const QString canonicalRoot = rootInfo.canonicalFilePath();
    if (canonicalRoot.isEmpty())
        return false;

    QDir current(root);
    const QStringList parts = relative.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        const QString next = current.filePath(part);
        const QFileInfo info(next);
        if (info.isSymLink())
            return false;  // 不跟随归档中创建的符号链接
        if (info.exists()) {
            if (!info.isDir())
                return false;
        } else if (!current.mkdir(part)) {
            return false;
        }
        if (!current.cd(part))
            return false;
    }

    const QString canonicalDirectory = QFileInfo(directory).canonicalFilePath();
    return !canonicalDirectory.isEmpty() && isWithin(canonicalRoot, canonicalDirectory);
}

bool ensureParentWithinRoot(const QString &root, const QString &path)
{
    if (QFileInfo(path).isSymLink())
        return false;  // 拒绝写穿已存在的符号链接

    const QString parent = QFileInfo(path).path();
    return ensureDirectoryWithinRoot(root, parent);
}

bool symlinkTargetWithinRoot(const QString &root, const QString &linkPath,
                             const QString &linkTarget)
{
    if (linkTarget.isEmpty() || QDir::isAbsolutePath(linkTarget))
        return false;
    const QString rootPath = QDir::cleanPath(QFileInfo(root).absoluteFilePath());
    const QString resolved = QDir::cleanPath(
            QFileInfo(linkPath).absolutePath() + QLatin1Char('/') + linkTarget);
    return resolved == rootPath
            || resolved.startsWith(rootPath + QLatin1Char('/'));
}

QString normalizedArchivePath(const QString &path)
{
    QString normalized = QDir::fromNativeSeparators(path);
    if (normalized.startsWith(QLatin1String("./")))
        normalized.remove(0, 2);
    return normalized;
}

bool archivePathWithin(const QString &path, const QString &prefix)
{
    const QString normalizedPath = normalizedArchivePath(path);
    QString normalizedPrefix = normalizedArchivePath(prefix);
    if (normalizedPrefix.isEmpty())
        return true;
    if (!normalizedPrefix.endsWith(QLatin1Char('/')))
        normalizedPrefix += QLatin1Char('/');
    return normalizedPath == normalizedPrefix.chopped(1)
            || normalizedPath.startsWith(normalizedPrefix);
}

QString archivePathRelativeTo(const QString &path, const QString &prefix)
{
    const QString normalizedPath = normalizedArchivePath(path);
    if (!archivePathWithin(normalizedPath, prefix))
        return normalizedPath;
    QString normalizedPrefix = normalizedArchivePath(prefix);
    if (normalizedPrefix.isEmpty())
        return normalizedPath;
    if (!normalizedPrefix.endsWith(QLatin1Char('/')))
        normalizedPrefix += QLatin1Char('/');
    return normalizedPath.mid(normalizedPrefix.size());
}

QString renamedArchivePath(const QString &path, const QString &oldPath, const QString &newPath)
{
    QString root = newPath;
    if (!root.endsWith(QLatin1Char('/')))
        root += QLatin1Char('/');
    const QString suffix = archivePathRelativeTo(path, oldPath);
    if (suffix.isEmpty() && !path.endsWith(QLatin1Char('/')))
        root.chop(1);
    return root + suffix;
}

QStringList relativeSourceExclusions(const QStringList &sources, const QStringList &excludedPaths)
{
    QStringList relativePaths;
    for (const QString &source : sources) {
        const QFileInfo info(source);
        if (!info.isDir())
            continue;
        const QString root = QDir::cleanPath(info.absoluteFilePath());
        for (const QString &path : excludedPaths) {
            const QString absolutePath = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
            if (absolutePath != root && archivePathWithin(absolutePath, root)) {
                const QString relative = QDir(info.absolutePath()).relativeFilePath(absolutePath);
                if (!relativePaths.contains(relative))
                    relativePaths.append(relative);
            }
        }
    }
    return relativePaths;
}

} // namespace dcarchive
