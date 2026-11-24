// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_PATH_GUARD_H
#define ARCHIVE_PATH_GUARD_H

#include <QString>
#include <QStringList>

namespace dcarchive {

// 词法清洗：拒绝绝对路径和 ..，返回 targetDir 下的拼接路径。
QString safeOutputPath(const QString &targetDir, const QString &entryPath);

// 逐级创建目录，拒绝任何已存在的符号链接组件。
bool ensureDirectoryWithinRoot(const QString &root, const QString &directory);

// 校验 path 的父目录在解析符号链接后仍位于 root 内，并拒绝覆盖符号链接文件。
bool ensureParentWithinRoot(const QString &root, const QString &path);

// 校验符号链接目标（相对链接按链接所在目录解析）不会逃出 root。
bool symlinkTargetWithinRoot(const QString &root, const QString &linkPath,
                             const QString &linkTarget);

QString normalizedArchivePath(const QString &path);
bool archivePathWithin(const QString &path, const QString &prefix);
QString archivePathRelativeTo(const QString &path, const QString &prefix);
QString renamedArchivePath(const QString &path, const QString &oldPath, const QString &newPath);
QStringList relativeSourceExclusions(const QStringList &sources, const QStringList &excludedPaths);

} // namespace dcarchive

#endif // ARCHIVE_PATH_GUARD_H
