#include "fileViewer.h"
#include <QLayout>
#include <QDir>
#include <QDesktopServices>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>
#include <DPalette>
#include <QFileIconProvider>


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

fileViewer::fileViewer(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle( tr( "File Viewer" ) );
    setMinimumSize(520, 340);
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
                                  "min-height:32px; background-color:white}");
    pTableViewFile->setGeometry(0,0,520,340);

    plabel->setText(" ...返回上一层");
    DPalette palette;
    palette.setColor(DPalette::Background, QColor(245, 245, 245));
    plabel->setAutoFillBackground(true);
    plabel->setPalette(palette);
    plabel->hide();

    pTableViewFile->setGeometry(0,0,520,340);
    plabel->setGeometry(pTableViewFile->x(),pTableViewFile->y()+MyFileSystemDefine::gTableHeight,pTableViewFile->width(),MyFileSystemDefine::gTableHeight);
    refreshTableview();
}

void fileViewer::refreshTableview()
{
    QStandardItem* item = nullptr;
    firstmodel->clear();
    QStringList labels = QObject::trUtf8("名称,大小,类型,修改日期").simplified().split(",");
    firstmodel->setHorizontalHeaderLabels(labels);

    int rowindex = 0;
    QFileIconProvider icon_provider;
    foreach(QFileInfo fileinfo , m_curfilelist)
    {
        item = new QStandardItem(icon_provider.icon(fileinfo), fileinfo.fileName());
        firstmodel->setItem(rowindex,0,item);
        item = new QStandardItem(QString::number(fileinfo.size()) + " KB");
        firstmodel->setItem(rowindex,1,item);
        item = new QStandardItem(getfiletype(fileinfo));
        firstmodel->setItem(rowindex,2,item);
        item = new QStandardItem(fileinfo.lastModified().toString());
        firstmodel->setItem(rowindex,3,item);
        rowindex++;
    }

    pTableViewFile->setModel(firstmodel);
}



void fileViewer::InitConnection()
{
    // connect the signals to the slot function.
    connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));
    connect(plabel, SIGNAL(labelDoubleClickEvent(QMouseEvent *)), this, SLOT(slotRePreviousDoubleClicked(QMouseEvent *)));
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

void fileViewer::slotRePreviousDoubleClicked(QMouseEvent *event){
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

void fileViewer::slotRowDoubleClicked(const QModelIndex index){
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

void fileViewer::ScrollBarShowEvent ( QShowEvent * event ){
    plabel->setGeometry(pTableViewFile->x(),pTableViewFile->y()+MyFileSystemDefine::gTableHeight,pTableViewFile->width()-10,pTableViewFile->horizontalHeader()->height());
}
void fileViewer::ScrollBarHideEvent ( QHideEvent * event ){
    plabel->setGeometry(pTableViewFile->x(),pTableViewFile->y()+MyFileSystemDefine::gTableHeight,pTableViewFile->width(),pTableViewFile->horizontalHeader()->height());

}

void fileViewer::showFileInfoList( QFileInfoList list )
{

}

void fileViewer::slotShowDir( QListWidgetItem * item )
{

}

