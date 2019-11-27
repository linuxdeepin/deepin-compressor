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
#include "fileViewer.h"
#include <QLayout>
#include <QDir>
#include <QDesktopServices>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>
#include <DPalette>
#include <QFileIconProvider>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDrag>
#include "utils.h"
#include "myfileitem.h"
#include "kprocess.h"


MyScrollBar::MyScrollBar(QWidget *parent)
    : QScrollBar(parent)
{

}

void MyScrollBar::showEvent(QShowEvent *event)
{
    emit ScrollBarShowEvent(event);
}

void MyScrollBar::hideEvent(QHideEvent *event)
{
    emit ScrollBarHideEvent(event);
}

MyLabel::MyLabel(QWidget *parent)
    : QLabel(parent)
{

}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit labelDoubleClickEvent(event);
}


FirstRowDelegate::FirstRowDelegate(QObject *parent)
    : QItemDelegate(parent)
{

}

void FirstRowDelegate::setPathIndex(int *index)
{
    ppathindex = index;
}

void FirstRowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int row = index.row();
    int x = option.rect.x();
    int y = option.rect.y();
    int width = option.rect.width();
    int height = option.rect.height();

    if (row == 0 && ppathindex && *ppathindex > 0) { //UE
        //选项
        QStyleOptionFrame *FrameOption = new QStyleOptionFrame();
        FrameOption->rect = QRect(x, y, width, height / 2);
        //绘制
        QApplication::style()->drawControl(QStyle::CE_ShapedFrame, FrameOption, painter);
        QStyleOptionViewItem loption = option;
        loption.rect = QRect(x, y + height / 2, width, height / 2);
        return QItemDelegate::paint(painter, loption, index);
    }

    return QItemDelegate::paint(painter, option, index);
}

MyTableView::MyTableView(QWidget *parent)
    : QTableView(parent)
{
    setMinimumSize(580, 300);
}

void MyTableView::paintEvent(QPaintEvent *e)
{
//    qDebug()<<y();
    QTableView::paintEvent(e);
}


void MyTableView::mousePressEvent(QMouseEvent *e)
{

    if (e->button() == Qt::MouseButton::LeftButton) {
        dragpos = e->pos();
    }
    QTableView::mousePressEvent(e);
}

