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
#ifndef LIBMINIZIPPLUGIN_H
#define LIBMINIZIPPLUGIN_H

#include "kpluginfactory.h"
#include "archiveinterface.h"

#include <QObject>

#include <minizip/unzip.h>

class Common;

class LibminizipPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libminizip.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibminizipPluginFactory();
    ~LibminizipPluginFactory() override;
};

class LibminizipPlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibminizipPlugin(QObject *parent, const QVariantList &args);
    ~LibminizipPlugin() override;


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

private:
    /**
     * @brief handleArchiveData 处理压缩包数据
     * @param archive   压缩包
     * @param index 索引
     * * @return
     */
    bool handleArchiveData(unzFile zipfile);

    /**
     * @brief extractEntry  解压单文件
     * @param zipfile       压缩包数据
     * @param file_info     文件信息
     * @param options       解压参数
     * @param qExtractSize  已解压的大小
     * @param strFileName   当前解压的文件名
     * @return              错误类型
     */
    ErrorType extractEntry(unzFile zipfile, unz_file_info file_info, const ExtractionOptions &options, qlonglong &qExtractSize, QString &strFileName);

    /**
     * @brief getSelFiles    根据选择的文件获取所有需要操作的
     * @param setFiles             选择的文件
     */
    QStringList getSelFiles(const QList<FileEntry> &files);


Q_SIGNALS:
    //void error(const QString &message = "", const QString &details = "");

private:


};

#endif // LibminizipPLUGIN_H
