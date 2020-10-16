/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LIBARCHIVEPLUGIN_H
#define LIBARCHIVEPLUGIN_H

#include "archiveinterface.h"

struct FileProgressInfo {
    float fileProgressProportion = 0.0; //内部百分值范围
    float fileProgressStart;            //上次的百分值
    float totalFileSize;
};

class LibarchivePlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibarchivePlugin(QObject *parent, const QVariantList &args);
    ~LibarchivePlugin() override;

    // ReadOnlyArchiveInterface interface
public:
    bool list() override;
    bool testArchive() override;
    bool extractFiles(const QVector<FileEntry> &files, const ExtractionOptions &options) override;

    // ReadWriteArchiveInterface interface
public:
    bool addFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    bool moveFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    bool copyFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    bool deleteFiles(const QVector<FileEntry> &files) override;
    bool addComment(const QString &comment) override;
};





#endif // LIBARCHIVEPLUGIN_H
