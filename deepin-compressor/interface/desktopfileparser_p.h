#ifndef DESKTOPFILEPARSER_H
#define DESKTOPFILEPARSER_H

#include <QByteArray>
#include <QLoggingCategory>
#include <QVector>
class QJsonObject;
class QJsonValue;

Q_DECLARE_LOGGING_CATEGORY(DESKTOPPARSER)

struct CustomPropertyDefinition;
struct ServiceTypeDefinition
{
    ServiceTypeDefinition();

    static ServiceTypeDefinition fromFiles(const QStringList &paths);
    QJsonValue parseValue(const QByteArray &key, const QString &value) const;
    bool addFile(const QString &path);

private:
    QVector<CustomPropertyDefinition> m_definitions;
};

namespace DesktopFileParser
{
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
}


#endif // DESKTOPFILEPARSER_H
