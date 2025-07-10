// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopfile.h"
#include "properties.h"
#include <QFile>
#include <QSettings>
#include <QDebug>
#include <QLocale>

/**
 * @brief Loads desktop file
 * @param fileName
 */
DesktopFile::DesktopFile(const QString &fileName)
    : m_fileName(fileName)
{
    qDebug() << "DesktopFile constructor started, file:" << fileName;
    // File validity
    if (m_fileName.isEmpty() || !QFile::exists(fileName)) {
        qWarning() << "Desktop file is empty or does not exist:" << fileName;
        return;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(fileName, "Desktop Entry");

    if (desktop.contains("X-Deepin-AppID")) {
        m_deepinId = desktop.value("X-Deepin-AppID", settings.value("X-Deepin-AppID")).toString();
        qInfo() << "Loaded X-Deepin-AppID:" << m_deepinId;
    } else {
        qDebug() << "No X-Deepin-AppID found";
    }

    if (desktop.contains("X-Deepin-Vendor")) {
        m_deepinVendor = desktop.value("X-Deepin-Vendor", settings.value("X-Deepin-Vendor")).toString();
        qInfo() << "Loaded X-Deepin-Vendor:" << m_deepinVendor;
    } else {
        qDebug() << "No X-Deepin-Vendor found";
    }

    if (desktop.contains("NoDisplay")) {
        m_noDisplay = desktop.value("NoDisplay", settings.value("NoDisplay").toBool()).toBool();
        qDebug() << "NoDisplay set to:" << m_noDisplay;
    }
    if (desktop.contains("Hidden")) {
        m_hidden = desktop.value("Hidden", settings.value("Hidden").toBool()).toBool();
        qDebug() << "Hidden set to:" << m_hidden;
    }

    //由于获取的系统语言简写与.desktop的语言简写存在不对应关系，经决定先采用获取的系统值匹配
    //若没匹配到则采用系统值"_"左侧的字符串进行匹配，均为匹配到，才走原未匹配流程
    auto getValueFromSys = [&desktop, &settings](const QString & type, const QString & sysName)->QString {
        const QString key = QString("%0[%1]").arg(type).arg(sysName);
        QString result = desktop.value(key, settings.value(key)).toString();
        qDebug() << "Getting localized value for key:" << key << "result:" << result;
        return result;
    };

    auto getNameByType = [&desktop, &settings, &getValueFromSys](const QString & type)->QString{
        qDebug() << "Getting name by type:" << type;
        QString tempSysName = QLocale::system().name();
        QString targetName = getValueFromSys(type, tempSysName);
        if (targetName.isEmpty())
        {
            qDebug() << "No direct match found, trying language prefix";
            auto strSize = tempSysName.trimmed().split("_");
            if (!strSize.isEmpty()) {
                tempSysName = strSize.first();
                targetName = getValueFromSys(type, tempSysName);
            }

            if (targetName.isEmpty()) {
                qDebug() << "No localized name found, using default";
                targetName = desktop.value(type, settings.value(type)).toString();
            }
        }

        qDebug() << "Final name for type" << type << ":" << targetName;
        return targetName;
    };
    m_localName = getNameByType("Name");
    m_genericName = getNameByType("GenericName");
    qDebug() << "Localized name:" << m_localName << "Generic name:" << m_genericName;

    m_exec = desktop.value("Exec", settings.value("Exec")).toString();
    m_icon = desktop.value("Icon", settings.value("Icon")).toString();
    m_type = desktop.value("Type", settings.value("Type", "Application")).toString();
    m_categories = desktop.value("Categories", settings.value("Categories").toString()).toString().remove(" ").split(";");
    qDebug() << "Loaded basic properties - exec:" << m_exec << "icon:" << m_icon << "type:" << m_type;

    QString mime_type = desktop.value("MimeType", settings.value("MimeType").toString()).toString().remove(" ");
    qDebug() << "Raw MIME type string:" << mime_type;

    if (!mime_type.isEmpty()) {
        m_mimeType = mime_type.split(";");
        qDebug() << "Parsed MIME types:" << m_mimeType;
    }
    // Fix categories
    if (m_categories.first().compare("") == 0) {
        m_categories.removeFirst();
        qDebug() << "Removed empty category";
    }
    qDebug() << "DesktopFile constructor finished, loaded" << m_mimeType.size() << "MIME types";
}
//---------------------------------------------------------------------------

QString DesktopFile::getFileName() const
{
    qDebug() << "Getting desktop file name:" << m_fileName;
    return m_fileName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getPureFileName() const
{
    QString pureName = m_fileName.split("/").last().remove(".desktop");
    qDebug() << "Getting pure desktop file name:" << pureName;
    return pureName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getName() const
{
    qDebug() << "Getting desktop file name:" << m_name;
    return m_name;
}

QString DesktopFile::getLocalName() const
{
    qDebug() << "Getting desktop file local name:" << m_localName;
    return m_localName;
}

QString DesktopFile::getDisplayName() const
{
    QString displayName;
    if (m_deepinVendor == QStringLiteral("deepin") && !m_genericName.isEmpty()) {
        qDebug() << "Using generic name for deepin vendor";
        displayName = m_genericName;
    } else {
        displayName = m_localName.isEmpty() ? m_name : m_localName;
        qDebug() << "Using" << (m_localName.isEmpty() ? "name" : "local name") << "for display";
    }
    qDebug() << "Display name:" << displayName;
    return displayName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getExec() const
{
    // qDebug() << "Getting desktop file exec command:" << m_exec;
    return m_exec;
}
//---------------------------------------------------------------------------

QString DesktopFile::getIcon() const
{
    // qDebug() << "Getting desktop file icon:" << m_icon;
    return m_icon;
}
//---------------------------------------------------------------------------

QString DesktopFile::getType() const
{
    // qDebug() << "Getting desktop file type:" << m_type;
    return m_type;
}

QString DesktopFile::getDeepinId() const
{
    // qDebug() << "Getting Deepin ID:" << m_deepinId;
    return m_deepinId;
}

QString DesktopFile::getDeepinVendor() const
{
    // qDebug() << "Getting Deepin vendor:" << m_deepinVendor;
    return m_deepinVendor;
}

bool DesktopFile::getNoShow() const
{
    bool noShow = m_noDisplay || m_hidden;
    // qDebug() << "Getting NoShow status:" << noShow << "(NoDisplay:" << m_noDisplay << "Hidden:" << m_hidden << ")";
    return noShow;
}

//---------------------------------------------------------------------------

QStringList DesktopFile::getCategories() const
{
    // qDebug() << "Getting desktop file categories:" << m_categories;
    return m_categories;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getMimeType() const
{
    // qDebug() << "Getting desktop file MIME types:" << m_mimeType;
    return m_mimeType;
}
//---------------------------------------------------------------------------
