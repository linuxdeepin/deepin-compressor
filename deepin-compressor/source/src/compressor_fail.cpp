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
#include "compressor_fail.h"

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>


Compressor_Fail::Compressor_Fail(QWidget *parent)
    : QWidget(parent)
{
    m_stringinfo = tr("抱歉，解压失败！");
    m_stringdetail = tr("压缩文件已损坏");
    InitUI();
    InitConnection();
}

void Compressor_Fail::InitUI()
{
    DPalette pa;

    m_compressicon = Utils::renderSVG(":/images/fail.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel();
    m_stringinfolabel->setText(m_stringinfo);
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
    font.setBold(true);
    m_stringinfolabel->setFont(font);

    pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
    m_stringinfolabel->setPalette(pa);

    m_stringdetaillabel = new DLabel();
    pa = DApplicationHelper::instance()->palette(m_stringdetaillabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::TextTips));
    m_stringdetaillabel->setPalette(pa);
    m_stringdetaillabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    m_stringdetaillabel->setText(m_stringdetail);
    m_retrybutton = new DPushButton();
    m_retrybutton->setFixedSize(340, 36);
    m_retrybutton->setText(tr("重 试"));
    m_retrybutton->setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringdetaillabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_retrybutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    setBackgroundRole(DPalette::Base);
}

void Compressor_Fail::InitConnection()
{
    connect(m_retrybutton, &DPushButton::clicked, this, &Compressor_Fail::sigFailRetry);
}

void Compressor_Fail::setFailStr(const QString& str)
{
    m_stringinfo = str;
    m_stringinfolabel->setText(m_stringinfo);
}

void Compressor_Fail::setFailStrDetail(const QString& str)
{
    m_stringdetail = str;
    m_stringdetaillabel->setText(m_stringdetail);
}
