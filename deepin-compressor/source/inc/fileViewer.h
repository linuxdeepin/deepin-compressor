/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#ifndef FILEVIWER_H
#define FILEVIWER_H

#include "archiveentry.h"

//#include <QFileInfo>
//#include <QLineEdit>
#include <DTableView>
#include <DLabel>
#include <DFileDragServer>
#include <DMenu>

#include <QItemDelegate>
#include <QUrl>
#include <QReadWriteLock>
#include <QFileInfoList>
//#include <QPainter>
//#include "myfilesystemmodel.h"
//#include <DScrollBar>
//#include <QStandardItemModel>
//#include <dfiledragserver.h>
//#include <dfiledrag.h>
//#include "archivemodel.h"
//#include <DApplicationHelper>
//#include <DFontSizeManager>
//#include <QReadWriteLock>
//#include "openwithdialog/openwithdialog.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

// 列表显示类型
enum PAGE_TYPE {
    PAGE_COMPRESS,
    PAGE_UNCOMPRESS,
};

// 解压类型
enum EXTRACT_TYPE {
    EXTRACT_HEAR,
    EXTRACT_TO,
    EXTRACT_DRAG,
    EXTRACT_TEMP,
    EXTRACT_TEMP_OPEN,
    EXTRACT_TEMP_CHOOSE_OPEN,
    EXTRACT_DELETE
};

// 子菜单操作类型
enum SUBACTION_MODE {
    ACTION_INVALID,
    ACTION_DRAG,
    ACTION_DELETE,
    ACTION_OPEN
};

// 子菜单信息
struct SubActionInfo {
    SubActionInfo()
        : mode(ACTION_INVALID)
    {

    }
    SUBACTION_MODE mode;
    QString archive;
    QString packageFile;
    QStringList ActionFiles;
};

class LogViewHeaderView;
class QLineEdit;
class MyFileSystemModel;
class QStandardItemModel;
class ArchiveModel;
class MimeTypeDisplayManager;
class ArchiveSortFilterModel;
class MyTableView;

class FirstRowDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    FirstRowDelegate(MyTableView *pTableView, QObject *parent = nullptr);
    void setPathIndex(int *index);
    /**
     * @brief paint     绘制显示效果
     * @param painter   画笔
     * @param option    item
     * @param index     索引值
     */
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void drawFocus(QPainter *, const QStyleOptionViewItem &,
                           const QRect &) const
    {
    }

    virtual void drawCheck(QPainter */*painter*/, const QStyleOptionViewItem &/*option*/,
                           const QRect &/*rect*/, Qt::CheckState /*state*/) const
    {
    }

    /**
     * @brief drawBackground    绘制焦点外框
     */
    virtual void drawBackground(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;

protected:

private:
    int *ppathindex; // 目录层级
    MyTableView *m_pTableView; // 列表
};

class MyTableView: public DTableView
{
    Q_OBJECT
public:
    MyTableView(QWidget *parent = nullptr);
    void setPreviousButtonVisible(bool visible);

    Qt::FocusReason m_reson;

protected:
    bool event(QEvent *event) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;

signals:
    void sigdragLeave(QString path);
    void signalDrop(QStringList file);
    void signalDoubleClicked(const QModelIndex &);

public slots:
//    void slotDragpath(QUrl url);

private:
    // get parent archive::entry pointer
    Archive::Entry *getParentArchiveEntry();
private:
    QPointF touchpos; // 触摸屏点击位置
    QPoint dragpos; // 鼠标拖拽点击位置
    DFileDragServer *s = nullptr; // 文件拖拽服务
    //QString m_path; // 拖拽路径
    bool m_isPressed = false; // 触摸按下标志  true: 按下; false: 松开
    // 记录触摸按下事件，在mouse move事件中使用，用于判断手指移动的距离，当大于
    // QPlatformTheme::TouchDoubleTapDistance 的值时认为触发触屏滚动
    //QPoint lastTouchBeginPos;
    // QPointF m_lastTouchBeginPos;
    QTime m_lastTouchTime; // 触摸屏最后触摸时间

public:
    //bool m_bTouch = false;
    //bool m_bDrag = false;

public:
    LogViewHeaderView *header_; // 表头

