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
#ifndef LIBZIPPLUGIN_H
#define LIBZIPPLUGIN_H

#include "kpluginfactory.h"
#include "archiveinterface.h"

#include <QObject>

#include <zip.h>

class Common;

class LibzipPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libzip.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibzipPluginFactory();
    ~LibzipPluginFactory() override;
};

class LibzipPlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibzipPlugin(QObject *parent, const QVariantList &args);
    ~LibzipPlugin() override;


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
    PluginFinishType updateArchiveData() override;

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

private:
    /**
     * @brief writeEntry 添加新的Entry
     * @param archive 压缩包数据
     * @param entry 新文件
     * @param strDestination 压缩包内路径
     * @param options 压缩配置参数
     * @param isDir
     * @param strRoot 文件前缀路径
     * @return
     */
    bool writeEntry(zip_t *archive, const QString &entry, const CompressOptions &options, bool isDir = false, const QString &strRoot = "");

    /**
     * @brief progressCallback  进度回调函数
     * @param progress  进度
     * @param that
     */
    static void progressCallback(zip_t *, double progress, void *that);

    /**
     * @brief cancelCallback    取消回调函数
     * @param that
     * @return
     */
    static int cancelCallback(zip_t *, void *that);

    /**
     * @brief handleArchiveData 处理压缩包数据
     * @param archive   压缩包
     * @param index 索引
     * * @return
     */
    bool handleArchiveData(zip_t *archive, zip_int64_t index);

    /**
     * @brief statBuffer2FileEntry  数据转换
     * @param statBuffer    压缩包中结构体数据
     * @return      通用结构体
     */
    void statBuffer2FileEntry(const zip_stat_t &statBuffer, FileEntry &entry);

    /**
     * @brief extractEntry  解压单文件
     * @param archive       压缩包
     * @param index         文件索引
     * @param options       解压参数
     * @param qExtractSize  已解压的大小\
     * @param strFileName   当前解压的文件名
     * @return              错误类型
     */
    ErrorType extractEntry(zip_t *archive, zip_int64_t index, const ExtractionOptions &options, qlonglong &qExtractSize, QString &strFileName);

    /**
     * @brief emitProgress  发送进度信号
     * @param dPercentage    百分比
     */
    void emitProgress(double dPercentage);

    /**
     * @brief passwordUnicode   密码编码转换（中文密码）
     * @param strPassword   密码
     * @param iIndex    编码索引
     * @return
     */
    const char *passwordUnicode(const QString &strPassword, int iIndex);

    /**
     * @brief deleteEntry   从压缩包中删除指定文件
     * @param index     文件索引
     * @param archive   压缩包数据
     * @return
     */
    bool deleteEntry(int index, zip_t *archive);

    /**
     * @brief getIndexBySelEntry    根据选择的文件获取所有需要操作的entry
     * @param listEntry             选择的文件
     */
    void getIndexBySelEntry(const QList<FileEntry> &listEntry);


Q_SIGNALS:
    //void error(const QString &message = "", const QString &details = "");

private:
    int m_curFileCount = 0;             // 文件数目
    zip_t *m_pCurArchive = nullptr;     // 当前正在操作的压缩包
    QList<int> m_listCurIndex;      // 当前操作的所有文件索引
    QStringList m_listCurName;      // 当前操作的所有文件索引
    QStringList m_listCodecs;   // 中文编码格式

    QMap<zip_int64_t, QByteArray> m_mapFileCode;   // 存储文件编码

};

#endif // LIBZIPPLUGIN_H
