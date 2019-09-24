#include "myfileitem.h"
#include <QDebug>


MyFileItem::MyFileItem()
{

}

MyFileItem::MyFileItem(const QString &text)
   :QStandardItem(text)
{
}

MyFileItem::MyFileItem(const QIcon &icon, const QString &text)
   : QStandardItem(icon, text)
{
}


bool MyFileItem::operator<(const QStandardItem &other) const
{
    const QVariant l = data(Qt::DisplayRole), r = other.data(Qt::DisplayRole);
    switch (column()) {
    case 1:
    {
        if (column() == other.column())
        {
            qint64 lint = Utils::humanReadableToSize(l.toString());
            qint64 rint = Utils::humanReadableToSize(r.toString());
            qDebug()<<lint;
            qDebug()<<rint;
            return lint < rint;
        }
        break;
    }
    default:
        break;


    }

    return QStandardItem::operator<(other);
}
