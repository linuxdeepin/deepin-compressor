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
#include <QApplication>
#include <QIcon>
#include <DStandardPaths>

#include "fileViewer.h"
#include "utils.h"
#include "myfileitem.h"
#include "kprocess.h"
#include "logviewheaderview.h"
#include "mimetypes.h"
#include "mainwindow.h"
#include "openwithdialog/openwithdialog.h"


const QString rootPathUnique = "_&_&_&_";
const QString zipPathUnique = "_&_&_";

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

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
//    painter->setOpacity(1);
    QPainterPath path;
    QPainterPath clipPath;
    QRect rect = option.rect;
    rect.setY(rect.y() + 1);
    rect.setHeight(rect.height() - 1);

    if (index.column() == 0) {
        rect.setX(rect.x());  // left margin
        QPainterPath rectPath, roundedPath;
        roundedPath.addRoundedRect(rect.x(), rect.y(), rect.width() * 2, rect.height(), 8, 8);
        rectPath.addRect(rect.x() + rect.width(), rect.y(), rect.width(), rect.height());
        clipPath = roundedPath.subtracted(rectPath);
        painter->setClipPath(clipPath);
        path.addRect(rect);
    } else if (index.column() == 3) {
        rect.setWidth(rect.width());  // right margin
        QPainterPath rectPath, roundedPath;
        roundedPath.addRoundedRect(rect.x() - rect.width(), rect.y(), rect.width() * 2, rect.height(), 8, 8);
        rectPath.addRect(rect.x() - rect.width(), rect.y(), rect.width(), rect.height());
        clipPath = roundedPath.subtracted(rectPath);
        painter->setClipPath(clipPath);
        path.addRect(rect);
    } else {
        path.addRect(rect);
    }

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    if (ppathindex && *ppathindex > 0) {
        if (row % 2) {
            painter->fillPath(path, palette.alternateBase());
        }
    } else {
        if (!(row % 2)) {
            painter->fillPath(path, palette.alternateBase());
        }
    }

    if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
        painter->fillPath(path, palette.highlight());
    }
    QStyleOptionViewItem opt = setOptions(index, option);

    // prepare
    // get the data and the rectangles
    QVariant value;
    QPixmap pixmap;
    QIcon icon;
    QRect decorationRect;
    value = index.data(Qt::DecorationRole);
    if (value.isValid()) {
        // ### we need the pixmap to call the virtual function
        pixmap = decoration(opt, value);
        if (value.type() == QVariant::Icon) {
//            QIcon icon = qvariant_cast<QIcon>(value);
//            decorationRect = QRect(QPoint(10, 0), pixmap.size());
            icon = qvariant_cast<QIcon>(value);
            decorationRect = QRect(QPoint(0, 0), QSize(24, 24));
        } else {
//            decorationRect = QRect(QPoint(10, 0), pixmap.size());
            icon = QIcon(pixmap);
            decorationRect = QRect(QPoint(0, 0), QSize(24, 24));
        }
    } else {
        decorationRect = QRect();
    }

    QRect checkRect;
    Qt::CheckState checkState = Qt::Unchecked;
    value = index.data(Qt::CheckStateRole);
    if (value.isValid()) {
        checkState = static_cast<Qt::CheckState>(value.toInt());
        checkRect = doCheck(opt, opt.rect, value);
    }
    QFontMetrics fm(opt.font);
    QRect displayRect;
    displayRect.setX(decorationRect.x() + decorationRect.width());
    displayRect.setWidth(opt.rect.width() - decorationRect.width() - decorationRect.x());
    QString text = fm.elidedText(index.data(Qt::DisplayRole).toString(), opt.textElideMode, displayRect.width() - 18);

    // do the layout
    doLayout(opt, &checkRect, &decorationRect, &displayRect, false);
    if (index.column() == 0 /*&& decorationRect.x() < 10*/) {
//        decorationRect.setX(decorationRect.x() + 23);
//        displayRect.setX(displayRect.x() + 10);
//        displayRect.setWidth(displayRect.width() - 10);
        decorationRect.setX(decorationRect.x() + 8);
        decorationRect.setWidth(decorationRect.width() + 8);
        displayRect.setX(displayRect.x() + 16);
    } else {
        displayRect.setX(displayRect.x() + 6);
    }

    // draw the item
    //drawBackground(painter, opt, index);
    drawCheck(painter, opt, checkRect, checkState);
