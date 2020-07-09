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
#include "myfileitem.h"
#include "utils.h"

#include <QDateTime>
#include <QDebug>
#include <QObject>

MyFileItem::MyFileItem() {}

MyFileItem::MyFileItem(const QString &text)
    : QStandardItem(text)
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
    case 3: {
        if (column() == other.column()) {
            QString lstr = l.toString();
            QString rstr = r.toString();

            if (lstr.contains(QObject::tr("item(s)")) &&
                    !rstr.contains(QObject::tr("item(s)"))) {
                return true;
            } else if (!lstr.contains(QObject::tr("item(s)")) &&
                       rstr.contains(QObject::tr("item(s)"))) {
                return false;
            } else if (lstr.contains(QObject::tr("item(s)")) &&
                       rstr.contains(QObject::tr("item(s)"))) {
                lstr = lstr.remove(QRegExp("\\s"));
                lstr = lstr.remove(QObject::tr("item(s)"));
                rstr = rstr.remove(QRegExp("\\s"));
                rstr = rstr.remove(QObject::tr("item(s)"));
                return lstr.toInt() < rstr.toInt();
            }
            lstr = l.toString().simplified();
            rstr = r.toString().simplified();

            qint64 lint = Utils::humanReadableToSize(lstr);
            qint64 rint = Utils::humanReadableToSize(rstr);
            return lint < rint;
        }
        break;
    }
    case 1: {
        if (column() == other.column()) {
            QString lstr = l.toString();
            QString rstr = r.toString();
            lstr = lstr.remove(4, 1);
            lstr = lstr.remove(6, 1);
            lstr = lstr.remove(8, 1);
            rstr = rstr.remove(4, 1);
            rstr = rstr.remove(6, 1);
            rstr = rstr.remove(8, 1);
            //            QDateTime ldate = QDateTime::fromString(lstr, "yyyyMMdd hh:mm:ss");
            //            //delete by Airy

            //            QDateTime rdate = QDateTime::fromString(rstr, "yyyyMMdd hh:mm:ss");
            //            //delete by Airy

            QDateTime ldate = QDateTime::fromString(lstr, "yyyyMMddhh:mm:ss");  // add by Airy

            QDateTime rdate = QDateTime::fromString(rstr, "yyyyMMddhh:mm:ss");  // add by Airy
            return !rdate.operator < (ldate);
        }
        break;
    }
    case 0: {
        if (column() == other.column()) {
            QString lstr = l.toString();
            QString rstr = r.toString();
            lstr = lstr.remove(QRegExp("\\s"));
            rstr = rstr.remove(QRegExp("\\s"));
            return lstr < rstr;
        }
        break;
    }
    default:
        break;
    }

    return QStandardItem::operator<(other);
}
