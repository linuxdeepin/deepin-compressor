// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_REGISTRY_H
#define ARCHIVE_REGISTRY_H

#include "archive_backend.h"

#include <QString>
#include <memory>

namespace dcarchive {

enum class ArchiveOperation {
    Read,
    Create,
    CreateSplit,
    Append,
    Modify,
};

enum class BackendPreference {
    Auto,
    SevenZip,
    Libarchive,
    Libzip,
    Pigz,
};

class ArchiveRegistry
{
public:
    static std::unique_ptr<ArchiveBackend> create(const QString &path,
                                                  const QString &mimeType,
                                                  ArchiveOperation operation,
                                                  BackendPreference preference);
    static std::unique_ptr<ArchiveBackend> create(const QString &path,
                                                  const QString &mimeType,
                                                  ArchiveOperation operation);
    static std::unique_ptr<ArchiveBackend> create(const QString &path,
                                                  const QString &mimeType,
                                                  bool forWrite = false);
};

} // namespace dcarchive

#endif // ARCHIVE_REGISTRY_H