//    drawDecoration(painter, opt, decorationRect, pixmap);
    icon.paint(painter, decorationRect);

    //drawDisplay(painter, opt, displayRect, text);
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }
    if (text.isEmpty() == false) {
        if (index.column() == 0) {
            QFont pFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);
            pFont.setWeight(QFont::Weight::Medium);
            painter->setFont(pFont);
        } else {
            QFont pFont = DFontSizeManager::instance()->get(DFontSizeManager::T7);
            pFont.setWeight(QFont::Weight::Normal);
            painter->setFont(pFont);
        }
        painter->drawText(displayRect, static_cast<int>(opt.displayAlignment), text);
    }
    drawFocus(painter, opt, displayRect);

    // done
    painter->restore();

//    if(index.column() == 0)
//    {
//        QStyleOptionViewItem loption = option;
//        loption.rect.setX(loption.rect.x() + 6);
//        return QItemDelegate::paint(painter, loption, index);;
//    }
//    return QItemDelegate::paint(painter, option, index);
}


MyTableView::MyTableView(QWidget *parent)
    : DTableView(parent)
{
    setMinimumSize(580, 300);
    header_ = new LogViewHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(header_);

    auto changeTheme = [this]() {
        DPalette pa = palette();
        //pa.setBrush(DPalette::ColorType::ItemBackground, pa.highlight());
        pa.setBrush(QPalette::Highlight, pa.base());
        pa.setBrush(DPalette::AlternateBase, pa.base());
        //pa.setBrush(QPalette::Base, pa.highlight());
        setPalette(pa);
        //update();
    };

    changeTheme();
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, changeTheme);

    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
}

void MyTableView::setPreviousButtonVisible(bool visible)
{
    header_->gotoPreviousLabel_->setVisible(visible);
    updateGeometries();
}

void MyTableView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MouseButton::LeftButton) {
        dragpos = e->pos();
    }
    DTableView::mousePressEvent(e);
}

