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
#ifndef ENCODINGPAGE_H
#define ENCODINGPAGE_H
#include <QWidget>
#include <DPushButton>
#include <DLabel>
#include "utils.h"
#include <DComboBox>
#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class EncodingPage: public QWidget
{
    Q_OBJECT
public:
    EncodingPage(QWidget *parent = 0);
    void InitUI();
    void InitConnection();

    void setTypeImage(QString type);
    void setFilename(QString filename);
private:
    QPixmap m_fileicon;
    DLabel *m_pixmaplabel;
    DLabel *m_filenamelabel;
    DComboBox *m_codebox;
    DLabel *m_codelabel;
    DLabel *m_detaillabel;
    DPushButton *m_cancelbutton;
    DPushButton *m_confirmbutton;
};

#endif // ENCODINGPAGE_H
