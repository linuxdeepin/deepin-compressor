#include "fileViewer.h"
#include <QLayout>
#include <QDir>
#include <QDesktopServices>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>
//#include "../RbTableHeaderView.h"

MyScrollBar::MyScrollBar(QWidget* parent)
    : QScrollBar(parent)
{

}

void MyScrollBar::showEvent(QShowEvent *event){
     qDebug()<<"showEvent";
     emit ScrollBarShowEvent ( event );
}

void MyScrollBar::hideEvent(QHideEvent *event) {
     qDebug()<<"hideEvent";
     emit ScrollBarHideEvent ( event );
}

MyLabel::MyLabel(QWidget* parent)
    : QLabel(parent)
{

}

void MyLabel::mouseDoubleClickEvent ( QMouseEvent * event ){
    //     qDebug()<<"mouseDoubleClickEvent";
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
        //qDebug()<<"row == 0";

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
    //    pwidget=new QWidget(this);
    pTableViewFile = new QTableView(this);

    pdelegate = new FirstRowDelegate(this);
    pdelegate->setPathIndex(&m_pathindex);
    pTableViewFile->setItemDelegate(pdelegate);
    //    pModel= new QFileSystemModel(this);
    pModel= new MyFileSystemModel(this);
    pModel->setNameFilterDisables(false);
    pModel->setTableView(pTableViewFile);
    pModel->setPathIndex(&m_pathindex);

    plabel=new MyLabel(this);
    pScrollbar= new MyScrollBar(this);
    //    RbTableHeaderView* hHead = new RbTableHeaderView(Qt::Horizontal, 1, 6);
    ////    QAbstractItemModel* hModel = hHead->model();

    //    hHead->setSectionsClickable(true);
    //MyQHeaderView *myHeader=new MyQHeaderView(Qt::Horizontal, pTableViewFile);
    // pTableViewFile->setHorizontalHeader(myHeader);
    //pTableViewFile->setHorizontalScrollBar(pScrollbar);
    pTableViewFile->setVerticalScrollBar(pScrollbar);
    pTableViewFile->setModel(pModel);
    m_indexmode=pModel->setRootPath(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first());
    pTableViewFile->setRootIndex(m_indexmode);
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
    plabel->setText(" ...返回上一层");
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(245, 245, 245));
    plabel->setAutoFillBackground(true);
    plabel->setPalette(palette);
    plabel->hide();
    //    QVBoxLayout * pVLayout = new QVBoxLayout( this );
    //    pVLayout->addWidget( pwidget );
    pTableViewFile->setGeometry(0,0,520,340);
    plabel->setGeometry(pTableViewFile->x(),pTableViewFile->y()+MyFileSystemDefine::gTableHeight,pTableViewFile->width(),pTableViewFile->horizontalHeader()->height());
    qDebug()<<"height:"<<pTableViewFile->horizontalHeader()->height();
}

void fileViewer::InitConnection()
{
    // connect the signals to the slot function.
    connect(pTableViewFile, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));
    connect(plabel, SIGNAL(labelDoubleClickEvent(QMouseEvent *)), this, SLOT(slotRePreviousDoubleClicked(QMouseEvent *)));
    connect(pScrollbar, SIGNAL(ScrollBarShowEvent ( QShowEvent *)), this, SLOT(ScrollBarShowEvent ( QShowEvent *)));
    connect(pScrollbar, SIGNAL(ScrollBarHideEvent ( QHideEvent *)), this, SLOT(ScrollBarHideEvent ( QHideEvent *)));
}

void fileViewer::slotRePreviousDoubleClicked(QMouseEvent *event){
    qDebug()<<"RePreviousDoubleClicked";
    if(m_pathindex>0){
        m_pathindex--;
        m_indexmode=pModel->setRootPath(pModel->fileInfo(m_indexmode).path());
        pTableViewFile->setRootIndex(m_indexmode);
        if(0==m_pathindex){
            plabel->hide();
        }
    }
}

void fileViewer::slotRowDoubleClicked(const QModelIndex index){
    QModelIndex curindex = pTableViewFile->currentIndex();
    if (curindex.isValid())
    {
        if(pModel->fileInfo(curindex).isDir()){
            m_pathindex++;
            plabel->show();
            plabel->raise();
            qDebug()<<"filePath"<<pModel->fileInfo(curindex).filePath();
            m_indexmode=pModel->setRootPath(pModel->fileInfo(curindex).filePath());
            pTableViewFile->setRootIndex(m_indexmode);
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

