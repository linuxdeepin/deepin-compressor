// Copyright (C) 2020 ~ 2023, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusadpator.h"
#include "mainwindow.h"
#include "archivemanager.h"

#include <QFileInfo>
#include <QtDebug>
#include <DSettings>
#include <QMainWindow>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>

const QString SOPENLIST = "openfiles";

ApplicationAdaptor::ApplicationAdaptor(MainWindow *mainwindow)
    : QDBusAbstractAdaptor(mainwindow), m_mainWindow(mainwindow)
{
    qDebug() << "ApplicationAdaptor initialized";
}

void ApplicationAdaptor::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
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

bool ApplicationAdaptor::raise(const QString &sFile)
{
    qDebug() << "Raise window request for file:" << sFile;
    if(m_sFile.isEmpty() || m_sFile.isNull()) {
        qWarning() << "No compress file set, cannot raise window";
        return false;
    }
    if(m_curShowWidget && (m_sFile == sFile)) {
        m_curShowWidget->activateWindow();
        return true;
    }
    return false;
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
        QDBusInterface dockDbusInterfaceV23(
                "org.deepin.dde.daemon.Dock1", "/org/deepin/dde/daemon/Dock1", "org.deepin.dde.daemon.Dock1");
        QDBusInterface *dockDbusInterface = dockDbusInterfaceV23.isValid() ? &dockDbusInterfaceV23 : &dockDbusInterfaceV20;
        if (dockDbusInterface->isValid()) {
            QDBusReply<void> reply = dockDbusInterface->call("ActivateWindow", m_curShowWidget->winId());
            if (!reply.isValid()) {
                qWarning() << "Dock dbus activateWindow failed via" << dockDbusInterface->service() << ", error:" << reply.error();
            } else {
                qInfo() << "Dock dbus activateWindow success!";
            }
        }
    }
}

// Window management methods
bool ApplicationAdaptor::showWindow()
{
    if (m_mainWindow) {
        m_mainWindow->show();
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
        return true;
    }
    return false;
}

bool ApplicationAdaptor::hideWindow()
{
    if (m_mainWindow) {
        m_mainWindow->hide();
        return true;
    }
    return false;
}

bool ApplicationAdaptor::raiseWindow()
{
    if (m_mainWindow) {
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
        return true;
    }
    return false;
}

bool ApplicationAdaptor::quitWindow()
{
    if (m_mainWindow) {
        m_mainWindow->close();
        return true;
    }
    return false;
}

bool ApplicationAdaptor::compressFiles(const QStringList &filePaths)
{
    if (filePaths.isEmpty()) {
        qWarning() << "No files provided for compression";
        return false;
    }
    
    if (!m_mainWindow) {
        qWarning() << "MainWindow not available";
        return false;
    }

    for (const QString &filePath : filePaths) {
        if (!QFileInfo::exists(filePath)) {
            qWarning() << "File does not exist:" << filePath;
            return false;
        }
    }

    showWindow();
    m_mainWindow->slotDragSelectedFiles(QStringList{filePaths});
    return true;
}

bool ApplicationAdaptor::extractFiles(const QString &archivePath, const QString &destinationPath)
{
    if (!QFileInfo::exists(archivePath)) {
        qWarning() << "Archive file does not exist:" << archivePath;
        return false;
    }
    
    if (!m_mainWindow) {
        qWarning() << "MainWindow not available";
        return false;
    }

    QFileInfo destInfo(destinationPath);
    if (!destInfo.isDir() || !destInfo.isWritable()) {
        qWarning() << "Destination path is not a writable directory:" << destinationPath;
        return false;
    }

    showWindow();
    m_mainWindow->slotDragSelectedFiles(QStringList{archivePath});
    m_mainWindow->slotUncompressClicked(destinationPath);
    return true;
}

bool ApplicationAdaptor::previewArchive(const QString &archivePath)
{
    if (!QFileInfo::exists(archivePath)) {
        qWarning() << "Archive file does not exist:" << archivePath;
        return false;
    }
    
    if (!m_mainWindow) {
        qWarning() << "MainWindow not available";
        return false;
    }

    showWindow();
    m_mainWindow->slotDragSelectedFiles(QStringList{archivePath});
    return true;
}
