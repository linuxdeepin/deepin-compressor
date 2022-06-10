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

#include "compressorapplication.h"
#include "mainwindow.h"

#include <DCheckBox>
#include <DSuggestButton>

#include <QKeyEvent>
#include <QTimer>

CompressorApplication::CompressorApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{

}

CompressorApplication::~CompressorApplication()
{

}

bool CompressorApplication::notify(QObject *watched, QEvent *event)
{
    if (QEvent::KeyPress == event->type()) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
        int keyOfEvent = keyEvent->key();
        if (Qt::Key_Enter == keyOfEvent || Qt::Key_Return == keyOfEvent) {
            //checkbox响应"回车键"

            const QMetaObject *pObj = watched->metaObject();
            if (pObj && pObj->className() == QStringLiteral("QCheckBox")) {
                DCheckBox *checkBox = qobject_cast<DCheckBox *>(watched);
                // 模拟空格键按下事件
                QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
                QApplication::sendEvent(checkBox, &pressSpace);
                // 设置定时
                QTimer::singleShot(80, this, [checkBox]() {
                    // 模拟空格键松开事件
                    QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
                    QApplication::sendEvent(checkBox, &releaseSpace);
                });

                return true;
            }
        } else if (Qt::Key_Tab == keyOfEvent || Qt::Key_Backtab == keyOfEvent
                   || Qt::Key_Left == keyOfEvent || Qt::Key_Up == keyOfEvent) {
            QWidget *pWgt = activeWindow();
            MainWindow *pWindow = qobject_cast<MainWindow *>(pWgt); //在对应的MainWindow操作
            if (pWindow) {
                bool rs = pWindow->handleApplicationTabEventNotify(watched, keyEvent);
                if (rs) {
                    return rs;
                }
            }
        }
    }

    return DApplication::notify(watched, event);

}

void CompressorApplication::handleQuitAction()
{
    QWidget *pWgt = activeWindow();
    MainWindow *pWindow = qobject_cast<MainWindow *>(pWgt); //在对应的MainWindow操作

    // 处理点击退出事件
    if (pWindow)
        pWindow->handleQuit();
}
