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
#include "logwidget.h"

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent)
{
    m_logEdit = new QTextEdit();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_logEdit);
    setLayout(layout);
    resize(600, 400);
}

LogWidget::~LogWidget()
{

}

void LogWidget::onAppendLog(const QString &log)
{
    QMutexLocker lock(&m_mutex);
    m_logEdit->insertPlainText(log);
}
