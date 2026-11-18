// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEFILE_BACKEND_H
#define SINGLEFILE_BACKEND_H

#include "archiveinterface.h"
#include "stream.h"

// gz / bz2 / xz 等单文件压缩后端。
// 具体编解码器与可识别后缀由构造时传入的 MIME 决定。
class SingleFileBackend : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    explicit SingleFileBackend(QObject *parent, const QVariantList &args);
    ~SingleFileBackend() override;

    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    void pauseOperation() override;
    void continueOperation() override;
    bool doKill() override;

private:
    QString uncompressedFileName() const;

    Codec m_codec = Codec::Gzip;
    QStringList m_suffixes;
    qint64 m_extractedSize = 0;
};

#endif // SINGLEFILE_BACKEND_H
