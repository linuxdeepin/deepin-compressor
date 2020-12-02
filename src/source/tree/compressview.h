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

#ifndef COMPRESSVIEW_H
#define COMPRESSVIEW_H

#include "datatreeview.h"
#include "commonstruct.h"

#include <DWidget>
#include <DFileWatcher>

#include <QFileInfo>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE


class QFileSystemWatcher;

// 压缩列表
class CompressView : public DataTreeView
{
    Q_OBJECT
public:
    explicit CompressView(QWidget *parent = nullptr);
    ~CompressView() override;

    /**
     * @brief addCompressFiles  添加压缩文件
     * @param listFiles     添加的文件
     */
    void addCompressFiles(const QStringList &listFiles);

    /**
     * @brief getCompressFiles  获取待压缩的文件
     * @return      待压缩的文件
     */
    QStringList getCompressFiles();

    /**
     * @brief refreshCompressedFiles    刷新压缩文件
     * @param bChanged                  是否有文件改变
     * @param strFileName               改变的文件名
     */
    void refreshCompressedFiles(bool bChanged = false, const QString &strFileName = "");

    /**
     * @brief clear 清空数据
     */
    void clear();

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
     * @brief fileInfo2Entry    文件信息结构体转换
     * @param fileInfo          本地文件信息
     * @return
     */
    FileEntry fileInfo2Entry(const QFileInfo &fileInfo);

    /**
     * @brief handleDoubleClick    处理双击事件
     * @param index 双击的index
     */
    void handleDoubleClick(const QModelIndex &index);

    /**
     * @brief getCurrentDirFiles   获取当前路径下所有文件
     * @return
     */
    QList<FileEntry> getCurrentDirFiles();

    /**
     * @brief handleLevelChanged    处理目录层级变化
     */
    void handleLevelChanged();

    /**
     * @brief getPrePathByLevel    根据层级获取父路径（相对）
     * @param strPath              传入的绝对路径
     * @return
     */
    QString getPrePathByLevel(const QString &strPath);

    /**
     * @brief refreshDataByCurrentPath  根据当前路径刷新数据
     */
    void refreshDataByCurrentPath();

signals:
    /**
     * @brief signalLevelChanged    目录层级变化
     * @param bRootIndex    是否是根目录
     */
    void signalLevelChanged(bool bRootIndex);

private slots:
    /**
     * @brief slotShowRightMenu     显示右键菜单
     * @param pos   右键位置
     */
    void slotShowRightMenu(const QPoint &pos);

    /**
     * @brief slotDeleteFile    删除待压缩文件
     */
    void slotDeleteFile();

    /**
     * @brief slotDirChanged   监听当前目录变化
     */
    void slotDirChanged();

    /**
     * @brief slotDragFiles     外部文件拖拽至列表处理
     * @param listFiles         外部拖拽文件
     */
    void slotDragFiles(const QStringList &listFiles);

    /**
     * @brief slotOpenStyleClicked  打开方式点击
     */
    void slotOpenStyleClicked();



protected Q_SLOTS:
    /**
     * @brief slotPreClicked    返回上一级
     */
    virtual void slotPreClicked() override;

private:
    QStringList m_listCompressFiles;    // 待压缩的文件
    QList<FileEntry> m_listEntry;       // 待待压缩的文件数据
    QFileSystemWatcher *m_pFileWatcher; // 对当前目录进行监控(层级大于1时)
    FileEntry m_stRightEntry;       // 右键点击的文件

    QStringList m_listSelFiles;         // 新添加的文件
};

#endif // COMPRESSVIEW_H
