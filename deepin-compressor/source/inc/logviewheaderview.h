/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LZ <zhou.lu@archermind.com>
 *
 * Maintainer: LZ <zhou.lu@archermind.com>
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

#pragma once

#include <DLabel>
#include <DHeaderView>

class LogViewHeaderView;

#define SCROLLMARGIN 10

class MyLabel: public Dtk::Widget::DLabel
{
    Q_OBJECT
public:
    MyLabel(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *e) override;
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
signals:
    void labelDoubleClickEvent(QMouseEvent *event);
};

class PreviousLabel: public Dtk::Widget::DLabel
{
    Q_OBJECT
public:
    PreviousLabel(const QString &text, LogViewHeaderView *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

signals:
    void doubleClickedSignal();

private:
    LogViewHeaderView *headerView_;

    bool focusIn_ = false;
};

class LogViewHeaderView : public Dtk::Widget::DHeaderView
{
    Q_OBJECT
public:
    LogViewHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    QSize sizeHint() const override;

    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

protected:
    void paintEvent(QPaintEvent *e) override;
    virtual void paintSection(QPainter *painter, const QRect &rect,
                              int logicalIndex) const override;

private:
    void resizeEvent(QResizeEvent *event) override;

private:
    int m_spacing {1};

public:
    PreviousLabel *gotoPreviousLabel_;
};

