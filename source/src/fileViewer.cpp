#include "fileViewer.h"
#include <QLayout>
#include <QDir>
#include <QDesktopServices>
#include <QDebug>
#include <QStandardItemModel>

fileViewer::fileViewer(QWidget *parent)
       : QWidget(parent)
{
       setWindowTitle( tr( "File Viewer" ) );
       setMinimumSize(520, 340);


       pListWidgetFile = new QTreeView();

       QFileSystemModel* model= new QFileSystemModel();
       model->setNameFilterDisables(false);

       pListWidgetFile->setModel(model);
       pListWidgetFile->setRootIndex(model->setRootPath(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first()));
//       pListWidgetFile->setViewMode(QListView::IconMode);


       QVBoxLayout * pVLayout = new QVBoxLayout( this );
       pVLayout->addWidget( pListWidgetFile );


}

fileViewer::~fileViewer()
{

}

void fileViewer::showFileInfoList( QFileInfoList list )
{

}

void fileViewer::slotShowDir( QListWidgetItem * item )
{

}

