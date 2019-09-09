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


MyScrollBar::MyScrollBar(QWidget* parent)
    : QScrollBar(parent)
{

}

void MyScrollBar::showEvent(QShowEvent *event){
     emit ScrollBarShowEvent ( event );
}

void MyScrollBar::hideEvent(QHideEvent *event) {
     emit ScrollBarHideEvent ( event );
}

MyLabel::MyLabel(QWidget* parent)
    : QLabel(parent)
{

}

void MyLabel::mouseDoubleClickEvent ( QMouseEvent * event ){
    emit labelDoubleClickEvent ( event );
}


FirstRowDelegate::FirstRowDelegate(QObject *parent)
    : QItemDelegate(parent)
{

}

void FirstRowDelegate::setPathIndex(int *index){
    ppathindex=index;
}

void FirstRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int row = index.row();
    int x = option.rect.x();
    int y = option.rect.y();
    int width = option.rect.width();
    int height = option.rect.height();

    if (row == 0&& ppathindex && *ppathindex>0)//UE
    {
        //选项
        QStyleOptionFrame *FrameOption = new QStyleOptionFrame();
        FrameOption->rect = QRect(x,y,width,height/2);
        //绘制
        QApplication::style()->drawControl(QStyle::CE_ShapedFrame, FrameOption , painter);
        QStyleOptionViewItem loption=option;
        loption.rect = QRect(x,y+height/2,width,height/2);
        return QItemDelegate::paint (painter, loption, index);
    }

    return QItemDelegate::paint (painter, option, index);
}

fileViewer::fileViewer(QWidget *parent, PAGE_TYPE type)
    : QWidget(parent), m_pagetype(type)
{
    setWindowTitle( tr( "File Viewer" ) );
    setMinimumSize(580, 300);
    m_pathindex=0;
    InitUI();
    InitConnection();
}

fileViewer::~fileViewer()
{
}

void fileViewer::InitUI()
{
    pTableViewFile = new QTableView(this);
    pdelegate = new FirstRowDelegate(this);
    pdelegate->setPathIndex(&m_pathindex);
    pTableViewFile->setItemDelegate(pdelegate);
    plabel=new MyLabel(pTableViewFile);
    firstmodel = new QStandardItemModel();
    pModel= new MyFileSystemModel(this);
    pModel->setNameFilterDisables(false);
    pModel->setTableView(pTableViewFile);
    QStringList labels = QObject::trUtf8("名称,大小,类型,修改时间").simplified().split(",");
    firstmodel->setHorizontalHeaderLabels(labels);


    pScrollbar= new MyScrollBar(this);
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
    pTableViewFile->setStyleSheet("QHeaderView::section{border: 0px solid white;"
                                  "min-height:36px; background-color:white}");
    pTableViewFile->setGeometry(0,0,580,300);

    plabel->setText(" ...返回上一层");
//    DPalette palette;
//    palette.setColor(DPalette::Background, QColor(245, 245, 245));
    plabel->setAutoFillBackground(true);
//    plabel->setPalette(palette);
    plabel->hide();

    pTableViewFile->setGeometry(0,0,580,300);
    plabel->setGeometry(pTableViewFile->x(),pTableViewFile->y()+MyFileSystemDefine::gTableHeight,pTableViewFile->width(),MyFileSystemDefine::gTableHeight);
    refreshTableview();
}

void fileViewer::refreshTableview()
{
    QStandardItem* item = nullptr;
    firstmodel->clear();

    item = new QStandardItem(QObject::trUtf8("名称"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(0, item);
    item = new QStandardItem(QObject::trUtf8("大小"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(1, item);
    item = new QStandardItem(QObject::trUtf8("类型"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(2, item);
    item = new QStandardItem(QObject::trUtf8("修改日期"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(3, item);

    int rowindex = 0;
    QFileIconProvider icon_provider;
    foreach(QFileInfo fileinfo , m_curfilelist)
    {
        item = new QStandardItem(icon_provider.icon(fileinfo), fileinfo.fileName());
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,0,item);
        item = new QStandardItem(QString::number(fileinfo.size()) + " KB");
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,1,item);
        item = new QStandardItem(getfiletype(fileinfo));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,2,item);
        item = new QStandardItem(fileinfo.lastModified().toString());
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,3,item);
        rowindex++;
    }

    pTableViewFile->setModel(firstmodel);
}



void fileViewer::InitConnection()
{
    // connect the signals to the slot function.
    if(PAGE_COMPRESS == m_pagetype)
    {
        connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotCompressRowDoubleClicked(const QModelIndex &)));
    }
    else {
        connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotDecompressRowDoubleClicked(const QModelIndex &)));
//        connect(plabel, SIGNAL(labelDoubleClickEvent(QMouseEvent *)), this, SLOT(slotDeCompressRePreviousDoubleClicked(QMouseEvent *)));
    }
    connect(plabel, SIGNAL(labelDoubleClickEvent(QMouseEvent *)), this, SLOT(slotCompressRePreviousDoubleClicked(QMouseEvent *)));

    connect(pScrollbar, SIGNAL(ScrollBarShowEvent ( QShowEvent *)), this, SLOT(ScrollBarShowEvent ( QShowEvent *)));
    connect(pScrollbar, SIGNAL(ScrollBarHideEvent ( QHideEvent *)), this, SLOT(ScrollBarHideEvent ( QHideEvent *)));
}

