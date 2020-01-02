/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LZ <zhou.lu@archermind.com>
 *
 * Maintainer: LZ <zhou.lu@archermind.com>
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

#include "logviewheaderview.h"
#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include "DFontSizeManager"
static const int kSpacingMargin = 4;

DWIDGET_USE_NAMESPACE

MyLabel::MyLabel(QWidget *parent)
    : DLabel(parent)
{

}

void MyLabel::paintEvent(QPaintEvent *e)
{
    QRectF rectangle(0, 0, width(), height());
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor;
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        bgColor = QColor(247, 247, 247);
    } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        bgColor = QColor(38, 38, 38);
    }

    QPainterPath pp;
    pp.addRoundedRect(rectangle, 8, 8);
    painter.fillPath(pp, bgColor);
    QLabel::paintEvent(e);
}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit labelDoubleClickEvent(event);
}

PreviousLabel::PreviousLabel(const QString &text, LogViewHeaderView *parent ): DLabel(text, parent),headerView_(parent)
{

}

void PreviousLabel::paintEvent(QPaintEvent *e)
{
    QRectF rectangle(0, 0, width(), height());
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor;
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType())
    {
        if(focusIn_)
        {
            bgColor = QColor(212, 212, 212);
        }
        else
        {
            bgColor = QColor(247, 247, 247);
        }
    }
    else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType())
    {
        if(focusIn_)
        {
            bgColor = QColor(44, 44, 44);
        }
        else
        {
            bgColor = QColor(38, 38, 38);
        }
    }

    QPainterPath pp;
    pp.addRoundedRect(rectangle, 8, 8);
    painter.fillPath(pp, bgColor);
    QLabel::paintEvent(e);
}

void PreviousLabel::hideEvent(QHideEvent *event)
{
    move(0, 0);
    headerView_->setFixedHeight(38);
    QLabel::hideEvent(event);
    headerView_->update();
}

void PreviousLabel::showEvent(QShowEvent *event)
{
    move(0, 38);
    headerView_->setFixedHeight(74);
    QLabel::showEvent(event);
    headerView_->update();
}

void PreviousLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit doubleClickedSignal();
    QLabel::mouseDoubleClickEvent(event);
}

void PreviousLabel::enterEvent(QEvent *event)
{
    focusIn_ = true;
    DLabel::enterEvent(event);
    update();
}

void PreviousLabel::leaveEvent(QEvent *event)
{
    focusIn_ = false;
    DLabel::leaveEvent(event);
    update();
}

LogViewHeaderView::LogViewHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent), gotoPreviousLabel_( new PreviousLabel("     .. " + tr("Back"), this))
{
    setFixedHeight(38);
    viewport()->setAutoFillBackground(false);
    setSectionsClickable(true);
    setHighlightSections(true);
    setFrameShape(DHeaderView::NoFrame);

    gotoPreviousLabel_->setFixedHeight(36);
    gotoPreviousLabel_->hide();
    DFontSizeManager::instance()->bind(gotoPreviousLabel_, DFontSizeManager::T6, QFont::Weight::Medium);
}

void LogViewHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    QStyleOptionHeader option;
    initStyleOption(&option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    // title
    QRect contentRect(rect.x(), rect.y(), rect.width(), 36 - m_spacing);
    QRect hSpacingRect(rect.x(), contentRect.height() - 1, rect.width(),
                       36 - contentRect.height() - 2);

    QBrush contentBrush(palette.color(cg, DPalette::Base));
    // horizontal spacing
    QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    // vertical spacing
    QBrush vSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    QRectF vSpacingRect(rect.x(), rect.y() + kSpacingMargin+ 2, m_spacing,
                        36 - kSpacingMargin * 2 - 6);
    QBrush clearBrush(palette.color(cg, DPalette::Window));

    painter->fillRect(hSpacingRect, clearBrush);
    painter->fillRect(hSpacingRect, hSpacingBrush);

    if (visualIndex(logicalIndex) > 0) {
        painter->fillRect(vSpacingRect, clearBrush);
        painter->fillRect(vSpacingRect, vSpacingBrush);
    }

    // TODO: dropdown icon (8x5)
    QRect textRect(contentRect.x() + 6, contentRect.y(), contentRect.width() - 23,
                   contentRect.height());
    QString title = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    //    int align = model()->headerData(logicalIndex, orientation(),
    //    Qt::TextAlignmentRole).toInt();
    static int align = Qt::AlignLeft | Qt::AlignVCenter;
    QFont pFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    pFont.setWeight(QFont::Weight::Medium);
    painter->setFont(pFont);
    if (logicalIndex == 0) {
        QRect col0Rect = textRect;
        col0Rect.setX(textRect.x()+2);
        painter->drawText(col0Rect, static_cast<int>(align), title);
    }
//    else if(logicalIndex == 3){
//        painter->drawText(textRect, static_cast<int>(Qt::AlignRight | Qt::AlignVCenter), title);
//    }
    else {
        painter->drawText(textRect, static_cast<int>(align), title);
    }

    // sort indicator
    if (isSortIndicatorShown() && logicalIndex == sortIndicatorSection()) {
        // TODO: arrow size (8x5)
        QRect sortIndicator(textRect.x() + textRect.width() - ((logicalIndex == 0) ? 0: 1),
                            textRect.y() + (textRect.height() - 5) / 2, 8, 8);
        option.rect = sortIndicator;
        if (sortIndicatorOrder() == Qt::DescendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowDown, &option, painter, this);
        } else if (sortIndicatorOrder() == Qt::AscendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowUp, &option, painter, this);
        }
    }

    painter->restore();
}

void LogViewHeaderView::resizeEvent(QResizeEvent *event)
{
    gotoPreviousLabel_->setFixedWidth(event->size().width());
    DHeaderView::resizeEvent(event);
}

void LogViewHeaderView::paintEvent(QPaintEvent *event)
{
//    QPainter painter(viewport());
//    painter.save();

//    QWidget *wnd = DApplication::activeWindow();
//    DPalette::ColorGroup cg;
//    if (!wnd) {
//        cg = DPalette::Inactive;
//    } else {
//        cg = DPalette::Active;
//    }

//    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
//    DPalette palette = dAppHelper->applicationPalette();

//    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

//    QBrush bgBrush(palette.color(cg, DPalette::Base));

//    QStyleOptionHeader option;
//    initStyleOption(&option);
//    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

//    QRect rect = viewport()->rect();
//    rect.setHeight(36);
//    QRectF clipRect(rect.x(), rect.y(), rect.width(), rect.height() * 2);
//    QRectF subRect(rect.x(), rect.y() + rect.height(), rect.width(), rect.height());
//    QPainterPath clipPath, subPath;
//    clipPath.addRoundedRect(clipRect, radius, radius);
//    subPath.addRect(subRect);
//    clipPath = clipPath.subtracted(subPath);

//    painter.fillPath(clipPath, bgBrush);

//    painter.restore();

    DHeaderView::paintEvent(event);
}

QSize LogViewHeaderView::sizeHint() const
{
    QSize size = sectionSizeFromContents(0);
    return QSize(size.width(), size.height() + m_spacing);
}