void MyTableView::mouseMoveEvent(QMouseEvent *e)
{
    if (dragEnabled() == false) {
        return;
    }

    QModelIndexList lst = selectedIndexes();

    if (lst.size() < 1) {
        return;
    }

    if (!(e->buttons() & Qt::MouseButton::LeftButton) || s) {
        return;
    }
    if ((e->pos() - dragpos).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    s = new DFileDragServer(this);
    DFileDrag *drag = new DFileDrag(this, s);
    QMimeData *m = new QMimeData();
    //m->setText("your stuff here");

    QVariant value = lst[0].data(Qt::DecorationRole);

    if (value.isValid()) {
        if (value.type() == QVariant::Pixmap) {
            drag->setPixmap(qvariant_cast<QPixmap>(value));
        } else if (value.type() == QVariant::Icon) {
            drag->setPixmap((qvariant_cast<QIcon>(value)).pixmap(24, 24));
        }
    }

    drag->setMimeData(m);

//    connect(drag, &DFileDrag::targetUrlChanged, [drag] {
//        static_path = drag->targetUrl().toString();
//        qDebug()<<static_path;
//    });

    connect(drag, &DFileDrag::targetUrlChanged, this, &MyTableView::slotDragpath);
    Qt::DropAction result = drag->exec(Qt::CopyAction);


    s->setProgress(100);
    s->deleteLater();
    s = nullptr;
    qDebug() << "sigdragLeave";

    if (result == Qt::DropAction::CopyAction) {
        emit sigdragLeave(m_path);
    }

    m_path.clear();
}

void MyTableView::slotDragpath(QUrl url)
{
    m_path = url.toLocalFile();
    qDebug() << m_path;
}


fileViewer::fileViewer(QWidget *parent, PAGE_TYPE type)
    : DWidget(parent), m_pagetype(type)
{
    //setWindowTitle(tr("File Viewer"));
    setMinimumSize(580, 300);
    m_pathindex = 0;
    m_mimetype = new MimeTypeDisplayManager(this);
    InitUI();
    InitConnection();

    if (PAGE_COMPRESS == type) {
        pTableViewFile->setDragEnabled(false);
    }
}

void fileViewer::InitUI()
{
    QHBoxLayout *mainlayout = new QHBoxLayout;

    pTableViewFile = new MyTableView(this);


    connect(pTableViewFile->header_->gotoPreviousLabel_, SIGNAL(doubleClickedSignal()), this, SLOT(slotCompressRePreviousDoubleClicked()));
    connect(pTableViewFile->header_, &QHeaderView::sortIndicatorChanged, this, &fileViewer::onSortIndicatorChanged);

    pTableViewFile->verticalHeader()->setDefaultSectionSize(MyFileSystemDefine::gTableHeight);
    pdelegate = new FirstRowDelegate(this);
    pdelegate->setPathIndex(&m_pathindex);
    pTableViewFile->setItemDelegate(pdelegate);
//    plabel = new MyLabel(pTableViewFile);
//    plabel->setFixedSize(580, 36);
    firstmodel = new QStandardItemModel(this);
    firstSelectionModel = new QItemSelectionModel(firstmodel);

    pModel = new MyFileSystemModel(this);
    pModel->setNameFilterDisables(false);
    pModel->setTableView(pTableViewFile);
    QStringList labels = QString("Name,Size,Type,Time modified").simplified().split(",");
    firstmodel->setHorizontalHeaderLabels(labels);

    pTableViewFile->setEditTriggers(QAbstractItemView::NoEditTriggers);
    pTableViewFile->horizontalHeader()->setStretchLastSection(true);
    pTableViewFile->setShowGrid(false);
    pTableViewFile->verticalHeader()->hide();
    pTableViewFile->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableViewFile->setAlternatingRowColors(true);
    pTableViewFile->verticalHeader()->setVisible(false);
    pTableViewFile->horizontalHeader()->setHighlightSections(false);  //防止表头塌陷
    pTableViewFile->setSortingEnabled(true);
    //pTableViewFile->sortByColumn(0, Qt::AscendingOrder);
    pTableViewFile->setIconSize(QSize(24, 24));

    //QHeaderView *headerview = pTableViewFile->horizontalHeader();
    //headerview->setMinimumHeight(MyFileSystemDefine::gTableHeight);
//    DPalette pa;
//    pa = DApplicationHelper::instance()->palette(headerview);
//    pa.setBrush(DPalette::Background, pa.color(DPalette::Base));
//    headerview->setPalette(pa);
    pTableViewFile->setFrameShape(DTableView::NoFrame);
    pTableViewFile->setSelectionMode(QAbstractItemView::ExtendedSelection);
//    plabel->setText("     .. " + tr("Back"));
//    DFontSizeManager::instance()->bind(plabel, DFontSizeManager::T6, QFont::Weight::Medium);
////    plabel->setAutoFillBackground(true);
//    plabel->hide();

//    plabel->setGeometry(0, MyFileSystemDefine::gTableHeight, 580, MyFileSystemDefine::gTableHeight - 7);
    mainlayout->addWidget(pTableViewFile);
    setLayout(mainlayout);


    if (PAGE_UNCOMPRESS == m_pagetype) {
        pTableViewFile->setContextMenuPolicy(Qt::CustomContextMenu);
        m_pRightMenu = new DMenu(this);
        m_pRightMenu->setMinimumWidth(202);
        m_pRightMenu->addAction(tr("Extract", "slotDecompressRowDoubleClicked"));
        m_pRightMenu->addAction(tr("Extract to current directory"));
        m_pRightMenu->addAction(tr("Open"));

        openWithDialogMenu = new  DMenu(tr("Open style"), this);
        m_pRightMenu->addMenu(openWithDialogMenu);


        pTableViewFile->setDragDropMode(QAbstractItemView::DragDrop);
        pTableViewFile->setAcceptDrops(false);
    }
    if (PAGE_COMPRESS == m_pagetype) {
        pTableViewFile->setContextMenuPolicy(Qt::CustomContextMenu);
        m_pRightMenu = new DMenu(this);
        m_pRightMenu->setMinimumWidth(202);
        deleteAction = new QAction(tr("Delete"), this);
        m_pRightMenu->addAction(deleteAction);
        m_pRightMenu->addAction(tr("Open"));

        openWithDialogMenu = new  DMenu(tr("Open style"), this);
        m_pRightMenu->addMenu(openWithDialogMenu);
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
    if (false == curFileListModified) {
        pTableViewFile->setModel(firstmodel);
        pTableViewFile->setSelectionModel(firstSelectionModel);

        restoreHeaderSort(rootPathUnique);
        return;
    }


    curFileListModified = false;

    MyFileItem *item = nullptr;
    firstmodel->clear();

    item = new MyFileItem(QObject::tr("Name"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(0, item);
    item = new MyFileItem(QObject::tr("Time modified"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(1, item);
    item = new MyFileItem(QObject::tr("Type"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(2, item);
    item = new MyFileItem(QObject::tr("Size"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(3, item);

    int rowindex = 0;
    QFileIconProvider icon_provider;
    foreach (QFileInfo fileinfo, m_curfilelist) {
        item = new MyFileItem(icon_provider.icon(fileinfo), fileinfo.fileName());

        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
        font.setWeight(QFont::Medium);
        item->setFont(font);

        firstmodel->setItem(rowindex, 0, item);
        if (fileinfo.isDir()) {
//            item = new MyFileItem("-");
            QDir dir(fileinfo.filePath());
//            QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir.entryInfoList(QDir::NoDotDot));

            item = new MyFileItem(QString::number(dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files).count()) + " " + tr("item(s)"));
        } else {
            item = new MyFileItem(Utils::humanReadableSize(fileinfo.size(), 1));
        }
        font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
        font.setWeight(QFont::Normal);
        item->setFont(font);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex, 3, item);
        QMimeType mimetype = determineMimeType(fileinfo.filePath());
        item = new MyFileItem(m_mimetype->displayName(mimetype.name()));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
        font.setWeight(QFont::Normal);
        item->setFont(font);
        firstmodel->setItem(rowindex, 2, item);
        item = new MyFileItem(QLocale().toString(fileinfo.lastModified(), tr("yyyy/MM/dd hh:mm:ss")));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
        font.setWeight(QFont::Normal);
        item->setFont(font);
        firstmodel->setItem(rowindex, 1, item);
        rowindex++;
    }

    pTableViewFile->setModel(firstmodel);

    firstSelectionModel->clear();
    pTableViewFile->setSelectionModel(firstSelectionModel);

    restoreHeaderSort(rootPathUnique);
    resizecolumn();

//    foreach (int row, m_fileaddindex) {
//        pTableViewFile->selectRow(row);
//    }
}

void fileViewer::restoreHeaderSort(const QString &currentPath)
{
    if (sortCache_.contains(currentPath)) {
        pTableViewFile->header_->setSortIndicator(sortCache_[currentPath].sortRole, sortCache_[currentPath].sortOrder);
    } else {
        pTableViewFile->header_->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    }
}

void fileViewer::updateAction(const QString &fileType)
{
    QList<QAction *> listAction =  OpenWithDialog::addMenuOpenAction(fileType);

    openWithDialogMenu->addActions(listAction);
    openWithDialogMenu->addAction(new QAction(tr("Choose default programma")));
}

void fileViewer::openWithDialog(const QModelIndex &index)
{
    QModelIndex curindex = pTableViewFile->currentIndex();
    if (curindex.isValid()) {
        if (0 == m_pathindex) {
            QStandardItem *item = firstmodel->itemFromIndex(index);
            QString itemText = item->text().trimmed();
            int row = 0;
            foreach (QFileInfo file, m_curfilelist) {
                if (file.fileName() == itemText) {
                    break;
                }
                row++;
            }
            if (row >= m_curfilelist.count()) {
                row = 0;
            }
            if (m_curfilelist.at(row).isDir()) {
                pModel->setPathIndex(&m_pathindex);
                pTableViewFile->setModel(pModel);
                m_indexmode = pModel->setRootPath(m_curfilelist.at(row).filePath());
                m_pathindex++;
                restoreHeaderSort(pModel->rootPath());
                pTableViewFile->setRootIndex(m_indexmode);

                QDir dir(m_curfilelist.at(row).filePath());

                if (0 == dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files).count()) {
                    showPlable();
                }

                resizecolumn();
            } else {
                OpenWithDialog *openDialog = new OpenWithDialog(DUrl(m_curfilelist.at(row).filePath()), this);
                openDialog->exec();
            }
        } else if (pModel && pModel->fileInfo(curindex).isDir()) {
            m_indexmode = pModel->setRootPath(pModel->fileInfo(curindex).filePath());
            m_pathindex++;
            restoreHeaderSort(pModel->rootPath());
            pTableViewFile->setRootIndex(m_indexmode);
        } else if (pModel && !pModel->fileInfo(curindex).isDir()) {
            OpenWithDialog *openDialog = new OpenWithDialog(DUrl(pModel->fileInfo(curindex).filePath()), this);
            openDialog->exec();
        }
    }

}

void fileViewer::openWithDialog(const QModelIndex &index, const QString &programma)
{
    QModelIndex curindex = pTableViewFile->currentIndex();
    if (curindex.isValid()) {
        if (0 == m_pathindex) {
            QStandardItem *item = firstmodel->itemFromIndex(index);
            QString itemText = item->text().trimmed();
            int row = 0;
            foreach (QFileInfo file, m_curfilelist) {
                if (file.fileName() == itemText) {
                    break;
                }
                row++;
            }
            if (row >= m_curfilelist.count()) {
                row = 0;
            }
            if (m_curfilelist.at(row).isDir()) {
                pModel->setPathIndex(&m_pathindex);
                pTableViewFile->setModel(pModel);
                m_indexmode = pModel->setRootPath(m_curfilelist.at(row).filePath());
                m_pathindex++;
                restoreHeaderSort(pModel->rootPath());
                pTableViewFile->setRootIndex(m_indexmode);

                QDir dir(m_curfilelist.at(row).filePath());

                if (0 == dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files).count()) {
                    showPlable();
                }

                resizecolumn();
            } else {
                OpenWithDialog::chooseOpen(programma, m_curfilelist.at(row).filePath());

            }
        } else if (pModel && pModel->fileInfo(curindex).isDir()) {
            m_indexmode = pModel->setRootPath(pModel->fileInfo(curindex).filePath());
            m_pathindex++;
            restoreHeaderSort(pModel->rootPath());
            pTableViewFile->setRootIndex(m_indexmode);
        } else if (pModel && !pModel->fileInfo(curindex).isDir()) {
            OpenWithDialog::chooseOpen(programma, pModel->fileInfo(curindex).filePath());
        }
    }

}


void fileViewer::InitConnection()
{
    // connect the signals to the slot function.
    if (PAGE_COMPRESS == m_pagetype) {
        connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotCompressRowDoubleClicked(const QModelIndex &)));
        //        connect(m_pRightMenu, &DMenu::triggered, this, &fileViewer::DeleteCompressFile);
    } else {
        connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotDecompressRowDoubleClicked(const QModelIndex &)));
        connect(pTableViewFile, &MyTableView::sigdragLeave, this, &fileViewer::slotDragLeave);
    }

    connect(openWithDialogMenu, &DMenu::triggered, this, &fileViewer::onRightMenuOpenWithClicked);
    connect(m_pRightMenu, &DMenu::triggered, this, &fileViewer::onRightMenuClicked);
    connect(pTableViewFile, &MyTableView::customContextMenuRequested, this, &fileViewer::showRightMenu);
    connect(pModel, &MyFileSystemModel::sigShowLabel, this, &fileViewer::showPlable);
}

void fileViewer::resizecolumn()
{
    pTableViewFile->setColumnWidth(0, pTableViewFile->width() * 25 / 58);
    pTableViewFile->setColumnWidth(1, pTableViewFile->width() * 17 / 58);
    pTableViewFile->setColumnWidth(2, pTableViewFile->width() * 8 / 58);
    pTableViewFile->setColumnWidth(3, pTableViewFile->width() * 8 / 58);
}

void fileViewer::resizeEvent(QResizeEvent */*size*/)
{
    resizecolumn();
}


void fileViewer::keyPressEvent(QKeyEvent *event)
{
    DWidget::keyPressEvent(event);

    if (!event) {
        return;
    }
//    if (event->key() == Qt::Key_Delete) {
//        deleteCompressFile();
//    }
    if (event->key() == Qt::Key_Delete && 0 == m_pathindex) {
        deleteCompressFile();
    }
}

void fileViewer::openTempFile(QString path)
{
    QFileInfo file(path);
    KProcess *cmdprocess = new KProcess;
    QStringList arguments;
    QString programPath = QStandardPaths::findExecutable("xdg-open"); //查询本地位置
    if (file.fileName().contains("%")) {
        QString tmppath = DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles";
        QDir dir(tmppath);
        if (!dir.exists()) {
            dir.mkdir(tmppath);
        }

        QProcess p;
        QFileInfo tempFile(path);
        QString openTempFile = QString("%1").arg(openFileTempLink) + "." + file.suffix();
        openFileTempLink++;
        QString commandCreate = "ln";
        QStringList args;
        args.append(path);
        args.append(DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles"
                    + QDir::separator() + openTempFile);
        arguments << DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles"
                  + QDir::separator() + openTempFile;
        p.execute(commandCreate, args);
    } else {
        arguments << path;
    }

//                arguments << m_curfilelist.at(row).filePath();
    cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
    cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    cmdprocess->setProgram(programPath, arguments);
    cmdprocess->start();
}

void fileViewer::resetTempFile()
{
    openFileTempLink = 0;
}

void fileViewer::deleteCompressFile()
{
    int row = pModel->rowCount();
    QItemSelectionModel *selections =  pTableViewFile->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    QSet< int>  selectlist;

    foreach (QModelIndex index, selected) {
        selectlist.insert(index.row());
    }

    foreach (int index, selectlist) {
        if (m_curfilelist.size() > index) {
            m_curfilelist.replace(index, QFileInfo(""));
        }
    }

    foreach (QFileInfo file, m_curfilelist) {
        if (file.path() == "") {
            m_curfilelist.removeOne(file);
        }
    }

    QStringList filelist;
    foreach (QFileInfo fileinfo, m_curfilelist) {
        filelist.append(fileinfo.filePath());
    }

    curFileListModified = true;
    emit sigFileRemoved(filelist);
}

int fileViewer::getPathIndex()
{
    return m_pathindex;
}

/*
*Summary: Return to the primary directory. For example, delete the source files when the secondary directory is opened.
*Parameters: void
*Return: void
*/
void fileViewer::setRootPathIndex()
{
    m_pathindex = 0;
    refreshTableview();
    pTableViewFile->setPreviousButtonVisible(false);
    emit  sigpathindexChanged();
}

void fileViewer::setFileList(const QStringList &files)
{
    curFileListModified = true;

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

void fileViewer::setSelectFiles(const QStringList &files)
{
    QItemSelection selection;

    foreach (auto file, files) {
        for (int i = 0; i < firstmodel->rowCount(); ++i) {
            QStandardItem *item = firstmodel->item(i);
            if (item == nullptr) {
                return;
            }

            QString itemStr = item->text();

            if (itemStr != QFileInfo(file).fileName()) {
                continue;
            }

            QModelIndex index = firstmodel->index(i, 0);

            QItemSelectionRange selectionRange(index, firstmodel->index(i, firstmodel->columnCount() - 1));
            if (false == selection.contains(index)) {
                selection.push_back(selectionRange);
                break;
            }
        }
    }

    firstSelectionModel->select(selection, QItemSelectionModel::Select);
}

int fileViewer::getFileCount()
{
    return m_curfilelist.size();
}

int fileViewer::getDeFileCount()
{
    return pTableViewFile->model()->rowCount();
}

void fileViewer::slotCompressRePreviousDoubleClicked()
{
    /*
    if (PAGE_COMPRESS == m_pagetype) {
        if (m_pathindex > 1) {
            if (m_curfilelist.size())
                m_curfilelist = QFileInfo(m_curfilelist.at(0).path()).dir().entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
            else {
                m_curfilelist = m_parentDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
            }
        } else {
            pTableViewFile->setPreviousButtonVisible(false);
            m_curfilelist = m_parentFileList;
        }

        curFileListModified = true;
        m_pathindex--;
        showPlable();
        refreshTableview();

    }
    */

    if (PAGE_COMPRESS == m_pagetype) {
        if (m_pathindex > 1) {
            m_pathindex--;
            m_indexmode = pModel->setRootPath(pModel->fileInfo(m_indexmode).path());
            restoreHeaderSort(pModel->rootPath());
            pTableViewFile->setRootIndex(m_indexmode);
        } else {
            m_pathindex--;
            refreshTableview();
            pTableViewFile->setPreviousButtonVisible(false);
        }
        qDebug() << pModel->fileInfo(m_indexmode).path() << m_indexmode.data();
    } else {
        m_pathindex--;
        if (0 == m_pathindex) {
            pTableViewFile->setRootIndex(QModelIndex());
            pTableViewFile->setPreviousButtonVisible(false);
            restoreHeaderSort(rootPathUnique);
            //pTableViewFile->setRowHeight(0, ArchiveModelDefine::gTableHeight);
        } else {
            //            pTableViewFile->setRootIndex(m_sortmodel->mapFromSource(m_decompressmodel->parent(m_indexmode)));
            //            Archive::Entry* entry = m_decompressmodel->entryForIndex(m_sortmodel->mapFromSource(m_decompressmodel->parent(m_indexmode)));
            m_indexmode = m_decompressmodel->parent(m_indexmode);
            pTableViewFile->setRootIndex(m_sortmodel->mapFromSource(m_indexmode));
            Archive::Entry *entry = m_decompressmodel->entryForIndex(m_indexmode);
            restoreHeaderSort(zipPathUnique + MainWindow::getLoadFile() + "/" + entry->fullPath());
        }
    }
    emit  sigpathindexChanged();
}



void fileViewer::showPlable()
{
    pTableViewFile->horizontalHeader()->show();
    pTableViewFile->setPreviousButtonVisible(true);
}

void fileViewer::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if (m_pathindex == 0) {
        SortInfo info;
        info.sortOrder = order;
        info.sortRole = logicalIndex;
        sortCache_[rootPathUnique] = info;
        return;
    }

    QAbstractItemModel *model = pTableViewFile->model();

    MyFileSystemModel *fileModel = qobject_cast<MyFileSystemModel *>(model);
    if (fileModel) {
        SortInfo info;
        info.sortOrder = order;
        info.sortRole = logicalIndex;
        QString rootPath = fileModel->rootPath();
        sortCache_[fileModel->rootPath()] = info;
        return;
    }

    QStandardItemModel *standModel = qobject_cast<QStandardItemModel *>(model);

    if (standModel) {
        SortInfo info;
        info.sortOrder = order;
        info.sortRole = logicalIndex;
        sortCache_[rootPathUnique] = info;
        return;
    }

    ArchiveSortFilterModel *archiveModel = qobject_cast<ArchiveSortFilterModel *>(model);

    if (archiveModel) {
        Archive::Entry *item = static_cast<Archive::Entry *>(m_indexmode.internalPointer());
        if (item) {
            SortInfo info;
            info.sortOrder = order;
            info.sortRole = logicalIndex;
            QString rootPath = zipPathUnique + MainWindow::getLoadFile() + "/" + item->fullPath();
            sortCache_[rootPath] = info;
            return;
        }
    }
}

void fileViewer::slotCompressRowDoubleClicked(const QModelIndex index)
{
    QModelIndex curindex = pTableViewFile->currentIndex();
    if (curindex.isValid()) {
        if (0 == m_pathindex) {

            QStandardItem *item = firstmodel->itemFromIndex(index);
            QString itemText = item->text().trimmed();
            int row = 0;
            foreach (QFileInfo file, m_curfilelist) {
                if (file.fileName() == itemText) {
                    break;
                }
                row++;
            }
            if (row >= m_curfilelist.count()) {
                row = 0;
            }
            if (m_curfilelist.at(curindex.row()).isDir()) {
                pModel->setPathIndex(&m_pathindex);
                pTableViewFile->setModel(pModel);
                m_indexmode = pModel->setRootPath(m_curfilelist.at(curindex.row()).filePath());
                m_pathindex++;
                restoreHeaderSort(pModel->rootPath());
                pTableViewFile->setRootIndex(m_indexmode);

                QDir dir(m_curfilelist.at(curindex.row()).filePath());
                //                qDebug() << dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
                if (0 == dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files).count()) {
                    showPlable();
                }

                resizecolumn();
            } else {
                openTempFile(m_curfilelist.at(row).filePath());
            }
        } else if (pModel && pModel->fileInfo(curindex).isDir()) {
            m_indexmode = pModel->setRootPath(pModel->fileInfo(curindex).filePath());
            m_pathindex++;
            restoreHeaderSort(pModel->rootPath());
            pTableViewFile->setRootIndex(m_indexmode);
        } else if (pModel && !pModel->fileInfo(curindex).isDir()) {
            KProcess *cmdprocess = new KProcess;
            QStringList arguments;
            QString programPath = QStandardPaths::findExecutable("xdg-open");
            arguments << pModel->fileInfo(curindex).filePath();
            cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
            cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
            cmdprocess->setProgram(programPath, arguments);
            cmdprocess->start();
        }

        emit sigpathindexChanged();
    }
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
                Archive::Entry *entry = m_decompressmodel->entryForIndex(m_sortmodel->mapToSource(index));
                restoreHeaderSort(zipPathUnique + MainWindow::getLoadFile() + "/" + entry->fullPath());
                if (0 == entry->entries().count()) {
                    showPlable();
                }
            } else {
                QVector<Archive::Entry *> fileList = filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows()));
                QString fileName = DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles" + QDir::separator() + fileList.at(0)->name();
                QFile tempFile(fileName);
                if (tempFile.exists()) {
                    tempFile.remove();
                }
                emit sigextractfiles(fileList, EXTRACT_TEMP);
            }
        } else if (m_decompressmodel->isentryDir(m_sortmodel->mapToSource(index))) {
            QModelIndex sourceindex = m_decompressmodel->createNoncolumnIndex(m_sortmodel->mapToSource(index));
            pTableViewFile->setRootIndex(m_sortmodel->mapFromSource(sourceindex));
            m_pathindex++;
            m_indexmode = sourceindex;
            Archive::Entry *entry = m_decompressmodel->entryForIndex(m_sortmodel->mapToSource(index));
            restoreHeaderSort(zipPathUnique + MainWindow::getLoadFile() + "/" + entry->fullPath());
            if (0 == entry->entries().count()) {
                showPlable();
            }
        } else {
            QVector<Archive::Entry *> fileList = filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows()));
            QString fileName = DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles" + QDir::separator() + fileList.at(0)->name();
            QFile tempFile(fileName);
            if (tempFile.exists()) {
                tempFile.remove();
            }
            emit sigextractfiles(fileList, EXTRACT_TEMP);
        }
    }
}
void fileViewer::showRightMenu(const QPoint &pos)
{
    if (!pTableViewFile->indexAt(pos).isValid()) {
        return;
    }
    if (m_pagetype == PAGE_COMPRESS) {
        if (m_pathindex > 0) {
            m_pRightMenu->removeAction(deleteAction);
        } else {
            m_pRightMenu->addAction(deleteAction);
        }
    }

    openWithDialogMenu->clear();

    if (m_pagetype == PAGE_COMPRESS) {
        updateAction(pTableViewFile->indexAt(pos).data().toString());
    } else {
        QModelIndex currentparent = pTableViewFile->model()->parent(pTableViewFile->currentIndex());
        //    qDebug() << pTableViewFile->rowAt(pos.y()) << pTableViewFile->columnAt(pos.x()) << pTableViewFile->currentIndex() << pTableViewFile->model()->index(pTableViewFile->currentIndex().row(), 0, currentparent).data();
        QModelIndex selectIndex = pTableViewFile->model()->index(pTableViewFile->currentIndex().row(), 0, currentparent);
        QVector<Archive::Entry *> selectEntry = filesForIndexes(QModelIndexList() << selectIndex);
        if (selectEntry.at(0)->isDir()) {
            updateAction(selectIndex.data().toString() + QDir::separator()); //如果是文件夹，添加'/'
        } else {
            updateAction(selectIndex.data().toString());
        }
    }

    m_pRightMenu->popup(QCursor::pos());
}

