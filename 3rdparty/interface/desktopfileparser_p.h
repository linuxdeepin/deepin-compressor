/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#ifndef DESKTOPFILEPARSER_H
#define DESKTOPFILEPARSER_H

#include <QByteArray>
#include <QLoggingCategory>
#include <QVector>

class QJsonObject;
class QJsonValue;

Q_DECLARE_LOGGING_CATEGORY(DESKTOPPARSER)

struct CustomPropertyDefinition;
struct ServiceTypeDefinition {
    ServiceTypeDefinition();

    static ServiceTypeDefinition fromFiles(const QStringList &paths);
    QJsonValue parseValue(const QByteArray &key, const QString &value) const;
    bool addFile(const QString &path);

private:
    QVector<CustomPropertyDefinition> m_definitions;
};

namespace DesktopFileParser {
QByteArray escapeValue(const QByteArray &input);
QStringList deserializeList(const QString &data, char separator = ',');
bool convert(const QString &src, const QStringList &serviceTypes, QJsonObject &json, QString *libraryPath);
void convertToJson(const QByteArray &key, ServiceTypeDefinition &serviceTypes, const QString &value,
                   QJsonObject &json, QJsonObject &kplugin, int lineNr);
#ifdef BUILDING_DESKTOPTOJSON_TOOL
void convertToCompatibilityJson(const QString &key, const QString &value, QJsonObject &json, int lineNr);
extern bool s_verbose;
extern bool s_compatibilityMode;
#endif
} // namespace DesktopFileParser

#endif // DESKTOPFILEPARSER_H
