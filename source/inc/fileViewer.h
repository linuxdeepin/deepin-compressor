#ifndef FILEVIWER_H
#define FILEVIWER_H
#include <QDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <dlabel.h>
#include <QTableWidget>
#include "dfmheaderview.h"
#include <dlistview.h>

class fileViewer : public QWidget
{
       Q_OBJECT
public:
       fileViewer(QWidget *parent = 0);
       ~fileViewer();

//       void updateListHeaderViewProperty();

protected slots:
       void slotShowDir( QListWidgetItem * item );

private:
       void showFileInfoList( QFileInfoList list );

private:
//       QLineEdit * pLineEditDir;
//       QTreeView * pListWidgetFile;
       DFMHeaderView * m_headerView = nullptr;

};

#endif // FILEVIWER_H
