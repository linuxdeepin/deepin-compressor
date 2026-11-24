// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACY_LIBARCHIVE_ADAPTER_H
#define LEGACY_LIBARCHIVE_ADAPTER_H

#include "archive_backend.h"
#include "archiveui_types.h"

#include <QObject>

#include <memory>

class ReadWriteArchiveInterface;
class Query;

namespace dcarchive {

// 迁移期适配器：把旧 3rdparty libarchive 实现包装成新的 ArchiveBackend。
class LegacyLibarchiveAdapter : public QObject, public ArchiveBackend
{
    Q_OBJECT

public:
    explicit LegacyLibarchiveAdapter(const QString &path,
                                     const QString &mimeType,
                                     QObject *parent = nullptr);
    ~LegacyLibarchiveAdapter() override;

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
    bool ensureLegacy();
    void handleQuery(Query *query);
    ArchiveResult mapFinish(PluginFinishType type) const;
    ArchiveError mapError(ErrorType type) const;
    void notifyFinished(BackendObserver &observer, const ArchiveResult &result);

    std::unique_ptr<ReadWriteArchiveInterface> m_legacy;
    QString m_path;
    QString m_mimeType;
    BackendObserver *m_observer = nullptr;
    bool m_cancelled = false;
};

} // namespace dcarchive

#endif // LEGACY_LIBARCHIVE_ADAPTER_H
