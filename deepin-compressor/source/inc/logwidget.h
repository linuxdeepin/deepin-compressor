/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     hanshuai <hanshuai@uniontech.com>
* Maintainer: hanshuai <hanshuai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QMutex>
#include <QVBoxLayout>

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    LogWidget(QWidget *parent = nullptr);
    ~LogWidget();

private slots:
    void onAppendLog(const QString &log);

private:
    QTextEdit *m_logEdit;
    QMutex m_mutex;
};

#endif // LOGWIDGET_H
