// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_STAGING_MERGE_H
#define ARCHIVE_STAGING_MERGE_H

#include "archive_types.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace dcarchive {

bool mergeStagedEntries(const QString &stagingRoot, const QString &targetRoot,
                        const QVector<ArchiveEntry> &entries,
                        const QString &destinationPrefix, ArchiveResult &result);

bool stageLocalEntries(const QVector<ArchiveEntry> &entries, const QString &destinationPath,
                       const QString &stagingRoot, QStringList *stagedPaths,
                       ArchiveResult &result);

} // namespace dcarchive

#endif // ARCHIVE_STAGING_MERGE_H
