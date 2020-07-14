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
#ifndef LOGWIDGETAPPENDER_H
#define LOGWIDGETAPPENDER_H
#include <log4qt/appenderskeleton.h>
#include <QDebug>

namespace Log4Qt {

class LogWidgetAppender : public AppenderSkeleton
{
    Q_OBJECT
public:
    LogWidgetAppender(QObject *parent = nullptr);
    ~LogWidgetAppender();

    void setLogWidget(QWidget *widget);

signals:
    void logAppend(const QString &msg);

protected:
    virtual bool requiresLayout() const;
    virtual void append(const Log4Qt::LoggingEvent &rEvent);

#ifndef QT_NO_DEBUG_STREAM
    virtual QDebug debug(QDebug &rDebug) const;
#endif //QT_NO_DEBUG_STREAM

private:
    QWidget *m_logWidget;
};

}
#endif // LOGWIDGETAPPENDER_H
