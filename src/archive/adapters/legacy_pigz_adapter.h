// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACY_PIGZ_ADAPTER_H
#define LEGACY_PIGZ_ADAPTER_H

#include "archive_backend.h"
#include "archiveui_types.h"

#include <QObject>

#include <memory>

class ReadWriteArchiveInterface;
class Query;

namespace dcarchive {

// 迁移期适配器：把旧 3rdparty singlefile 实现包装成新的 ArchiveBackend。
// 仅用于基线/差分，不作为最终自有实现。
class LegacyPigzAdapter : public QObject, public ArchiveBackend
{
    Q_OBJECT

public:
    explicit LegacyPigzAdapter(const QString &path,
                                     const QString &mimeType,
                                     QObject *parent = nullptr);
    ~LegacyPigzAdapter() override;

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
    bool ensureLegacy();
    void handleQuery(Query *query);
    ArchiveResult mapFinish(PluginFinishType type) const;
    ArchiveError mapError(ErrorType type) const;
    void notifyFinished(BackendObserver &observer, const ArchiveResult &result);
    VoidResult notSupported() const;

    std::unique_ptr<ReadWriteArchiveInterface> m_legacy;
    QString m_path;
    QString m_mimeType;
    BackendObserver *m_observer = nullptr;
    bool m_cancelled = false;
};

} // namespace dcarchive

#endif // LEGACY_PIGZ_ADAPTER_H
