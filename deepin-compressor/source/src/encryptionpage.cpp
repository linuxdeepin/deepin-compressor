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
#include "encryptionpage.h"
#include <QBoxLayout>
#include <utils.h>
#include <DDesktopServices>
#include <QDebug>

EncryptionPage::EncryptionPage(QWidget *parent)
    : QWidget(parent)
{
    m_inputflag = false;
    InitUI();
    InitConnection();
}

void EncryptionPage::InitUI()
{
    m_encrypticon = Utils::renderSVG(":/images/lock.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
    m_pixmaplabel->setPixmap(m_encrypticon);
    m_stringinfolabel = new DLabel();
//    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
//    font.setWeight(QFont::DemiBold);
//    m_stringinfolabel->setFont(font);
    DFontSizeManager::instance()->bind(m_stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(m_stringinfolabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ToolTipText));
    m_stringinfolabel->setPalette(pa);
    m_stringinfolabel->setText(tr("This file is encrypted, please enter the password"));
    m_nextbutton = new DPushButton();
    m_nextbutton->setFixedSize(340, 36);
    m_nextbutton->setText(tr("Next"));
    m_password = new DPasswordEdit();
    m_password->setFixedSize(340, 36);
    QLineEdit *edit = m_password->lineEdit();
    edit->setPlaceholderText(tr("Please enter password to extract"));

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(4);
    mainlayout->addWidget(m_stringinfolabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(33);
    mainlayout->addWidget(m_password, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_nextbutton, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    setBackgroundRole(DPalette::Base);

}

void EncryptionPage::InitConnection()
{
    connect(m_nextbutton, &DPushButton::clicked, this, &EncryptionPage::nextbuttonClicked);
}

void EncryptionPage::nextbuttonClicked()
{
    m_inputflag = true;
    emit sigExtractPassword(m_password->text());
}

void EncryptionPage::wrongPassWordSlot()
{
    if (m_inputflag) {
        qDebug() << "wrongPassWordSlot";
        m_password->setAlert(true);
        m_password->showAlertMessage(tr("Wrong Password"));
    }

}
