/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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
#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include "commonstruct.h"
#include "archivejob.h"
#include "archiveinterface.h"
#include "plugin.h"
#include "queries.h"

#include <QObject>
#include <QMimeType>
#include <QFileInfo>

class ArchiveManager : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief get_instance  获取单例实例对象
     * @return
     */
    static ArchiveManager *get_instance(void);


    ArchiveJob *archiveJob();

    /**
     * @brief createArchive     创建压缩包
     * @param files             待压缩文件
     * @param strDestination    文件存储在压缩包的路径（为空时，处于根目录）
     * @param options           压缩参数
     * @param bBatch            是否批量压缩（多路径）
     */
    void createArchive(const QList<FileEntry> &files, const QString &strDestination, const CompressOptions &stOptions, bool useLibArchive = false/*, bool bBatch = false*/);

    /**
     * @brief loadArchive
     * @param strArchiveName
     */
    void loadArchive(const QString &strArchiveName);

    /**
     * @brief getLoadArchiveData    获取加载完之后的压缩包数据
     * @param stArchiveData         压缩包数据
     */
    //void getLoadArchiveData(ArchiveData &stArchiveData);

    /**
     * @brief extractFiles    解压文件
     * @param strArchiveName    压缩包名称
     * @param files             待解压的文件（若数目为空，属于全部解压，否则为提取）
     * @param options           解压参数
     */
    void extractFiles(const QString &strArchiveName, const QList<FileEntry> &files, const ExtractionOptions &stOptions);

    /**
     * @brief extractFiles2Path     提取文件至指定目录
     * @param strArchiveName        压缩包名称
     * @param listSelEntry          选中的提取文件
     * @param stOptions             提取参数
     */
    void extractFiles2Path(const QString &strArchiveName, const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions);

    /**
     * @brief deleteFiles       删除压缩包中的文件
     * @param strArchiveName    压缩包名称
     * @param listSelEntry      当前选中的文件
     */
    void deleteFiles(const QString &strArchiveName, const QList<FileEntry> &listSelEntry);

    /**
     * @brief batchExtractFiles 批量解压
     * @param listFiles          压缩文件
     * @param strTargetPath     解压路径
     * @param bAutoCreatDir     是否自动创建文件夹
     */
    void batchExtractFiles(const QStringList &listFiles, const QString &strTargetPath, bool bAutoCreatDir);

    /**
     * @brief openFile          打开压缩包中文件
     * @param strArchiveName    压缩包名称
     * @param stEntry           待打开文件数据
     * @param strTargetPath     临时解压路径
     * @param strProgram        应用程序名
     */
    void openFile(const QString &strArchiveName, const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram);

private:


Q_SIGNALS:
    /**
     * @brief signalError       错误信号
     * @param eErrorType        错误类型
     */
    void signalError(ErrorType eErrorType);

    /**
     * @brief signalFinished    操作结束信号
     */
    void signalJobFinished();

    /**
     * @brief signalprogress    进度信号
     * @param iPercent  进度值
     */
    void signalprogress(double dPercentage);

    /**
     * @brief signalCurFileName 发送当前正在操作的文件名
     * @param strName   当前文件名
     */
    void signalCurFileName(const QString &strName);

    /**
     * @brief signalQuery   发送询问信号
     * @param query 询问类型
     */
    void signalQuery(Query *query);

    /**
     * @brief signalCurArchiveName  当前正在操作的压缩包信号
     * @param strArchiveName        压缩包名称
     */
    void signalCurArchiveName(const QString &strArchiveName);

private:
    explicit ArchiveManager(QObject *parent = nullptr);
    ~ArchiveManager() override;

private Q_SLOTS:
    /**
     * @brief slotJobFinished
     */
    void slotJobFinished();

private:
    ArchiveJob *m_pArchiveJob = nullptr;     // 当前操作指针

    ReadOnlyArchiveInterface *m_pInterface = nullptr;   // 当前插件指针（只存储load操作的interface，方便解压等操作）

    static QMutex m_mutex;//实例互斥锁。
    static QAtomicPointer<ArchiveManager> m_instance;/*!<使用原子指针,默认初始化为0。*/
};

#endif // ARCHIVEMANAGER_H