void MyTableView::mouseMoveEvent(QMouseEvent *e)
{

    if (!(e->buttons() & Qt::MouseButton::LeftButton) || s) {
        return;
    }
    if ((e->pos() - dragpos).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    s = new DFileDragServer();
    DFileDrag *drag = new DFileDrag(this, s);
    QMimeData *m = new QMimeData();
    m->setText("your stuff here");
    drag->setMimeData(m);

//    connect(drag, &DFileDrag::targetUrlChanged, [drag] {
//        static_path = drag->targetUrl().toString();
//        qDebug()<<static_path;
//    });

    connect(drag, &DFileDrag::targetUrlChanged, this, &MyTableView::slotDragpath);
    drag->exec();


    s->setProgress(100);
    s->deleteLater();
    s = nullptr;
    qDebug() << "sigdragLeave";
    emit sigdragLeave(m_path);
    m_path.clear();
}

void MyTableView::slotDragpath(QUrl url)
{
    m_path = url.toLocalFile();
    qDebug() << m_path;
}

fileViewer::fileViewer(QWidget *parent, PAGE_TYPE type)
    : QWidget(parent), m_pagetype(type)
{
    setWindowTitle(tr("File Viewer"));
    setMinimumSize(580, 300);
    m_pathindex = 0;
    m_mimetype = new MimeTypeDisplayManager();
    InitUI();
    InitConnection();
}

fileViewer::~fileViewer()
{
}

void fileViewer::InitUI()
{
    QHBoxLayout *mainlayout = new QHBoxLayout;

    pTableViewFile = new MyTableView();
    pdelegate = new FirstRowDelegate();
    pdelegate->setPathIndex(&m_pathindex);
    pTableViewFile->setItemDelegate(pdelegate);
    plabel = new MyLabel(pTableViewFile);
    plabel->setFixedSize(580, 36);
    firstmodel = new QStandardItemModel();
    pModel = new MyFileSystemModel();
    pModel->setNameFilterDisables(false);
    pModel->setTableView(pTableViewFile);
    QStringList labels = QObject::trUtf8("Name,Size,Type,Modify").simplified().split(",");
    firstmodel->setHorizontalHeaderLabels(labels);

    pScrollbar = new MyScrollBar();
    pTableViewFile->setVerticalScrollBar(pScrollbar);
    pTableViewFile->setEditTriggers(QAbstractItemView::NoEditTriggers);
    pTableViewFile->horizontalHeader()->setStretchLastSection(true);
    pTableViewFile->setShowGrid(false);
    pTableViewFile->verticalHeader()->hide();
    pTableViewFile->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableViewFile->setAlternatingRowColors(true);
    pTableViewFile->verticalHeader()->setVisible(false);
    pTableViewFile->horizontalHeader()->setHighlightSections(false);  //防止表头塌陷
    pTableViewFile->setSortingEnabled(true);
    pTableViewFile->sortByColumn(0, Qt::AscendingOrder);

    QHeaderView *headerview = pTableViewFile->horizontalHeader();
    headerview->setMinimumHeight(MyFileSystemDefine::gTableHeight);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(headerview);
    pa.setBrush(DPalette::Background, pa.color(DPalette::Base));
    headerview->setPalette(pa);
    pTableViewFile->setFrameShape(DTableView::NoFrame);
    pTableViewFile->setSelectionMode(QAbstractItemView::ExtendedSelection);

    plabel->setText(".. " + tr("Back to previous"));
    plabel->setAutoFillBackground(true);
    plabel->hide();

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if(themeType == DGuiApplicationHelper::LightType)
    {
        DPalette pal;
        pal.setBrush(DPalette::Background,QColor(247,247,247));
        plabel->setPalette(pal);
        plabel->setBackgroundRole(DPalette::Background);
    }
    else if (themeType == DGuiApplicationHelper::DarkType) {
        DPalette pal;
        pal.setBrush(DPalette::Background,QColor(50,50,50));
        plabel->setPalette(pal);
        plabel->setBackgroundRole(DPalette::Background);
    }
    else {
        DPalette pal;
        pal.setBrush(DPalette::Background,QColor(247,247,247));
        plabel->setPalette(pal);
        plabel->setBackgroundRole(DPalette::Background);
    }

//    plabel->setGeometry(0, MyFileSystemDefine::gTableHeight, 580, MyFileSystemDefine::gTableHeight - 7);

    mainlayout->addWidget(pTableViewFile);
    setLayout(mainlayout);

    if (PAGE_UNCOMPRESS == m_pagetype) {
        pTableViewFile->setContextMenuPolicy(Qt::CustomContextMenu);
        m_pRightMenu = new DMenu();
        m_pRightMenu->setFixedWidth(200);
        m_pRightMenu->addAction(tr("Extract files"));
        m_pRightMenu->addAction(tr("Extract files here"));
        pTableViewFile->setDragDropMode(QAbstractItemView::DragDrop);
        pTableViewFile->setAcceptDrops(false);
    }

    pTableViewFile->setBackgroundRole(DPalette::Base);
    pTableViewFile->setAutoFillBackground(true);
    setBackgroundRole(DPalette::Base);

    refreshTableview();
}

void fileViewer::refreshTableview()
{
    MyFileItem *item = nullptr;
    firstmodel->clear();

    item = new MyFileItem(QObject::tr("Name"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(0, item);
    item = new MyFileItem(QObject::tr("Modify"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(1, item);
    item = new MyFileItem(QObject::tr("Type"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(2, item);
    item = new MyFileItem("      " + QObject::tr("Size"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(3, item);

    int rowindex = 0;
    QFileIconProvider icon_provider;
    foreach (QFileInfo fileinfo, m_curfilelist) {
        item = new MyFileItem(icon_provider.icon(fileinfo), "  " + fileinfo.fileName());

        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
        font.setWeight(QFont::Medium);
        item->setFont(font);

//        DPalette pal ;
//        pal.setBrush(DPalette::WindowText,pal.color(DPalette::WindowText));


        firstmodel->setItem(rowindex, 0, item);
        if (fileinfo.isDir()) {
//            item = new MyFileItem("-");
            QDir dir(fileinfo.filePath());
            QStringList filter;
            QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir.entryInfoList(filter));

            int count = fileInfo->count();
            if (count > 2) {
                count -= 2;
            }
            item = new MyFileItem(QString::number(count) + " " + tr("Item") +  "    ");
        } else {
            item = new MyFileItem(Utils::humanReadableSize(fileinfo.size(), 1) +  "    ");
        }
        font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
        font.setWeight(QFont::Normal);
        item->setFont(font);
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        firstmodel->setItem(rowindex, 3, item);
        QMimeType mimetype = determineMimeType(fileinfo.filePath());
        item = new MyFileItem(" " + m_mimetype->displayName(mimetype.name()));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
        font.setWeight(QFont::Normal);
        item->setFont(font);
        firstmodel->setItem(rowindex, 2, item);
        item = new MyFileItem(" " + QLocale().toString(fileinfo.lastModified(), tr("yyyy/MM/dd hh:mm:ss")));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
        font.setWeight(QFont::Normal);
        item->setFont(font);
        firstmodel->setItem(rowindex, 1, item);
        rowindex++;
    }

    pTableViewFile->setModel(firstmodel);
    resizecolumn();

    foreach (int row, m_fileaddindex) {
        pTableViewFile->selectRow(row);
    }
}



void fileViewer::InitConnection()
{
    // connect the signals to the slot function.
    if (PAGE_COMPRESS == m_pagetype) {
        connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotCompressRowDoubleClicked(const QModelIndex &)));
    } else {
        connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotDecompressRowDoubleClicked(const QModelIndex &)));
        connect(pTableViewFile, &MyTableView::customContextMenuRequested, this, &fileViewer::showRightMenu);
        connect(pTableViewFile, &MyTableView::sigdragLeave, this, &fileViewer::slotDragLeave);
        if (m_pRightMenu) {
            connect(m_pRightMenu, &DMenu::triggered, this, &fileViewer::onRightMenuClicked);
        }
    }
    connect(plabel, SIGNAL(labelDoubleClickEvent(QMouseEvent *)), this, SLOT(slotCompressRePreviousDoubleClicked(QMouseEvent *)));

    connect(pScrollbar, SIGNAL(ScrollBarShowEvent(QShowEvent *)), this, SLOT(ScrollBarShowEvent(QShowEvent *)));
    connect(pScrollbar, SIGNAL(ScrollBarHideEvent(QHideEvent *)), this, SLOT(ScrollBarHideEvent(QHideEvent *)));
    connect(pModel,&MyFileSystemModel::sigShowLabel , this, &fileViewer::showPlable);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &fileViewer::themeChanged);
}

void fileViewer::resizecolumn()
{
    qDebug() << pTableViewFile->width();
    pTableViewFile->setColumnWidth(0, pTableViewFile->width() * 14 / 29);
    pTableViewFile->setColumnWidth(1, pTableViewFile->width() * 8 / 29);
    pTableViewFile->setColumnWidth(2, pTableViewFile->width() * 3 / 29);
    pTableViewFile->setColumnWidth(3, pTableViewFile->width() * 3 / 29);
}

void fileViewer::resizeEvent(QResizeEvent *size)
{
    qDebug() << pTableViewFile->width();
    resizecolumn();
}


void fileViewer::keyPressEvent(QKeyEvent *event)
{
    if (!event) {
        return;
    }
    if (event->key() == Qt::Key_Delete && 0 == m_pathindex) {
        QItemSelectionModel *selections =  pTableViewFile->selectionModel();
        QModelIndexList selected = selections->selectedIndexes();

        QSet<unsigned int>  selectlist;

        foreach (QModelIndex index, selected) {
            selectlist.insert(index.row());
        }

        QStringList filelist;
        foreach (unsigned int index, selectlist) {
            m_curfilelist.replace(index, QFileInfo(""));
        }

        foreach (QFileInfo file, m_curfilelist) {
            if (file.path() == "") {
                m_curfilelist.removeOne(file);
            }
        }

        foreach (QFileInfo fileinfo, m_curfilelist) {
            filelist.append(fileinfo.filePath());
        }

        emit sigFileRemoved(filelist);
    }


}

int fileViewer::getPathIndex()
{
    return m_pathindex;
}

void fileViewer::setFileList(const QStringList &files)
{

    if (files.count() > m_curfilelist.count()) {
        m_fileaddindex.clear();
        for (int i = 0; i < files.count() - m_curfilelist.count(); i++) {
            m_fileaddindex.append(files.count() - 1 + i);
        }
    }


    m_curfilelist.clear();
    foreach (QString filepath, files) {
        QFile file(filepath);
        m_curfilelist.append(file);
    }
    refreshTableview();
}

void fileViewer::slotCompressRePreviousDoubleClicked(QMouseEvent *event)
{

    if (PAGE_COMPRESS == m_pagetype) {
        if (m_pathindex > 1) {
            m_pathindex--;
            m_indexmode = pModel->setRootPath(pModel->fileInfo(m_indexmode).path());
            qDebug() << pModel->fileInfo(m_indexmode).path();
            pTableViewFile->setRootIndex(m_indexmode);
        } else {
            m_pathindex--;
            refreshTableview();
            if (0 == m_pathindex) {
                plabel->hide();
                pTableViewFile->setRowHeight(0, MyFileSystemDefine::gTableHeight);
            }
        }
    } else {
        m_pathindex--;
        if (0 == m_pathindex) {
            pTableViewFile->setRootIndex(QModelIndex());
            plabel->hide();
            pTableViewFile->setRowHeight(0, ArchiveModelDefine::gTableHeight);
        } else {
            pTableViewFile->setRootIndex(m_sortmodel->mapFromSource(m_decompressmodel->parent(m_indexmode)));
        }
    }

    emit  sigpathindexChanged();
}

void fileViewer::showPlable()
{
    plabel->raise();
    plabel->show();
    plabel->move(0, 36);
}

void fileViewer::slotCompressRowDoubleClicked(const QModelIndex index)
{
    QModelIndex curindex = pTableViewFile->currentIndex();
    if (curindex.isValid()) {
        if (0 == m_pathindex) {
            if (m_curfilelist.at(curindex.row()).isDir()) {
                pModel->setPathIndex(&m_pathindex);
                pTableViewFile->setModel(pModel);
                m_indexmode = pModel->setRootPath(m_curfilelist.at(curindex.row()).filePath());
                pTableViewFile->setRootIndex(m_indexmode);
                m_pathindex++;

                resizecolumn();
            } else {
                if (Utils::isCompressed_file(m_curfilelist.at(curindex.row()).filePath())) {
                    KProcess *cmdprocess = new KProcess;
                    QStringList arguments;
                    QString programPath = QStandardPaths::findExecutable("deepin-compressor");
                    if (programPath.isEmpty()) {
                        qDebug() << "error can't find xdg-mime";
                        return;
                    }

                    arguments << m_curfilelist.at(curindex.row()).filePath();

                    qDebug() << arguments;
                    cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
                    cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
                    cmdprocess->setProgram(programPath, arguments);
                    cmdprocess->start();
                }
            }
        } else if (pModel && pModel->fileInfo(curindex).isDir()) {
            m_indexmode = pModel->setRootPath(pModel->fileInfo(curindex).filePath());
            pTableViewFile->setRootIndex(m_indexmode);
            m_pathindex++;
        } else if (pModel && !pModel->fileInfo(curindex).isDir()) {
            if (Utils::isCompressed_file(pModel->fileInfo(curindex).filePath())) {
                KProcess *cmdprocess = new KProcess;
                QStringList arguments;
                QString programPath = QStandardPaths::findExecutable("deepin-compressor");
                if (programPath.isEmpty()) {
                    qDebug() << "error can't find xdg-mime";
                    return;
                }
                arguments << pModel->fileInfo(curindex).filePath();
                qDebug() << arguments;
                cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
                cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
                cmdprocess->setProgram(programPath, arguments);
                cmdprocess->start();
            }
        }
    }

    emit sigpathindexChanged();
}

void fileViewer::slotDecompressRowDoubleClicked(const QModelIndex index)
{
    if (index.isValid()) {
        qDebug() << m_decompressmodel->isentryDir(m_sortmodel->mapToSource(index));
        if (0 == m_pathindex) {
            if (m_decompressmodel->isentryDir(m_sortmodel->mapToSource(index))) {
                m_decompressmodel->setPathIndex(&m_pathindex);
                QModelIndex sourceindex = m_decompressmodel->createNoncolumnIndex(m_sortmodel->mapToSource(index));
                pTableViewFile->setRootIndex(m_sortmodel->mapFromSource(sourceindex));
                m_pathindex++;
                m_indexmode = sourceindex;
            }
        } else if (m_decompressmodel->isentryDir(m_sortmodel->mapToSource(index))) {
            QModelIndex sourceindex = m_decompressmodel->createNoncolumnIndex(m_sortmodel->mapToSource(index));
            pTableViewFile->setRootIndex(m_sortmodel->mapFromSource(sourceindex));
            m_pathindex++;
            m_indexmode = sourceindex;
        }
    }
}

void fileViewer::ScrollBarShowEvent(QShowEvent *event)
{
    plabel->setGeometry(0, MyFileSystemDefine::gTableHeight, 1920, pTableViewFile->horizontalHeader()->height());
}
void fileViewer::ScrollBarHideEvent(QHideEvent *event)
{
    plabel->setGeometry(0, MyFileSystemDefine::gTableHeight, 1920, pTableViewFile->horizontalHeader()->height());

}

void fileViewer::showRightMenu(const QPoint &pos)
{
    if (!pTableViewFile->indexAt(pos).isValid()) {
        return;
    }

    m_pRightMenu->popup(QCursor::pos());

}

void fileViewer::slotDragLeave(QString path)
{
    if (path.isEmpty()) {
        qDebug() << "path is empty";
        return;
    }
    emit sigextractfiles(filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows())), EXTRACT_DRAG, path);
}

void fileViewer::onRightMenuClicked(QAction *action)
{
    if (action->text() == tr("Extract files")) {
        emit sigextractfiles(filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows())), EXTRACT_TO);
    } else {
        emit sigextractfiles(filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows())), EXTRACT_HEAR);
    }
}


