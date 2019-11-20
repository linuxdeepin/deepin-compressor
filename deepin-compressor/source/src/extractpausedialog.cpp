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
#include "extractpausedialog.h"
#include <QBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <utils.h>

ExtractPauseDialog::ExtractPauseDialog(QWidget *parent):
    DDialog(parent)
{
    initUI();
    initConnect();
}

void ExtractPauseDialog::initUI()
{


    QPixmap pixmap = Utils::renderSVG(":/images/warning.svg", QSize(32, 32));
    setIconPixmap(pixmap);

    DPalette pa;

    DLabel* strlabel = new DLabel;
    strlabel->setFixedHeight(20);
    pa = DApplicationHelper::instance()->palette(strlabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::Text));
    strlabel->setPalette(pa);
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    font.setWeight(QFont::Medium);
    strlabel->setFont(font);
    strlabel->setText(tr("Stop extracting!"));
    DLabel* strlabel2 = new DLabel;
    strlabel2->setFixedHeight(20);
    pa = DApplicationHelper::instance()->palette(strlabel2);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    strlabel2->setPalette(pa);

    font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    font.setWeight(QFont::DemiBold);
    strlabel2->setFont(font);

    strlabel2->setText(tr("There is currently a extraction tasks in progress"));

    addButton(tr("Stop extracting"));
    addButton(tr("Continue extracting"), true, ButtonRecommend);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    effect->setOffset(0,4);
    effect->setColor(QColor(0,145,255, 76));
    effect->setBlurRadius(4);
    getButton(1)->setGraphicsEffect(effect);

    QVBoxLayout* mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(15);

    DWidget* widget = new DWidget;

    widget->setLayout(mainlayout);
    addContent(widget);

//    moveToCenter();

}

void ExtractPauseDialog::initConnect()
{
    connect(this, &DDialog::buttonClicked, this, &ExtractPauseDialog::clickedSlot);
}

void ExtractPauseDialog::clickedSlot(int index, const QString &text)
{
    emit sigbuttonpress(index);
}

void ExtractPauseDialog::closeEvent(QCloseEvent *event)
{
    emit sigbuttonpress(2);
}

