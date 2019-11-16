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
#include "compressor_success.h"

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>
#include <DDesktopServices>


Compressor_Success::Compressor_Success(QWidget *parent)
    : QWidget(parent)
{
    m_stringinfo = tr("压缩成功！");
    InitUI();
    InitConnection();
}

void Compressor_Success::InitUI()
{
    DPalette pa;

    m_compressicon = Utils::renderSVG(":/images/success.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_compressicon);
    m_stringinfolabel = new DLabel();
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
    font.setWeight(QFont::DemiBold);
    m_stringinfolabel->setFont(font);
    pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ToolTipText));
    m_stringinfolabel->setPalette(pa);
    m_stringinfolabel->setText(m_stringinfo);
    m_showfilebutton = new DPushButton();
    m_showfilebutton->setFixedSize(340, 36);
    m_showfilebutton->setText(tr("显示文件"));
    m_showfilebutton->setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_showfilebutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    setBackgroundRole(DPalette::Base);
}

void Compressor_Success::InitConnection()
{
    connect(m_showfilebutton, &DPushButton::clicked, this, &Compressor_Success::showfiledirSlot);

    auto changeTheme = [this](){
            DPalette pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
            pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
            m_stringinfolabel->setPalette(pa);
        };

   connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, changeTheme);
}

void Compressor_Success::showfiledirSlot()
{
    DDesktopServices::showFolder(QUrl(m_path, QUrl::TolerantMode));
    emit sigQuitApp();
}

void Compressor_Success::setstringinfo(QString str)
{
    m_stringinfolabel->setText(str);
}

void Compressor_Success::setCompressPath(QString path)
{
    m_path = path;
}

QString Compressor_Success::getPath()
{
    return m_path;
}
