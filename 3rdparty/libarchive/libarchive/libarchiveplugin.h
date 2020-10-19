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
#include "commonstruct.h"

#include <archive.h>

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

protected:
    /**
     * @brief initializeReader 读取压缩包数据之前一系列操作
     * @return
     */
    bool initializeReader();

    struct ArchiveReadCustomDeleter {
        static inline void cleanup(struct archive *a)
        {
            if (a) {
                archive_read_free(a);
            }
        }
    };
    typedef QScopedPointer<struct archive, ArchiveReadCustomDeleter> ArchiveRead;
    ArchiveRead m_archiveReader;

private:
    bool list_New();
    QString convertCompressionName(const QString &method);
    /**
     * @brief emitEntryForIndex 构建压缩包内数据
     * @param aentry
     */
    void emitEntryForIndex(archive_entry *aentry);
    /**
     * @brief deleteTempTarPkg 删除list时解压出来的临时tar包
     * @param tars
     */
    void deleteTempTarPkg(const QStringList &tars);

private:
    int m_ArchiveEntryCount = 0; //压缩包内文件(夹)总数量
    QString m_strOldArchiveName; //压缩包名(全路径)
    QStringList m_tars; //list时解压出来的临时tar包
    qlonglong m_extractedFilesSize; //压缩包内文件原始总大小
};





#endif // LIBARCHIVEPLUGIN_H
