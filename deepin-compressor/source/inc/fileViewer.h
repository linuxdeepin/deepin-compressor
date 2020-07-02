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

#include <QFileInfo>
#include <QLineEdit>
#include <DTableView>
#include <DLabel>
#include <QItemDelegate>
#include <QPainter>
#include "myfilesystemmodel.h"
#include <DScrollBar>
#include <QStandardItemModel>
#include <dfiledragserver.h>
#include <dfiledrag.h>
#include "archivemodel.h"
#include <DMenu>
#include "mimetypedisplaymanager.h"
#include "archivesortfiltermodel.h"
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <QUrl>

#include "openwithdialog/openwithdialog.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

enum PAGE_TYPE {
    PAGE_COMPRESS,
    PAGE_UNCOMPRESS,
};

enum EXTRACT_TYPE {
    EXTRACT_HEAR,
    EXTRACT_TO,
    EXTRACT_DRAG,
    EXTRACT_TEMP,
    EXTRACT_TEMP_OPEN,
    EXTRACT_TEMP_CHOOSE_OPEN
};

class LogViewHeaderView;

class FirstRowDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    FirstRowDelegate(QObject *parent = nullptr);
    void setPathIndex(int *index);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void drawFocus(QPainter */*painter*/, const QStyleOptionViewItem &/*option*/,
                           const QRect &/*rect*/) const
    {
    }

    virtual void drawCheck(QPainter */*painter*/, const QStyleOptionViewItem &/*option*/,
                           const QRect &/*rect*/, Qt::CheckState /*state*/) const
    {
    }

protected:

private:
    int *ppathindex;
};

class MyTableView: public DTableView
{
    Q_OBJECT
public:
    MyTableView(QWidget *parent = nullptr);
    void setPreviousButtonVisible(bool visible);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

signals:
    void sigdragLeave(QString path);

public slots:
    void slotDragpath(QUrl url);

private:
    QPoint dragpos;
    DFileDragServer *s = nullptr;
    QString m_path;

public:
    LogViewHeaderView *header_;
};

struct SortInfo {
    Qt::SortOrder sortOrder;
    int sortRole;
};


class MimesAppsManager;
class fileViewer : public DWidget
{
    Q_OBJECT
public:
    fileViewer(QWidget *parent = nullptr, PAGE_TYPE type = PAGE_COMPRESS);

    void InitUI();
    void InitConnection();

    int getPathIndex();
    void setRootPathIndex();
    void setFileList(const QStringList &files);
    void setSelectFiles(const QStringList &files);
    int getFileCount();
    int getDeFileCount();
    void setDecompressModel(ArchiveSortFilterModel *model);

    QVector<Archive::Entry *> filesAndRootNodesForIndexes(const QModelIndexList &list) const;
    QVector<Archive::Entry *> filesForIndexes(const QModelIndexList &list) const;
    QModelIndexList addChildren(const QModelIndexList &list) const;

    void startDrag(Qt::DropActions supportedActions);

    void deleteCompressFile();
    void resetTempFile();

public slots:
    void showPlable();
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);

protected:
    void resizecolumn();
    void resizeEvent(QResizeEvent *size) override;

protected slots:
    void slotCompressRowDoubleClicked(const QModelIndex index);
    void slotDecompressRowDoubleClicked(const QModelIndex index);
    void slotCompressRePreviousDoubleClicked();
    int showWarningDialog(const QString &msg);

    void showRightMenu(const QPoint &pos);
    void onRightMenuClicked(QAction *action);
    void onRightMenuOpenWithClicked(QAction *action);
    void slotDragLeave(QString path);



signals:
    void sigFileRemoved(const QStringList &filelist);
    void sigextractfiles(QVector<Archive::Entry *> fileList, EXTRACT_TYPE type, QString path = "");
    void sigpathindexChanged();
    void sigOpenWith(QVector<Archive::Entry *> fileList, const QString &programma);

private:
    void refreshTableview();

    void restoreHeaderSort(const QString &currentPath);

    void updateAction(const QString &fileType);
    void updateAction(bool isdirectory, const QString &fileType);
    void openWithDialog(const QModelIndex &index);
    void openWithDialog(const QModelIndex &index, const QString &programma);

    void keyPressEvent(QKeyEvent *event) override;
    void openTempFile(QString path);

    void combineEntryDirectory(Archive::Entry *entry, QString &pathstr);
private:
    QLineEdit *pLineEditDir;
    MyTableView *pTableViewFile;

    int m_pathindex;
    MyFileSystemModel *pModel;

    QStandardItemModel *firstmodel;
    QItemSelectionModel *firstSelectionModel = nullptr;

    ArchiveModel *m_decompressmodel;
    ArchiveSortFilterModel *m_sortmodel;
    FirstRowDelegate *pdelegate;
    //MyLabel *plabel;
    QModelIndex m_indexmode;
    QFileInfoList m_curfilelist;

//    QFileInfoList m_parentFileList;
//    QDir m_parentDir;

    bool curFileListModified = true;
    QList<int> m_fileaddindex;
    DMenu *m_pRightMenu = nullptr;
    DMenu *openWithDialogMenu  = nullptr;

    PAGE_TYPE m_pagetype;
    MimeTypeDisplayManager *m_mimetype;

    QMap<QString, SortInfo> sortCache_;
    QAction *deleteAction;

    int openFileTempLink = 0;

};

#endif // FILEVIWER_H
