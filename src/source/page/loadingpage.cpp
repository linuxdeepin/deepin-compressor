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

#include "loadingpage.h"

#include <DLabel>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QHBoxLayout>

LoadingPage::LoadingPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
}

LoadingPage::~LoadingPage()
{

}

void LoadingPage::startLoading()
{
    m_pSpinner->start();
}

void LoadingPage::stopLoading()
{
    m_pSpinner->stop();
}

void LoadingPage::setDes(const QString &strDes)
{
    m_pTextLbl->setText(strDes);
}

void LoadingPage::initUI()
{
    // 初始化加载动画
    m_pSpinner = new DSpinner(this);
    m_pSpinner->setMinimumSize(32, 32);

    // 初始化加载提示
    m_pTextLbl = new DLabel(this);
    m_pTextLbl->setMinimumSize(293, 20);
    m_pTextLbl->setText(tr("Loading, please wait..."));
    DFontSizeManager::instance()->bind(m_pTextLbl, DFontSizeManager::T6, QFont::Medium);  // 设置字体
    DPalette pa = DApplicationHelper::instance()->palette(m_pTextLbl);
    pa.setBrush(DPalette::ButtonText, pa.color(DPalette::TextTitle));   // 设置颜色
    DApplicationHelper::instance()->setPalette(m_pTextLbl, pa);
    m_pTextLbl->setAlignment(Qt::AlignCenter);

    // 加载布局
    QHBoxLayout *pSpinnerLayout = new QHBoxLayout;
    pSpinnerLayout->addStretch();
    pSpinnerLayout->addWidget(m_pSpinner);
    pSpinnerLayout->addStretch();

    // 主布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pSpinnerLayout);
    pMainLayout->addWidget(m_pTextLbl);
    pMainLayout->addStretch();
    pMainLayout->setAlignment(Qt::AlignCenter);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}
