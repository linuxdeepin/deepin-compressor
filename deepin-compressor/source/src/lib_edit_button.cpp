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

#include "lib_edit_button.h"

Lib_Edit_Button::Lib_Edit_Button(DLineEdit *edit)
    : DPushButton(edit)
{
    parent = edit;
    InitUI(edit);
    InitConnection();
}

void Lib_Edit_Button::InitUI(DLineEdit *edit)
{
    QSize size = QSize(46, 36);
    setMinimumSize(size);
    setMaximumSize(size);
    setFocusPolicy(Qt::ClickFocus);
    setText("Path");

    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(this);

    DWidget *empty = new DWidget(this);
    empty->setFixedSize(150, 36);
    QHBoxLayout *mainlayout = new QHBoxLayout;
    mainlayout->addWidget(empty, 0, Qt::AlignLeft);
    mainlayout->addStretch();
    mainlayout->addWidget(this, 0, Qt::AlignRight);
    mainlayout->setStretch(0, 5);
    mainlayout->setStretch(1, 1);

    edit->setLayout(mainlayout);

    QLineEdit *qedit = edit->lineEdit();
    // 设置输入框中文件输入区，不让输入的文字在被隐藏在按钮下
    qedit->setTextMargins(0, 1, size.width(), 1);
}

void Lib_Edit_Button::removebutton()
{

}

void Lib_Edit_Button::addbutton()
{

}

void Lib_Edit_Button::InitConnection()
{
//    connect(m_pathbutton, &DPushButton::clicked, this, &CompressSetting::onPathButoonClicked);
}

void Lib_Edit_Button::onPathButoonClicked()
{

}
