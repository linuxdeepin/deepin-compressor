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

#include "popupdialog.h"
#include "uitools.h"

#include <DWidget>
#include <DFontSizeManager>

#include <QVBoxLayout>

TipDialog::TipDialog(QWidget *parent)
    : DDialog(parent)
{
    setMinimumSize(380, 140);

    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

TipDialog::~TipDialog()
{

}

int TipDialog::showDialog(const QString &strDesText, const QString btnMsg, ButtonType btnType)
{
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setMinimumSize(293, 20);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setWordWrap(true);
    pDesLbl->setAlignment(Qt::AlignCenter);

    // 确定按钮
    addButton(btnMsg, true, btnType);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addWidget(pDesLbl, 0, Qt::AlignVCenter);
    pMainLayout->addSpacing(10);

    DWidget *widget = new DWidget(this);

    widget->setLayout(pMainLayout);
    addContent(widget);


    return exec();
}




SimpleQueryDialog::SimpleQueryDialog(QWidget *parent)
    : DDialog(parent)
{
    setMinimumSize(380, 140);

    // 设置对话框图标
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    setIcon(pixmap);
}

SimpleQueryDialog::~SimpleQueryDialog()
{

}

int SimpleQueryDialog::showDialog(const QString &strDesText, const QString btnMsg1, DDialog::ButtonType btnType1, const QString btnMsg2, DDialog::ButtonType btnType2)
{
    // 描述内容
    DLabel *pDesLbl = new DLabel(this);
    pDesLbl->setMinimumSize(293, 20);
    pDesLbl->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(pDesLbl, DFontSizeManager::T6, QFont::Medium);
    pDesLbl->setText(strDesText);
    pDesLbl->setWordWrap(true);
    pDesLbl->setAlignment(Qt::AlignCenter);

    // 确定按钮
    addButton(btnMsg1, true, btnType1);
    addButton(btnMsg2, true, btnType2);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addWidget(pDesLbl, 0, Qt::AlignVCenter);
    pMainLayout->addSpacing(10);

    DWidget *widget = new DWidget(this);

    widget->setLayout(pMainLayout);
    addContent(widget);

    return exec();
}
