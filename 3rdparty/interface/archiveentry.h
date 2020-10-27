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

// 文件数据
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
    Q_PROPERTY(QString archiveComment MEMBER m_archiveComment)


public:
    static int count;
    explicit Entry(QObject *parent = nullptr, const QString &fullPath = {}, const QString &rootNode = {});
    ~Entry() override;

    /**
     * @brief copyMetaData  entry数据拷贝
     * @param sourceEntry   源数据
     */
    void copyMetaData(const Archive::Entry *sourceEntry);

    /**
     * @brief entries   获取子数据
     * @return
     */
    QVector<Entry *> entries();
    const QVector<Entry *> entries() const;

    void setEntryAt(int index, Entry *value);
    /**
     * @brief appendEntry   添加子数据
     * @param entry         子数据
     */
    void appendEntry(Entry *entry);

    /**
     * @brief removeEntryAt 移除子数据
     * @param index 索引值
     */
    void removeEntryAt(int index);

    /**
     * @brief getParent     获取父节点数据
     * @return
     */
    Entry *getParent() const;

    /**
     * @brief setParent     设置父节点数据
     * @param parent
     */
    void setParent(Entry *parent);

    /**
     * @brief getSize       获取文件大小
     * @return
     */
    qint64 getSize();

    /**
     * @brief getTime       获取最后一次修改时间
     * @return
     */
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

    /**
     * @brief setSize   设置文件大小
     * @param size
     */
    void setSize(qint64 size);

    /**
     * @brief setFullPath   设置文件全路径
     * @param fullPath
     */
    void setFullPath(const QString &fullPath);

    /**
     * @brief fullPath      获取文件全路径
     * @param format
     * @return
     */
    QString fullPath(PathFormat format = WithTrailingSlash) const;

    /**
     * @brief name  获取文件名
     * @return
     */
    QString name() const;

    /**
     * @brief setIsDirectory    设置是否是文件夹
     * @param isDirectory
     */
    void setIsDirectory(const bool isDirectory);

    /**
     * @brief isDir     判断此文件是否是文件夹
     * @return
     */
    bool isDir() const;

    /**
     * @brief row   获取行号
     * @return
     */
    int row() const;

    /**
     * @brief find  根据文件名寻找子节点数据
     * @param name  文件名
     * @return
     */
    Entry *find(const QString &name) const;

    /**
     * @brief findByPath    根据
     * @param pieces
     * @param index
     * @return
     */
    Entry *findByPath(const QStringList &pieces, int index = 0) const;

    /**
     * @brief countChildren     计算子文件和文件夹数目
     * @param dirs              文件夹数目
     * @param files             文件数目
     */
    void countChildren(uint &dirs, uint &files) const;

    /**
     * @brief getVector     获取某个文件夹下所有文件数目
     * @param pE            需要处理的文件夹
     * @param vector        所有子文件数据
     */
    void getVector(Entry *pE, QVector<Archive::Entry *> &vector);

    /**
     * @brief getAllNodesFullPath   获取所有子节点的fullpath列表
     * @param pList
     */
    void getAllNodesFullPath(QStringList &pList);

    /**
     * @brief getFilesCount     获取子文件夹数目
     * @param pEntry            文件数据
     * @param count             子文件数目
     */
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
    QDateTime m_timestamp;      // 最后一次修改时间
    QString rootNode;           // 根节点数据
    bool compressedSizeIsSet;   // 压缩大小是否已经设置了

private:
    QVector<Entry *> m_entries; // 子节点数据
    QMap<QString, int> m_mapIndex;  // 文件名-索引
    int m_iIndex;               // 索引值
    QString m_name;             // 文件名
    Entry *m_parent = nullptr; //此处应该赋值nullptr

    QString m_fullPath;         // 文件全路径
    QString m_permissions;      // 文件权限
    QString m_owner;            // 所属者
    QString m_group;            // 所属组
    qulonglong m_size;          // 文件大小
    qulonglong m_compressedSize;    // 文件压缩后大小
    QString m_link;         // 链接
    QString m_ratio;        // 压缩率
    QString m_CRC;
    QString m_BLAKE2;
    QString m_method;       // 压缩算法
    QString m_version;      // 版本
//    QDateTime m_timestamp;
    bool m_isDirectory;     // 是否是文件夹
    bool m_isPasswordProtected; // 是否加密
    QString m_archiveComment; // 压缩包的注释信息

    // int m_iCompressIndex =  0;
};

QDebug operator<<(QDebug d, const Archive::Entry &entry);
QDebug operator<<(QDebug d, const Archive::Entry *entry);

Q_DECLARE_METATYPE(Archive::Entry *)

#endif //ARCHIVEENTRY_H
