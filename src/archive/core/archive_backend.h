// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_BACKEND_H
#define ARCHIVE_BACKEND_H

#include "archive_types.h"
#include "backend_capabilities.h"
#include "backend_observer.h"

#include <QString>

namespace dcarchive {

class ArchiveBackend
{
public:
    virtual ~ArchiveBackend() = default;

    virtual BackendCapabilities capabilities() const = 0;

    virtual VoidResult open(const OpenRequest &request) = 0;
    virtual EntryResult list() = 0;
    virtual VoidResult testArchive() = 0;
    virtual VoidResult extractFiles(const ExtractRequest &request, BackendObserver &observer) = 0;
    virtual VoidResult addFiles(const AddRequest &request, BackendObserver &observer) = 0;
    virtual VoidResult moveFiles(const MoveRequest &request, BackendObserver &observer) = 0;
    virtual VoidResult copyFiles(const CopyRequest &request, BackendObserver &observer) = 0;
    virtual VoidResult deleteFiles(const DeleteRequest &request) = 0;
    virtual VoidResult renameFiles(const RenameRequest &request) = 0;
    virtual VoidResult addComment(const QString &comment) = 0;
    virtual VoidResult addComment(const QString &comment, BackendObserver &) { return addComment(comment); }
    virtual VoidResult updateArchiveData(const UpdateRequest &request) = 0;

    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void cancel() = 0;
};

} // namespace dcarchive

#endif // ARCHIVE_BACKEND_H
