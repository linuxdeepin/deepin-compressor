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

/**
 * @brief The HandleWorkingDir class
 * change 用于更改应用所在当前路径及恢复
 * ~HandleWorkingDir 析构时自动恢复应用所在当前路径
 */
class HandleWorkingDir
{
public:
    HandleWorkingDir(QString *oldWorkingDir);
    void change(const QString &newWorkingDir);
    ~HandleWorkingDir();
private:
    QString *m_oldWorkingDir;
};

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
    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;
    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    PluginFinishType addComment(const QString &comment) override;
    /**
     * @brief updateArchiveData   更新压缩包数据
     * @return
     */
    PluginFinishType updateArchiveData(const UpdateOptions &options) override;

    /**
     * @brief pauseOperation    暂停操作
     */
    void pauseOperation() override;

    /**
     * @brief continueOperation 继续操作
     */
    void continueOperation() override;

    /**
     * @brief doKill 强行取消
     */
    bool doKill() override;

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
    struct ArchiveWriteCustomDeleter {
        static inline void cleanup(struct archive *a)
        {
            if (a) {
                archive_write_free(a);
            }
        }
    };


    typedef QScopedPointer<struct archive, ArchiveReadCustomDeleter> ArchiveRead;
    typedef QScopedPointer<struct archive, ArchiveWriteCustomDeleter> ArchiveWrite;

    ArchiveRead m_archiveReader;
    ArchiveRead m_archiveReadDisk;

    /**
     * @brief copyDataFromSource 压缩包数据写到本地文件
     * @param source 读句柄
     * @param dest 写句柄
     */
    void copyDataFromSource(struct archive *source, struct archive *dest, const qlonglong &totalSize);
    /**
     * @brief copyDataFromSource_ArchiveEntry 压缩包数据写到本地文件(提取用)
     * @param source
     * @param dest
     * @param extractFileSize 待提取文件总大小
     */
    void copyDataFromSource_ArchiveEntry(struct archive *source, struct archive *dest, qint64 extractFileSize);

private:
    PluginFinishType list_New();
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
    /**
     * @brief extractionFlags 选择要还原的属性
     * @return
     */
    int extractionFlags() const;

private:
    int m_ArchiveEntryCount = 0; //压缩包内文件(夹)总数量
    QString m_strOldArchiveName; //压缩包名(全路径)
    QStringList m_tars; //list时解压出来的临时tar包
    QString m_oldWorkingDir;
    QString m_extractDestDir; //解压目的路径
//    QString destDirName; //取消解压，需要该变量

protected:
    qlonglong m_currentExtractedFilesSize = 0;//当前已经解压出来的文件大小（能展示出来的都已经解压）
};





#endif // LIBARCHIVEPLUGIN_H
