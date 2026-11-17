// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIZIP_BACKEND_H
#define MINIZIP_BACKEND_H

#include "archiveinterface.h"

#include <minizip/unzip.h>

// Chrome 扩展包（.crx）：带前缀的 zip，只能读
class MinizipBackend : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    explicit MinizipBackend(QObject *parent, const QVariantList &args);
    ~MinizipBackend() override;

    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    void pauseOperation() override;
    void continueOperation() override;
    bool doKill() override;

private:
    QByteArray m_archiveCode;   // 整包探测出的编码
    bool loadEntry(unzFile zipfile, FileEntry &entry);
    ErrorType extractEntry(unzFile zipfile, const unz_file_info &fileInfo,
                           const ExtractionOptions &options, QString fileName);

    // 跨条目累计的已解压量，进度按它折算
    qlonglong m_extractedSize = 0;
};

#endif // MINIZIP_BACKEND_H
