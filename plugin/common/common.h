#pragma once
#include <QString>

QString  trans2uft8(const char *str);
QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
