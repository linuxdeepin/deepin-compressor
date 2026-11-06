// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DSettings>
#include <QDebug>
#include "common/ddlog.h"

void GenerateSettingTranslate()
{
    // qCDebug(appLog) << "Generating setting translations";
    auto setting_Text = QObject::tr("General");  //设置
    auto decompress = QObject::tr("Extraction"); //解压
    auto create_folder = QObject::tr("Intelligently create a folder for extracted files");  //智能新增文件夹
    auto open_folder = QObject::tr("Show extracted files when completed");  //当解压完成后自动打开对应的文件夹
    auto auto_quit_after_open_folder = QObject::tr("Quit after opening the extracted folder");  //打开对应文件夹后自动退出
    auto file_management = QObject::tr("File Management");  //文件管理
    auto delete_file = QObject::tr("Delete files after compression");  //压缩后删除原来的文件
    auto file_association = QObject::tr("Files Associated");  //关联文件
    auto file_type = QObject::tr("File Type");  //文件类型
    // qCDebug(appLog) << "Setting translations generated successfully";
}
