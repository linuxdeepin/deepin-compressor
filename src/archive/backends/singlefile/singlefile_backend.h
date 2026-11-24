// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_SINGLEFILE_BACKEND_H
#define ARCHIVE_SINGLEFILE_BACKEND_H

#include "archive_backend.h"
#include "stream_decoder.h"

#include <QStringList>

#include <atomic>

namespace dcarchive {

// 自有单文件压缩后端：gz / bz2 / xz / lzma。
// 只读；列表生成一条虚拟条目，解压输出一个文件。
class SingleFileBackend : public ArchiveBackend
{
public:
    explicit SingleFileBackend(const QString &path, const QString &mimeType);
    ~SingleFileBackend() override;

    BackendCapabilities capabilities() const override;

    VoidResult open(const OpenRequest &request) override;
    EntryResult list() override;
    VoidResult testArchive() override;
    VoidResult extractFiles(const ExtractRequest &request, BackendObserver &observer) override;

    VoidResult addFiles(const AddRequest &, BackendObserver &) override;
    VoidResult moveFiles(const MoveRequest &, BackendObserver &) override;
    VoidResult copyFiles(const CopyRequest &, BackendObserver &) override;
    VoidResult deleteFiles(const DeleteRequest &) override;
    VoidResult renameFiles(const RenameRequest &) override;
    VoidResult addComment(const QString &) override;
    VoidResult updateArchiveData(const UpdateRequest &) override;

    void pause() override;
    void resume() override;
    void cancel() override;

private:
    void configureCodec();
    QString uncompressedFileName() const;
    VoidResult unsupported() const;
    void notifyFinished(BackendObserver &observer, const ArchiveResult &result);

    QString m_path;
    QString m_mimeType;
    SingleFileCodec m_codec = SingleFileCodec::Gzip;
    QStringList m_suffixes;
    std::atomic_bool m_paused{false};
    std::atomic_bool m_cancelled{false};
};

} // namespace dcarchive

#endif // ARCHIVE_SINGLEFILE_BACKEND_H
