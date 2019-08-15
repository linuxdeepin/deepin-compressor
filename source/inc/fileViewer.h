#ifndef FILEVIWER_H
#define FILEVIWER_H
#include <QDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QFileSystemModel>
#include <QTreeView>

class fileViewer : public QWidget
{
       Q_OBJECT
public:
       fileViewer(QWidget *parent = 0);
       ~fileViewer();

protected slots:
       void slotShowDir( QListWidgetItem * item );

private:
       void showFileInfoList( QFileInfoList list );

private:
       QLineEdit * pLineEditDir;
       QTreeView * pListWidgetFile;
};

#endif // FILEVIWER_H
