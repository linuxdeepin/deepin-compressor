

#include "archivesortfiltermodel.h"
#include "archiveentry.h"
#include "archivemodel.h"



ArchiveSortFilterModel::ArchiveSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

ArchiveSortFilterModel::~ArchiveSortFilterModel()
{
}

bool ArchiveSortFilterModel::lessThan(const QModelIndex &leftIndex,
                                      const QModelIndex &rightIndex) const
{
    ArchiveModel *srcModel = qobject_cast<ArchiveModel*>(sourceModel());
    const int col = srcModel->shownColumns().at(leftIndex.column());
    const QByteArray property = srcModel->propertiesMap().value(col);

    const Archive::Entry *left = srcModel->entryForIndex(leftIndex);
    const Archive::Entry *right = srcModel->entryForIndex(rightIndex);

    if (left->isDir() && !right->isDir()) {
        return true;
    } else if (!left->isDir() && right->isDir()) {
        return false;
    } else {
        switch (col) {
        {
        case Size:
            uint dirs;
            uint files;
            left->countChildren(dirs, files);
            uint files_l = dirs + files;
            right->countChildren(dirs, files);
            uint files_r = dirs + files;
            qDebug()<<QString::number(dirs + files);
            if(left->isDir()&&right->isDir())
            {
                return files_l < files_r;
            }
            else if (left->isDir()) {
                return true;
            }
            else if (right->isDir()) {
                return false;
            }

            if (left->property(property.constData()).toULongLong() < right->property(property.constData()).toULongLong()) {
                return true;
            }
            break;
        }
        default:
            if (left->property(property.constData()).toString() < right->property(property.constData()).toString()) {
                return true;
            }
        }
    }
    return false;
}
