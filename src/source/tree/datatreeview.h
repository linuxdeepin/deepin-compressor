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

#include "uistruct.h"

#include <DTreeView>

#include <QStyledItemDelegate>
#include <QDir>

DWIDGET_USE_NAMESPACE

class DataModel;
class TreeHeaderView;

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

    /**
     * @brief resetLevel    重置根节点
     */
    void resetLevel();

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief dragEnterEvent    拖拽进入
     */
    void dragEnterEvent(QDragEnterEvent *) override;

    /**
     * @brief dragMoveEvent     拖拽移动
     */
    void dragMoveEvent(QDragMoveEvent *) override;

    /**
     * @brief dropEvent 拖拽放下
     */
    void dropEvent(QDropEvent *) override;

    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief setPreLblVisible  设置返回上一级是否可见
     * @param bVisible          是否可见标志
     * @param strPat            上一级路径
     */
    void setPreLblVisible(bool bVisible, const QString &strPat = "");

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

Q_SIGNALS:
    /**
     * @brief signalDragFiles   外部文件拖拽至列表信号
     * @param listFiles         外部拖拽文件
     */
    void signalDragFiles(const QStringList &listFiles);

protected Q_SLOTS:
    /**
     * @brief slotPreClicked    返回上一级
     */
    virtual void slotPreClicked() = 0;

protected:

    DataModel *m_pModel;

    int m_iLevel = 0;       // 目录层级
    QString m_strCurrentPath = QDir::separator();   // 当前目录

    TreeHeaderView *m_pHeaderView;

private:
    Qt::FocusReason m_reson;

};

#endif // DATATREEVIEW_H
