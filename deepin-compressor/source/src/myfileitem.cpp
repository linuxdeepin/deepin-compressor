/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
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
#include "myfileitem.h"
#include <QDateTime>
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
    case 3:
    {
        if (column() == other.column())
        {
            qint64 lint = Utils::humanReadableToSize(l.toString());
            qint64 rint = Utils::humanReadableToSize(r.toString());
            return lint < rint;
        }
        break;
    }
    case 1:
    {
        if (column() == other.column())
        {
            QDateTime ldate = QDateTime::fromString(l.toString(), "yyyy/MM/dd hh:mm:ss");
            QDateTime rdate = QDateTime::fromString(r.toString(), "yyyy/MM/dd hh:mm:ss");
            return !rdate.operator<(ldate);
        }
        break;
    }
    default:
        break;


    }

    return QStandardItem::operator<(other);
}
