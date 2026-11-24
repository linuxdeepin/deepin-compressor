// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_MINIZIP_BACKEND_H
#define ARCHIVE_MINIZIP_BACKEND_H

#include "archive_backend.h"

#include <QString>

#include <atomic>

namespace dcarchive {

class MinizipBackend : public ArchiveBackend
{
public:
    explicit MinizipBackend(const QString &path, const QString &mimeType);
    ~MinizipBackend() override;

    BackendCapabilities capabilities() const override;

    VoidResult open(const OpenRequest &request) override;
    EntryResult list() override;
    VoidResult testArchive() override;
    VoidResult extractFiles(const ExtractRequest &request, BackendObserver &observer) override;
    VoidResult addFiles(const AddRequest &request, BackendObserver &observer) override;
    VoidResult moveFiles(const MoveRequest &request, BackendObserver &observer) override;
    VoidResult copyFiles(const CopyRequest &request, BackendObserver &observer) override;
    VoidResult deleteFiles(const DeleteRequest &request) override;
    VoidResult renameFiles(const RenameRequest &request) override;
    VoidResult addComment(const QString &comment) override;
    VoidResult updateArchiveData(const UpdateRequest &request) override;

    void pause() override;
    void resume() override;
    void cancel() override;

private:
    QString safeOutputPath(const QString &targetDir, const QString &entryPath) const;
    VoidResult unsupported() const;
    void notifyFinished(BackendObserver &observer, const ArchiveResult &result);

    QString m_path;
    QString m_mimeType;
    std::atomic_bool m_paused{false};
    std::atomic_bool m_cancelled{false};
};

} // namespace dcarchive

#endif // ARCHIVE_MINIZIP_BACKEND_H
