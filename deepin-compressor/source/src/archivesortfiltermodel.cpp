/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "archivesortfiltermodel.h"
#include "archiveentry.h"
#include "archivemodel.h"
#include "mimetypes.h"
#include "source/inc/mimetypedisplaymanager.h"

#include <QCollator>

ArchiveSortFilterModel::ArchiveSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    m_mimetype = new MimeTypeDisplayManager(this);
}

bool ArchiveSortFilterModel::lessThan(const QModelIndex &leftIndex,
                                      const QModelIndex &rightIndex) const
{
    if (leftIndex.column() < 0 || rightIndex.column() < 0)
        return false;

    ArchiveModel *srcModel = qobject_cast<ArchiveModel *>(sourceModel());
    const int col = srcModel->shownColumns().at(leftIndex.column());
    const QByteArray property = srcModel->propertiesMap().value(col);

    const Archive::Entry *left = srcModel->entryForIndex(leftIndex);
    const Archive::Entry *right = srcModel->entryForIndex(rightIndex);

    if (left->isDir() && !right->isDir()) {
        if (sortOrder() == Qt::DescendingOrder) {
            return false;
        } else {
            return true;
        }
    } else if (!left->isDir() && right->isDir()) {
        return false;
    } else {
        switch (col) {

        case Timestamp: {
            const QDateTime leftTime = left->property("timestamp").toDateTime();
            const QDateTime rightTime = right->property("timestamp").toDateTime();

            if (leftTime < rightTime) {
                return true;
            }
        }
        break;
        case Size: {
            uint dirs;
            uint files;
            left->countChildren(dirs, files);
            uint files_l = dirs + files;
            right->countChildren(dirs, files);
            uint files_r = dirs + files;
            qDebug() << QString::number(dirs + files);
            if (left->isDir() && right->isDir()) {
                return files_l < files_r;
            } else if (left->isDir()) {
                return true;
            } else if (right->isDir()) {
                return false;
            }

            if (left->property(property.constData()).toULongLong() < right->property(property.constData()).toULongLong()) {
                return true;
            }
        }
        break;
        case Type: {
            QMimeType mimeLeftType = determineMimeType(left->fullPath());
            QMimeType mimeRightType = determineMimeType(right->fullPath());

//            if (m_mimetype->displayName(mimeLeftType.name()) > m_mimetype->displayName(mimeRightType.name())) {
//                return true;
//            }

            QCollator col;
            if (col.compare(m_mimetype->displayName(mimeLeftType.name()), m_mimetype->displayName(mimeRightType.name())/*, Qt::CaseInsensitive*/) < 0) {
                return false;
            } else {
                return true;
            }
        }
        break;
        case FullPath: {
            if (left->name().at(0).script() == QChar::Script_Han) {
                if (right->name().at(0).script() == QChar::Script_Han) {
                    QCollator col;
                    if (col.compare(left->name(), right->name()) < 0) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            } else {
                if (right->name().at(0).script() == QChar::Script_Han) {
                    return true;
                } else {
                    if (QString::compare(left->name(), right->name(), Qt::CaseInsensitive) < 0) {
                        return true;
                    } else {
                        return false;
                    }
                }
            }
        }
        break;
        }
    }
    return false;
}

bool ArchiveSortFilterModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);

    endInsertRows();

    return true;
}

void ArchiveSortFilterModel::refreshNow()
{
    emit insertRows(0, 0, this->index(0, 0));
}
