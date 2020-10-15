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

// 错误类型
enum ErrorType {
    ET_NoError = 0,     // 无错误

};

// 文件数据
struct FileEntry {
    FileEntry()
    {
        qSize = 0;
        isDirectory = false;
    }

    QString strFullPath;    // 文件名（含绝对路径：/../../xx.xx）
    QString strFileName;        // 文件名（不含绝对路径：xx.xx）
    QString strType;        //文件类型
    bool isDirectory;        // 是否为文件夹
    qlonglong qSize;        // 文件真实大小（文件夹显示项）
    QDateTime lastModifiedTime; // 文件最后修改时间
};
Q_DECLARE_METATYPE(FileEntry)

// 压缩选项
struct CompressOptions {
    CompressOptions()
    {
        bEncryption = false;
        bHeaderEncryption = false;
        bSplit = false;
        iVolumeSize = 0;
        iCompressionLevel = -1;
        bTar_7z = false;
    }

    bool bEncryption;       // 是否加密
    QString strPassword;        // 密码
    QString strEncryptionMethod;    // 加密算法
    QString strCompressionMethod;    // 压缩算法
    bool bHeaderEncryption;     // 是否列表加密
    bool bSplit;        // 是否分卷
    int iVolumeSize;    // 分卷大小（kb）
    int iCompressionLevel;      // 压缩等级

    bool bTar_7z;       // 是否是tar.7z格式（补充）
};
Q_DECLARE_METATYPE(CompressOptions)

// 解压选项
struct ExtractionOptions {

};
Q_DECLARE_METATYPE(ExtractionOptions)


#endif // COMMONSTRUCT_H
