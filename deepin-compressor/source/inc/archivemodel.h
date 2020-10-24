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
#ifndef ARCHIVEMODEL_H
#define ARCHIVEMODEL_H

#include "archiveentry.h"
#include "mimetypedisplaymanager.h"

#include <QAbstractItemModel>
#include <QScopedPointer>
#include <QTableView>

class Query;

/**
 * Meta data related to one entry in a compressed archive.
 *
 * This is used for indexing entry properties as numbers
 * and for determining data displaying order in part's view.
 */
enum EntryMetaDataType {
    FullPath,            /**< The entry's file name */
    Timestamp,           /**< The timestamp for the current entry */
    Type,
    Size                /**< The entry's original size */
};

namespace  ArchiveModelDefine {
const int  gTableHeight = 36;
}

class ArchiveModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ArchiveModel(QObject *parent = nullptr);
    ~ArchiveModel()override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //drag and drop related
    Qt::DropActions supportedDropActions() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
//    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    /**
     * @brief resetmparent  重置父节点
     */
    void resetmparent();

    /**
     * @brief reset     重置数据
     */
    void reset();

    //void createEmptyArchive(const QString &path, const QString &mimeType, QObject *parent);

    /**
     * @brief loadArchive   加载压缩包数据
     * @param path  压缩包路径
     * @param mimeType  压缩包类型
     * @param parent    父节点
     * @return job
     */
    KJob *loadArchive(const QString &path, const QString &mimeType, QObject *parent);

    /**
     * @brief archive   返回管理接口类
     * @return
     */
    Archive *archive() const;

    /**
     * @brief shownColumns  获取显示的列
     * @return 返回列
     */
    QList<int> shownColumns() const;

    /**
     * @brief propertiesMap     获取列对应的属性参数
     * @return  属性参数
     */
    QMap<int, QByteArray> propertiesMap() const;

    /**
     * @brief indexForEntry     创建新的modelIndex
     * @return  新创建的index
     */
    QModelIndex indexForEntry(Archive::Entry *);

    /**
     * @brief entryForIndex     获取对应index上的entry数据
     * @param index     索引值
     * @return  entry数据
     */
    Archive::Entry *entryForIndex(const QModelIndex &index);

    /**
     * @brief isentryDir    判断当前索引文件是否是文件夹
     * @param index 索引值
     * @return  是否为文件夹
     */
    bool isentryDir(const QModelIndex &index);

    /**
     * @brief setPathIndex  设置目录层级
     * @param index 目录层级
     */
    void setPathIndex(int *index);

    /**
     * @brief setParentEntry    设置父节点
     * @param index 索引
     */
    void setParentEntry(const QModelIndex &index);

    /**
     * @brief getParentEntry    获取父节点
     * @return
     */
    Archive::Entry *getParentEntry();

    /**
     * @brief getRootEntry  获取根节点
     * @return
     */
    Archive::Entry *getRootEntry();

    /**
     * @brief 检查entry是否已存在
     * @param fullPath  entry路径
     * @return
     */
    Archive::Entry *isExists(QString fullPath);

    /**
     * @brief setTableView  设置显示列表
     * @param tableview 列表
     */
    void setTableView(QTableView *tableview);

    /**
     * @brief createNoncolumnIndex
     * @param index
     * @return
     */
    QModelIndex createNoncolumnIndex(const QModelIndex &index) const;

    //ExtractJob *extractFile(Archive::Entry *file, const QString &destinationDir, const ExtractionOptions &options = ExtractionOptions()) const;

    /**
     * @brief extractFiles  解压/提取操作
     * @param files     需要解压的文件，如果数目为0,为解压，否则是部分提取
     * @param destinationDir    解压路径
     * @param options   解压相关参数
     * @return
     */
    ExtractJob *extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDir, const ExtractionOptions &options = ExtractionOptions()) const;

    /**
     * @brief preview   预览压缩包内文件
     * @param file  文件数据
     * @return
     */
    PreviewJob *preview(Archive::Entry *file) const;

    /**
     * @brief open  打开压缩包内文件
     * @param file  文件数据
     * @return
     */
    OpenJob *open(Archive::Entry *file) const;

    /**
     * @brief openWith  以...打开压缩包内文件
     * @param file  文件数据
     * @return
     */
    OpenWithJob *openWith(Archive::Entry *file) const;

    //AddJob *addFilesOld(QVector<Archive::Entry *> &entries, const Archive::Entry *destination, const CompressionOptions &options = CompressionOptions());

    /**
     * @brief addFiles      压缩操作
     * @param entries   待压缩文件
     * @param destination   压缩路径
     * @param pIface    插件
     * @param options   压缩参数
     * @return
     */
    AddJob *addFiles(QVector<Archive::Entry *> &entries, const Archive::Entry *destination, ReadOnlyArchiveInterface *pIface = nullptr, const CompressionOptions &options = CompressionOptions());

    /**
     * @brief moveFiles     移动操作
     * @param entries   待移动的文件
     * @param destination   压缩包路径
     * @param options   压缩参数
     * @return
     */
    MoveJob *moveFiles(QVector<Archive::Entry *> &entries, Archive::Entry *destination, const CompressionOptions &options = CompressionOptions());

    /**
     * @brief copyFiles     复制操作
     * @param entries   待复制文件
     * @param destination   压缩包路径
     * @param options   压缩参数
     * @return
     */
    CopyJob *copyFiles(QVector<Archive::Entry *> &entries, Archive::Entry *destination, const CompressionOptions &options = CompressionOptions());

    /**
     * @brief deleteFiles   删除操作
     * @param entries   待删除的文件
     * @return
     */
    DeleteJob *deleteFiles(QVector<Archive::Entry *> entries);

    /**
     * @brief insertEntryIcons  存储文件类型图标
     * @param map
     */
    void appendEntryIcons(const QHash<QString, QIcon> &map);

    /**
     * @param password The password to encrypt the archive with.
     * @param encryptHeader Whether to encrypt also the list of files.
     */
    void encryptArchive(const QString &password, bool encryptHeader);

    /**
     * @brief getPlugin     获取插件
     * @return
     */
    ReadOnlyArchiveInterface *getPlugin();

