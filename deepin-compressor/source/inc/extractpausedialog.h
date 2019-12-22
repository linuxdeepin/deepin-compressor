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
#ifndef EXTRACTPAUSEDIALOG_H
#define EXTRACTPAUSEDIALOG_H

#include <DDialog>

class ExtractPauseDialog: public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    explicit ExtractPauseDialog(QWidget *parent = nullptr);
    void initUI();
    void initConnect();

public slots:
    void clickedSlot(int index, const QString &text);
    void closeEvent(QCloseEvent *event) override;

signals:
    void sigbuttonpress(int index);
};

#endif // EXTRACTPAUSEDIALOG_H
