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
#ifndef LIB_EDIT_BUTTON_H
#define LIB_EDIT_BUTTON_H

#include <DLineEdit>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class QHBoxLayout;

class Lib_Edit_Button : public DPushButton
{
public:
    Lib_Edit_Button(DLineEdit *edit = nullptr);
    void InitUI(DLineEdit *edit);
    void InitConnection();

public:
    void removebutton();
    void addbutton();

public slots:
    void onPathButoonClicked();

private:
    DLineEdit *parent;
    QHBoxLayout *m_buttonLayout;
};

#endif // LIB_EDIT_BUTTON_H
