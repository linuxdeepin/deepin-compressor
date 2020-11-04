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
#ifndef UISTRUCT_H
#define UISTRUCT_H

#include <DStandardPaths>

#include <QString>
#include <QMap>
#include <QDir>

const QStringList g_listColumn = QStringList() << QObject::tr("Name") << QObject::tr("Time modified") << QObject::tr("Type") << QObject::tr("Size");

// 进度类型
enum Progress_Type {
    PT_None,            // 无进度
    PT_Compress,     // 压缩进度
    PT_UnCompress,   // 解压缩进度
    PT_Delete,       // 删除进度
    PT_CompressAdd, // 追加压缩进度
    PT_Convert,         // 转换格式进度
};

/**
 * @brief The Archive_OperationType enum
 * 针对压缩包的操作，比如解压、提取、打开内容等
 * 不同的操作可能共用相同的job
 */
enum Archive_OperationType {
    Operation_NULL, // 无操作
    Operation_Load, // 加载
    Operation_Extract, // 解压
    Operation_SingleExtract, // 提取
    Operation_ExtractHere, // 解压到当前
    Operation_TempExtract, // 临时解压
    Operation_TempExtract_Open, // 打开
    Operation_TempExtract_Open_Choose, // 选择打开
    Operation_DRAG, // 拖拽
    Operation_CONVERT, // 格式转换
    Operation_DELETE // 删除
};

// 压缩/解压列表的列号
enum DataView_Column {
    DC_Name,            // 名称
    DC_Time,            // 修改时间
    DC_Type,            // 类型
    DC_Size,            // 大小
};

// 界面标识
enum Page_ID {
    PI_Home,                // 首页
    PI_Compress,            // 压缩列表
    PI_CompressSetting,     // 压缩设置
    PI_UnCompress,          // 解压列表
    PI_CompressProgress,    // 压缩进度
    PI_UnCompressProgress,  // 解压进度
    PI_DeleteProgress,      // 删除进度
    PI_ConvertProgress,     // 转换进度
    PI_CompressSuccess,     // 压缩成功
    PI_UnCompressSuccess,   // 解压成功
    PI_ConvertSuccess,      // 转换成功
    PI_CompressFailure,     // 压缩失败
    PI_UnCompressFailure,   // 解压失败
    PI_ConvertFailure,      // 转换失败
    PI_Loading,             // 加载
};

// 压缩设置界面参数
struct CompressParameter {
    CompressParameter()
    {
        bEncryption = false;
        bHeaderEncryption = false;
        bSplit = false;
        iVolumeSize = 0;
        iCompressionLevel = -1;
        bTar_7z = false;
        qSize = 0;
    }


    QString strMimeType;    // 格式类型（application/x-tar）
    QString strArchiveName; // 压缩包名称(无路径)
    QString strTargetPath;  // 保存路径
    QString strPassword;        // 密码
    QString strEncryptionMethod;    // 加密算法
    QString strCompressionMethod; //压缩算法
    bool bEncryption = false;       // 是否加密
    bool bHeaderEncryption;     // 是否列表加密
    bool bSplit;        // 是否分卷
    bool bTar_7z;       // 是否是tar.7z格式（补充）
    int iVolumeSize;    // 分卷大小
    int iCompressionLevel;      // 压缩等级
    qint64 qSize;       // 文件总大小
};
Q_DECLARE_METATYPE(CompressParameter)


#endif // CUSTOMDATAINFO_H
