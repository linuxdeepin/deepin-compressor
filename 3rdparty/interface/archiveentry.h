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

//#include <iostream>
//#include <vector>

// Factory
//template<typename T>
//class EntryFactory
//{
//public:
//    T *create();
//    void load(T *);
//    void cleanup();
//    EntryFactory();
//private:
//    std::vector<T *> objs_;
//};

//template<typename T>
//EntryFactory<T>::EntryFactory()
//{
//}

//template<typename T>
//void EntryFactory<T>::cleanup()
//{
//    foreach (T *pOjb, objs_) {
//        if (pOjb) {
//            std::cout << "release " << pOjb << std::endl;
//            delete pOjb;
//            pOjb = nullptr;
//        }
//    }
//    objs_.clear();
//}

//template<typename T>
//T *EntryFactory<T>::create()
//{
//    T *obj = new T;
//    objs_.push_back(obj);

//    return obj;
//}

//template<typename T>
//void EntryFactory<T>::load(T *obj)
//{
//    objs_.push_back(obj);
//}

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
    static int count;
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
    qint64 getSize();
    QDateTime getTime();
    /**
     * @brief calAllSize(include child entry's size)
     * @param size
     * @see 计算当前entry节点及其子节点的文件总大小
     */
    void calAllSize(qint64 &size);
    /**
     * @brief calEntriesCount
     * @param count
     * @see 计算entry树的所有节点数目,文件夹和文件都算1个
     */
    void calEntriesCount(qint64 &count) const;
    void setSize(qint64 size);
    void setFullPath(const QString &fullPath);
    QString fullPath(PathFormat format = WithTrailingSlash) const;
    QString name() const;
    void setIsDirectory(const bool isDirectory);
    bool isDir() const;
    int row() const;
    Entry *find(const QString &name) const;
    Entry *findByPath(const QStringList &pieces, int index = 0) const;
    void countChildren(uint &dirs, uint &files) const;
    void getVector(Entry *pE, QVector<Archive::Entry *> &vector);
    /**
     * @brief getAllNodesFullPath
     * @param pE
     * @param pList
     * @see 获取所有子节点的fullpath列表
     */
    void getAllNodesFullPath(QStringList &pList);
    void getFilesCount(Archive::Entry *pEntry, int &count);
    bool operator==(const Archive::Entry &right) const;
    /**
     * @brief clean
     * @see 释放内存
     */
    void clean();

    // 压缩包索引
    //void setCompressIndex(int iIndex);
    //int compressIndex();

public:
    QDateTime m_timestamp;
    QString rootNode;
    bool compressedSizeIsSet;

private:
    QVector<Entry *> m_entries;
    QMap<QString, int> m_mapIndex;
    int m_iIndex;
    QString m_name;
    Entry *m_parent = nullptr; //此处应该赋值nullptr

    QString m_fullPath;
    QString m_permissions;
    QString m_owner;
    QString m_group;
    qint64 m_size;
    qulonglong m_compressedSize;
    QString m_link;
    QString m_ratio;
    QString m_CRC;
    QString m_BLAKE2;
    QString m_method;
    QString m_version;
//    QDateTime m_timestamp;
    bool m_isDirectory;
    bool m_isPasswordProtected;

    // int m_iCompressIndex =  0;
};

QDebug operator<<(QDebug d, const Archive::Entry &entry);
QDebug operator<<(QDebug d, const Archive::Entry *entry);

Q_DECLARE_METATYPE(Archive::Entry *)

#endif //ARCHIVEENTRY_H
