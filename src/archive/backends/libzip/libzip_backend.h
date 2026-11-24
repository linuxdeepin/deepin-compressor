// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_LIBZIP_BACKEND_H
#define ARCHIVE_LIBZIP_BACKEND_H

#include "archive_backend.h"

#include <QString>

#include <atomic>
#include <zip.h>

namespace dcarchive {

class LibzipBackend : public ArchiveBackend
{
public:
    explicit LibzipBackend(const QString &path, const QString &mimeType);
    ~LibzipBackend() override;

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
    VoidResult addComment(const QString &comment, BackendObserver &observer) override;
    VoidResult updateArchiveData(const UpdateRequest &request) override;

    void pause() override;
    void resume() override;
    void cancel() override;

private:
    zip_t *openArchive(int flags, ArchiveResult &result) const;
    zip_t *openArchiveAt(const QString &path, int flags, ArchiveResult &result) const;
    QString safeOutputPath(const QString &targetDir, const QString &entryPath) const;
    void notifyFinished(BackendObserver &observer, const ArchiveResult &result);

    QString m_path;
    QString m_mimeType;
    QByteArray m_password;
    std::atomic_bool m_paused{false};
    std::atomic_bool m_cancelled{false};
};

} // namespace dcarchive

#endif // ARCHIVE_LIBZIP_BACKEND_H
