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

MyTableView::MyTableView(QWidget* parent)
    : QTableView(parent)
{

}

void MyTableView::paintEvent(QPaintEvent *e)
{
//    qDebug()<<y();
    QTableView::paintEvent(e);
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
    QHBoxLayout* mainlayout = new QHBoxLayout;

    pTableViewFile = new MyTableView();
    pdelegate = new FirstRowDelegate();
    pdelegate->setPathIndex(&m_pathindex);
    pTableViewFile->setItemDelegate(pdelegate);
    plabel=new MyLabel(pTableViewFile);
    firstmodel = new QStandardItemModel();
    pModel= new MyFileSystemModel();
    pModel->setNameFilterDisables(false);
    pModel->setTableView(pTableViewFile);
    QStringList labels = QObject::trUtf8("名称,大小,类型,修改时间").simplified().split(",");
    firstmodel->setHorizontalHeaderLabels(labels);


    pScrollbar= new MyScrollBar();
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
    pTableViewFile->setSelectionMode(QAbstractItemView::ExtendedSelection);
//    pTableViewFile->setGeometry(0,0,580,300);

    plabel->setText(" ...返回上一层");
//    DPalette palette;
//    palette.setColor(DPalette::Background, QColor(245, 245, 245));
    plabel->setAutoFillBackground(true);
//    plabel->setPalette(palette);
    plabel->hide();

    plabel->setGeometry(0,MyFileSystemDefine::gTableHeight,1920,MyFileSystemDefine::gTableHeight);

    mainlayout->addWidget(pTableViewFile);
    setLayout(mainlayout);

    if(PAGE_UNCOMPRESS == m_pagetype)
    {
        pTableViewFile->setContextMenuPolicy(Qt::CustomContextMenu);
        m_pRightMenu = new DMenu();
        m_pRightMenu->setFixedWidth(200);
        m_pRightMenu->addAction(tr("提取文件"));
        m_pRightMenu->addAction(tr("提取文件到当前文件夹"));
        pTableViewFile->setDragDropMode(QAbstractItemView::DragDrop);
    }


    refreshTableview();
}

void fileViewer::refreshTableview()
{
    MyFileItem* item = nullptr;
    firstmodel->clear();

    item = new MyFileItem(QObject::trUtf8("名称"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(0, item);
    item = new MyFileItem(QObject::trUtf8("大小"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(1, item);
    item = new MyFileItem(QObject::trUtf8("类型"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(2, item);
    item = new MyFileItem(QObject::trUtf8("修改日期"));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    firstmodel->setHorizontalHeaderItem(3, item);

    int rowindex = 0;
    QFileIconProvider icon_provider;
    foreach(QFileInfo fileinfo , m_curfilelist)
    {
        item = new MyFileItem(icon_provider.icon(fileinfo), fileinfo.fileName());
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,0,item);
        if(fileinfo.isDir())
        {
            item = new MyFileItem("-");
        }
        else
        {
            item = new MyFileItem(Utils::humanReadableSize(fileinfo.size(), 1));
        }
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,1,item);
        item = new MyFileItem(getfiletype(fileinfo));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,2,item);
        item = new MyFileItem(fileinfo.lastModified().toString());
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        firstmodel->setItem(rowindex,3,item);
        rowindex++;
    }

    pTableViewFile->setModel(firstmodel);

    foreach(int row ,m_fileaddindex)
    {
        pTableViewFile->selectRow(row);
    }
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
        connect(pTableViewFile, &MyTableView::customContextMenuRequested, this, &fileViewer::showRightMenu);
        if(m_pRightMenu)
        {
            connect(m_pRightMenu, &DMenu::triggered, this, &fileViewer::onRightMenuClicked);
        }
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

    if(files.count() > m_curfilelist.count())
    {
        m_fileaddindex.clear();
        for(int i = 0; i < files.count() - m_curfilelist.count(); i++)
        {
            m_fileaddindex.append(files.count() - 1 + i);
        }
    }


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
    plabel->setGeometry(0,MyFileSystemDefine::gTableHeight,1920,pTableViewFile->horizontalHeader()->height());
}
void fileViewer::ScrollBarHideEvent ( QHideEvent * event ){
    plabel->setGeometry(0,MyFileSystemDefine::gTableHeight,1920,pTableViewFile->horizontalHeader()->height());

}

void fileViewer::showRightMenu(const QPoint &pos)
{
    if (!pTableViewFile->indexAt(pos).isValid())
    {
        return;
    }

    m_pRightMenu->popup(QCursor::pos());

}

void fileViewer::onRightMenuClicked(QAction *action)
{
    if(action->text() == tr("提取文件"))
    {
        emit sigextractfiles(filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows())), EXTRACT_TO);
    }
    else {
        emit sigextractfiles(filesAndRootNodesForIndexes(addChildren(pTableViewFile->selectionModel()->selectedRows())), EXTRACT_HEAR);
    }
}


QModelIndexList fileViewer::addChildren(const QModelIndexList &list) const
{
    Q_ASSERT(m_decompressmodel);

    QModelIndexList ret = list;

    // Iterate over indexes in list and add all children.
    for (int i = 0; i < ret.size(); ++i) {
        QModelIndex index = ret.at(i);

        for (int j = 0; j < m_decompressmodel->rowCount(index); ++j) {
            QModelIndex child = m_decompressmodel->index(j, 0, index);
            if (!ret.contains(child)) {
                ret << child;
            }
        }
    }

    return ret;
}


QVector<Archive::Entry*> fileViewer::filesForIndexes(const QModelIndexList& list) const
{
    QVector<Archive::Entry*> ret;

    for (const QModelIndex& index : list) {
        ret << m_decompressmodel->entryForIndex(index);
    }

    return ret;
}

QVector<Archive::Entry*> fileViewer::filesAndRootNodesForIndexes(const QModelIndexList& list) const
{
    QVector<Archive::Entry*> fileList;
    QStringList fullPathsList;

    for (const QModelIndex& index : list) {
        QModelIndex selectionRoot = index.parent();
        while (pTableViewFile->selectionModel()->isSelected(selectionRoot) ||
               list.contains(selectionRoot)) {
            selectionRoot = selectionRoot.parent();
        }

        // Fetch the root node for the unselected parent.
        const QString rootFileName = selectionRoot.isValid()
            ? m_decompressmodel->entryForIndex(selectionRoot)->fullPath()
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


void fileViewer::setDecompressModel(ArchiveModel* model)
{
    m_pathindex = 0;
    m_indexmode = QModelIndex();
    m_decompressmodel = model;
    m_decompressmodel->setPathIndex(&m_pathindex);
    m_decompressmodel->setTableView(pTableViewFile);

    pTableViewFile->setModel(m_decompressmodel);

}

void fileViewer::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
}

void fileViewer::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();
}

void fileViewer::dropEvent(QDropEvent *e)
{


    e->accept();



}

void fileViewer::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void fileViewer::startDrag(Qt::DropActions supportedActions)
{
    QMimeData* mimeData = new QMimeData;
    connect(mimeData, SIGNAL(dataRequested(QString)),
            this, SLOT(createData(QString)), Qt::DirectConnection);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}

