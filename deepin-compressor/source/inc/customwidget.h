/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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
#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H
#include <DSuggestButton>
#include <DComboBox>
#include <DPushButton>
#include <DSwitchButton>
#include <DCheckBox>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE
class CustomSuggestButton: public DSuggestButton
{
    Q_OBJECT
public:
    explicit CustomSuggestButton(QWidget *parent = nullptr);
    explicit CustomSuggestButton(const QString &text, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
};

class CustomCombobox: public DComboBox
{
    Q_OBJECT
public:
    explicit CustomCombobox(QWidget *parent = nullptr);

private:
    Qt::FocusReason m_reson;

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
};

class CustomPushButton: public QPushButton
{
    Q_OBJECT
public:
    explicit CustomPushButton(QWidget *parent = nullptr);
    explicit CustomPushButton(const QString &text, QWidget *parent = nullptr);
    CustomPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
};

class CustomSwitchButton: public DSwitchButton
{
    Q_OBJECT
public:
    explicit CustomSwitchButton(QWidget *parent = Q_NULLPTR);

private:
    Qt::FocusReason m_reson = Qt::NoFocusReason;

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
//    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
};

class CustomCheckBox: public DCheckBox
{
    Q_OBJECT
public:
    explicit CustomCheckBox(QWidget *parent = nullptr);
    explicit CustomCheckBox(const QString &text, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
};

class CustomCommandLinkButton: public DCommandLinkButton
{
    Q_OBJECT

public:
    explicit CustomCommandLinkButton(const QString text, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
};
#endif // CUSTOMWIDGET_H