void fileViewer::slotDragLeave(QString path)
{
    if (path.isEmpty()) {
        return;
    }
    emit sigextractfiles(filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows())), EXTRACT_DRAG, path);
}

void fileViewer::onRightMenuClicked(QAction *action)
{
    if (PAGE_UNCOMPRESS == m_pagetype) {
        QVector<Archive::Entry *> fileList = filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows()));
        if (action->text() == tr("Extract") || action->text() == tr("Extract", "slotDecompressRowDoubleClicked")) {
            emit sigextractfiles(fileList, EXTRACT_TO);
        } else if (action->text() == tr("Extract to current directory")) {
//            QString nam = fileList.at(0)->name();
//            qDebug() << fileList.at(0)->fullPath();
            emit sigextractfiles(fileList, EXTRACT_HEAR);
        } else if (action->text() == tr("Open")) {
            slotDecompressRowDoubleClicked(pTableViewFile->currentIndex());
        }
    } else {
        if (action->text() == tr("Open")) {
            slotCompressRowDoubleClicked(pTableViewFile->currentIndex());
        }
        if (action->text() == tr("Delete")) {
            deleteCompressFile();
        }
    }
}

void fileViewer::onRightMenuOpenWithClicked(QAction *action)
{
    if (PAGE_UNCOMPRESS == m_pagetype) {
        QVector<Archive::Entry *> fileList = filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows()));
        QString fileName = DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles" + QDir::separator() + fileList.at(0)->name();
        QFile tempFile(fileName);
        if (tempFile.exists()) {
            tempFile.remove();
        }
        emit sigOpenWith(filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows())), action->text());
    } else {
        if (action->text() != tr("Choose default programma")) {
            openWithDialog(pTableViewFile->currentIndex(), action->text());
        } else {
            openWithDialog(pTableViewFile->currentIndex());
        }
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

    m_decompressmodel = dynamic_cast<ArchiveModel *>(m_sortmodel->sourceModel());
    m_decompressmodel->setPathIndex(&m_pathindex);
    m_decompressmodel->setTableView(pTableViewFile);
    connect(m_decompressmodel, &ArchiveModel::sigShowLabel, this, &fileViewer::showPlable);

    pTableViewFile->setModel(model);
    resizecolumn();
}

void fileViewer::startDrag(Qt::DropActions /*supportedActions*/)
{
    QMimeData *mimeData = new QMimeData;
    connect(mimeData, SIGNAL(dataRequested(QString)), this, SLOT(createData(QString)), Qt::DirectConnection);
    QDrag *drag = new QDrag(this);
    foreach (QFileInfo file, m_curfilelist) {
        if (file.path() == "") {
            m_curfilelist.removeOne(file);
        }
    }


    QStringList filelist;
    foreach (QFileInfo fileinfo, m_curfilelist) {
        filelist.append(fileinfo.filePath());
    }
    curFileListModified = true;
    emit sigFileRemoved(filelist);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}


