

//#include "myfilesystemmodelprivate.h"
#include "myfilesystemmodel.h"
#include <QDebug>
#include <utils.h>

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

QVariant MyFileSystemModel::data(const QModelIndex &index, int role) const
{
    if(0==index.row() && 0==index.column())
    {
        if(ppathindex && *ppathindex > 0)
        {
            m_tableview->setRowHeight(0,MyFileSystemDefine::gTableHeight*2);
        }
        else {
            m_tableview->setRowHeight(0,MyFileSystemDefine::gTableHeight);
        }
    }

    if(0!=index.row())
    {
        m_tableview->setRowHeight(index.row(),MyFileSystemDefine::gTableHeight);
    }

    if (index.isValid())
    {
        switch (role) {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        case Qt::DisplayRole: {
            switch (index.column()) {
            case 1:
            {
                QFileInfo file = fileInfo(index);
                if(file.isDir())
                {
                    return "-";
                }
                else {

                    return Utils::humanReadableSize(file.size(), 1);
                }
            }

            }
        }

        }
    }
    return QFileSystemModel::data(index,role);
}

