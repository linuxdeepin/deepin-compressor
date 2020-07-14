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
#include <log4qt/ttcclayout.h>
#include "logwidgetappender.h"
#include "logwidget.h"

namespace Log4Qt {

LogWidgetAppender::LogWidgetAppender(QObject *parent)
    : AppenderSkeleton(parent)
{
    m_logWidget = nullptr;
}

LogWidgetAppender::~LogWidgetAppender()
{

}

void LogWidgetAppender::setLogWidget(QWidget *widget)
{
    m_logWidget = qobject_cast<LogWidget *>(widget);
    connect(this, SIGNAL(logAppend(const QString &)), m_logWidget, SLOT(onAppendLog(const QString &)));
}

bool LogWidgetAppender::requiresLayout() const
{
    return true;
}

void LogWidgetAppender::append(const LoggingEvent &rEvent)
{
    QString message = dynamic_cast<TTCCLayout *>(layout())->format(rEvent);
    emit logAppend(message);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug LogWidgetAppender::debug(QDebug &rDebug) const
{
    return rDebug.space();
}
#endif //QT_NO_DEBUG_STREAM

}

