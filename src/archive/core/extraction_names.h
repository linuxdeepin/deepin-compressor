// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_EXTRACTION_NAMES_H
#define ARCHIVE_EXTRACTION_NAMES_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <limits.h>

namespace dcarchive {

inline bool hasOversizedComponent(const QString &path)
{
    if (path.size() <= NAME_MAX / 4)
        return false;
    const QByteArray encoded = QFile::encodeName(path);
    if (encoded.size() <= NAME_MAX)
        return false;
    for (const auto &part : encoded.split('/')) {
        if (part.size() > NAME_MAX)
            return true;
    }
    return false;
}

// Allocate in archive order even when a local conflict is skipped.
class ExtractionNames
{
public:
    enum class Suffix { Complete, Zip };
    explicit ExtractionNames(Suffix suffix = Suffix::Complete) : m_suffix(suffix) {}

    QString outputName(const QString &relativePath, bool directory)
    {
        if (!hasOversizedComponent(relativePath))
            return relativePath;

        const QStringList parts = QDir::cleanPath(relativePath).split(QLatin1Char('/'));
        QString parent;
        QString output;
        for (int i = 0; i < parts.size(); ++i) {
            const QString &part = parts.at(i);
            const bool isDir = directory || i + 1 < parts.size();
            const QString original = parent + part;
            QString diskName = part;
            if (QFile::encodeName(part).size() > NAME_MAX) {
                const auto known = m_directories.constFind(original);
                if (isDir && known != m_directories.constEnd()) {
                    diskName = *known;
                } else {
                    QString stem = part.left(60);
                    if (stem.back().isHighSurrogate())
                        stem.chop(1);
                    auto &counts = isDir ? m_directoryCounts : m_fileCounts;
                    const int serial = ++counts[parent + stem];
                    if (serial > 999)
                        return {};
                    diskName = stem + QStringLiteral("(%1)").arg(serial, 3, 10, QLatin1Char('0'));
                    if (!isDir) {
                        const QFileInfo info(part);
                        QString suffix = info.completeSuffix();
                        if (m_suffix == Suffix::Zip && suffix.size() > 10)
                            suffix = info.suffix().right(10);
                        diskName += QLatin1Char('.') + suffix;
                        if (QFile::encodeName(diskName).size() > NAME_MAX)
                            return {};
                    } else {
                        m_directories.insert(original, diskName);
                    }
                }
            }
            if (i != 0)
                output += QLatin1Char('/');
            output += diskName;
            parent = original + QLatin1Char('/');
        }
        m_shortened = true;
        return output;
    }

    bool shortened() const { return m_shortened; }

private:
    Suffix m_suffix;
    bool m_shortened = false;
    QHash<QString, int> m_fileCounts;
    QHash<QString, int> m_directoryCounts;
    QHash<QString, QString> m_directories;
};

} // namespace dcarchive

#endif // ARCHIVE_EXTRACTION_NAMES_H
