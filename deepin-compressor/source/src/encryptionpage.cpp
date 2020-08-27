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
#include "utils.h"
#include "customwidget.h"

#include <DFontSizeManager>

#include <QBoxLayout>
#include <QDebug>

EncryptionPage::EncryptionPage(QWidget *parent)
    : DWidget(parent)
{
    InitUI();
    InitConnection();
}

void EncryptionPage::InitUI()
{
    QPixmap m_encrypticon = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_lock_128px.svg", QSize(128, 128));
    DLabel *pixmaplabel = new DLabel(this);
    pixmaplabel->setPixmap(m_encrypticon);
    DLabel *stringinfolabel = new DLabel(this);

    DFontSizeManager::instance()->bind(stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    stringinfolabel->setForegroundRole(DPalette::ToolTipText);
    stringinfolabel->setText(tr("Encrypted file, please enter the password"));
    m_nextbutton = new CustomPushButton(this);
    m_nextbutton->setMinimumSize(340, 36);
    m_nextbutton->setText(tr("Next"));
    m_nextbutton->setDisabled(true);
    m_password = new DPasswordEdit(this);
    m_password->setMinimumSize(340, 36);
    QLineEdit *edit = m_password->lineEdit();
    edit->setPlaceholderText(tr("Password"));

    m_password->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(4);
    mainlayout->addWidget(stringinfolabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(33);

    QHBoxLayout *passwordHBoxLayout = new QHBoxLayout;
    passwordHBoxLayout->addStretch(1);
    passwordHBoxLayout->addWidget(m_password, 2);
    passwordHBoxLayout->addStretch(1);

    mainlayout->addLayout(passwordHBoxLayout);
    mainlayout->addStretch();

    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_nextbutton, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);

    mainlayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);

    m_password->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
}

void EncryptionPage::InitConnection()
{
    connect(m_password, &DPasswordEdit::textChanged, this, &EncryptionPage::onPasswordChanged);
    connect(m_nextbutton, &DPushButton::clicked, this, &EncryptionPage::nextbuttonClicked);
    connect(m_password, &DPasswordEdit::echoModeChanged, this, &EncryptionPage::slotEchoModeChanged);
}

void EncryptionPage::setPassowrdFocus()
{
    m_password->setFocus(Qt::OtherFocusReason);
}

void EncryptionPage::resetPage()
{
    m_inputflag = false;
    m_password->clear();
    m_password->setAlert(false);
}

void EncryptionPage::setInputflag(bool bFlag)
{
    m_inputflag = bFlag;
}

DPasswordEdit *EncryptionPage::getPasswordEdit()
{
    return m_password;
}

void EncryptionPage::nextbuttonClicked()
{
    m_inputflag = true;
//    if (pwdCheckDown) {
    emit sigExtractPassword(m_password->text());
    //pwdCheckDown:to solve the 7z flash retreat when password is too long and wrong
    //if pwdCheckDown is true,t indicates that the work of determining the password has been done
//    pwdCheckDown = false;
//    }
}

void EncryptionPage::wrongPassWordSlot()
{
    if (m_inputflag) {
        qDebug() << "wrongPassWordSlot";
        m_password->clear();
        m_password->setAlert(true);
        m_password->showAlertMessage(tr("Wrong password"));
    }

//    pwdCheckDown = true;
}

void EncryptionPage::onPasswordChanged()
{
    if (m_password->text().isEmpty()) {
        m_nextbutton->setDisabled(true);
    } else {
        m_nextbutton->setEnabled(true);
    }
}

void EncryptionPage::slotEchoModeChanged(bool echoOn)
{
    qDebug() << echoOn;
    m_password->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, echoOn);
}
