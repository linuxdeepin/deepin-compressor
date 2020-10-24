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

int Archive::Entry::count = 0;

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
    count++;
    //qDebug() << "自增后count:" << count;
    m_mapIndex.clear();
    if (!fullPath.isEmpty())
        setFullPath(fullPath);
    m_iIndex = 0;
}

Archive::Entry::~Entry()
{
    count--;
    //qDebug() << "自减后count:" << count;

    for (int i = 0; i < m_entries.count(); ++i) {
        auto aa = m_entries[i];
        if (aa) {
            delete aa;
            aa = nullptr;
        }
    }
    m_entries.clear();
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
    m_mapIndex[entry->name()] = m_iIndex;
    m_iIndex++;
}

void Archive::Entry::removeEntryAt(int index)
{
    Q_ASSERT(isDir());
    Q_ASSERT(index < m_entries.count());
    m_mapIndex.remove(m_entries[index]->name());
    m_entries.remove(index);
    m_iIndex--;

    QMap<QString, int>::iterator iter = m_mapIndex.begin();
    while (iter != m_mapIndex.end()) {
        if (iter.value() > index) {
            iter.value() -= 1;
        }

        iter++;
    }
}

Archive::Entry *Archive::Entry::getParent() const
{
    return m_parent;
}

void Archive::Entry::setParent(Archive::Entry *parent)
{
    m_parent = parent;
}

qint64 Archive::Entry::getSize()
{
    return m_size;
}

QDateTime Archive::Entry::getTime()
{
    return m_timestamp;
}

void Archive::Entry::calAllSize(qint64 &size)
{
    if (this->isDir() == false) {
        size += getSize();
        //size += 1;
        return;
    }

    const auto archiveEntries = this->entries();
    for (auto entry : archiveEntries) {
        if (entry->isDir() == true) {
            entry->calAllSize(size);
        } else {
            size += entry->getSize();
            //size += 1;//如果计算真实的大小就用上面getSize，如果计算有效文件的个数，就+1
        }
    }
}

void Archive::Entry::calEntriesCount(qint64 &count) const
{
    if (this->isDir() == false) {
        count += 1;
        return;
    } else {
        count += 1;
    }

    const auto archiveEntries = this->entries();
    for (auto entry : archiveEntries) {
        if (entry->isDir() == true) {
            entry->calEntriesCount(count);
        } else {
            count += 1;
        }
    }
}

void Archive::Entry::setSize(qint64 size)
{
    m_size = size;
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
    //    for (Entry *entry : qAsConst(m_entries)) {
    //        if (entry && (entry->name() == name)) {
    //            return entry;
    //        }
    //    }

    if (m_mapIndex.contains(name) && m_mapIndex[name] < m_entries.count()) {
        return m_entries[m_mapIndex[name]];
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

void Archive::Entry::getAllNodesFullPath(QStringList &pList)
{
    pList.append(this->fullPath());
    if (this->isDir() == false) {
        return;
    }

    const auto archiveEntries = this->entries();
    for (Archive::Entry *entry : archiveEntries) {
        pList.append(entry->fullPath());
        if (entry->isDir() == true) {
            entry->getAllNodesFullPath(pList);
        }
    }
}

/**
 * @brief Archive::Entry::getFilesCount,include file,exclude dir
 * @param pEntry
 * @param count
 */
void Archive::Entry::getFilesCount(Archive::Entry *pEntry, int &count)
{
    if (pEntry->isDir() == false) {
        count++;
        return;
    }

    const auto archiveEntries = pEntry->entries();
    for (auto entry : archiveEntries) {
        this->getFilesCount(entry, count);
    }
}

void Archive::Entry::getVector(Entry *pE, QVector<Archive::Entry *> &vector)
{
    if (pE->isDir()) {
        const auto archiveEntries = pE->entries();
        for (auto entry : archiveEntries) {
            if (entry->isDir() == true) {
                this->getVector(entry, vector);
            } else {
                vector.append(entry);
            }
        }
    }

    vector.append(pE);
}

bool Archive::Entry::operator==(const Archive::Entry &right) const
{
    return m_fullPath == right.m_fullPath;
}

//void Archive::Entry::setCompressIndex(int iIndex)
//{
//    m_iCompressIndex = iIndex;
//}

//int Archive::Entry::compressIndex()
//{
//    return m_iCompressIndex;
//}

void Archive::Entry::clean()
{
    Archive::Entry *p = this;
    if (p->isDir() == false) {
        delete p;
        p = nullptr;
    }

    const auto archiveEntries = this->entries();
    for (auto entry : archiveEntries) {
        if (entry->isDir() == true && entry != nullptr) {
            entry->clean();
        }

        if (entry) {
            delete entry;
            entry = nullptr;
        }
    }
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
