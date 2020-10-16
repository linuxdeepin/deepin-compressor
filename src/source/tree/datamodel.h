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
#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "commonstruct.h"

#include <QAbstractTableModel>
#include <QFileInfoList>
#include <QDir>

class MimeTypeDisplayManager;

class DataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DataModel(QObject *parent = nullptr);
    ~DataModel() override;

public:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    void sort(int column, Qt::SortOrder order) override;

    /**
     * @brief setFileEntry  刷新列表数据
     * @param listEntry     需要显示的数据
     */
    void refreshFileEntry(const QList<FileEntry> &listEntry);

private:
    QList<FileEntry> m_listEntry;       // 显示的数据
    MimeTypeDisplayManager *m_pMimetype; // 类型管理
};

#endif // DATAMODEL_H
