/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
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
#include <QDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QTableView>
#include <DLabel>
#include <QItemDelegate>
#include <QApplication>
#include <QPainter>
#include "myfilesystemmodel.h"
#include <QPushButton>
#include <QScrollBar>
#include <QDateTime>
#include <QStandardItemModel>
#include <dfiledragserver.h>
#include <dfiledrag.h>
#include "archivemodel.h"
#include <DMenu>
#include "mimetypedisplaymanager.h"
#include "archivesortfiltermodel.h"
#include <DApplicationHelper>
#include <DFontSizeManager>

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
};

class MyScrollBar: public QScrollBar
{
    Q_OBJECT
public:
    MyScrollBar(QWidget *parent = nullptr);
protected:
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;
signals:
    void ScrollBarShowEvent(QShowEvent *event);
    void ScrollBarHideEvent(QHideEvent *event);
};

class MyLabel: public QLabel
{
    Q_OBJECT
public:
    MyLabel(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *e) override;
protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
signals:
    void labelDoubleClickEvent(QMouseEvent *event);
};

class FirstRowDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    FirstRowDelegate(QObject *parent =nullptr);
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

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

signals:
    void sigdragLeave(QString path);

public slots:
    void slotDragpath(QUrl url);

private:
    QPoint dragpos;
    DFileDragServer *s = nullptr;
    QString m_path;

};


class fileViewer : public QWidget
{
    Q_OBJECT
public:
    fileViewer(QWidget *parent = nullptr, PAGE_TYPE type = PAGE_COMPRESS);

    void InitUI();
    void InitConnection();

    int getPathIndex();
    void setFileList(const QStringList &files);
    void setDecompressModel(ArchiveSortFilterModel *model);

    QVector<Archive::Entry *> filesAndRootNodesForIndexes(const QModelIndexList &list) const;
    QVector<Archive::Entry *> filesForIndexes(const QModelIndexList &list) const;
    QModelIndexList addChildren(const QModelIndexList &list) const;

    void startDrag(Qt::DropActions supportedActions);
    void DeleteCompressFile();

public slots:
    void showPlable();
protected:
    void resizecolumn();
    void resizeEvent(QResizeEvent *size) override;

protected slots:
    void slotCompressRowDoubleClicked(const QModelIndex index);
    void slotDecompressRowDoubleClicked(const QModelIndex index);
    void slotCompressRePreviousDoubleClicked(QMouseEvent *event);
    void ScrollBarShowEvent(QShowEvent *event);
    void ScrollBarHideEvent(QHideEvent *event);
    void showRightMenu(const QPoint &pos);
    void onRightMenuClicked(QAction *action);
    void slotDragLeave(QString path);

signals:
    void sigFileRemoved(const QStringList &filelist);
    void sigextractfiles(QVector<Archive::Entry *> fileList, EXTRACT_TYPE type, QString path = nullptr);
    void sigpathindexChanged();

private:
    void refreshTableview();
    void keyPressEvent(QKeyEvent *event) override;

private:
    QLineEdit *pLineEditDir;
    MyTableView *pTableViewFile;

    int m_pathindex;
    MyFileSystemModel *pModel;
    QStandardItemModel *firstmodel;
    ArchiveModel *m_decompressmodel;
    ArchiveSortFilterModel *m_sortmodel;
    FirstRowDelegate *pdelegate;
    MyLabel *plabel;
    QModelIndex m_indexmode;
    MyScrollBar *pScrollbar;
    QFileInfoList m_curfilelist;
    QList<int> m_fileaddindex;
    DMenu *m_pRightMenu = nullptr;

    PAGE_TYPE m_pagetype;
    MimeTypeDisplayManager *m_mimetype;


};

#endif // FILEVIWER_H
