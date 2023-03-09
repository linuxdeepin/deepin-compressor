// Copyright (C) 2020 ~ 2023, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QFileInfo>
#include <QtDebug>
#include <DSettings>
#include "dbusadpator.h"
const QString SOPENLIST = "openfiles";

ApplicationAdaptor::ApplicationAdaptor(QApplication *application)
    : QDBusAbstractAdaptor(application), app(application)
{
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
    m_sFile = sFile;
}


void ApplicationAdaptor::raise(const QString &sFile)
{
    qInfo() << "raise window from dbus";
    if(m_sFile.isEmpty() || m_sFile.isNull())
        return;
    if(m_curShowWidget && (m_sFile == sFile))
        m_curShowWidget->activateWindow();
}

/**
 * @brief isExistPid    压缩包文件名是否存在打开窗口
 * @param sFile         压缩包文件名
 * @param pid           进程号
 */
bool CompressSetting::isExistPid(const QString &sFile, const int &pid)
{
    bool bRet = QFileInfo::exists(QString("/proc/%1/status").arg(pid));
    if(!bRet) {
        removeDataSetting(sFile);
    }
    return bRet;
}

CompressSetting::CompressSetting(QObject *pMainWid)
    :QObject (pMainWid)
{
    m_pSettings = new QSettings("Compress");
}

/**
 * @brief isExistSetFile    压缩包文件名是否存在打开窗口
 * @param sFile    压缩包文件名
 */
bool CompressSetting::isExistSetFile(const QString &sFile)
{
    QVariantMap mapData = m_pSettings->value(SOPENLIST).toMap();
    if(mapData.contains(sFile)) {
        return isExistPid(sFile, mapData.value(sFile).toInt());
    }
    return false;
}

/**
 * @brief appendDataSetting    seting中添加压缩包文件名与进程号绑定
 * @param sFile     压缩包文件名
 * @param pid       进程号
 */
void CompressSetting::appendDataSetting(const QString &sFile, const int &pid)
{
    QVariantMap mapData = m_pSettings->value(SOPENLIST).toMap();
    mapData.insert(sFile, pid);
    m_pSettings->setValue(SOPENLIST, mapData);
    m_pSettings->sync();
}

/**
 * @brief removeDataSetting    seting中移除压缩包文件名绑定
 * @param sFile     压缩包文件名
 */
void CompressSetting::removeDataSetting(const QString &sFile)
{
    QVariantMap mapData = m_pSettings->value(SOPENLIST).toMap();
    mapData.remove(sFile);
    m_pSettings->setValue(SOPENLIST, mapData);
    m_pSettings->sync();
}

/**
 * @brief dataSetting   获取seting中绑定压缩包文件名的进程号
 * @param sFile         压缩包文件名
 * @return              绑定的进程号
 */
QString CompressSetting::dataSetting(const QString &sFile)
{
    QVariantMap mapData = m_pSettings->value(SOPENLIST).toMap();
    return mapData.value(sFile).toString();
}
