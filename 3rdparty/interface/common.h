#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QLocale>

class Common: public QObject
{
    Q_OBJECT
public:
    Common(QObject *parent)
        : QObject(parent) {}

public:
    static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country);
    QString  trans2uft8(const char *str);
    QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);
    QByteArray textCodecDetect(const QByteArray &data, const QString &fileName);

    QByteArray m_codecStr;
};
//QString  trans2uft8(const char *str);
//QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
//int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);
//QByteArray textCodecDetect(const QByteArray &data, const QString &fileName);

#endif
