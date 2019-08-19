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


//       pListWidgetFile = new QTreeView();

//       QFileSystemModel* model= new QFileSystemModel();
//       model->setNameFilterDisables(false);

//       pListWidgetFile->setModel(model);
//       pListWidgetFile->setRootIndex(model->setRootPath(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first()));
////       pListWidgetFile->setViewMode(QListView::IconMode);


//       QVBoxLayout * pVLayout = new QVBoxLayout( this );
//       pVLayout->addWidget( pListWidgetFile );

       m_headerView = new DFMHeaderView(Qt::Horizontal, this);

       m_headerView->setHighlightSections(false);
       m_headerView->setSectionsClickable(true);
       m_headerView->setSortIndicatorShown(true);
       m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
       m_headerView->setContextMenuPolicy(Qt::CustomContextMenu);

}

fileViewer::~fileViewer()
{

}

//DFileSystemModel *fileViewer::model() const
//{
//    return qobject_cast<DFileSystemModel*>(DListView::model());
//}

//void fileViewer::updateListHeaderViewProperty()
//{
//    if (!m_headerView)
//        return;

//    m_headerView->setModel(Q_NULLPTR);
//    m_headerView->setModel(model());

//    m_headerView->setDefaultSectionSize(140);

//    m_headerView->setSectionResizeMode(QHeaderView::Fixed);
//    m_headerView->setMinimumSectionSize(140);

//    m_headerView->setSortIndicator(model()->sortColumn(), model()->sortOrder());
//    d->columnRoles.clear();

//    // set value from config file.
//    const QVariantMap &state = DFMApplication::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();

//    for (int i = 0; i < m_headerView->count(); ++i) {
//        d->columnRoles << model()->columnToRole(i);

//        if (d->allowedAdjustColumnSize) {
//            int colWidth = state.value(QString::number(d->columnRoles.last()), -1).toInt();
//            if (colWidth > 0) {
//                m_headerView->resizeSection(model()->roleToColumn(d->columnRoles.last()), colWidth);
//            }
//        } else {
//            int column_width = model()->columnWidth(i);
//            if (column_width >= 0) {
//                m_headerView->resizeSection(i, column_width + COLUMU_PADDING * 2);
//            } else {
//                m_headerView->setSectionResizeMode(i, QHeaderView::Stretch);
//            }
//        }

//        const QString &column_name = model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();

//        if (!d->columnForRoleHiddenMap.contains(column_name)) {
//            m_headerView->setSectionHidden(i, !model()->columnDefaultVisibleForRole(model()->columnToRole(i)));
//        } else {
//            m_headerView->setSectionHidden(i, d->columnForRoleHiddenMap.value(column_name));
//        }
//    }

//    if (d->adjustFileNameCol) {
//        d->doFileNameColResize();
//    }

//    updateColumnWidth();
//}

void fileViewer::showFileInfoList( QFileInfoList list )
{

}

void fileViewer::slotShowDir( QListWidgetItem * item )
{

}

