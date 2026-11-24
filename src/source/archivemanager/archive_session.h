// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_SESSION_H
#define ARCHIVE_SESSION_H

#include "archive_registry.h"
#include "archiveui_types.h"
#include "archivequeries.h"

#include <QMutex>
#include <QObject>

#include <memory>


namespace dcarchive {

class ArchiveSession : public QObject
{
    Q_OBJECT

public:
    ArchiveSession(const QString &path, const QString &mimeType,
                   BackendPreference preference, QObject *parent = nullptr);
    ~ArchiveSession() override;

    static ArchiveSession *create(const QString &path, const QString &mimeType, bool forWrite,
                                  BackendPreference preference = BackendPreference::Auto,
                                  QObject *parent = nullptr);

    PluginFinishType list();
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options,
                                  QString *outputPath = nullptr);
    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options);
    PluginFinishType deleteFiles(const QList<FileEntry> &files);
    PluginFinishType renameFiles(const QList<FileEntry> &files);
    PluginFinishType addComment(const QString &comment);
    PluginFinishType updateArchiveData(const UpdateOptions &options);

    void pauseOperation();
    void continueOperation();
    void cancel();
    bool status();
    void prepareOperation();
    void setPassword(const QString &password);
    QString getPassword();
    ErrorType errorType() const { return m_error; }

Q_SIGNALS:
    void signalprogress(double percentage);
    void signalCurFileName(const QString &path);
    void signalFileWriteErrorName(const QString &message);
    void signalQuery(Query *query);

private:
    ArchiveBackend *backendFor(ArchiveOperation operation, const QString &path = {});
    void populateViewState(const ArchiveInfo &info);
    PluginFinishType applyResult(const ArchiveResult &result);
    static ArchiveEntry toArchiveEntry(const FileEntry &entry);
    static FileEntry toFileEntry(const ArchiveEntry &entry);
    static ErrorType toErrorType(ArchiveError error);

    std::unique_ptr<ArchiveBackend> m_backend;
    ArchiveOperation m_operation = ArchiveOperation::Read;
    QString m_backendPassword;
    QString m_backendPath;
    QString m_path;
    QString m_mimeType;
    QString m_password;
    ErrorType m_error = ET_NoError;
    BackendPreference m_preference;
    bool m_paused = false;
    QMutex m_backendMutex;
    bool m_reopen = true;
    bool m_cancelRequested = false;
    bool m_partialRead = false;
};

} // namespace dcarchive

#endif // ARCHIVE_SESSION_H
