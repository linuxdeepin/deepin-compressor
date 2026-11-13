// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadingpage.h"
#include "common/ddlog.h"

#include <DLabel>
#include <DFontSizeManager>
#include <DPaletteHelper>

#include <QHBoxLayout>

LoadingPage::LoadingPage(QWidget *parent)
    : DWidget(parent)
{
    qCDebug(appLog) << "LoadingPage constructor called";
    initUI();
    qCDebug(appLog) << "LoadingPage initialization completed";
}

LoadingPage::~LoadingPage()
{
    qCDebug(appLog) << "LoadingPage destructor called";
}

void LoadingPage::startLoading()
{
    qCInfo(appLog) << "Starting loading animation";
    m_pSpinner->start();
}

void LoadingPage::stopLoading()
{
    qCInfo(appLog) << "Stopping loading animation";
    m_pSpinner->stop();
}

void LoadingPage::setDes(const QString &strDes)
{
    qCInfo(appLog) << "Setting loading description:" << strDes;
    m_pTextLbl->setText(strDes);
}

void LoadingPage::initUI()
{
    qCDebug(appLog) << "Initializing loading page UI";
    // 初始化加载动画
    m_pSpinner = new DSpinner(this);
    m_pSpinner->setMinimumSize(32, 32);

    // 初始化加载提示
    m_pTextLbl = new DLabel(this);
    m_pTextLbl->setMinimumSize(293, 20);
    m_pTextLbl->setText(tr("Loading, please wait..."));
    DFontSizeManager::instance()->bind(m_pTextLbl, DFontSizeManager::T6, QFont::Medium);  // 设置字体
    DPalette pa = DPaletteHelper::instance()->palette(m_pTextLbl);
    pa.setBrush(DPalette::ButtonText, pa.color(DPalette::TextTitle));   // 设置颜色
    m_pTextLbl->setPalette(pa);
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
