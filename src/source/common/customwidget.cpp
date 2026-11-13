// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customwidget.h"
#include "common/ddlog.h"

#include <DStyle>
#include <DApplication>
#include <DStyleOptionButton>

#include <QKeyEvent>
#include <QApplication>
#include <QTimer>
#include <QDebug>

CustomSuggestButton::CustomSuggestButton(QWidget *parent)
    : DSuggestButton(parent)
{
    qCDebug(appLog) << "CustomSuggestButton initialized";
    setFocusPolicy(Qt::TabFocus);
}

CustomSuggestButton::CustomSuggestButton(const QString &text, QWidget *parent)
    : DSuggestButton(text, parent)
{
    qCDebug(appLog) << "CustomSuggestButton initialized with text:" << text;
    setFocusPolicy(Qt::TabFocus);
}

void CustomSuggestButton::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "Key press event, key:" << event->key();
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行按下按钮
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        DSuggestButton::keyPressEvent(event);
    }
}

void CustomSuggestButton::keyReleaseEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "Key release event, key:" << event->key();
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行弹起按钮
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        DSuggestButton::keyReleaseEvent(event);
    }
}

CustomCombobox::CustomCombobox(QWidget *parent): DComboBox(parent)
{
    qCDebug(appLog) << "CustomCombobox initialized";
    setFocusPolicy(Qt::TabFocus);
}

void CustomCombobox::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "ComboBox key press event, key:" << event->key();
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        DComboBox::keyPressEvent(event);
    }
}

void CustomCombobox::keyReleaseEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "ComboBox key release event, key:" << event->key();
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        DComboBox::keyReleaseEvent(event);
    }
}

/**
 * @brief combobox获取到焦点效果
 * @param event
 */
void CustomCombobox::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "ComboBox paint event";
    DComboBox::paintEvent(event);
    if (hasFocus() && (m_reson & (Qt::TabFocusReason | Qt::BacktabFocusReason))) {
        DStylePainter painter(this);
//    painter.setPen(palette().color(QPalette::Text));
        DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
//    style->drawControl()
//    // draw the combobox frame, focusrect and selected etc.
        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        const  QStyleOptionComboBox *opt1 = &opt;
//    painter.draw(QStyle::SE_ComboBoxFocusRect, opt);

//    // draw the icon and text
//    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
        DStyleHelper dstyle(style);
        int border_width = dstyle.pixelMetric(DStyle::PM_FocusBorderWidth, opt1, this);
//        int border_space = style->pixelMetric(DStyle::PM_FocusFrameVMargin, opt1, this);
//        int frame_radius = dstyle.pixelMetric(DStyle::PM_FrameRadius, opt1, this);
        QColor color = dstyle.getColor(opt1, QPalette::Highlight);
        // QRect borderRect = style->subElementRect(DStyle::SE_ComboBoxFocusRect, opt1, this);

        painter.setPen(QPen(color, border_width, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing);

        //.adjusted(1, 1, -1, -1)

        //painter.drawRoundedRect(borderRect, frame_radius, frame_radius);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, opt1, & painter, this);
    }
}

void CustomCombobox::focusInEvent(QFocusEvent *event)
{
    qCDebug(appLog) << "ComboBox focus in, reason:" << event->reason();
    m_reson = event->reason();
    DComboBox::focusInEvent(event);
}

CustomPushButton::CustomPushButton(QWidget *parent)
    : QPushButton(parent)
{
    qCDebug(appLog) << "CustomPushButton initialized";
    setFocusPolicy(Qt::TabFocus);
}

CustomPushButton::CustomPushButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    qCDebug(appLog) << "CustomPushButton initialized with text:" << text;
    setFocusPolicy(Qt::TabFocus);
}

CustomPushButton::CustomPushButton(const QIcon &icon, const QString &text, QWidget *parent): QPushButton(icon, text, parent)
{
    qCDebug(appLog) << "CustomPushButton initialized with icon and text:" << text;
    setFocusPolicy(Qt::TabFocus);
}

void CustomPushButton::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "PushButton key press event, key:" << event->key();
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行按下按钮
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        QPushButton::keyPressEvent(event);
    }
}

void CustomPushButton::keyReleaseEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "PushButton key release event, key:" << event->key();
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行弹起按钮
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        QPushButton::keyReleaseEvent(event);
    }
}

CustomSwitchButton::CustomSwitchButton(QWidget *parent)
    : DSwitchButton(parent)
{
    qCDebug(appLog) << "CustomSwitchButton initialized";
    setFocusPolicy(Qt::TabFocus);
}

