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
#include "extractpausedialog.h"
#include <utils.h>

#include <DFontSizeManager>
#include <DLabel>
#include <DWidget>
#include <DApplicationHelper>

#include <QBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QAbstractButton>

DWIDGET_USE_NAMESPACE

ExtractPauseDialog::ExtractPauseDialog(QWidget *parent):
    DDialog(parent)
{
    initUI();
    initConnect();
}

void ExtractPauseDialog::initUI()
{
    QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);

    DPalette pa;

//    DLabel *strlabel = new DLabel(this);
//    strlabel->setFixedHeight(20);
//    strlabel->setForegroundRole(DPalette::WindowText);

//    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
//    strlabel->setText(tr("Stop extracting"));
    DLabel *strlabel2 = new DLabel(this);
    strlabel2->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    pa = DApplicationHelper::instance()->palette(strlabel2);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ToolTipText));
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);

//    strlabel2->setFixedHeight(20);
//    strlabel2->setForegroundRole(DPalette::ToolTipText);
//    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::DemiBold);

    strlabel2->setText(tr("Are you sure you want to stop the extraction?"));

    addButton(tr("Cancel"));
    addButton(tr("Confirm"), true, ButtonRecommend);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 4);
    effect->setColor(QColor(0, 145, 255, 76));
    effect->setBlurRadius(4);
    getButton(1)->setGraphicsEffect(effect);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    //mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(15);

    DWidget *widget = new DWidget(this);

    widget->setLayout(mainlayout);
    addContent(widget);

//    moveToCenter();
}

void ExtractPauseDialog::initConnect()
{
    connect(this, &DDialog::buttonClicked, this, &ExtractPauseDialog::clickedSlot);
}

void ExtractPauseDialog::clickedSlot(int index, const QString &/*text*/)
{
    emit sigbuttonpress(index);
}

void ExtractPauseDialog::closeEvent(QCloseEvent */*event*/)
{
    emit sigbuttonpress(2);
}
