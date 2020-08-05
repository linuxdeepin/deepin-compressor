#pragma once
#include <QString>

QString  trans2uft8(const char *str);
QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);
QByteArray textCodecDetect(const QByteArray &data, const QString &fileName);