QModelIndexList fileViewer::addChildren(const QModelIndexList &list) const
{
    Q_ASSERT(m_sortmodel);

    QModelIndexList ret = list;

    // Iterate over indexes in list and add all children.
    for (int i = 0; i < ret.size(); ++i) {
        QModelIndex index = ret.at(i);

        for (int j = 0; j < m_sortmodel->rowCount(index); ++j) {
            QModelIndex child = m_sortmodel->index(j, 0, index);
            if (!ret.contains(child)) {
                ret << child;
            }
        }
    }

    return ret;
}


QVector<Archive::Entry *> fileViewer::filesForIndexes(const QModelIndexList &list) const
{
    QVector<Archive::Entry *> ret;

    for (const QModelIndex &index : list) {
        ret << m_decompressmodel->entryForIndex(m_sortmodel->mapToSource(index));
    }

    return ret;
}

QVector<Archive::Entry *> fileViewer::filesAndRootNodesForIndexes(const QModelIndexList &list) const
{
    QVector<Archive::Entry *> fileList;
    QStringList fullPathsList;

    for (const QModelIndex &index : list) {
        QModelIndex selectionRoot = index.parent();
        while (pTableViewFile->selectionModel()->isSelected(selectionRoot) ||
                list.contains(selectionRoot)) {
            selectionRoot = selectionRoot.parent();
        }

        // Fetch the root node for the unselected parent.
        const QString rootFileName = selectionRoot.isValid()
                                     ? m_decompressmodel->entryForIndex(m_sortmodel->mapToSource(selectionRoot))->fullPath()
                                     : QString();


        // Append index with root node to fileList.
        QModelIndexList alist = QModelIndexList() << index;
        const auto filesIndexes = filesForIndexes(alist);
        for (Archive::Entry *entry : filesIndexes) {
            const QString fullPath = entry->fullPath();
            if (!fullPathsList.contains(fullPath)) {
                entry->rootNode = rootFileName;
                fileList.append(entry);
                fullPathsList.append(fullPath);
            }
        }
    }
    return fileList;
}


