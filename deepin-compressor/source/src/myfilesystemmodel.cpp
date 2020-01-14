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

//#include "myfilesystemmodelprivate.h"
#include "myfilesystemmodel.h"
#include <QDateTime>
#include <QDebug>
#include <utils.h>
#include "DFontSizeManager"
#include "mimetypes.h"

MyFileSystemModel::MyFileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    m_mimetype = new MimeTypeDisplayManager;
    m_showreprevious = false;
    setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
}

void MyFileSystemModel::setPathIndex(int *index)
{
    ppathindex = index;
}

void MyFileSystemModel::setTableView(DTableView *tableview)
{
    m_tableview = tableview;
}

QVariant MyFileSystemModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Name");
        case 3:
            return tr("Size");
        case 2:
            return tr("Type");
        case 1:
            return tr("Time modified");
        default:
            return "-";
        }
    } else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QVariant();
}

QVariant MyFileSystemModel::data(const QModelIndex &index, int role) const
{
    if ((1 == index.row() || 0 == index.row()) && 0 == index.column()) {
        if (ppathindex && *ppathindex > 0) {
            //m_tableview->setRowHeight(0, MyFileSystemDefine::gTableHeight);
            emit sigShowLabel();
        } /*else {
            m_tableview->setRowHeight(0, MyFileSystemDefine::gTableHeight);
        }*/
    }

//    if (0 != index.row()) {
//        m_tableview->setRowHeight(index.row(), MyFileSystemDefine::gTableHeight);
//    }

    if (index.isValid()) {
        QFileInfo file = fileInfo(index);
        switch (role) {
        case Qt::TextColorRole:{
            DPalette pa;
            pa.setBrush(DPalette::WindowText,pa.color(DPalette::WindowText));
            return pa;
        }
        case Qt::TextAlignmentRole:
            /*if (3 == index.column()) {
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
            } else */{
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            }
        case Qt::FontRole:
            if (0 == index.column()) {
                QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
                font.setWeight(QFont::Medium);
                return font;
            } else {
                QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
                font.setWeight(QFont::Normal);
                return font;
            }
        case Qt::DisplayRole: {
            switch (index.column()) {
            case 3: {
                if (file.isDir()) {
//                    return "-";
                    QDir dir(file.filePath());
                    QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files));

                    return QString::number(fileInfo->count()) + " " + tr("item(s)");
                } else {

                    return Utils::humanReadableSize(file.size(), 1);
                }
            }
            case 2: {
                QMimeType mimetype = determineMimeType(file.filePath());
                return m_mimetype->displayName(mimetype.name());
            }
            case 1: {
                return QLocale().toString(lastModified(index), tr("yyyy/MM/dd hh:mm:ss"));
            }
            case 0:{
                return file.fileName();
            }
            }
        }
        }
    }
    return QFileSystemModel::data(index, role);
}

void MyFileSystemModel::sort(int column, Qt::SortOrder order)
{
    if (1 == column) {
        QFileSystemModel::sort(3, order);
    } else if (3 == column) {
        QFileSystemModel::sort(1, order);
    } else {
        QFileSystemModel::sort(column, order);
    }
}

