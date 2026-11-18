// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PIGZ_BACKEND_H
#define PIGZ_BACKEND_H

#include "archiveinterface.h"

#include <QByteArray>
#include <QList>
#include <QString>

class QProcess;
class QTimer;

// tar.gz 的多线程压缩后端：tar 打包后交给 pigz 压缩。
// 与旧实现一致，只有 addFiles 有实际动作，其余操作返回成功但不做事。
class PigzBackend : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit PigzBackend(QObject *parent, const QVariantList &args);
    ~PigzBackend() override;

    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    PluginFinishType renameFiles(const QList<FileEntry> &files) override;
    PluginFinishType addComment(const QString &comment) override;
    PluginFinishType updateArchiveData(const UpdateOptions &options) override;

    void pauseOperation() override;
    void continueOperation() override;
    bool doKill() override;

private:
    void readStdout();
    void reportProgress();
    void finishProcess(int exitCode);
    QList<qint64> childPids() const;

    QProcess *m_process = nullptr;
    QTimer *m_timer = nullptr;
    qint64 m_totalSize = 0;
    QByteArray m_pending;
    bool m_killed = false;
};

#endif // PIGZ_BACKEND_H
