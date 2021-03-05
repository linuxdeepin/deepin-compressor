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

#ifndef UITOOLS_H
#define UITOOLS_H

#include "archiveinterface.h"
#include "plugin.h"
#include "uistruct.h"
#include "mimetypes.h"

#include <QObject>


#define SAFE_DELETE_ELE( ptr ) \
    if (ptr != nullptr)      \
    {                     \
        delete ptr;       \
        ptr = nullptr;       \
    }

#define SAFE_DELETE_ARRAY( ptr ) \
    if (ptr != nullptr)            \
    {                           \
        delete[] ptr;           \
        ptr = nullptr;             \
    }

#define SAFE_DELETE_TABLE( ptr ) \
    if (ptr != nullptr)            \
    {                           \
        for (i = 0; i < row; i++)\
        {\
            SAFE_DELETE_ARRAY(ptr[i])\
            delete [] ptr;\
        }\
        ptr = nullptr; \
    }

// 界面通用工具
class UiTools : public QObject
{
    Q_OBJECT

public:
    explicit UiTools(QObject *parent = nullptr);
    ~UiTools();

    enum AssignPluginType {
        APT_Auto,           // 自动识别
        APT_Cli7z,          // cli7zplugin
        APT_Libarchive,     // libarchive
        APT_Libzip          // libzipplugin
    };

    /**
     * @brief getConfigPath 获取配置路径
     * @return
     */
    static QString getConfigPath();

    /**
     * @brief renderSVG     渲染图标
     * @param filePath      图标路径
     * @param size          图标大小
     * @return              新的图标
     */
    static QPixmap renderSVG(const QString &filePath, const QSize &size);

    /**
     * @brief humanReadableSize 分卷计算大小
     * @param size
     * @param precision
     * @return
     */
    static QString humanReadableSize(const qint64 &size, int precision);

    /**
     * @brief isArchiveFile     判断文件是否为压缩包
     * @param strFileName          文件名（含路径）
     * @return                  true：压缩包    false：普通文件
     */
    static bool isArchiveFile(const QString &strFileName);


    /**
     * @brief judgeFileMime     判断文件类型
     * @param strFileName  文件名
     * @return
     */
    static QString judgeFileMime(const QString &strFileName);

    /**
     * @brief isExistMimeType 判断此文件关联类型是否存在
     * @param strMimeType      文件类型
     * @return
     */
    static bool isExistMimeType(const QString &strMimeType, bool &bArchive);

    /**
     * @brief readConf   读取配置信息
     * @return  配置信息
     */
    static QString readConf();

    /**
     * @brief Utils::toShortString 将过长的字符串中间换成“...”代替
     * @param strSrc
     * @param limitCounts
     * @param left
     * @return
     */
    static QString toShortString(QString strSrc, int limitCounts = 16, int left = 8);

    /**
     * @brief createInterface   创建插件
     * @param fileName          压缩包名称
     * @param bWrite            是否压缩
     * @param bUseLibArchive    是否使用LibArchive
     * @return
     */
    static ReadOnlyArchiveInterface *createInterface(const QString &fileName, bool bWrite = false, AssignPluginType eType = APT_Auto/*bool bUseLibArchive = false*/);
    static ReadOnlyArchiveInterface *createInterface(const QString &fileName, const CustomMimeType &mimeType, Plugin *plugin);

    /**
     * @brief transSplitFileName 处理7z、rar分卷压缩包名称
     * @param fileName 原始名称
     * @param eSplitType 分卷类型
     * * @return    是否是分卷压缩包
     */
    static void transSplitFileName(QString &fileName, UnCompressParameter::SplitType &eSplitType);

    /**
    * @brief handleFileName    处理文件名xx.xx.xx去除后面
    * @param strFileName
    * @return
    */
    static QString handleFileName(const QString &strFileName);

    /**
     * @brief isLocalDeviceFile 是否是本地设备文件
     * @param strFileName
     * @return
     */
    static bool isLocalDeviceFile(const QString &strFileName);

    /**
     * @brief hasSameFileName   是否存在同名文件（不考虑路径）
     * @param listFiles
     * @return
     */
    static QStringList removeSameFileName(const QStringList &listFiles);

public:
    static QStringList m_associtionlist;
};

#endif  // UITOOLS_H
