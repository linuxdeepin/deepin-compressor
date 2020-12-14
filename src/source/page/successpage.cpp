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

#include "successpage.h"
#include "uitools.h"
#include "customwidget.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QDebug>

SuccessPage::SuccessPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

SuccessPage::~SuccessPage()
{

}

void SuccessPage::setCompressFullPath(const QString &strFullPath)
{
    m_strFullPath = strFullPath;
}

void SuccessPage::setSuccessDes(const QString &strDes)
{
    m_pSuccessLbl->setText(strDes);
}

void SuccessPage::setSuccessType(SuccessInfo successInfo)
{
    m_successInfoType = successInfo;
}

SuccessInfo SuccessPage::getSuccessType()
{
    return m_successInfoType;
}

void SuccessPage::initUI()
{
    //成功图标
    m_pSuccessPixmapLbl = new DLabel(this);
    QPixmap m_pSuccessPixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_128px.svg", QSize(128, 128));
    m_pSuccessPixmapLbl->setPixmap(m_pSuccessPixmap);

    //成功文字
    m_pSuccessLbl = new DLabel(this);
    DFontSizeManager::instance()->bind(m_pSuccessLbl, DFontSizeManager::T5, QFont::DemiBold);
    m_pSuccessLbl->setForegroundRole(DPalette::ToolTipText);
    m_pSuccessLbl->setText(tr("Compression successful"));

    //查看文件按钮
    m_pShowFileBtn = new CustomPushButton(this);
    m_pShowFileBtn->setMinimumSize(340, 36);
    m_pShowFileBtn->setText(tr("View"));

    //返回按钮
    m_pReturnBtn = new CustomCommandLinkButton(tr("Back"), this);

    //界面布局
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pSuccessPixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_pSuccessLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();

    QHBoxLayout *commandLinkButtonLayout = new QHBoxLayout;
    commandLinkButtonLayout->addStretch();
    commandLinkButtonLayout->addWidget(m_pReturnBtn);
    commandLinkButtonLayout->addStretch();

    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_pShowFileBtn, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);
    mainlayout->addLayout(commandLinkButtonLayout);
    mainlayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void SuccessPage::initConnections()
{
    connect(m_pShowFileBtn, &DPushButton::clicked, this, &SuccessPage::signalViewFile);
    connect(m_pReturnBtn, &DCommandLinkButton::clicked, this, &SuccessPage::sigBackButtonClicked);
}

//void SuccessPage::slotShowfiledirSlot()
//{

//}
