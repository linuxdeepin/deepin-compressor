// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACY_CLI7Z_ADAPTER_H
#define LEGACY_CLI7Z_ADAPTER_H

#include "archive_backend.h"
#include "archiveui_types.h"

#include <QObject>

#include <memory>

class ReadWriteArchiveInterface;
class Query;

namespace dcarchive {

class LegacyCli7zAdapter : public QObject, public ArchiveBackend
{
    Q_OBJECT

public:
    explicit LegacyCli7zAdapter(const QString &path,
                                const QString &mimeType,
                                const QString &metaDataFile = QString(),
                                QObject *parent = nullptr);
    ~LegacyCli7zAdapter() override;

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
    bool waitForLegacy(int timeoutMs = 60000);
    void handleQuery(Query *query);
    ArchiveResult mapFinish(PluginFinishType type) const;
    ArchiveError mapError(ErrorType type) const;
    void notifyFinished(BackendObserver &observer, const ArchiveResult &result);

    std::unique_ptr<ReadWriteArchiveInterface> m_legacy;
    QString m_path;
    QString m_mimeType;
    QString m_metaDataFile;
    BackendObserver *m_observer = nullptr;
    bool m_cancelled = false;
    bool m_finished = false;
    PluginFinishType m_finishType = PFT_Nomral;
};

} // namespace dcarchive

#endif // LEGACY_CLI7Z_ADAPTER_H
