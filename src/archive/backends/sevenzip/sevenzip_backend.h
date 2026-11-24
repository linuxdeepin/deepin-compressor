// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_SEVENZIP_BACKEND_H
#define ARCHIVE_SEVENZIP_BACKEND_H

#include "archive_backend.h"

#include <QString>
#include <QHash>

#include <atomic>
#include <functional>

namespace dcarchive {

class SevenZipBackend : public ArchiveBackend
{
public:
    explicit SevenZipBackend(const QString &path, const QString &mimeType);
    ~SevenZipBackend() override;

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
    struct RunResult {
        bool started = false;
        int exitCode = -1;
        QString output;
        QString errors;
    };

    using OutputCallback = std::function<void(const QString &)>;
    using DataCallback = std::function<bool(const QByteArray &)>;

    RunResult run7z(const QStringList &arguments, const OutputCallback &onOutput = {},
                    const QString &workingDirectory = {}, const DataCallback &onData = {});
    RunResult runArchiveCommand(QStringList switches, const QStringList &entries = {},
                                const OutputCallback &onOutput = {},
                                const QString &workingDirectory = {},
                                const QString &archivePath = {}, const DataCallback &onData = {});
    ArchiveResult resultForRun(const RunResult &run, ArchiveError fallback) const;
    EntryResult readListing(bool includeTimes, QHash<int, quint32> *checksums = nullptr);
    ArchiveResult extractFromIncompleteZip(const ExtractRequest &request,
                                           const QVector<ArchiveEntry> &entries,
                                           const QHash<int, quint32> &checksums,
                                           BackendObserver &observer);
    void notifyFinished(BackendObserver &observer, const ArchiveResult &result);

    QString m_path;
    QString m_mimeType;
    QString m_password;
    std::atomic<qint64> m_processId{0};
    std::atomic_bool m_paused{false};
    std::atomic_bool m_cancelled{false};
};

} // namespace dcarchive

#endif // ARCHIVE_SEVENZIP_BACKEND_H
