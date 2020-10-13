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

#ifndef DATATREEVIEW_H
#define DATATREEVIEW_H

#include <DTreeView>

#include <QStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class SortFilterModel;

// treeview代理，用来设置高度和去掉部分样式
class StyleTreeViewDelegate : public QStyledItemDelegate
{
public:
    StyleTreeViewDelegate(QObject *parent = nullptr);
    ~StyleTreeViewDelegate() override;

    // QAbstractItemDelegate interface
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // QAbstractItemDelegate interface
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

// 自定义树形列表
class DataTreeView : public DTreeView
{
    Q_OBJECT
public:
    explicit DataTreeView(QWidget *parent = nullptr);
    ~DataTreeView() override;

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const;
    void focusInEvent(QFocusEvent *event)override;

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

protected:
    /**
     * @brief resizeColumnWidth 重置列宽度
     */
    void resizeColumnWidth();

private:
    Qt::FocusReason m_reson;
};

#endif // DATATREEVIEW_H
