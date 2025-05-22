// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog/properties.h"

#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDebug>

/**
 * @brief Creates properties
 * @param fileName
 * @param group
 */
Properties::Properties(const QString &fileName, const QString &group)
{
    qDebug() << "Creating Properties object for file:" << fileName << "group:" << group;
    if (!fileName.isEmpty()) {
        load(fileName, group);
    }
    qDebug() << "Properties object created with" << data.size() << "items";
}

/**
 * @brief Creates properties
 * @param other properies
 */
Properties::Properties(const Properties &other)
{
    qDebug() << "Creating Properties copy";
    this->data = other.data;
    qDebug() << "Copied" << data.size() << "properties";
}

/**
 * @brief Loads property file
 * @param fileName
 * @param group
 * @return true if load was successful
 */
bool Properties::load(const QString &fileName, const QString &group)
{
    qDebug() << "Loading properties from file:" << fileName << "group:" << group;

    // NOTE: This class is used for reading of property files instead of QSettings
    // class, which considers separator ';' as comment

    // Try open file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open properties file:" << fileName;
        return false;
    }
    qDebug() << "File opened successfully";

    // Clear old data
    data.clear();
    qDebug() << "Cleared existing properties";

    // Indicator whether group was found or not, if name of group was not
    // specified, groupFound is always true
    bool groupFound = group.isEmpty();
    qDebug() << "Group search:" << (groupFound ? "disabled" : "enabled");

    int loadedProperties = 0;
    // Read propeties
    QTextStream in(&file);
    while (!in.atEnd()) {
        // Read new line
        QString line = in.readLine();

        // Skip empty line or line with invalid format
        if (line.trimmed().isEmpty()) {
            continue;
        }

        // Read group
        // NOTE: symbols '[' and ']' can be found not only in group names, but
        // only group can start with '['
        if (!group.isEmpty() && line.trimmed().startsWith("[")) {
            QString tmp = line.trimmed().replace("[", "").replace("]", "");
            groupFound = group.trimmed().compare(tmp) == 0;
            qDebug() << "Found group:" << tmp << "match:" << groupFound;
        }

        // If we are in correct group and line contains assignment then read data
        int first_equal = line.indexOf('=');

        if (groupFound && first_equal >= 0) {
            QString key = line.left(first_equal).trimmed();
            QString value = line.mid(first_equal + 1).trimmed();
            data.insert(key, value);
            loadedProperties++;
            qDebug() << "Loaded property:" << key << "=" << value;
        }
    }

    file.close();
    qInfo() << "Loaded" << loadedProperties << "properties from file";

    return true;
}

/**
 * @brief Saves properties to file
 * @param fileName
 * @param group
 * @return true if success
 */
bool Properties::save(const QString &fileName, const QString &group)
{
    qDebug() << "Saving properties to file:" << fileName << "group:" << group;

    // Try open file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open properties file for writing:" << fileName;
        return false;
    }
    qDebug() << "File opened for writing";

    // Write group
    QTextStream out(&file);
    if (!group.isEmpty()) {
        out << "[" + group + "]\n";
        qDebug() << "Wrote group header:" << group;
    }

    // Write data
    int savedProperties = 0;
    foreach (QString key, data.keys()) {
        QString value = data.value(key).toString();
        out << key << "=" << value << "\n";
        savedProperties++;
        qDebug() << "Saved property:" << key << "=" << value;
    }

    // Exit
    file.close();
    qInfo() << "Saved" << savedProperties << "properties to file";

    return true;
}

/**
 * @brief Returns true if property with given key is present in properties
 * @param key
 * @return true if property with given key is present in properties
 */
bool Properties::contains(const QString &key) const
{
    return data.contains(key);
}

/**
 * @brief Returns value
 * @param key
 * @param defaultValue
 * @return value
 */
QVariant Properties::value(const QString &key, const QVariant &defaultValue)
{
    qDebug() << "Getting property value for key:" << key;
    if (!data.contains(key)) {
        qDebug() << "Key not found, using default value";
    }
    return data.value(key, defaultValue);
}

/**
 * @brief Sets value to properties
 * @param key
 * @param value
 */
void Properties::set(const QString &key, const QVariant &value)
{
    qDebug() << "Setting property value for key:" << key;
    if (data.contains(key)) {
        qDebug() << "Key already exists, replacing value";
        data.take(key);
    }

    data.insert(key, value);
    qDebug() << "Property set successfully";
}

/**
 * @brief Returns keys (names of properties)
 * @param key
 * @param value
 */
QStringList Properties::getKeys() const
{
    qDebug() << "Getting all property keys, count:" << data.size();
    return data.keys();
}
