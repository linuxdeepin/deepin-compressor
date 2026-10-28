// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compressorapplication.h"
#include "mainwindow.h"
#include "common/dbusadpator.h"
#include "common/ddlog.h"

#include <QDBusConnection>
#include <QDBusError>

#include <DCheckBox>
#include <DSuggestButton>

#include <QKeyEvent>
#include <QTimer>

CompressorApplication::CompressorApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{
    qCDebug(appLog) << "CompressorApplication constructor";
    
    // Register DBus service
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.Compressor")) {
        qCWarning(appLog) << "Failed to register DBus service:" << connection.lastError().message();
    } else {
        qCDebug(appLog) << "DBus service registered successfully";
    }
}

CompressorApplication::~CompressorApplication()
{
    qCDebug(appLog) << "CompressorApplication destructor";
}

bool CompressorApplication::notify(QObject *watched, QEvent *event)
{
    // qCDebug(appLog) << "Processing application event notification";
    
    if (QEvent::KeyPress == event->type()) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
        int keyOfEvent = keyEvent->key();
        
        if (Qt::Key_Enter == keyOfEvent || Qt::Key_Return == keyOfEvent) {
            qCDebug(appLog) << "Handling Enter/Return key press";
            
            const QMetaObject *pObj = watched->metaObject();
            if (pObj && pObj->className() == QStringLiteral("QCheckBox")) {
                qCDebug(appLog) << "Converting Enter key to Space for checkbox";
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
            // qCDebug(appLog) << "Handling navigation key press";
            QWidget *pWgt = activeWindow();
            MainWindow *pWindow = qobject_cast<MainWindow *>(pWgt); //在对应的MainWindow操作
            if (pWindow) {
                qCDebug(appLog) << "Forwarding navigation event to MainWindow";
                bool rs = pWindow->handleApplicationTabEventNotify(watched, keyEvent);
                if (rs) {
                    qCDebug(appLog) << "MainWindow handled navigation event";
                    return rs;
                }
            }
        }
    }

    // qCDebug(appLog) << "Default event notification handling";
    return DApplication::notify(watched, event);
}

void CompressorApplication::handleQuitAction()
{
    qCDebug(appLog) << "Handling application quit action";
    QWidget *pWgt = activeWindow();
    MainWindow *pWindow = qobject_cast<MainWindow *>(pWgt); //在对应的MainWindow操作

    // 处理点击退出事件
    if (pWindow) {
        qCInfo(appLog) << "Forwarding quit action to MainWindow";
        pWindow->handleQuit();
    } else {
        qCWarning(appLog) << "No active MainWindow found for quit action";
    }
}
