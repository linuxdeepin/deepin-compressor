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
#include "archiveentry.h"


Archive::Entry::Entry(QObject *parent, const QString &fullPath, const QString &rootNode)
    : QObject(parent)
    , rootNode(rootNode)
    , compressedSizeIsSet(true)
    , m_parent((Entry *)(parent))
    , m_size(0)
    , m_compressedSize(0)
    , m_isDirectory(false)
    , m_isPasswordProtected(false)
{
    if (!fullPath.isEmpty())
        setFullPath(fullPath);
}

Archive::Entry::~Entry()
{
}

void Archive::Entry::copyMetaData(const Archive::Entry *sourceEntry)
{
    setProperty("fullPath", sourceEntry->property("fullPath"));
    setProperty("permissions", sourceEntry->property("permissions"));
    setProperty("owner", sourceEntry->property("owner"));
    setProperty("group", sourceEntry->property("group"));
    setProperty("size", sourceEntry->property("size"));
    setProperty("compressedSize", sourceEntry->property("compressedSize"));
    setProperty("link", sourceEntry->property("link"));
    setProperty("ratio", sourceEntry->property("ratio"));
    setProperty("CRC", sourceEntry->property("CRC"));
    setProperty("BLAKE2", sourceEntry->property("BLAKE2"));
    setProperty("method", sourceEntry->property("method"));
    setProperty("version", sourceEntry->property("version"));
    setProperty("timestamp", sourceEntry->property("timestamp").toDateTime());
    setProperty("isDirectory", sourceEntry->property("isDirectory"));
    setProperty("isPasswordProtected", sourceEntry->property("isPasswordProtected"));
}

QVector<Archive::Entry *> Archive::Entry::entries()
{
    Q_ASSERT(isDir());
    return m_entries;
}

const QVector<Archive::Entry *> Archive::Entry::entries() const
{
    Q_ASSERT(isDir());
    return m_entries;
}

void Archive::Entry::setEntryAt(int index, Entry *value)
{
    Q_ASSERT(isDir());
    Q_ASSERT(index < m_entries.count());
    m_entries[index] = value;
}

void Archive::Entry::appendEntry(Entry *entry)
{
    Q_ASSERT(isDir());
    m_entries.append(entry);
}

void Archive::Entry::removeEntryAt(int index)
{
    Q_ASSERT(isDir());
    Q_ASSERT(index < m_entries.count());
    m_entries.remove(index);
}

Archive::Entry *Archive::Entry::getParent() const
{
    return m_parent;
}

void Archive::Entry::setParent(Archive::Entry *parent)
{
    m_parent = parent;
}

void Archive::Entry::setFullPath(const QString &fullPath)
{
    m_fullPath = fullPath;
    const QStringList pieces = m_fullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    m_name = pieces.isEmpty() ? QString() : pieces.last();
}

QString Archive::Entry::fullPath(PathFormat format) const
{
    if (format == NoTrailingSlash && m_fullPath.endsWith(QLatin1Char('/'))) {
        return m_fullPath.left(m_fullPath.size() - 1);
    } else {
        return m_fullPath;
    }
}

QString Archive::Entry::name() const
{
    return m_name;
}

void Archive::Entry::setIsDirectory(const bool isDirectory)
{
    m_isDirectory = isDirectory;
}

bool Archive::Entry::isDir() const
{
    return m_isDirectory;
}

int Archive::Entry::row() const
{
    if (getParent()) {
        return getParent()->entries().indexOf(const_cast<Archive::Entry *>(this));
    }
    return 0;
}

Archive::Entry *Archive::Entry::find(const QString &name) const
{
    for (Entry *entry : qAsConst(m_entries)) {
        if (entry && (entry->name() == name)) {
            return entry;
        }
    }
    return nullptr;
}

Archive::Entry *Archive::Entry::findByPath(const QStringList &pieces, int index) const
{
    if (index == pieces.count()) {
        return nullptr;
    }

    Entry *next = find(pieces.at(index));
    if (index == pieces.count() - 1) {
        return next;
    }
    if (next && next->isDir()) {
        return next->findByPath(pieces, index + 1);
    }
    return nullptr;
}

void Archive::Entry::countChildren(uint &dirs, uint &files) const
{
    dirs = files = 0;
    if (!isDir()) {
        return;
    }

    const auto archiveEntries = entries();
    for (auto entry : archiveEntries) {
        if (entry->isDir()) {
            dirs++;
        } else {
            files++;
        }
    }
}

bool Archive::Entry::operator==(const Archive::Entry &right) const
{
    return m_fullPath == right.m_fullPath;
}

QDebug operator<<(QDebug d, const Archive::Entry &entry)
{
    d.nospace() << "Entry(" << entry.property("fullPath");
    if (!entry.rootNode.isEmpty()) {
        d.nospace() << "," << entry.rootNode;
    }
    d.nospace() << ")";
    return d.space();
}

QDebug operator<<(QDebug d, const Archive::Entry *entry)
{
    d.nospace() << "Entry(" << entry->property("fullPath");
    if (!entry->rootNode.isEmpty()) {
        d.nospace() << "," << entry->rootNode;
    }
    d.nospace() << ")";
    return d.space();
}


