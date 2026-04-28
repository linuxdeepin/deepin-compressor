// Copyright (C) 2022 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMVIEWROWHOVERPROXYSTYLE_H
#define ITEMVIEWROWHOVERPROXYSTYLE_H

#include <QProxyStyle>
#include <QPointer>

class DataTreeView;

/**
 * Clears State_MouseOver before PE_PanelItemViewRow so DStyle does not paint
 * per-cell hover; DataTreeView draws full-row hover instead (mirrors how
 * selection is not double-drawn per cell).
 */
class ItemViewRowHoverProxyStyle : public QProxyStyle
{
public:
    explicit ItemViewRowHoverProxyStyle(QStyle *baseStyle, DataTreeView *treeView);

protected:
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const override;

private:
    QPointer<DataTreeView> m_treeView;
};

#endif
