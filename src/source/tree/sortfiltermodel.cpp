/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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
#include "sortfiltermodel.h"
#include "uistruct.h"

#include <QCollator>
#include <QDateTime>
#include <QDebug>

SortFilterModel::SortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

bool SortFilterModel::lessThan(const QModelIndex &leftIndex, const QModelIndex &rightIndex) const
{
    qDebug() << "LessThan";
    return QSortFilterProxyModel::lessThan(leftIndex, rightIndex);

    if (leftIndex.column() < 0 || rightIndex.column() < 0)
        return false;

    int iColumn = leftIndex.column();

    bool bLeftIsDir = leftIndex.data(Qt::UserRole + 1).toBool();
    bool bRightIsDir = rightIndex.data(Qt::UserRole + 1).toBool();

    if (bLeftIsDir && !bRightIsDir) {
        if (sortOrder() == Qt::DescendingOrder) {
            return false;
        } else {
            return true;
        }
    } else if (!bLeftIsDir && bRightIsDir) {
        return false;
    } else {
        switch (iColumn) {
        case DC_Name: {

            QString strLeftName = leftIndex.data(Qt::DisplayRole).toString();
            QString strRightName = rightIndex.data(Qt::DisplayRole).toString();

            if (strLeftName.at(0).script() == QChar::Script_Han) {          // 左侧第一个是汉字
                if (strRightName.at(0).script() == QChar::Script_Han) {     // 右侧第一个是汉字
                    QCollator col;
                    if (col.compare(strLeftName, strRightName) < 0) {
                        return true;
                    } else {
                        return false;
                    }
                } else {    // 右侧第一个不是汉字
                    return false;
                }
            } else {    // 左侧第一个不是汉字
                if (strRightName.at(0).script() == QChar::Script_Han) {     // 右侧第一个是汉字
                    return true;
                } else {    // 右侧第一个不是汉字
                    if (QString::compare(strLeftName, strRightName, Qt::CaseInsensitive) < 0) {
                        return true;
                    } else {
                        return false;
                    }
                }
            }
        }
        case DC_Time: {
            QDateTime leftTime = leftIndex.data(Qt::UserRole).toDateTime();
            QDateTime rightTime = rightIndex.data(Qt::UserRole).toDateTime();

            // 比较文件最后一次修改时间
            if (leftTime < rightTime) {
                return true;
            } else {
                return false;
            }
        }
        case DC_Type: {
            QString strLeftType = leftIndex.data(Qt::UserRole).toString();
            QString strRightType = rightIndex.data(Qt::UserRole).toString();

            // 比较显示类型
            QCollator col;
            if (col.compare(strLeftType, strRightType) < 0) {
                return false;
            } else {
                return true;
            }
        }
        case DC_Size: {

            qint64 qLeftSize = leftIndex.data(Qt::UserRole).toLongLong();
            qint64 qRightSize = rightIndex.data(Qt::UserRole).toLongLong();

            if (bLeftIsDir && bRightIsDir) {    // 如果都是文件夹，比较子文件数目
                return qLeftSize < qRightSize;
            } else if (bLeftIsDir) {
                return true;
            } else if (bRightIsDir) {
                return false;
            }

            if (qLeftSize < qRightSize) {
                return true;
            } else {
                return false;
            }
        }
        }
    }
    return QSortFilterProxyModel::lessThan(leftIndex, rightIndex);
}
