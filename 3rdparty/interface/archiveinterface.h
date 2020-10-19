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
#ifndef ARCHIVEINTERFACE_H
#define ARCHIVEINTERFACE_H

#include "commonstruct.h"
#include "kpluginmetadata.h"
#include "common.h"

#include <QObject>
#include <QString>
#include <QMimeType>

// 只读（查看和解压等）
class ReadOnlyArchiveInterface : public QObject
{
    Q_OBJECT

public:
    explicit ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadOnlyArchiveInterface() override;

    // 插件类型
    enum Plugintype {
        PT_Cliinterface,
        PT_LibArchive,
        PT_Libzip
    };

    /**
     * @brief list      加载压缩包
     * @return
     */
    virtual bool list() = 0;

    /**
     * @brief testArchive   测试压缩包
     * @return
     */
    virtual bool testArchive() = 0;

    /**
     * @brief extractFiles          解压
     * @param files                 待解压的文件，若数目为0,则是全部解压，否则为部分提取
     * @param options               解压参数
     * @return                      是否解压成功
     */
    virtual bool extractFiles(const QVector<FileEntry> &files, const ExtractionOptions &options) = 0;

    /**
     * @brief waitForFinished   判断是否通过线程调用
     * @return true：线程调用
     */
    bool waitForFinished();

    /**
     * @brief getArchiveData    获取压缩包数据
     * @param stArchiveData     压缩包数据
     */
    void getArchiveData(ArchiveData &stArchiveData);

protected:
    /**
     * Setting this option to true will NOT run the functions in their own thread.
     * Instead it will be necessary to call finished(bool) when the operation is actually finished.
     */
    void setWaitForFinishedSignal(bool value);

Q_SIGNALS:
    /**
     * @brief signalFinished    结束信号
     * @param bNormal   是否正确结束
     */
    void signalFinished(bool bRight);
    void error(const QString &message = "", const QString &details = "");

    /**
     * @brief signalprogress    进度信号
     * @param iPercent  进度值
     */
    void signalprogress(double dPercentage);

    /**
     * @brief signalCurFileName     发送当前正在操作的文件名
     * @param filename      文件名
     */
    void signalCurFileName(const QString &filename);

public:
    Plugintype m_ePlugintype;

protected:
    bool m_bWaitForFinished = false;    // 等待结束
    uint m_numberOfEntries; //原有的归档数量
    KPluginMetaData m_metaData;
    QString m_strArchiveName; //1、压缩：最终的压缩包名 2、解压：加载的压缩包名
    QMimeType m_mimetype;
    Common *m_common = nullptr; // 通用工具类
    ArchiveData m_stArchiveData;
};

// 可读可写（可用来压缩、查看、解压等）
class ReadWriteArchiveInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    explicit ReadWriteArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadWriteArchiveInterface() override;

    enum WorkType {
        WT_List,
        WT_Extract,
        WT_Add,
        WT_Delete,
        WT_Move,
        WT_Copy,
        WT_Comment,
        WT_Test
    };

    /**
     * @brief addFiles          压缩文件
     * @param files             待压缩文件
     * @param options           压缩参数
     * @return
     */
    virtual bool addFiles(const QVector<FileEntry> &files, const CompressOptions &options) = 0;

    /**
     * @brief addFiles          移动压缩文件
     * @param files             待移动文件
     * @param options           压缩参数
     * @return
     */
    virtual bool moveFiles(const QVector<FileEntry> &files, const CompressOptions &options) = 0;

    /**
     * @brief addFiles          拷贝压缩文件
     * @param files             待拷贝文件
     * @param options           压缩参数
     * @return
     */
    virtual bool copyFiles(const QVector<FileEntry> &files, const CompressOptions &options) = 0;

    /**
     * @brief addFiles          删除压缩文件
     * @param files             待删除文件
     * @return
     */
    virtual bool deleteFiles(const QVector<FileEntry> &files) = 0;

    /**
     * @brief addComment        添加注释
     * @param comment           注释内容
     * @return
     */
    virtual bool addComment(const QString &comment) = 0;
};

#endif // ARCHIVEINTERFACE_H
