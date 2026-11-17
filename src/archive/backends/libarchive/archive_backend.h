// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_BACKEND_H
#define ARCHIVE_BACKEND_H

#include "archiveinterface.h"

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

#include <archive.h>

// libarchive 后端：tar 家族（读写）、deb / iso 等（只解压）。
// 与旧实现一致，两者都是 ReadWriteArchiveInterface：
// 只读格式的写操作返回 PFT_Error，界面据此禁用。
class ArchiveBackend : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit ArchiveBackend(QObject *parent, const QVariantList &args);
    ~ArchiveBackend() override;

    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    PluginFinishType renameFiles(const QList<FileEntry> &files) override;
    PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType addComment(const QString &comment) override;
    PluginFinishType updateArchiveData(const UpdateOptions &options) override;

    void pauseOperation() override;
    void continueOperation() override;
    bool doKill() override;

private:
    struct Entry {
        FileEntry file;
        quint32 mode = 0;
        QString linkTarget;
        bool isLink = false;
    };

    bool openArchive();
    void closeArchive();
    bool nextEntry(Entry &entry);
    qint64 uncompressedStreamSize();

    // 重建归档到临时文件后原子替换，避免就地读写把包写坏
    bool rebuildArchive(const QString &outPath, const QSet<QString> &drop,
                        const QHash<QString, QString> &renames,
                        const QStringList &inputs, const QString &destination,
                        qint64 totalSize);
    bool copyExistingEntries(struct archive *writer, const QSet<QString> &drop,
                             const QHash<QString, QString> &renames, qint64 totalSize);
    bool appendLocalPath(struct archive *writer, const QString &path,
                         const QString &destination, qint64 totalSize);
    bool replaceArchive(const QString &tempPath);

    struct archive *m_reader = nullptr;
    qint64 m_progressDone = 0;
    qint64 m_lastPercent = -1;
};

#endif // ARCHIVE_BACKEND_H