void fileViewer::setDecompressModel(ArchiveSortFilterModel *model)
{
    m_pathindex = 0;
    m_indexmode = QModelIndex();
    m_sortmodel = model;

    m_decompressmodel = (ArchiveModel *)m_sortmodel->sourceModel();
    m_decompressmodel->setPathIndex(&m_pathindex);
    m_decompressmodel->setTableView(pTableViewFile);
    connect(m_decompressmodel,&ArchiveModel::sigShowLabel , this, &fileViewer::showPlable);

    pTableViewFile->setModel(model);
    resizecolumn();

}

void fileViewer::startDrag(Qt::DropActions supportedActions)
{
    QMimeData *mimeData = new QMimeData;
    connect(mimeData, SIGNAL(dataRequested(QString)),
            this, SLOT(createData(QString)), Qt::DirectConnection);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}

void fileViewer::themeChanged(){
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if(themeType == DGuiApplicationHelper::LightType)
    {
        DPalette pal;
        pal.setBrush(DPalette::Background,QColor(247,247,247));
        plabel->setPalette(pal);
        plabel->setBackgroundRole(DPalette::Background);
    }
    else if (themeType == DGuiApplicationHelper::DarkType) {
        DPalette pal;
        pal.setBrush(DPalette::Background,QColor(50,50,50));
        plabel->setPalette(pal);
        plabel->setBackgroundRole(DPalette::Background);
    }
    else {
        DPalette pal;
        pal.setBrush(DPalette::Background,QColor(247,247,247));
        plabel->setPalette(pal);
        plabel->setBackgroundRole(DPalette::Background);
    }
}