Q_SIGNALS:
    void loadingStarted();
    void loadingFinished(KJob *);
    void extractionFinished(bool success);
    void error(const QString &error, const QString &details);
    void droppedFiles(const QStringList &files, const Archive::Entry *);
    void sigShowLabel() const;
    void signalUserQuery(Query *query);

private Q_SLOTS:
    /**
     * @brief slotNewEntry  列表刷新添加新的entry
     * @param entry 新数据
     */
    void slotNewEntry(Archive::Entry *entry);

    /**
     * @brief slotAddEntry  追加新的文件
     * @param entry 文件数据
     */
    void slotAddEntry(Archive::Entry *entry);

    /**
     * @brief slotListEntry     加载时添加新数据
     * @param entry
     */
    void slotListEntry(Archive::Entry *entry);

    /**
     * @brief slotLoadingFinished   加载结束
     * @param job
     */
    void slotLoadingFinished(KJob *job);

    /**
     * @brief slotEntryRemoved  删除文件数据
     * @param path  路径
     */
    void slotEntryRemoved(const QString &path);
    //void slotUserQuery(Query *query);
    /**
     * @brief slotCleanupEmptyDirs  删除/移动文件之后删除空文件夹
     */
    void slotCleanupEmptyDirs();

private:
    /**
     * Strips file names that start with './'.
     *
     * For more information, see bug 194241.
     *
     * @param fileName The file name that will be stripped.
     *
     * @return @p fileName without the leading './'
     */
    QString cleanFileName(const QString &fileName);

    /**
     * @brief initRootEntry 初始化根节点
     */
    void initRootEntry();

    enum InsertBehaviour { NotifyViews, DoNotNotifyViews };

    /**
     * @brief parentFor 获取某个entry的父节点数据
     * @param entry     当前节点
     * @param behaviour
     * @return
     */
    Archive::Entry *parentFor(const Archive::Entry *entry, InsertBehaviour behaviour = NotifyViews);
    /**
     * Insert the node @p node into the model, ensuring all views are notified
     * of the change.
     */

    /**
     * @brief insertEntry   插入新的entry
     * @param entry     新的entry
     * @param behaviour
     */
    void insertEntry(Archive::Entry *entry, InsertBehaviour behaviour = NotifyViews);

    /**
     * @brief newEntry      处理新的entry
     * @param receivedEntry 传递的待处理的entry
     * @param behaviour
     */
    void newEntry(Archive::Entry *receivedEntry, InsertBehaviour behaviour);


    /**
     * @brief setPlugin     设置插件
     * @param interface     插件
     */
    void setPlugin(ReadOnlyArchiveInterface *interface);

public:
    QMap<QString, Archive::Entry *> mapFilesUpdate; // 更新的文件

private:
    QList<int> m_showColumns; // 显示列
    QScopedPointer<Archive> m_archive; // 插件管理
    QScopedPointer<Archive::Entry> m_rootEntry; // 根节点
    QHash<QString, QIcon> m_entryIcons; // 类型图标
    QMap<int, QByteArray> m_propertiesMap; // 属性列表

    QString m_dbusPathName; // dbus路径

    // Whether a file entry has been listed. Used to ensure all relevent columns are shown,
    // since directories might have fewer columns than files.
    bool m_fileEntryListed;

    int *m_ppathindex = nullptr; // 目录层级
    QTableView *m_tableview; // 显示表格
    MimeTypeDisplayManager *m_mimetype; // 类型管理
    Archive::Entry *m_parent = nullptr; // 父节点
    //    Used to speed up the loading of large archives.
    Archive::Entry *s_previousMatch = nullptr;
    QStringList *s_previousPieces = new QStringList();
    ReadOnlyArchiveInterface *m_plugin;

    QVector<Archive::Entry *> vecSameEntry;
};
#endif // ARCHIVEMODEL_H
