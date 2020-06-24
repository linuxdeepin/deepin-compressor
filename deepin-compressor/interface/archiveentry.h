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
#ifndef ARCHIVEENTRY_H
#define ARCHIVEENTRY_H

#include "archive_manager.h"

#include <QDateTime>



enum PathFormat {
    NoTrailingSlash,
    WithTrailingSlash
};

class Archive::Entry : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fullPath MEMBER m_fullPath WRITE setFullPath)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString permissions MEMBER m_permissions)
    Q_PROPERTY(QString owner MEMBER m_owner)
    Q_PROPERTY(QString group MEMBER m_group)
    Q_PROPERTY(qulonglong size MEMBER m_size)
    Q_PROPERTY(qulonglong compressedSize MEMBER m_compressedSize)
    Q_PROPERTY(QString link MEMBER m_link)
    Q_PROPERTY(QString ratio MEMBER m_ratio)
    Q_PROPERTY(QString CRC MEMBER m_CRC)
    Q_PROPERTY(QString BLAKE2 MEMBER m_BLAKE2)
    Q_PROPERTY(QString method MEMBER m_method)
    Q_PROPERTY(QString version MEMBER m_version)
    Q_PROPERTY(QDateTime timestamp MEMBER m_timestamp)
    Q_PROPERTY(bool isDirectory MEMBER m_isDirectory WRITE setIsDirectory)
    Q_PROPERTY(bool isPasswordProtected MEMBER m_isPasswordProtected)

public:

    explicit Entry(QObject *parent = nullptr, const QString &fullPath = {}, const QString &rootNode = {});
    ~Entry() override;

    void copyMetaData(const Archive::Entry *sourceEntry);

    QVector<Entry *> entries();
    const QVector<Entry *> entries() const;
    void setEntryAt(int index, Entry *value);
    void appendEntry(Entry *entry);
    void removeEntryAt(int index);
    Entry *getParent() const;
    void setParent(Entry *parent);
    void setFullPath(const QString &fullPath);
    QString fullPath(PathFormat format = WithTrailingSlash) const;
    QString name() const;
    void setIsDirectory(const bool isDirectory);
    bool isDir() const;
    int row() const;
    Entry *find(const QString &name) const;
    Entry *findByPath(const QStringList &pieces, int index = 0) const;
    void countChildren(uint &dirs, uint &files) const;

    bool operator==(const Archive::Entry &right) const;

    // 压缩包索引
    void setCompressIndex(int iIndex);
    int compressIndex();

public:
    QString rootNode;
    bool compressedSizeIsSet;

private:
    QVector<Entry *> m_entries;
    QMap<QString, int> m_mapIndex;
    int m_iIndex;
    QString         m_name;
    Entry           *m_parent;

    QString m_fullPath;
    QString m_permissions;
    QString m_owner;
    QString m_group;
    qulonglong m_size;
    qulonglong m_compressedSize;
    QString m_link;
    QString m_ratio;
    QString m_CRC;
    QString m_BLAKE2;
    QString m_method;
    QString m_version;
    QDateTime m_timestamp;
    bool m_isDirectory;
    bool m_isPasswordProtected;

    int m_iCompressIndex =  0;
};

QDebug  operator<<(QDebug d, const Archive::Entry &entry);
QDebug  operator<<(QDebug d, const Archive::Entry *entry);


Q_DECLARE_METATYPE(Archive::Entry *)

#endif //ARCHIVEENTRY_H
