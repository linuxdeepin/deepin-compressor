// Copyright (C) 2022 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemviewrowhoverproxystyle.h"
#include "datatreeview.h"

#include <QStyleOptionViewItem>

ItemViewRowHoverProxyStyle::ItemViewRowHoverProxyStyle(QStyle *baseStyle, DataTreeView *treeView)
    : QProxyStyle(baseStyle)
    , m_treeView(treeView)
{
    // Lifetime is managed by the creator (we typically parent to qApp).
}

void ItemViewRowHoverProxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option,
                                                QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_PanelItemViewRow && option && m_treeView && widget == m_treeView) {
        if (const auto *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            if (vopt->state.testFlag(QStyle::State_MouseOver)) {
                QStyleOptionViewItem copy(*vopt);
                copy.state &= ~QStyle::State_MouseOver;
                QProxyStyle::drawPrimitive(element, &copy, painter, widget);
                return;
            }
        }
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