QString fileViewer::getfiletype(const QFileInfo &file)
{
    QString ret = "";
    if(file.isDir())
    {
        ret = tr("文件夹");
    }
    else {
        ret = tr("文件") + " " + file.suffix();
    }
    return ret;
}

void fileViewer::keyPressEvent(QKeyEvent *event)
{
    if(!event)
    {
        return;
    }
    if(event->key() == Qt::Key_Delete) {
        QItemSelectionModel *selections =  pTableViewFile->selectionModel();
        QModelIndexList selected = selections->selectedIndexes();

        QSet<unsigned int>  selectlist;

        foreach (QModelIndex index, selected)
        {
            selectlist.insert(index.row());
        }

        QStringList filelist;
        foreach(unsigned int index, selectlist)
        {
            m_curfilelist.removeAt(index);
        }

        foreach(QFileInfo fileinfo, m_curfilelist)
        {
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
    m_curfilelist.clear();
    foreach(QString filepath , files)
    {
        QFile file(filepath);
        m_curfilelist.append(file);
    }
    refreshTableview();
}

void fileViewer::slotCompressRePreviousDoubleClicked(QMouseEvent *event){
    if(PAGE_COMPRESS == m_pagetype)
    {
        if(m_pathindex>1){
            m_pathindex--;
            m_indexmode=pModel->setRootPath(pModel->fileInfo(m_indexmode).path());
            qDebug()<<pModel->fileInfo(m_indexmode).path();
            pTableViewFile->setRootIndex(m_indexmode);
        }
        else {
            m_pathindex--;
            refreshTableview();
            if(0==m_pathindex){
                plabel->hide();
                pTableViewFile->setRowHeight(0,MyFileSystemDefine::gTableHeight);
            }
        }
    }
    else {
        m_pathindex--;
        if(0 == m_pathindex)
        {
            pTableViewFile->setRootIndex(QModelIndex());
            plabel->hide();
            pTableViewFile->setRowHeight(0,ArchiveModelDefine::gTableHeight);
        }
        else {
            QModelIndex parent = m_decompressmodel->parent(m_indexmode);
            pTableViewFile->setRootIndex(parent);
        }
    }
}

void fileViewer::slotCompressRowDoubleClicked(const QModelIndex index){
    QModelIndex curindex = pTableViewFile->currentIndex();
    if (curindex.isValid())
    {
        if(0 == m_pathindex)
        {
            if(m_curfilelist.at(curindex.row()).isDir())
            {
                pModel->setPathIndex(&m_pathindex);
                pTableViewFile->setModel(pModel);
                m_indexmode = pModel->setRootPath(m_curfilelist.at(curindex.row()).filePath());
                pTableViewFile->setRootIndex(m_indexmode);
                m_pathindex++;
                plabel->show();
                plabel->raise();
            }
        }
        else if(pModel && pModel->fileInfo(curindex).isDir())
        {
            m_indexmode=pModel->setRootPath(pModel->fileInfo(curindex).filePath());
            pTableViewFile->setRootIndex(m_indexmode);
            m_pathindex++;
        }
    }
}

void fileViewer::slotDecompressRowDoubleClicked(const QModelIndex index)
{
    if (index.isValid())
    {
        qDebug()<<m_decompressmodel->isentryDir(index);
        if(0 == m_pathindex)
        {
            if(m_decompressmodel->isentryDir(index))
            {
                m_decompressmodel->setPathIndex(&m_pathindex);
                pTableViewFile->setRootIndex(m_decompressmodel->createNoncolumnIndex(index));
                m_pathindex++;
                plabel->show();
                plabel->raise();
                m_indexmode = index;
            }
        }
        else if(m_decompressmodel->isentryDir(index))
        {
            pTableViewFile->setRootIndex(m_decompressmodel->createNoncolumnIndex(index));
            m_pathindex++;
            m_indexmode = index;
        }
    }
}

void fileViewer::ScrollBarShowEvent ( QShowEvent * event ){
    plabel->setGeometry(pTableViewFile->x(),pTableViewFile->y()+MyFileSystemDefine::gTableHeight,pTableViewFile->width()-10,pTableViewFile->horizontalHeader()->height());
}
void fileViewer::ScrollBarHideEvent ( QHideEvent * event ){
    plabel->setGeometry(pTableViewFile->x(),pTableViewFile->y()+MyFileSystemDefine::gTableHeight,pTableViewFile->width(),pTableViewFile->horizontalHeader()->height());

}



void fileViewer::setDecompressModel(ArchiveModel* model)
{
    m_pathindex = 0;
    m_indexmode = QModelIndex();
    m_decompressmodel = model;
    m_decompressmodel->setPathIndex(&m_pathindex);
    m_decompressmodel->setTableView(pTableViewFile);

    pTableViewFile->setModel(m_decompressmodel);

}

