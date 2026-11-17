// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACY_BACKEND_ADAPTER_H
#define LEGACY_BACKEND_ADAPTER_H

#include "archive_backend.h"
#include "archive_types.h"
#include "backend_capabilities.h"
#include "backend_observer.h"

#include "commonstruct.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QList>
#include <memory>

// Forward declarations from the old KDE-based interface layer.
class ReadOnlyArchiveInterface;
class ReadWriteArchiveInterface;
class Query;

namespace archive {

class LegacyBackendAdapter : public ArchiveBackend
{
public:
    explicit LegacyBackendAdapter(ReadOnlyArchiveInterface *iface);
    explicit LegacyBackendAdapter(ReadWriteArchiveInterface *iface);
    explicit LegacyBackendAdapter(ReadOnlyArchiveInterface *iface,
                                  const BackendCapabilities &caps);
    explicit LegacyBackendAdapter(ReadWriteArchiveInterface *iface,
                                  const BackendCapabilities &caps);
    ~LegacyBackendAdapter() override;

    BackendCapabilities capabilities() const override;

    VoidResult open(const OpenRequest &request) override;
    EntryResult list() override;
    VoidResult testArchive() override;
    VoidResult extractFiles(const ExtractRequest &request,
                            BackendObserver &observer) override;
    VoidResult addFiles(const AddRequest &request,
                        BackendObserver &observer) override;
    VoidResult moveFiles(const MoveRequest &request,
                         BackendObserver &observer) override;
    VoidResult copyFiles(const CopyRequest &request,
                         BackendObserver &observer) override;
    VoidResult deleteFiles(const DeleteRequest &request) override;
    VoidResult renameFiles(const RenameRequest &request) override;
    VoidResult addComment(const QString &comment) override;
    VoidResult updateArchiveData(const UpdateRequest &request) override;

    void pause() override;
    void resume() override;
    void cancel() override;

    // Release ownership of the wrapped ReadOnlyArchiveInterface.
    //
    // Ownership contract:
    //   - Before returning the interface, all signal-forwarding connections
    //     established in setupSignalForwarding() are disconnected, so the
    //     released interface will no longer invoke any adapter callbacks.
    //   - The caller takes full ownership of the returned pointer and must
    //     delete it when done.  The adapter will not delete it.
    //   - After this call the adapter is in an empty state: it holds no
    //     interface pointer and must not be used for further operations.
    //   - The adapter itself may be safely destroyed immediately after this
    //     call; doing so will not affect the released interface.
    ReadOnlyArchiveInterface *releaseInterface();

private:
    void setupSignalForwarding();
    void disconnectForwardingConnections();
    static VoidResult toVoidResult(int finishType);
    static ErrorCode toErrorCode(int errorType);
    static QList<ArchiveEntry> toArchiveEntries(const QList<FileEntry> &oldEntries);

    // Convert new ExtractRequest to old ExtractionOptions + FileEntry list.
    // Returns false with ErrorCode::NotSupported when preservePaths is false
    // (the old interface always preserves directory structure).
    bool toExtractionOptions(const ExtractRequest &request,
                             ExtractionOptions &options,
                             QList<FileEntry> &entries,
                             ErrorCode &error);

    void toCompressOptions(const AddRequest &request,
                           CompressOptions &options,
                           QList<FileEntry> &entries);

    BackendObserver *m_observer = nullptr;

    ReadOnlyArchiveInterface *m_readOnlyIface = nullptr;
    ReadWriteArchiveInterface *m_readWriteIface = nullptr;
    bool m_isReadWrite = false;

    BackendCapabilities m_capabilities;

    // Extraction control flags set from ExtractRequest before calling
    // the old interface.  When true, the OverwriteQuery handler auto-responds
    // instead of forwarding to the observer.
    bool m_overwriteAll = false;
    bool m_skipAll = false;

    // Cached from DataManager after open() — reflects isListEncrypted.
    bool m_listEncrypted = false;

    // Stored signal-forwarding connections so they can be disconnected
    // in releaseInterface() and the destructor, preventing use-after-free
    // when the adapter is destroyed but the interface lives on.
    QList<QMetaObject::Connection> m_forwardingConnections;
};

} // namespace archive

#endif // LEGACY_BACKEND_ADAPTER_H