    // QWidget interface
protected:
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
};

// 排序信息
struct SortInfo {
    Qt::SortOrder sortOrder;
    int sortRole;
};


class MimesAppsManager;
class fileViewer : public DWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.archive.fileViewer.registry")
public:
    fileViewer(QWidget *parent = nullptr, PAGE_TYPE type = PAGE_COMPRESS);

    /**
     * @brief InitUI    初始化界面
     */
    void InitUI();

    /**
     * @brief InitConnection    初始化信号槽
     */
    void InitConnection();

    /**
     * @brief getPathIndex  获取目录层级
     * @return  目录层级
     */
    int getPathIndex();

    /**
     * @brief setRootPathIndex  设置显示根节点
     */
    void setRootPathIndex();

    /**
     * @brief setFileList   设置压缩文件（根目录显示）
     * @param files 文件名（含路径）
     */
    void setFileList(const QStringList &files);

    /**
     * @brief setSelectFiles    设置列表选中文件
     * @param files 待选中的文件名
     */
    void setSelectFiles(const QStringList &files);

    /**
     * @brief getFileCount  获取压缩文件数目
     * @return
     */
    int getFileCount();

    /**
     * @brief getDeFileCount    获取当前解压列表文件数目
     * @return 当前解压列表文件数目
     */
    int getDeFileCount();

    /**
     * @brief setDecompressModel    设置解压数据模型
     * @param model
     */

    void setDecompressModel(ArchiveSortFilterModel *model);
    /**
     * @brief selectRowByEntry
     * @param pSelectedEntry
     * @see 选中指定的Entry
     * @author hsw
     */
    void selectRowByEntry(Archive::Entry *pSelectedEntry);

    /**
     * @brief isDropAdd 判断追加压缩是否是拖拽追加
     * @return  是否是拖拽追加
     */
    bool isDropAdd();

    /**
     * @brief filesAndRootNodesForIndexes   获取选中索引的entry值
     * @param list  选中的索引
     * @return  选中的entry数据
     */
    QVector<Archive::Entry *> filesAndRootNodesForIndexes(const QModelIndexList &list) const;

    /**
     * @brief filesForIndexes   获取选中索引的entry值
     * @param list  选中的索引
     * @return  选中的entry数据
     */
    QVector<Archive::Entry *> filesForIndexes(const QModelIndexList &list) const;

    /**
     * @brief addChildren
     * @param list
     * @return
     */
    QModelIndexList addChildren(const QModelIndexList &list) const;

    // void startDrag(Qt::DropActions supportedActions);

    /**
     * @brief deleteCompressFile    删除选中的压缩文件
     */
    void deleteCompressFile();

    /**
     * @brief resetTempFile 重置临时路径标志
     */
    void resetTempFile();

    /**
     * @brief subWindowChangedMsg   子窗口变化消息处理
     * @param mode  变化模式
     * @param msg   消息内容
     */
    void subWindowChangedMsg(const SUBACTION_MODE &mode, const QStringList &msg);

    /**
     * @brief upDateArchive 更新压缩包数据（子窗口更新通知）
     * @param dragInfo  子窗口操作信息
     */
    void upDateArchive(const SubActionInfo &dragInfo);

    /**
     * @brief getTableView  获取列表控件
     * @return
     */
    MyTableView *getTableView();

    /**
     * @brief setLoadFilePath   设置加载的压缩包路径
     */
    void setLoadFilePath(const QString &);

