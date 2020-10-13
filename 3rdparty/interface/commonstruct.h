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

// 文件数据
struct FileEntry {
    FileEntry()
    {
        qSize = 0;
        isDirectory = false;
    }

    QString strFullPath;    //压缩包内文件绝对路径
    QString strFileName;        // 文件名
    QString strType; //文件类型
    bool isDirectory;        // 是否为文件夹
    qlonglong qSize;        // 文件真实大小（文件夹显示项）
    QDateTime lastModifiedTime; // 文件最后修改时间
};
Q_DECLARE_METATYPE(FileEntry)

// 压缩选项
struct CompressOptions {

};
Q_DECLARE_METATYPE(CompressOptions)

// 解压选项
struct ExtractionOptions {

};
Q_DECLARE_METATYPE(ExtractionOptions)


#endif // COMMONSTRUCT_H
