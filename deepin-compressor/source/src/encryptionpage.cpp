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
#include "encryptionpage.h"
#include <QBoxLayout>
#include <utils.h>
#include <QDebug>
#include "DFontSizeManager"

EncryptionPage::EncryptionPage(QWidget *parent)
    : DWidget(parent)
{
    InitUI();
    InitConnection();
}

void EncryptionPage::InitUI()
{
    QPixmap m_encrypticon = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_lock_128px.svg", QSize(128, 128));
    DLabel* pixmaplabel = new DLabel(this);
    pixmaplabel->setPixmap(m_encrypticon);
    DLabel* stringinfolabel = new DLabel(this);

    DFontSizeManager::instance()->bind(stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    stringinfolabel->setForegroundRole(DPalette::ToolTipText);
    stringinfolabel->setText(tr("Encrypted file, please enter the password"));
    m_nextbutton = new DPushButton(this);
    m_nextbutton->setFixedSize(340, 36);
    m_nextbutton->setText(tr("Next"));
    m_nextbutton->setDisabled(true);
    m_password = new DPasswordEdit(this);
    m_password->setFixedSize(340, 36);
    QLineEdit *edit = m_password->lineEdit();
    edit->setPlaceholderText(tr("Please enter password to extract"));

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(4);
    mainlayout->addWidget(stringinfolabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(33);
    mainlayout->addWidget(m_password, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_nextbutton, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    setBackgroundRole(DPalette::Base);

}

void EncryptionPage::InitConnection()
{
    connect(m_password, &DPasswordEdit::textChanged, this, &EncryptionPage::onPasswordChanged);
    connect(m_nextbutton, &DPushButton::clicked, this, &EncryptionPage::nextbuttonClicked);
}

void EncryptionPage::setPassowrdFocus()
{
    m_password->setFocus(Qt::OtherFocusReason);
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

void EncryptionPage::onPasswordChanged()
{
    if(m_password->text().isEmpty())
    {
        m_nextbutton->setDisabled(true);
    }
    else
    {
        m_nextbutton->setEnabled(true);
    }
}
