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
#include "openloadingpage.h"

#include <DLabel>

#include <QVBoxLayout>
#include <DFontSizeManager>
#include <DApplicationHelper>

OpenLoadingPage::OpenLoadingPage(DWidget *parent)
    : DWidget(parent)
{
    initUI();
}

void OpenLoadingPage::start()
{
    m_pSpinner->start();
}

void OpenLoadingPage::stop()
{
    m_pSpinner->stop();
}

void OpenLoadingPage::initUI()
{
    m_pSpinner = new DSpinner(this);
    m_pSpinner->setMinimumSize(32, 32);

    DLabel *pTextLbl = new DLabel(this);
    pTextLbl->setMinimumSize(293, 20);
    pTextLbl->setText(tr("Loading, please wait..."));
    DFontSizeManager::instance()->bind(pTextLbl, DFontSizeManager::T6, QFont::Medium);

    DPalette pa = DApplicationHelper::instance()->palette(pTextLbl);
    pa.setBrush(DPalette::ButtonText, pa.color(DPalette::TextTitle));
    DApplicationHelper::instance()->setPalette(pTextLbl, pa);
//    pTextLbl->setForegroundRole(DPalette::ButtonText);
    pTextLbl->setAlignment(Qt::AlignCenter);

    QHBoxLayout *pSpinnerLayout = new QHBoxLayout;
    pSpinnerLayout->addStretch();
    pSpinnerLayout->addWidget(m_pSpinner);
    pSpinnerLayout->addStretch();

    QVBoxLayout *pLayout = new QVBoxLayout(/*this*/);
    pLayout->addStretch();
    pLayout->addLayout(pSpinnerLayout);
    pLayout->addWidget(pTextLbl);
    pLayout->addStretch();
    pLayout->setAlignment(Qt::AlignCenter);

    setLayout(pLayout);
}