public slots:
    /**
     * @brief showPlable    显示表头和上一级按钮
     */
    void showPlable();

    /**
     * @brief onSortIndicatorChanged    表头排序点击变化
     * @param logicalIndex  列
     * @param order 升序/降序
     */
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);

    /**
     * @brief clickedSlot
     * @param index
     * @param text
     */
    //void clickedSlot(int index, const QString &text);

    /**
     * @brief SubWindowDragMsgReceive   子窗口拖拽消息接收处理
     * @param mode  消息模式
     * @param urls  消息内容
     */
    void SubWindowDragMsgReceive(int mode, const QStringList &urls);

protected:
    /**
     * @brief resizecolumn  重置列宽
     */
    void resizecolumn();
    void resizeEvent(QResizeEvent *size) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

protected slots:
    /**
     * @brief slotCompressRowDoubleClicked  压缩列表双击处理
     * @param index 双击的index
     */
    void slotCompressRowDoubleClicked(const QModelIndex index);

    /**
     * @brief slotDecompressRowDoubleClicked    解压列表双击处理
     * @param index 双击的index
     */
    void slotDecompressRowDoubleClicked(const QModelIndex index);

    /**
     * @brief slotCompressRePreviousDoubleClicked   压缩列表返回上一级处理
     */
    void slotCompressRePreviousDoubleClicked();

    /**
     * @brief slotDecompressRowDelete   解压列表删除处理
     */
    void slotDecompressRowDelete();
    //int showWarningDialog(const QString &msg);

    /**
     * @brief showRightMenu 显示右键菜单
     * @param pos   鼠标点击位置
     */
    void showRightMenu(const QPoint &pos);

    /**
     * @brief onRightMenuClicked    右键菜单选项点击处理
     * @param action    点击的选项
     */
    void onRightMenuClicked(QAction *action);

    /**
     * @brief onRightMenuOpenWithClicked    右键菜单打开方式点击处理
     * @param action    点击的选项
     */
    void onRightMenuOpenWithClicked(QAction *action);

    /**
     * @brief slotDragLeave 拖拽离开处理
     * @param path  目标路径
     */
    void slotDragLeave(QString path);

    /**
     * @brief onDropSlot    拖拽放下处理
     * @param files 拖拽的文件
     */
    void onDropSlot(QStringList files);

    /**
     * @brief slotDeletedFinshedAddStart    删除操作结束处理，删除之后再添加
     * @param pWorkEntry    需要添加的数据
     */
    void slotDeletedFinshedAddStart(Archive::Entry *pWorkEntry);


signals:
    /**
     * @brief sigFileRemoved    删除文件信号
     */
    void sigFileRemoved(const QStringList &);

    /**
     * @brief sigEntryRemoved
     * @param vectorDel
     * @param isManual,true:by action clicked; false: by message emited.
     */
    void sigEntryRemoved(QVector<Archive::Entry *> &vectorDel, bool isManual);

    /**
     * @brief sigFileRemovedFromArchive     从包中删除文件的信号
     */
    void sigFileRemovedFromArchive(const QStringList &, const QString &);

    /**
     * @brief sigextractfiles   解压信号
     * @param fileList  待解压的文件
     * @param type  解压类型
     * @param path  解压目标路径
     */
    void sigextractfiles(QVector<Archive::Entry *> fileList, EXTRACT_TYPE type, QString path = "");

    /**
     * @brief sigpathindexChanged   目录层级变化信号
     */
    void sigpathindexChanged();

    /**
     * @brief sigOpenWith   以...打开信号
     * @param fileList  需要打开的文件
     * @param programma 应用程序
     */
    void sigOpenWith(QVector<Archive::Entry *> fileList, const QString &programma);

    /**
     * @brief sigFileAutoCompress   追加压缩信号
     * @param pWorkEntry    新添加的文件
     */
    void sigFileAutoCompress(const QStringList &, Archive::Entry *pWorkEntry = nullptr);

    /**
     * @brief sigNeedConvert   rar需要转换信号
     */
    void sigNeedConvert();
//    void sigFileAutoCompressToArchive(const QStringList &, const QString &);//废弃，added by hsw 20200528

