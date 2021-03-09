/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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

#include <DSettings>

void GenerateSettingTranslate()
{
    auto setting_Text = QObject::tr("General");  //设置
    auto decompress = QObject::tr("Extraction"); //解压
    auto create_folder = QObject::tr("Auto create a folder for multiple extracted files");  //自动创建文件夹
    auto open_folder = QObject::tr("Show extracted files when completed");  //当解压完成后自动打开对应的文件夹
    auto file_management = QObject::tr("File Management");  //文件管理
    auto delete_file = QObject::tr("Delete files after compression");  //压缩后删除原来的文件
    auto file_association = QObject::tr("Files Associated");  //关联文件
    auto file_type = QObject::tr("File Type");  //文件类型
}
