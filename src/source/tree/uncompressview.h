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
    Q_OBJECT
public:
    explicit UnCompressView(QWidget *parent = nullptr);
    ~UnCompressView() override;

    /**
     * @brief setArchiveData   设置加载完之后的数据
     * @param stArchiveData 压缩包数据
     */
    void setArchiveData(const ArchiveData &stArchiveData);

protected:
    /**
     * @brief mouseDoubleClickEvent 鼠标双击事件
     * @param event 双击事件
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

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

    /**
     * @brief handleDoubleClick    处理双击事件
     * @param index 双击的index
     */
    void handleDoubleClick(const QModelIndex &index);

    /**
     * @brief refreshDataByCurrentPath  根据当前路径刷新数据
     */
    void refreshDataByCurrentPath();

    /**
     * @brief getCurrentDirFiles   获取当前路径下所有文件
     * @return
     */
    QList<FileEntry> getCurrentDirFiles();



protected Q_SLOTS:
    /**
     * @brief slotPreClicked    返回上一级
     */
    virtual void slotPreClicked() override;

private slots:
    /**
     * @brief slotDragFiles     外部文件拖拽至列表处理（追加压缩）
     * @param listFiles         外部拖拽文件
     */
    void slotDragFiles(const QStringList &listFiles);

    /**
     * @brief slotShowRightMenu     显示右键菜单
     * @param pos   右键位置
     */
    void slotShowRightMenu(const QPoint &pos);

    /**
     * @brief slotExtract   右键提取操作
     */
    void slotExtract();

    /**
     * @brief slotExtract2Hera   右键提取到当前文件夹操作
     */
    void slotExtract2Here();

    /**
     * @brief slotDeleteFile    右键删除操作
     */
    void slotDeleteFile();

    /**
     * @brief slotDeleteFile    右键打开操作
     */
    void slotOpen();

    /**
     * @brief slotOpenStyleClicked  右键打开方式操作
     */
    void slotOpenStyleClicked();

private:
    ArchiveData m_stArchiveData;
    QMap<QString, QList<FileEntry>> m_mapShowEntry; // 显示数据（缓存，目录层级切换时从此处取数据，避免再次对总数据进行操作）
    FileEntry m_stRightEntry;       // 右键点击的文件
};

#endif // UNCOMPRESSVIEW_H
