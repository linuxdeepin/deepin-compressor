/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     xxx <xxx@uniontech.com>
*
* Maintainer: xxx <xxx@uniontech.com>
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
#ifndef COMMONSTRUCT_H
#define COMMONSTRUCT_H

#include <QDateTime>
#include <QString>
#include <QMetaType>
#include <QVector>
#include <QMap>

// 插件结束类型
enum PluginFinishType {
    PFT_Nomral = 0,     // 正常退出
    PFT_Cancel,         // 取消
    PFT_Error,          // 错误
};
Q_DECLARE_METATYPE(PluginFinishType)

// 错误类型
enum ErrorType {
    ET_NoError = 0,     // 无错误
    ET_WrongPassword,     // 输入的密码错误
    ET_NeedPassword,     // 需要密码
    ET_LongNameError,     // 文件名过长错误
    ET_ArchiveOpenError,     // 压缩包打开错误
    ET_FileOpenError,     // 文件打开错误
    ET_FileReadError,     // 文件读取错误
    ET_FileWriteError,     // 文件写错误
    ET_DeleteError,         // 文件写错误

    ET_UserCancelOpertion,     // 用户取消操作
};

//加密类型
enum EncryptionType {
    Unencrypted,
    Encrypted,
    HeaderEncrypted
};

// 文件路径格式
enum PathFormat {
    NoTrailingSlash,
    WithTrailingSlash
};


// 文件数据
struct FileEntry {
    FileEntry()
    {
        reset();
    }

    void reset()
    {
        strFullPath = "";
        strFileName = "";
        isDirectory = false;
        qSize = 0;
        uLastModifiedTime = 0;
        iIndex = -1;
    }

    QString strFullPath;    // 文件全路径
    QString strFileName;        // 文件名
    bool isDirectory;        // 是否为文件夹
    qlonglong qSize;        // 文件真实大小（文件夹显示项）
    uint uLastModifiedTime;      // 文件最后修改时间

    int iIndex;         // 文件在压缩包中的索引位置（目前是只有zip格式会用到，通过索引查找）
};
Q_DECLARE_METATYPE(FileEntry)

// 压缩包数据
struct ArchiveData {
    ArchiveData()
    {
        reset();
    }

    void reset()
    {
        qSize = 0;
        qComressSize = false;
        strComment = "";
        mapFileEntry.clear();
        listRootEntry.clear();

        isListEncrypted = false;
        strPassword.clear();
    }

    qlonglong qSize;                            // 原始总大小
    qlonglong qComressSize;                     // 压缩包大小
    QString strComment;                         // 压缩包注释信息
    QMap<QString, FileEntry>  mapFileEntry;     // 压缩包内所有文件
    QList<FileEntry> listRootEntry;             // 第一层数据

    bool isListEncrypted;                       // 压缩包是否为列表加密
    QString strPassword;                        // 压缩包密码
};
Q_DECLARE_METATYPE(ArchiveData)

// 压缩选项
struct CompressOptions {
    CompressOptions()
    {
        bEncryption = false;
        bHeaderEncryption = false;
        bSplit = false;
        iVolumeSize = 0;
        iCompressionLevel = -1;
        qTotalSize = 0;
        bTar_7z = false;
    }

    QString strPassword;        // 密码
    QString strEncryptionMethod;    // 加密算法
    QString strCompressionMethod;    // 压缩算法
    int iVolumeSize;    // 分卷大小（kb）
    int iCompressionLevel;      // 压缩等级
    qlonglong qTotalSize; // 源文件总大小
    QString strDestination; // 压缩上级目录（若为空，代表在根目录压缩文件）
    bool bEncryption;       // 是否加密
    bool bHeaderEncryption;     // 是否列表加密
    bool bSplit;        // 是否分卷
    bool bTar_7z;       // 是否是tar.7z格式（补充）
};
Q_DECLARE_METATYPE(CompressOptions)

// 解压选项
struct ExtractionOptions {
    ExtractionOptions()
    {
        bAllExtract = true;
        qSize = 0;
        qComressSize = 0;
        bRightExtract = false;
        bAllExtract = false;
        bBatchExtract = false;
    }

    QString strTargetPath;      // 解压目标路径
    QString strDestination;     // 提取时的上级目录（若为空，代表提取的是根目录下的文件）
    qint64 qSize;               // 解压:原始大小; 提取:待提取的总大小
    qint64 qComressSize;        // 压缩包大小
    bool bRightExtract;         // 是否是右键解压到当前文件夹
    bool bAllExtract;           // 是否全部解压（true：全部解压 false：提取）
    bool bBatchExtract;           // 是否批量解压
};
Q_DECLARE_METATYPE(ExtractionOptions)

// 更新选项
struct UpdateOptions {
    enum Type {
        Delete,     // 删除
        Add         // 追加
    };

    UpdateOptions()
    {
        reset();
    }

    void reset()
    {
        eType = Delete;
        strParentPath.clear();
        listEntry.clear();
        qSize = 0;
    }

    Type eType;                     // 更新类型
    QString strParentPath;          // 父目录（若为空，代表操作的是压缩包首层目录下的文件）
    QList<FileEntry> listEntry;     // 操作的文件（删除：存储选中的文件     追加：存储本地所有文件）
    qint64 qSize;                  // 操作的文件大小
};

#endif // COMMONSTRUCT_H
