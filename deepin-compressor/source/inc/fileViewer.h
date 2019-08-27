#ifndef FILEVIWER_H
#define FILEVIWER_H
#include <QDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
#include <DLabel>
#include <QItemDelegate>
#include <QApplication>
#include <QPainter>
#include "myfilesystemmodel.h"
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
//#include "dfmheaderview.h"
#include <DListView>

class MyScrollBar:public QScrollBar
{
     Q_OBJECT
public:
   MyScrollBar(QWidget* parent = 0);
protected:
   void hideEvent(QHideEvent*) override;
   void showEvent(QShowEvent *) override;
signals:
   void ScrollBarShowEvent ( QShowEvent * event );
   void ScrollBarHideEvent ( QHideEvent * event );
};

class MyLabel:public QLabel
{
     Q_OBJECT
public:
   MyLabel(QWidget* parent = 0);
protected:
   virtual void mouseDoubleClickEvent ( QMouseEvent * event );
signals:
   void labelDoubleClickEvent ( QMouseEvent * event );
};

class FirstRowDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    FirstRowDelegate(QObject* parent = 0);
    void setPathIndex(int *index);
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:

private:
    int *ppathindex;
};


class fileViewer : public QWidget
{
       Q_OBJECT
public:
       fileViewer(QWidget *parent = 0);
       ~fileViewer();
       void InitUI();
       void InitConnection();
       void initTitleBar();

protected slots:
       void slotShowDir( QListWidgetItem * item );
       void slotRowDoubleClicked(const QModelIndex index);
       void slotRePreviousDoubleClicked(QMouseEvent *event);
       void ScrollBarShowEvent ( QShowEvent * event );
       void ScrollBarHideEvent ( QHideEvent * event );
private:
       void showFileInfoList( QFileInfoList list );

private:
       QLineEdit * pLineEditDir;
       QTableView * pTableViewFile;
//       QFileSystemModel* pModel;
       int m_pathindex;
       MyFileSystemModel* pModel;
       FirstRowDelegate *pdelegate;
       MyLabel *plabel;
       QModelIndex m_indexmode;
       MyScrollBar *pScrollbar;
//       QWidget *pwidget;
//      DFMHeaderView * m_headerView = nullptr;

};

#endif // FILEVIWER_H
