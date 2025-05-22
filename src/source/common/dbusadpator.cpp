// Copyright (C) 2020 ~ 2023, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QFileInfo>
#include <QtDebug>
#include <DSettings>
#include "dbusadpator.h"
#include <QMainWindow>
#include <QDBusInterface>
#include <QDBusReply>
const QString SOPENLIST = "openfiles";

ApplicationAdaptor::ApplicationAdaptor(QApplication *application)
    : QDBusAbstractAdaptor(application), app(application)
{
    qDebug() << "ApplicationAdaptor initialized";
    connect(application, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
    connect(application, SIGNAL(focusChanged(QWidget*, QWidget*)),
            SLOT(focusChangedSlot(QWidget*, QWidget*)));
}


/**
 * @brief setCurOpenFile    设置当前的压缩包文件名
 * @param sFile    压缩包文件名
 */
void ApplicationAdaptor::setCompressFile(const QString &sFile)
{
    qInfo() << "Set compress file:" << sFile;
    m_sFile = sFile;
}


void ApplicationAdaptor::raise(const QString &sFile)
{
    qDebug() << "Raise window request for file:" << sFile;
    if(m_sFile.isEmpty() || m_sFile.isNull()) {
        qWarning() << "No compress file set, cannot raise window";
        return;
    }
    if(m_curShowWidget && (m_sFile == sFile))
        m_curShowWidget->activateWindow();
}

void ApplicationAdaptor::onActiveWindow(qint64 pid)
{
    qDebug() << "Active window request from pid:" << pid << ", current pid:" << QGuiApplication::applicationPid();
    m_curShowWidget = nullptr;
    if(pid != QGuiApplication::applicationPid()) {
        qDebug() << "Pid mismatch, ignoring request";
        return;
    }
    if(!app) {
        qWarning() << "Application instance is null";
        return;
    }
    for(QWidget *w : app->topLevelWidgets()) {
        if(QMainWindow *mainWnd = dynamic_cast<QMainWindow *>(w)) {
            m_curShowWidget = mainWnd;
        }
    }
    if(m_curShowWidget) {
        qInfo() << "activateWindow by raise";
        m_curShowWidget->show();
        m_curShowWidget->raise();
        m_curShowWidget->activateWindow();
    }
    if(m_curShowWidget && !m_curShowWidget->isActiveWindow()) {
        qDebug() << "Window not active, trying to activate via Dock DBus";
        qInfo() << "activateWindow by Dock dbus";
        QDBusInterface dockDbusInterfaceV20(
                "com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock", "com.deepin.dde.daemon.Dock");
            if (dockDbusInterfaceV20.isValid()) {
                QDBusReply<void> reply = dockDbusInterfaceV20.call("ActivateWindow", m_curShowWidget->winId());
                if (!reply.isValid()) {
                    qWarning() << qPrintable("Call v20 com.deepin.dde.daemon.Dock failed") << reply.error();
                } else {
                    qInfo() << "Dock dbus activateWindow success!";
                }
            }
    }

}
