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

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>
#include "DFontSizeManager"
#include "DApplicationHelper"

Compressor_Fail::Compressor_Fail(QWidget *parent)
    : DWidget(parent)
{
    m_stringinfo = tr("Sorry,Extraction failed!");
    m_stringdetail = tr("Damaged file, unable to extract");
    InitUI();
    InitConnection();
}

void Compressor_Fail::InitUI()
{
    m_compressicon = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel(this);
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel(this);
    m_stringinfolabel->setText(m_stringinfo);
//    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
//    font.setWeight(QFont::DemiBold);
//    m_stringinfolabel->setFont(font);
    DFontSizeManager::instance()->bind(m_stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    m_stringinfolabel->setForegroundRole(DPalette::ToolTipText);

    m_stringdetaillabel = new DLabel(this);
    m_stringdetaillabel->setForegroundRole(DPalette::TextTips);
//    m_stringdetaillabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    DFontSizeManager::instance()->bind(m_stringdetaillabel, DFontSizeManager::T8);
    m_stringdetaillabel->setText(m_stringdetail);
    m_retrybutton = new DPushButton(this);
    m_retrybutton->setMinimumSize(340, 36);
    m_retrybutton->setText(tr("Retry"));
    m_retrybutton->setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringdetaillabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();

    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_retrybutton, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);

    mainlayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
}

void Compressor_Fail::InitConnection()
{
    connect(m_retrybutton, &DPushButton::clicked, this, &Compressor_Fail::sigFailRetry);
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

void Compressor_Fail::setFailStr(const QString &str)
{
    m_stringinfo = str;
    m_stringinfolabel->setText(m_stringinfo);
}

void Compressor_Fail::setFailStrDetail(const QString &str)
{
    m_stringdetail = str;
    m_stringdetaillabel->setText(m_stringdetail);
}
