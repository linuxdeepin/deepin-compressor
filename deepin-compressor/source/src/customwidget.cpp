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
#include "customwidget.h"
#include <QKeyEvent>
#include <QApplication>
#include <QTimer>
#include <QDebug>


CustomSuggestButton::CustomSuggestButton(QWidget *parent)
    : DSuggestButton(parent)
{
    setFocusPolicy(Qt::TabFocus);
}

CustomSuggestButton::CustomSuggestButton(const QString &text, QWidget *parent)
    : DSuggestButton(text, parent)
{
    setFocusPolicy(Qt::TabFocus);
}

void CustomSuggestButton::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        DSuggestButton::keyPressEvent(event);
    }
}

void CustomSuggestButton::keyReleaseEvent(QKeyEvent *event)
{
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        DSuggestButton::keyReleaseEvent(event);
    }
}

CustomCombobox::CustomCombobox(QWidget *parent): QComboBox(parent)
{
    setFocusPolicy(Qt::TabFocus);
}

void CustomCombobox::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        QComboBox::keyPressEvent(event);
    }
}

void CustomCombobox::keyReleaseEvent(QKeyEvent *event)
{
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        QComboBox::keyReleaseEvent(event);
    }
}

CustomPushButton::CustomPushButton(QWidget *parent)
    : QPushButton(parent)
{
    setFocusPolicy(Qt::TabFocus);
}

CustomPushButton::CustomPushButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setFocusPolicy(Qt::TabFocus);
}

CustomPushButton::CustomPushButton(const QIcon &icon, const QString &text, QWidget *parent): QPushButton(icon, text, parent)
{
    setFocusPolicy(Qt::TabFocus);
}

void CustomPushButton::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        QPushButton::keyPressEvent(event);
    }
}

void CustomPushButton::keyReleaseEvent(QKeyEvent *event)
{
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        QPushButton::keyReleaseEvent(event);
    }
}
