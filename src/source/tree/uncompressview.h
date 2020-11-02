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
#include <DFileDragServer>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

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

    /**
     * @brief setDefaultUncompressPath  设置默认解压路径
     * @param strPath       路径
     */
    void setDefaultUncompressPath(const QString &strPath);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
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
    /**
     * @brief getFilesByParentPath  获取当前路径下文件数据
     * @return
     */
    QList<FileEntry> getCurPathFiles();

    /**
     * @brief getAllFilesByParentPath   获取某路径下所有的文件
     * @param strFullPath               路径名称
     * @param listEntry                 子文件数据
     * @param qSize                     总大小
     */
    void getAllFilesByParentPath(const QString &strFullPath, QList<FileEntry> &listEntry, qint64 &qSize);

    /**
     * @brief getSelEntry       获取当前选择的文件数据
     */
    QList<FileEntry> getSelEntry();

    /**
     * @brief extract2Path      提取选中的文件至指定路径
     * @param strPath           指定的路径
     */
    void extract2Path(const QString &strPath);

Q_SIGNALS:
    /**
     * @brief signalExtract2Path    提取压缩包中文件
     * @param listCurEntry          当前选中的文件
     * @param listAllEntry          所有待提取文件
     * @param stOptions             提取参数
     */
    void signalExtract2Path(const QList<FileEntry> &listCurEntry, const QList<FileEntry> &listAllEntry, const ExtractionOptions &stOptions);

    /**
     * @brief signalDeleteFiles     从删除包删除文件
     * @param listEntry              待删除的数据
     */
    void signalDeleteFiles(const QList<FileEntry> &listEntry);

    /**
     * @brief signalOpenFile        打开压缩包中文件
     * @param strProgram            应用程序名
     * @param listEntry              待打开的文件数据
     */
    void signalOpenFile(const QString &strProgram, const QList<FileEntry> &listEntry);

    /**
     * @brief signalDelFiels    删除压缩包中文件
     * @param listCurEntry      当前选中的文件
     * @param listAllEntry      所有待删除文件
     * @param qTotalSize        删除文件总大小
     */
    void signalDelFiels(const QList<FileEntry> &listCurEntry, const QList<FileEntry> &listAllEntry, qint64 qTotalSize);

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

    /**
     * @brief slotDragPath  拖拽接收路径反馈
     * @param url
     */
    void slotDragPath(QUrl url);

private:
    ArchiveData m_stArchiveData;
    QMap<QString, QList<FileEntry>> m_mapShowEntry; // 显示数据（缓存，目录层级切换时从此处取数据，避免再次对总数据进行操作）
    FileEntry m_stRightEntry;       // 右键点击的文件
    QString m_strUnCompressPath;    // 默认解压路径
    QPoint m_dragPos; // 鼠标拖拽点击位置
    DFileDragServer *m_pFileDragServer = nullptr; // 文件拖拽服务


    QString m_strSelUnCompressPath;    // 选择的解压路径
};

#endif // UNCOMPRESSVIEW_H
