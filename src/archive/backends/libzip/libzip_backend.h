// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LIBZIP_BACKEND_H
#define LIBZIP_BACKEND_H

#include "archiveinterface.h"

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>

#include <zip.h>

// zip 后端：直接用 libzip 读写
class LibzipBackend : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibzipBackend(QObject *parent, const QVariantList &args);
    ~LibzipBackend() override;

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
    ErrorType extractEntry(zip_t *archive, zip_int64_t index, const ExtractionOptions &options,
                           qlonglong &extracted, QString &fileName);
    bool writeLocalPath(zip_t *archive, const QString &path, const QString &root);
    QByteArray passwordBytes(const QString &password) const;

    static void progressCallback(zip_t *archive, double progress, void *that);

    QMap<zip_int64_t, QByteArray> m_codeMap;
    QByteArray m_archiveCode;   // 整包探测出的编码
    QMap<QString, int> m_mapLongName;
    QMap<QString, int> m_mapLongDirName;
    QMap<QString, int> m_mapRealDirValue;
    bool m_bLnfs = false;
    bool m_handledLongName = false;
    int m_level = -1;
    QString m_comment;
    double m_scaleSize = 0.0;
    QList<zip_int64_t> m_selIndex;
};

#endif // LIBZIP_BACKEND_H
