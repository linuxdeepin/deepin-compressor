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
#ifndef MyFILESYSTEMMODEL_H
#define MyFILESYSTEMMODEL_H

#include <QtWidgets/qtwidgetsglobal.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpair.h>
#include <QtCore/qdir.h>
#include <QtGui/qicon.h>
#include <QtCore/qdiriterator.h>
#include <QFileSystemModel>
#include <QStandardItem>
#include <QTableView>
#include "mimetypedisplaymanager.h"
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

namespace  MyFileSystemDefine {
const int  gTableHeight = 36;
}

QT_REQUIRE_CONFIG(filesystemmodel);

QT_BEGIN_NAMESPACE

class ExtendedInformation;
class MyFileSystemModelPrivate;
class MyFileIconProvider;

class MyFileSystemModel : public QFileSystemModel
{
public:
    explicit MyFileSystemModel(QObject *parent = nullptr);
    ~MyFileSystemModel();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void setPathIndex(int *index);
    void setTableView(QTableView *tableview);



private:
    bool m_showreprevious;
    int *ppathindex;
    QTableView *m_tableview;
    MimeTypeDisplayManager *m_mimetype;

};

#endif // MyFILESYSTEMMODEL_H
