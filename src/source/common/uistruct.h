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
#define TEMPPATH DStandardPaths::writableLocation(QStandardPaths::TempLocation)     // 临时路径（打开等操作）
#define MAINWINDOW_WIDTH_NAME "MainWindowWidthName"      // 主界宽
#define MAINWINDOW_HEIGHT_NAME "MainWindowHeightName"    // 主界面高
#define HISTORY_DIR_NAME "dir"      // 历史打开路径
#define MAINWINDOW_DEFAULTW 620     // 默认宽度
#define MAINWINDOW_DEFAULTH 465     // 默认高度

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
    Operation_Create, // 创建压缩包
    Operation_Extract, // 解压
    Operation_SingleExtract, // 提取
    //Operation_ExtractHere, // 解压到当前
    //Operation_TempExtract, // 临时解压
    Operation_TempExtract_Open, // 打开
    //Operation_TempExtract_Open_Choose, // 选择打开
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
    PI_AddCompressProgress,    // 追加进度
    PI_UnCompressProgress,  // 解压进度
    PI_DeleteProgress,      // 删除进度
    PI_ConvertProgress,     // 转换进度
    PI_Success,             // 成功
    PI_Failure,             // 失败
    PI_Loading,             // 加载
};

// 压缩参数
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

// 解压参数（选择压缩包之后分析的数据）
struct UnCompressParameter {
    UnCompressParameter()
    {
        bRightOperation = false;
        bSplitVolume = false;
        bSplitVolume = false;
        bMultiplePassword = false;
        bModifiable = false;
        bCommentModifiable = false;
        qSize = 0;
        listExractFiles.clear();
    }

    QString strFullPath;        // 压缩包全路径
    QString strExtractPath;     // 解压路径
    bool bRightOperation;       // 是否右键快捷操作（解压到当前文件夹）
    bool bSplitVolume;          // 是否分卷包
    bool bMultiplePassword;     // 是否支持多密码追加
    bool bModifiable;           // 是否更改压缩包数据
    bool bCommentModifiable;    // 是否支持注释更改
    qint64 qSize;               // 压缩包大小
    QList<QString> listExractFiles; // 存储提取文件,用来结束之后自动打开文件夹时选中

};
Q_DECLARE_METATYPE(UnCompressParameter)

// 成功界面显示的信息
enum SuccessInfo {
    SI_Compress,    // 压缩成功
    SI_UnCompress,  // 解压成功
};

// 错误界面显示的信息
enum ErrorInfo {
    EI_NoPlugin,            // 无可用插件
    EI_ArchiveOpenFailed,   // 压缩包打开失败
    EI_ArchiveDamaged,   // 压缩包损坏
    EI_ArchiveMissingVolume,   // 分卷包缺失
    EI_WrongPasswordWhenLoad,       // 加载密码错误
    EI_WrongPasswordWhenUnCompress,       // 解压密码错误
    EI_LongFileName,        // 文件名过长
    EI_CreatFileFailed,     // 创建文件失败
    EI_CreatArchiveFailed,     // 创建压缩文件失败
};

#endif // CUSTOMDATAINFO_H
