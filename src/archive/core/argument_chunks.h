// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_ARGUMENT_CHUNKS_H
#define ARCHIVE_ARGUMENT_CHUNKS_H

#include <QStringList>
#include <QTemporaryFile>
#include <QVector>

namespace dcarchive {

inline bool writeArgumentList(QTemporaryFile &file, const QStringList &entries, bool asciiOnly = false)
{
    for (const QString &entry : entries) {
        if (entry.trimmed() != entry || entry.contains(QLatin1Char('\n'))
                || entry.contains(QLatin1Char('\r')) || entry.startsWith(QLatin1Char('"')))
            return false;
        if (asciiOnly && QString::fromLatin1(entry.toUtf8()) != entry)
            return false;
    }
    if (!file.open())
        return false;
    const QByteArray data = entries.join(QLatin1Char('\n')).toUtf8() + '\n';
    if (file.write(data) != data.size() || !file.flush())
        return false;
    file.close();
    return true;
}

inline QVector<QStringList> splitArgumentChunks(const QStringList &arguments,
                                                int maxArguments = 256,
                                                int maxCharacters = 60000)
{
    QVector<QStringList> chunks;
    QStringList current;
    int characters = 0;
    for (const QString &argument : arguments) {
        const int required = argument.size() + 1;
        if (!current.isEmpty()
                && (current.size() >= maxArguments
                    || characters + required > maxCharacters)) {
            chunks.append(current);
            current.clear();
            characters = 0;
        }
        current.append(argument);
        characters += required;
    }
    if (!current.isEmpty())
        chunks.append(current);
    return chunks;
}

} // namespace dcarchive

#endif // ARCHIVE_ARGUMENT_CHUNKS_H
