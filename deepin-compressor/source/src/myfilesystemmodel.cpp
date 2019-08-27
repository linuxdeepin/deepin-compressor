

//#include "myfilesystemmodelprivate.h"
#include "myfilesystemmodel.h"
#include <QDebug>


MyFileSystemModel::MyFileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    m_showreprevious=false;
}

MyFileSystemModel::~MyFileSystemModel(){

}

void MyFileSystemModel::setPathIndex(int *index){
    ppathindex=index;
}

void MyFileSystemModel::setTableView(QTableView *tableview){
    m_tableview=tableview;
}

QVariant MyFileSystemModel::data(const QModelIndex &index, int role) const  {

//    qDebug()<<"row : "<<index.row()<<" role : "<<role;
    if(0==index.row()&&
            0==index.column()){
        if(ppathindex&&*ppathindex>0){
            m_tableview->setRowHeight(0,MyFileSystemDefine::gTableHeight*2);
        }else {
            m_tableview->setRowHeight(0,MyFileSystemDefine::gTableHeight);
        }
    }
    if(0!=index.row())
    m_tableview->setRowHeight(index.row(),MyFileSystemDefine::gTableHeight);
    return QFileSystemModel::data(index,role);
}