void CustomSwitchButton::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "CustomSwitchButton::keyPressEvent" << event;
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行打开下拉列表功能
        qCDebug(appLog) << "CustomSwitchButton::keyPressEvent press enter";
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
        QTimer::singleShot(80, this, [&]() {
            // 模拟空格键松开事件
            QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
            QApplication::sendEvent(this, &releaseSpace);
        });
    } else {
        DSwitchButton::keyPressEvent(event);
    }
}

void CustomSwitchButton::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "CustomSwitchButton::paintEvent" << event;
    DSwitchButton::paintEvent(event);
    if (hasFocus() && (m_reson & (Qt::TabFocusReason | Qt::BacktabFocusReason))) {
        // qCDebug(appLog) << "CustomSwitchButton::paintEvent has focus";
        DStylePainter painter(this);
        //    painter.setPen(palette().color(QPalette::Text));
        DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
        //    style->drawControl()
        //    // draw the combobox frame, focusrect and selected etc.
        DStyleOptionButton opt;
        this->initStyleOption(&opt);
        const  QStyleOptionButton *opt1 = &opt;
        //    painter.draw(QStyle::SE_ComboBoxFocusRect, opt);

        //    // draw the icon and text
        //    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
        DStyleHelper dstyle(style);
        int border_width = dstyle.pixelMetric(DStyle::PM_FocusBorderWidth, opt1, this);
        //        int border_space = style->pixelMetric(DStyle::PM_FocusFrameVMargin, opt1, this);
        //        int frame_radius = dstyle.pixelMetric(DStyle::PM_FrameRadius, opt1, this);
        QColor color = dstyle.getColor(opt1, QPalette::Highlight);
        // QRect borderRect = style->subElementRect(DStyle::SE_ComboBoxFocusRect, opt1, this);

        painter.setPen(QPen(color, border_width, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing);

        //.adjusted(1, 1, -1, -1)

        //painter.drawRoundedRect(borderRect, frame_radius, frame_radius);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, opt1, & painter, this);
    }
}

void CustomSwitchButton::focusInEvent(QFocusEvent *event)
{
    // qCDebug(appLog) << "CustomSwitchButton::focusInEvent" << event;
    //    m_reson = event->reason(); //不使用自绘的焦点效果，注释掉
    DSwitchButton::focusInEvent(event);
}

CustomCheckBox::CustomCheckBox(QWidget *parent)
    : DCheckBox(parent)
{
    qCDebug(appLog) << "CustomCheckBox::CustomCheckBox";
}

CustomCheckBox::CustomCheckBox(const QString &text, QWidget *parent)
    : DCheckBox(text, parent)
{
    qCDebug(appLog) << "CustomCheckBox::CustomCheckBox" << text;
}

void CustomCheckBox::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "CustomCheckBox::keyPressEvent" << event;
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行切换状态
        qCDebug(appLog) << "CustomCheckBox::keyPressEvent press enter";
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        DCheckBox::keyPressEvent(event);
    }
}

void CustomCheckBox::keyReleaseEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "CustomCheckBox::keyReleaseEvent" << event;
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行切换状态
        qCDebug(appLog) << "CustomCheckBox::keyReleaseEvent press enter";
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        DCheckBox::keyReleaseEvent(event);
    }
}

CustomCommandLinkButton::CustomCommandLinkButton(const QString text, QWidget *parent)
    : DCommandLinkButton(text, parent)
{
    qCDebug(appLog) << "CustomCommandLinkButton::CustomCommandLinkButton" << text;
}

void CustomCommandLinkButton::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "CustomCommandLinkButton::keyPressEvent" << event;
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行切换状态
        qCDebug(appLog) << "CustomCommandLinkButton::keyPressEvent press enter";
        // 模拟空格键按下事件
        QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &pressSpace);
    } else {
        DCommandLinkButton::keyPressEvent(event);
    }
}

void CustomCommandLinkButton::keyReleaseEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "CustomCommandLinkButton::keyReleaseEvent" << event;
    if (Qt::Key_Enter == event->key() || Qt::Key_Return == event->key()) { //“回车键” 执行切换状态
        qCDebug(appLog) << "CustomCommandLinkButton::keyReleaseEvent press enter";
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(this, &releaseSpace);
    } else {
        DCommandLinkButton::keyReleaseEvent(event);
    }
}

CustomFloatingMessage::CustomFloatingMessage(const QIcon &ico, const QString &str, int msec,
                                             QWidget *parent, MessageType notifyType)
    : DFloatingMessage(notifyType, parent)
{
    qCDebug(appLog) << "CustomFloatingMessage::CustomFloatingMessage" << str << msec;
    setIcon(ico);
    setMessage(str);
    setDuration(msec);
}
