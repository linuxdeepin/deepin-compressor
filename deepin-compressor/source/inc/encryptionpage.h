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
#ifndef ENCRYPTIONPAGE_H
#define ENCRYPTIONPAGE_H
#include <QWidget>
#include <DPushButton>
#include <DLabel>
#include <dpasswordedit.h>
#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class EncryptionPage: public QWidget
{
    Q_OBJECT
public:
    EncryptionPage(QWidget *parent = 0);
    void InitUI();
    void InitConnection();


private:
    QPixmap m_encrypticon;
    DLabel *m_pixmaplabel;
    DLabel *m_stringinfolabel;
    DPushButton *m_nextbutton;
    DPasswordEdit *m_password;

    bool m_inputflag;
public slots:
    void nextbuttonClicked();
    void wrongPassWordSlot();

signals:
    void sigExtractPassword(QString password);
};

#endif // ENCRYPTIONPAGE_H
