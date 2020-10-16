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

#ifndef UNCOMPRESSVIEW_H
#define UNCOMPRESSVIEW_H

#include "datatreeview.h"
#include "commonstruct.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

// 解压列表
class UnCompressView : public DataTreeView
{
public:
    explicit UnCompressView(QWidget *parent = nullptr);
    ~UnCompressView() override;

    /**
     * @brief setLoadData   设置加载完之后的数据
     * @param stArchiveData 压缩包数据
     */
    void setLoadData(const ArchiveData &stArchiveData);

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief calFirSize    计算文件夹中子文件项数
     * @param strFilePath
     * @return
     */
    qlonglong calDirItemCount(const QString &strFilePath);

private slots:
    /**
     * @brief slotDragFiles     外部文件拖拽至列表处理（追加压缩）
     * @param listFiles         外部拖拽文件
     */
    void slotDragFiles(const QStringList &listFiles);

private:
    //QMap<QString, FileEntry> m_mapEntry;        // 总数据
    //

    ArchiveData m_stArchiveData;
    //QMap<QString, QList<FileEntry>> m_mapShowEntry; // 显示数据（缓存，目录层级切换时从此处取数据，避免再次对总数据进行操作）
};

#endif // UNCOMPRESSVIEW_H
