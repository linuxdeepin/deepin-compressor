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
#include "compressor_fail.h"
#include "customwidget.h"
#include "utils.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DFileDialog>

#include <QBoxLayout>

Compressor_Fail::Compressor_Fail(QWidget *parent)
    : DWidget(parent)
{
    m_stringinfo = tr("Extraction failed");
    m_stringdetail = tr("Damaged file, unable to extract");
    InitUI();
    InitConnection();
}

Compressor_Fail::~Compressor_Fail()
{
    SAFE_DELETE_ELE(m_stringdetaillabel);
    SAFE_DELETE_ELE(m_pixmaplabel);
    SAFE_DELETE_ELE(m_stringinfolabel);
    SAFE_DELETE_ELE(m_retrybutton);
    SAFE_DELETE_ELE(commandLinkBackButton);
}

void Compressor_Fail::InitUI()
{
    // 设置失败显示的图标
    m_compressicon = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel(this);
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel(this);
    m_stringinfolabel->setText(m_stringinfo); // 设置显示信息
    //    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
    //    font.setWeight(QFont::DemiBold);
    //    m_stringinfolabel->setFont(font);
    DFontSizeManager::instance()->bind(m_stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    m_stringinfolabel->setForegroundRole(DPalette::ToolTipText);
    // 展示详细信息的控件
    m_stringdetaillabel = new DLabel(this);
    m_stringdetaillabel->setForegroundRole(DPalette::TextTips);
//    m_stringdetaillabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    DFontSizeManager::instance()->bind(m_stringdetaillabel, DFontSizeManager::T8);
    m_stringdetaillabel->setText(m_stringdetail);
    m_retrybutton = new CustomPushButton(this);
    m_retrybutton->setAccessibleName("Retry_Btn");
    m_retrybutton->setMinimumSize(340, 36);
    m_retrybutton->setText(tr("Retry"));

    commandLinkBackButton = new CustomCommandLinkButton(tr("Back"), this);
    commandLinkBackButton->setAccessibleName("Back_btn");
    QHBoxLayout *commandLinkButtonLayout = new QHBoxLayout;
    commandLinkButtonLayout->addStretch();
    commandLinkButtonLayout->addWidget(commandLinkBackButton);
    commandLinkButtonLayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringdetaillabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    // 按钮layout
    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_retrybutton, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);
    mainlayout->addLayout(commandLinkButtonLayout);

    mainlayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
}
/**
 * @brief Compressor_Fail::InitConnection 初始化信号
 */
void Compressor_Fail::InitConnection()
{
    // 绑定失败重试信号
    connect(m_retrybutton, &DPushButton::clicked, this, &Compressor_Fail::sigFailRetry);
    connect(commandLinkBackButton, &DCommandLinkButton::clicked, this, &Compressor_Fail::commandLinkBackButtonClicked);
//    auto changeTheme = [this]() {
//        DPalette pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
//        pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
//        m_stringinfolabel->setPalette(pa);

//        pa = DApplicationHelper::instance()->palette(m_stringdetaillabel);
//        pa.setBrush(DPalette::Text, pa.color(DPalette::TextTips));
//        m_stringdetaillabel->setPalette(pa);
//    };

//    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, changeTheme);
}
/**
 * @brief Compressor_Fail::setFailStr 设置错误展示的标题
 * @param str
 */
void Compressor_Fail::setFailStr(const QString &str)
{
    m_stringinfo = str;
    m_stringinfolabel->setText(m_stringinfo);
}
/**
 * @brief Compressor_Fail::setFailStrDetail 设置带显示的
 * @param str
 */
void Compressor_Fail::setFailStrDetail(const QString &str)
{
    m_stringdetail = str;
    m_stringdetaillabel->setText(m_stringdetail);
}

void Compressor_Fail::setErrorTitle(const QString &strTitle)
{
    m_stringinfolabel->setText(strTitle);
}
/**
 * @brief Compressor_Fail::getRetrybutton 获取重试的失败按钮
 * @return
 */
//CustomPushButton *Compressor_Fail::getRetrybutton()
//{
//    return m_retrybutton;
//}
/**
 * @brief Compressor_Fail::commandLinkBackButtonClicked
 */
void Compressor_Fail::commandLinkBackButtonClicked()
{
    emit sigBackButtonClickedOnFail();
}
