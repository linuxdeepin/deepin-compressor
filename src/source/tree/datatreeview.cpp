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

#include "datatreeview.h"
#include "treeheaderview.h"
#include "datamodel.h"

#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>

#include <QMouseEvent>
#include <QPainter>
#include <QHeaderView>
#include <QMimeData>
#include <QDebug>

StyleTreeViewDelegate::StyleTreeViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

StyleTreeViewDelegate::~StyleTreeViewDelegate()
{

}

QSize StyleTreeViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QSize(option.rect.width(), 36);
}

void StyleTreeViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);
    //设置高亮文字色
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));
    if (opt.state & DStyle::State_Enabled) {
        if (opt.state & DStyle::State_Selected) {
            forground.setColor(palette.color(cg, DPalette::HighlightedText));
        }
    }
    painter->setPen(forground);
    QRect rect = opt.rect;
    QFontMetrics fm(opt.font);
    QPainterPath path, clipPath;
    QRect textRect = rect;
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::Beginning: {
        // 左间距
        rect.setX(rect.x() + margin);
    } break;
    case QStyleOptionViewItem::Middle: {
    } break;
    case QStyleOptionViewItem::End: {
        // 右间距
        rect.setWidth(rect.width() - margin);
    } break;
    case QStyleOptionViewItem::OnlyOne: {
        // 左间距
        rect.setX(rect.x() + margin);
        // 右间距
        rect.setWidth(rect.width() - margin);
    } break;
    default: {
        painter->restore();
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }

    //绘制图标
    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning &&
            index.data(Qt::DecorationRole).isValid()) {
        // icon size
        auto iconSize = style->pixelMetric(DStyle::PM_ListViewIconSize, &option);
        QRect iconRect = rect;
        iconRect.setX(rect.x() + margin);
        iconRect.setWidth(iconSize);
        // 缩放大小并绘制
        auto diff = (iconRect.height() - iconSize) / 2;
        opt.icon.paint(painter, iconRect.adjusted(0, diff, 0, -diff));
    }

    //绘制文字
    textRect = rect;
    if (index.column() == 0) {
        textRect.setX(textRect.x() + margin + 32 - 2);
    } else {
        textRect.setX(textRect.x() + margin - 2);
    }
    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}


DataTreeView::DataTreeView(QWidget *parent)
    : DTreeView(parent)
{
    initUI();
    initConnections();
}

DataTreeView::~DataTreeView()
{

}

void DataTreeView::resetLevel()
{
    m_iLevel = 0;
    m_strCurrentPath = QDir::separator();
    setPreLblVisible(false);
}

void DataTreeView::initUI()
{
    setObjectName("TableViewFile");
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setIconSize(QSize(24, 24));
    setViewportMargins(10, 10, 10, 0);
    setIndentation(0);
    setFrameShape(QFrame::NoFrame);     // 设置无边框
    resizeColumnWidth();
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setFocusPolicy(Qt::StrongFocus);

    // 设置样式代理
    StyleTreeViewDelegate *pDelegate = new StyleTreeViewDelegate;
    setItemDelegate(pDelegate);

    // 设置表头
    m_pHeaderView = new TreeHeaderView(Qt::Horizontal, this);
    m_pHeaderView->setStretchLastSection(true);
    setHeader(m_pHeaderView);

    m_pModel = new DataModel(this);
    setModel(m_pModel);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);    // 设置自定义右键菜单
    setAcceptDrops(true);

    m_selectionModel = selectionModel();
}

void DataTreeView::initConnections()
{
    connect(m_pHeaderView->getpreLbl(), &PreviousLabel::doubleClickedSignal, this, &DataTreeView::slotPreClicked);
}

void DataTreeView::resizeColumnWidth()
{
    setColumnWidth(0, width() * 25 / 58);
    setColumnWidth(1, width() * 17 / 58);
    setColumnWidth(2, width() * 8 / 58);
    setColumnWidth(3, width() * 8 / 58);
}

TreeHeaderView *DataTreeView::getHeaderView() const
{
    return m_pHeaderView;
}

void DataTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
#endif
    DPalette::ColorGroup cg;
    if (!(options.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;
            }
        }
#else
        cg = DPalette::Active;
#endif
    }

    auto *style = dynamic_cast<DStyle *>(DApplication::style());

    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &options);
    auto margin = style->pixelMetric(DStyle::PM_ContentsMargins, &options);
    //根据实际情况设置颜色，奇数行为灰色
    auto palette = options.palette;
    QBrush background;
    if (!(index.row() & 1)) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }
    if (options.state & DStyle::State_Enabled) {
        if (m_selectionModel->isSelected(index)) {
            background = palette.color(cg, DPalette::Highlight);
        }
    }

    // 绘制整行背景，高度-2以让高分屏非整数缩放比例下无被选中的蓝色细线，防止原来通过delegate绘制单元格交替颜色背景出现的高分屏非整数缩放比例下qrect精度问题导致的横向单元格间出现白色边框
    QPainterPath path;
    QRect rowRect { options.rect.x() - header()->offset(),
                    options.rect.y() + 1,
                    header()->length() - header()->sectionPosition(0),
                    options.rect.height() - 2 };
    rowRect.setX(rowRect.x() + margin);
    rowRect.setWidth(rowRect.width() - margin);

    path.addRoundedRect(rowRect, radius, radius);
    painter->fillPath(path, background);

    QTreeView::drawRow(painter, options, index);
    // draw focus
    if (hasFocus() && currentIndex().row() == index.row() && (m_reson == Qt::TabFocusReason || m_reson == Qt::BacktabFocusReason)) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(options);
        o.state |= QStyle::State_KeyboardFocusChange | QStyle::State_HasFocus;
        o.rect = style->visualRect(layoutDirection(), viewport()->rect(), rowRect);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, painter);
    }

    painter->restore();
}

void DataTreeView::focusInEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    // qDebug() << m_reson << model()->rowCount() << currentIndex();
    if (Qt::BacktabFocusReason == m_reson || Qt::TabFocusReason == m_reson) { // 修复不能多选删除
        if (model()->rowCount() > 0) {
            if (currentIndex().isValid()) {
                m_selectionModel->select(currentIndex(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            } else {
                QModelIndex firstModelIndex = model()->index(0, 0);
                m_selectionModel->select(firstModelIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }

    DTreeView::focusInEvent(event);
}

void DataTreeView::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // 判断是否有url
    if (!mime->hasUrls()) {
        e->ignore();
    } else {
        e->accept();
    }
}

void DataTreeView::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void DataTreeView::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (false == mime->hasUrls()) {
        e->ignore();
    }

    e->accept();

    // 判断本地文件
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        fileList << url.toLocalFile();
    }

    if (fileList.size() == 0) {
        return;
    }

    emit signalDragFiles(fileList);
}

void DataTreeView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    resizeColumnWidth();
}

void DataTreeView::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Delete == event->key() && m_selectionModel->selectedRows().count() > 0) { //删除键
        slotDeleteFile();
    } else if (Qt::Key_M == event->key() && Qt::AltModifier == event->modifiers()
               && m_selectionModel->selectedRows().count() > 0) { //Alt+M组合键调用右键菜单
        int y =   36 * currentIndex().row() + 36 / 2; //获取选中行y坐标+行高/2,列表行高36
        int x = static_cast<int>(width() * 0.618); //比较合适的x坐标

        slotShowRightMenu(QPoint(x, y));
    } else if ((Qt::Key_Enter == event->key() || Qt::Key_Return == event->key())
               && m_selectionModel->selectedRows().count() > 0) { //回车键以默认方式打开文件(夹)
        handleDoubleClick(currentIndex());
    } else {
        DTreeView::keyPressEvent(event);
    }
}

void DataTreeView::setPreLblVisible(bool bVisible, const QString &strPath)
{
    m_pHeaderView->getpreLbl()->setPrePath(strPath);
    m_pHeaderView->setPreLblVisible(bVisible);
}
