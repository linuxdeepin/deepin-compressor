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
#include "compressor_success.h"

#include <DFileDialog>
#include <QBoxLayout>
#include <utils.h>
#include <DDesktopServices>
#include <QDebug>
#include "DFontSizeManager"
#include "DApplicationHelper"

Compressor_Success::Compressor_Success(QWidget *parent)
    : DWidget(parent)
{
    m_stringinfo = tr("Compression successful");
    InitUI();
    InitConnection();
}

void Compressor_Success::InitUI()
{
    m_compressicon = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_success_128px.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel(this);
    m_pixmaplabel->setPixmap(m_compressicon);

    m_stringinfolabel = new DLabel(this);

    DFontSizeManager::instance()->bind(m_stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    m_stringinfolabel->setForegroundRole(DPalette::ToolTipText);
    m_stringinfolabel->setText(m_stringinfo);
    m_showfilebutton = new DPushButton(this);
    m_showfilebutton->setMinimumSize(340, 36);
    m_showfilebutton->setText(tr("View"));
    m_showfilebutton->setFocusPolicy(Qt::ClickFocus);

    commandLinkBackButton = new DCommandLinkButton(tr("Back"), this);
    QHBoxLayout *commandLinkButtonLayout = new QHBoxLayout;
    commandLinkButtonLayout->addStretch();
    commandLinkButtonLayout->addWidget(commandLinkBackButton);
    commandLinkButtonLayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_stringinfolabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();

    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_showfilebutton, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);
    mainlayout->addLayout(commandLinkButtonLayout);

    mainlayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
}

void Compressor_Success::InitConnection()
{
    connect(m_showfilebutton, &DPushButton::clicked, this, &Compressor_Success::showfiledirSlot);
    connect(commandLinkBackButton, &DCommandLinkButton::clicked, this, &Compressor_Success::commandLinkBackButtonClicked);

//    auto changeTheme = [this]() {
//        DPalette pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
//        pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
//        m_stringinfolabel->setPalette(pa);
//    };

//    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, changeTheme);
}

void Compressor_Success::showfiledirSlot()
{
    qDebug()<<m_path;
    if( newCreatePath_.isEmpty() == false)
    {
        DDesktopServices::showFolder(QUrl(newCreatePath_, QUrl::TolerantMode));
    }
    else if(m_fullpath.isEmpty())
    {
        DDesktopServices::showFolder(QUrl(m_path, QUrl::TolerantMode));
    }
    else {
        DDesktopServices::showFileItem(QUrl(m_fullpath, QUrl::TolerantMode));
    }

    //emit sigQuitApp();
}

void Compressor_Success::commandLinkBackButtonClicked()
{
    emit sigBackButtonClicked();
}

void Compressor_Success::setstringinfo(QString str)
{
    m_stringinfolabel->setText(str);
}

void Compressor_Success::setCompressPath(QString path)
{
    qDebug()<<path;
    m_path = path;
}

QString Compressor_Success::getPath()
{
    return m_path;
}

void Compressor_Success::setCompressFullPath(const QString& path)
{
    qDebug()<<path;
    m_fullpath = path;
}

void Compressor_Success::setCompressNewFullPath(const QString& path)
{
    newCreatePath_ = path;
}

void Compressor_Success::clear()
{
    setCompressFullPath("");
    setCompressNewFullPath("");
}