private:
    /**
     * @brief refreshTableview  刷新列表数据
     */
    void refreshTableview();

    /**
     * @brief restoreHeaderSort 重置列表排序
     * @param currentPath   当前目录
     */
    void restoreHeaderSort(const QString &currentPath);

    /**
     * @brief updateAction  动态更新打开方式菜单
     * @param fileType      文件类型
     */
    void updateAction(const QString &fileType);

    /**
     * @brief updateAction  动态更新打开方式菜单
     * @param isdirectory   是否为文件夹
     * @param fileType      文件类型
     */
    void updateAction(bool isdirectory, const QString &fileType);

    /**
     * @brief 更新打开方式菜单
     */
    bool updateOpenWithDialogMenu(QModelIndex &curindex);

    /**
     * @brief openWithDialog    使用默认打开方式打开
     * @param index     右键点击的索引
     */
    void openWithDialog(const QModelIndex &index);

    /**
     * @brief openWithDialog    使用选择的应用程序打开
     * @param index     右键点击的索引
     * @param programma 选择的应用程序
     */
    void openWithDialog(const QModelIndex &index, const QString &programma);

    /**
     * @brief popUpDialog   显示提示信息对话框
     * @param desc  描述信息
     * @return  窗口操作
     */
    int popUpDialog(const QString &desc);

    /**
     * @brief openTempFile  打开临时路径文件
     * @param path  文件路径
     */
    void openTempFile(QString path);

    /**
     * @brief showWarningDialog 显示警告对话框
     * @param msg   警告信息
     * @return  窗口操作
     */
    int showWarningDialog(const QString &msg);

    /**
     * @brief selectedEntriesVector
     * @return
     */
    //QVector<Archive::Entry *> selectedEntriesVector();

    /**
     * @brief combineEntryDirectory 组合文件路径和文件名
     * @param entry 对应索引值所存储的entry
     * @param pathstr   文件名
     */
    void combineEntryDirectory(Archive::Entry *entry, QString &pathstr);

    /**
     * @brief getSelEntries  获取选择的index对应的entry
     * @return  选择的entry
     */
    QVector<Archive::Entry *> getSelEntries();

private:
    //QLineEdit *pLineEditDir;
    MyTableView *pTableViewFile; // 显示列表

    int m_pathindex; // 目录层级
    MyFileSystemModel *pModel; // 压缩列表层级大于1时数据模型

    QStandardItemModel *firstmodel; // 压缩列表第一层级数据模型
    QItemSelectionModel *firstSelectionModel = nullptr; // 压缩列表第一层级选择项的数据模型

    ArchiveModel *m_decompressmodel; // 解压列表数据模型
    ArchiveSortFilterModel *m_sortmodel; // 解压列表排序模型
    FirstRowDelegate *pdelegate; // 列表代理
    //MyLabel *plabel;
    QModelIndex m_indexmode; // 父节点
    QFileInfoList m_curfilelist; // 待压缩文件/文件夹

    //    QFileInfoList m_parentFileList;
    //    QDir m_parentDir;

    bool curFileListModified = true; // 压缩文件是否修改
    //QList<int> m_fileaddindex;
    DMenu *m_pRightMenu = nullptr; // 右键菜单
    DMenu *openWithDialogMenu = nullptr; // 打开方式右键菜单

    PAGE_TYPE m_pagetype; // 显示类型（解压列表/压缩列表）
    MimeTypeDisplayManager *m_mimetype; // 文件显示类型管理器

    QMap<QString, SortInfo> sortCache_; // 排序信息
    QAction *deleteAction; // 删除操作选项

    bool isPromptDelete = false; // 是否提示删除
    SubActionInfo m_ActionInfo; // 子界面操作信息
    int openFileTempLink = 0; // 打开的临时文件计数
    QString m_loadPath = ""; // 加载的压缩包路径
    bool m_bDropAdd; // 是否是拖拽添加
};

#endif // FILEVIWER_H
