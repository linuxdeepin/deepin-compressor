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
#include "encodingpage.h"

#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QFileIconProvider>
#include <QDir>
#include <QTemporaryFile>
#include <QTextCodec>

EncodingPage::EncodingPage(QWidget *parent)
    : DWidget(parent)
{
    InitUI();
    InitConnection();
}

void EncodingPage::InitUI()
{
//    m_fileicon = Utils::renderSVG(":/images/Compression Packet.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel(this);
    m_pixmaplabel->setPixmap(m_fileicon);
    m_filenamelabel = new DLabel(this);
    m_filenamelabel->setText("");

    DFontSizeManager::instance()->bind(m_filenamelabel, DFontSizeManager::T5, QFont::DemiBold);

    QStringList encodeList;

    for (int mib : QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);
        QString encodeName = QString(codec->name()).toUpper();

        if (encodeName != "UTF-8" && !encodeList.contains(encodeName)) {
            encodeList.append(encodeName);
        }
    }

    encodeList.sort();
    encodeList.prepend("UTF-8");

    m_codelabel = new DLabel(this);
    m_codelabel->setText(tr("Filename Encoding") + ":");
    m_codebox = new DComboBox(this);
    m_codebox->setFixedWidth(260);
    m_codebox->setEditable(true);
    m_codebox->addItems(encodeList);

    QHBoxLayout *encodelayout = new QHBoxLayout;
    encodelayout->addStretch();
    encodelayout->addWidget(m_codelabel);
    encodelayout->addWidget(m_codebox);
    encodelayout->addStretch();

    m_detaillabel = new DLabel(this);
    m_detaillabel->setText(tr("Select an encoding to continue"));
//    m_detaillabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    DFontSizeManager::instance()->bind(m_detaillabel, DFontSizeManager::T8);

    m_cancelbutton = new DPushButton(tr("Cancel"), this);
    m_confirmbutton = new DPushButton(tr("Select Character Encoding"), this);
    m_cancelbutton->setFixedSize(165, 36);
    m_confirmbutton->setFixedSize(165, 36);
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch();
    buttonlayout->addWidget(m_cancelbutton);
    buttonlayout->addSpacing(10);
    buttonlayout->addWidget(m_confirmbutton);
    buttonlayout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
//    mainlayout->addSpacing(62);
    mainlayout->addWidget(m_pixmaplabel, 10, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(m_filenamelabel, 10, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(33);
    mainlayout->addLayout(encodelayout);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(m_detaillabel, 10, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(41);
    mainlayout->addLayout(buttonlayout);
    mainlayout->addSpacing(20);

    setBackgroundRole(DPalette::Base);
}

void EncodingPage::InitConnection()
{

}

void EncodingPage::setFilename(QString filename)
{
    QFileInfo fileinfo(filename);
    setTypeImage(fileinfo.suffix());
    m_filenamelabel->setText(filename);
    m_codebox->setEditText("Unicode(UTF-8) World city.txt");
}

void EncodingPage::setTypeImage(QString type)
{
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo("temp." + type));

    m_pixmaplabel->setPixmap(icon.pixmap(128, 128));
}
