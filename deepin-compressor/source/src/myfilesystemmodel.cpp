

//#include "myfilesystemmodelprivate.h"
#include "myfilesystemmodel.h"
#include <QDateTime>
#include <QDebug>
#include <utils.h>

MyFileSystemModel::MyFileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    m_mimetype = new MimeTypeDisplayManager;
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

QVariant MyFileSystemModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        qDebug()<<section;

        switch (section) {
        case 0:
            return tr("名称");
        case 3:
            return tr("大小");
        case 2:
            return tr("类型");
        case 1:
            return tr("修改时间");
        default:
            return tr("Unnamed column", "??");
        }
    }
    else if(role == Qt::TextAlignmentRole)
    {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QVariant();
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
        QFileInfo file = fileInfo(index);
        switch (role) {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        case Qt::DisplayRole: {
            switch (index.column()) {
            case 3:
            {
                if(file.isDir())
                {
                    return "-";
                }
                else {

                    return Utils::humanReadableSize(file.size(), 1);
                }
            }
            case 2:
            {
                QMimeType mimetype = determineMimeType(file.filePath());
                return m_mimetype->displayName(mimetype.name());
            }
            case 1:
            {
                return QLocale().toString(lastModified(index), "yyyy/MM/dd hh:mm:ss");
            }

            }
        }

        }
    }
    return QFileSystemModel::data(index,role);
}

void MyFileSystemModel::sort(int column, Qt::SortOrder order)
{
    if(1 == column)
    {
        QFileSystemModel::sort(3, order);
    }
    else if (3 == column) {
        QFileSystemModel::sort(1, order);
    }
    else {
        QFileSystemModel::sort(column, order);
    }
}

