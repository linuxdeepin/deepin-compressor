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
#include "archivemodel.h"
#include "jobs.h"
#include "mimetypes.h"
#include "utils.h"

#include <DPalette>
#include <DFontSizeManager>

#include <QDBusConnection>
#include <QFileIconProvider>
#include <QMimeData>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QTextCodec>
#include <QUrl>

DWIDGET_USE_NAMESPACE

ArchiveModel::ArchiveModel(QObject *parent)
    : QAbstractItemModel(parent)
    //    , m_numberOfFiles(0)
    //    , m_numberOfFolders(0)
    , m_fileEntryListed(false)
{
    // 初始化根节点
    initRootEntry();

    m_mimetype = new MimeTypeDisplayManager(this);

    // 初始化列数据
    m_propertiesMap = {
        { FullPath, "fullPath" },
        { Timestamp, "timestamp" },
        { Type, "type" },
        { Size, "size" },

    };
}

ArchiveModel::~ArchiveModel()
{
//    Archive::Entry *pRootEntry = this->getRootEntry();
//    if (pRootEntry) {
//        pRootEntry->clean();
//    }

    m_archive.reset(nullptr);
}

QVariant ArchiveModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        Archive::Entry *entry = static_cast<Archive::Entry *>(index.internalPointer());
        switch (role) {
        case Qt::DisplayRole: {
            // TODO: complete the columns.
            int column = m_showColumns.at(index.column());
            switch (column) {
            case FullPath:
                return entry->name();
            case Type: {
//                Directory can be identified by fullpath
                QMimeType mimetype = entry->isDir() ? determineMimeType(entry->fullPath()) : determineMimeType(entry->name());
                return m_mimetype->displayName(mimetype.name());
            }
            case Size:
                if (entry->isDir()) {
                    if (m_plugin) {
                        if (m_plugin->isAllEntry()) {
                            uint dirs;
                            uint files;
                            entry->countChildren(dirs, files);
                            return QString::number(dirs + files) + " " + tr("item(s)") + "    ";//KIO::itemsSummaryString(dirs + files, files, dirs, 0, false);
                        } else {
                            return QString::number(entry->property("size").toLongLong()) + " " + tr("item(s)") + "    ";
                        }
                    } else {
                        return QVariant();
                    }

                } else if (!entry->property("link").toString().isEmpty()) {
                    return QVariant();
                } else {
                    return Utils::humanReadableSize(entry->property("size").toLongLong(), 1);
                }
            case Timestamp: {
                const QDateTime timeStamp = entry->property("timestamp").toDateTime();
                return QLocale().toString(timeStamp, tr("yyyy/MM/dd hh:mm:ss"));
            }

            default:
                return entry->property(m_propertiesMap[column].constData());
            }
        }
        case Qt::DecorationRole:
            if (index.column() == 0) {
                const Archive::Entry *e = static_cast<Archive::Entry *>(index.internalPointer());
                //QIcon::Mode mode = (filesToMove.contains(e->fullPath())) ? QIcon::Disabled : QIcon::Normal;
                return m_entryIcons.value(e->fullPath(NoTrailingSlash)).pixmap(24, 24 /*, mode*/);
            }

            return QVariant();
        case Qt::TextAlignmentRole:

            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

        case Qt::TextColorRole: {
            DPalette pa;
            pa.setBrush(DPalette::WindowText, pa.color(DPalette::WindowText));
            return pa;
        }
        case Qt::FontRole:
            if (0 == index.column()) {
                QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
                font.setWeight(QFont::Medium);
                return font;
            } else {
                QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T7);
                font.setWeight(QFont::Normal);
                return font;
            }
        case Qt::AccessibleTextRole: {
            int col = index.column();
            switch (col) {
            case 3:
            case 2:
            case 1:
            case 0: {
                qDebug() << QString("UncompressContent_%1_%2").arg(index.row()).arg(col) << index.data();
                return QString("UncompressContent_%1_%2").arg(index.row()).arg(col);
            }
            default:
                return "";
            }
        }
        default:
            return QVariant();
        }
    }

    return QVariant();
}

Qt::ItemFlags ArchiveModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | defaultFlags;
    }

    return nullptr;
}

QVariant ArchiveModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (section >= m_showColumns.size() || section < 0) {
            qDebug() << "WEIRD: showColumns.size = " << m_showColumns.size()
                     << " and section = " << section;
            return QVariant();
        }

        int columnId = m_showColumns.at(section);

        switch (columnId) {
        case FullPath:
            return tr("Name");
        case Size:
            return tr("Size");
        case Type:
            return tr("Type");
        case Timestamp:
            return tr("Time modified");
        default:
            return "-";
        }
    } else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    return QVariant();
}

QModelIndex ArchiveModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent)) {
        const Archive::Entry *parentEntry = parent.isValid()
                                            ? static_cast<Archive::Entry *>(parent.internalPointer())
                                            : m_rootEntry.data();

        Q_ASSERT(parentEntry->isDir());

        const Archive::Entry *item = parentEntry->entries().value(row, nullptr);
        if (item != nullptr) {
            return createIndex(row, column, const_cast<Archive::Entry *>(item));
        }
    }

    return QModelIndex();
}

bool ArchiveModel::isentryDir(const QModelIndex &index)
{
    // 获取当前索引位置的entry，并且判断是否是文件夹
    const Archive::Entry *parentEntry = index.isValid()
                                        ? static_cast<Archive::Entry *>(index.internalPointer())
                                        : m_rootEntry.data();
    return parentEntry->isDir();
}

void ArchiveModel::setPathIndex(int *index)
{
    m_ppathindex = index;
}

void ArchiveModel::setParentEntry(const QModelIndex &index)
{
    // 获取当前索引位置的entry，并将此数据存储为父节点数据
    Archive::Entry *parentEntry = index.isValid()
                                  ? static_cast<Archive::Entry *>(index.internalPointer())
                                  : m_rootEntry.data();
    if (parentEntry->isDir()) {
        m_parent = parentEntry;
    }
}

Archive::Entry *ArchiveModel::getParentEntry()
{
    if (m_ppathindex == nullptr) {
        return nullptr;
    }

    if (*m_ppathindex == 0) { // 如果目录层级为0,即根目录，返回根目录数据
        return m_rootEntry.data();
    } else {
        return m_parent; // 否则返回父节点数据
    }
}

Archive::Entry *ArchiveModel::getRootEntry()
{
    return this->m_rootEntry.data();
}

Archive::Entry *ArchiveModel::isExists(QString fullPath)
{
    QFileInfo fileInfo(fullPath);
    Archive::Entry *parent = this->getParentEntry(); // 获取父节点数据
    //qint64 size = fileInfo.size();
    // 判断父节点是否为空，若为空，获取根节点
    if (parent == nullptr) {
        parent = this->getRootEntry();
        if (parent == nullptr) {
            return nullptr;
        }
    }

    QVector<Archive::Entry *> vector = parent->entries(); // 获取父节点下的所有子节点数据
    QVector<Archive::Entry *>::iterator it = vector.begin();
    while (it != vector.end()) { // 对所有子节点进行迭代判重
        Archive::Entry *entry = *it;
        //qint64 sizeOri = entry->property("size").toLongLong();
        if (entry->name() == fileInfo.fileName() /*&& size == sizeOri*/) {
            return entry;
        }

        it++;
    }

    return nullptr;
}

void ArchiveModel::setTableView(QTableView *tableview)
{
    m_tableview = tableview;
}

QModelIndex ArchiveModel::createNoncolumnIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        Archive::Entry *item = static_cast<Archive::Entry *>(index.internalPointer());
        Q_ASSERT(item);
        if (item->getParent()) {
            return createIndex(item->row(), 0, const_cast<Archive::Entry *>(item));
        }
    }

    return QModelIndex();
}

QModelIndex ArchiveModel::parent(const QModelIndex &index) const
{
    if (index.isValid()) {
        Archive::Entry *item = static_cast<Archive::Entry *>(index.internalPointer());
        Q_ASSERT(item);
        if (item->getParent() && (item->getParent() != m_rootEntry.data())) {
            return createIndex(item->getParent()->row(), 0, item->getParent());
        }
    }

    return QModelIndex();
}

Archive::Entry *ArchiveModel::entryForIndex(const QModelIndex &index)
{
    if (index.isValid()) {
        Archive::Entry *item = static_cast<Archive::Entry *>(index.internalPointer());
        Q_ASSERT(item);
        return item;
    }

    return nullptr;
}

int ArchiveModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() <= 0) {
        const Archive::Entry *parentEntry = parent.isValid()
                                            ? static_cast<Archive::Entry *>(parent.internalPointer())
                                            : m_rootEntry.data();

        if (parentEntry && parentEntry->isDir()) {
            return parentEntry->entries().count();
        }
    }

    return 0;
}

int ArchiveModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_showColumns.size();
}

Qt::DropActions ArchiveModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *ArchiveModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("application/x-kde-ark-dndextract-service"),
                      QDBusConnection::sessionBus().baseService().toUtf8());
    mimeData->setData(QStringLiteral("application/x-kde-ark-dndextract-path"),
                      m_dbusPathName.toUtf8());

    return mimeData;
}

bool ArchiveModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action)

    if (!data->hasUrls()) {
        return false;
    }

    if (archive()->isReadOnly() ||
            (archive()->encryptionType() != Archive::Unencrypted &&
             archive()->password().isEmpty())) {
        return false;
    }

    QStringList paths;
    const auto urls = data->urls();
    for (const QUrl &url : urls) {
        paths << url.toLocalFile();
    }

    const Archive::Entry *entry = nullptr;
    QModelIndex droppedOnto = index(row, column, parent);
    if (droppedOnto.isValid()) {
        entry = entryForIndex(droppedOnto);
        if (!entry->isDir()) {
            entry = entry->getParent();
        }
    }

    emit droppedFiles(paths, entry);

    return true;
}

void ArchiveModel::resetmparent()
{
    if (m_parent) {
        while (m_parent->getParent() != nullptr) {
            m_parent = m_parent->getParent();
        }

        m_parent = m_parent->getParent();
    }
}

// For a rationale, see bugs #194241, #241967 and #355839
QString ArchiveModel::cleanFileName(const QString &fileName)
{
    // Skip entries with filename "/" or "//" or "."
    // "." is present in ISO files.
    static QRegularExpression pattern(QStringLiteral("/+|\\."));
    QRegularExpressionMatch match;
    if (fileName.contains(pattern, &match) && match.captured() == fileName) {
        qDebug() << "Skipping entry with filename" << fileName;
        return QString();
    } else if (fileName.startsWith(QLatin1String("./"))) {
        return fileName.mid(2);
    }

    return fileName;
}

void ArchiveModel::initRootEntry()
{
    m_rootEntry.reset(new Archive::Entry());
    m_rootEntry->setProperty("isDirectory", true);
}

Archive::Entry *ArchiveModel::parentFor(const Archive::Entry *entry, InsertBehaviour behaviour)
{
    QStringList pieces = entry->fullPath().split(QLatin1Char('/'), QString::SkipEmptyParts);
    if (pieces.isEmpty()) {
        return nullptr;
    }

    pieces.removeLast();

    // Used to speed up loading of large archives.
    if (s_previousMatch) {
        // The number of path elements must be the same for the shortcut
        // to work.
        if (s_previousPieces->count() == pieces.count()) {
            bool equal = true;

            // Check if all pieces match.
            for (int i = 0; i < s_previousPieces->count(); ++i) {
                if (s_previousPieces->at(i) != pieces.at(i)) {
                    equal = false;
                    break;
                }
            }

            // If match return it.
            if (equal) {
                return s_previousMatch;
            }
        }
    }

    Archive::Entry *parent = m_rootEntry.data();

    for (const QString &piece : qAsConst(pieces)) {
        Archive::Entry *entry = parent->find(piece);
        if (!entry) {
            // Directory entry will be traversed later (that happens for some archive formats, 7z for instance).
            // We have to create one before, in order to construct tree from its children,
            // and then delete the existing one (see ArchiveModel::newEntry).
            entry = new Archive::Entry(parent);

            entry->setProperty("fullPath", (parent == m_rootEntry.data())
                               ? QString(piece + QLatin1Char('/'))
                               : QString(parent->fullPath(WithTrailingSlash) + piece + QLatin1Char('/')));
            entry->setProperty("isDirectory", true);
            insertEntry(entry, behaviour);
        }

        if (!entry->isDir()) {
            Archive::Entry *e = new Archive::Entry(parent);
            e->copyMetaData(entry);
            // Maybe we have both a file and a directory of the same name.
            // We avoid removing previous entries unless necessary.
            insertEntry(e, behaviour);
        }

        parent = entry;
    }

    s_previousMatch = parent;
    *s_previousPieces = pieces;

    return parent;
}

QModelIndex ArchiveModel::indexForEntry(Archive::Entry *entry)
{
    Q_ASSERT(entry);
    if (entry != m_rootEntry.data()) {
        Q_ASSERT(entry->getParent());
        Q_ASSERT(entry->getParent()->isDir());
        return createIndex(entry->row(), 0, entry);
    }

    return QModelIndex();
}

void ArchiveModel::slotEntryRemoved(const QString &path)
{
    const QString entryFileName(cleanFileName(path));
    if (entryFileName.isEmpty()) {
        return;
    }

    // 根据路径获取对应的entry
    Archive::Entry *entry = m_rootEntry->findByPath(entryFileName.split(QLatin1Char('/'), QString::SkipEmptyParts));
    if (entry) {
        Archive::Entry *parent = entry->getParent();

        if (parent == nullptr)
            return;

        QModelIndex index = indexForEntry(entry);
        Q_UNUSED(index)

        // 开始移除某条数据
        beginRemoveRows(indexForEntry(parent), entry->row(), entry->row());
        m_entryIcons.remove(parent->entries().at(entry->row())->fullPath(NoTrailingSlash));
        parent->removeEntryAt(entry->row());


        if (m_plugin) {
            if (!m_plugin->isAllEntry()) {
                parent->setProperty("size", parent->property("size").toLongLong() - 1);
            }
        }

        endRemoveRows();
    }
}


void ArchiveModel::slotNewEntry(Archive::Entry *entry)
{
    newEntry(entry, NotifyViews);
}

void ArchiveModel::slotAddEntry(Archive::Entry *receivedEntry)
{
    InsertBehaviour behaviour = NotifyViews;
    if (receivedEntry->fullPath().isEmpty()) {
        qDebug() << "Weird, received empty entry (no filename) - skipping";
        return;
    }

    QString parentPath = "";
    Archive::Entry *parentEntry = receivedEntry->getParent();
    if (parentEntry != nullptr) {
        parentPath = parentEntry->fullPath();

        if (m_plugin) {
            if (!m_plugin->isAllEntry()) {
                parentEntry->setProperty("size", parentEntry->property("size").toLongLong() + 1);
            }
        }
    }

    receivedEntry->setFullPath(parentPath + receivedEntry->name());

    // If there are no columns registered, then populate columns from entry. If the first entry
    // is a directory we check again for the first file entry to ensure all relevent columms are shown.
    //添加m_fileEntryListed判断，追加压缩时有概率多出几列空白列
    if (m_showColumns.isEmpty() /* || !m_fileEntryListed*/) {
        m_showColumns = {0, 1, 2, 3};//<< toInsert;
        if (behaviour == NotifyViews) {
            beginInsertColumns(QModelIndex(), 0, m_showColumns.size() - 1);
        }

        if (behaviour == NotifyViews) {
            endInsertColumns();
        }

        m_fileEntryListed = !receivedEntry->isDir();
    }

    // #194241: Filenames such as "./file" should be displayed as "file"
    // #241967: Entries called "/" should be ignored
    // #355839: Entries called "//" should be ignored
    QString entryFileName = cleanFileName(receivedEntry->fullPath());
    if (entryFileName.isEmpty()) { // The entry contains only "." or "./"
        return;
    }

    receivedEntry->setProperty("fullPath", entryFileName);

    // For some archive formats (e.g. AppImage and RPM) paths of folders do not
    // contain a trailing slash, so we append it.
    if (receivedEntry->property("isDirectory").toBool() &&
            !receivedEntry->property("fullPath").toString().endsWith(QLatin1Char('/'))) {
        receivedEntry->setProperty("fullPath", QString(receivedEntry->property("fullPath").toString() + QLatin1Char('/')));
        qDebug() << "Trailing slash appended to entry:" << receivedEntry->property("fullPath");
    }

    // Skip already created entries.
    Archive::Entry *existing = m_rootEntry->findByPath(entryFileName.split(QLatin1Char('/')));
    if (existing) {
        existing->setProperty("fullPath", entryFileName);
        // Multi-volume files are repeated at least in RAR archives.
        // In that case, we need to sum the compressed size for each volume
        qulonglong currentCompressedSize = existing->property("compressedSize").toULongLong();
        existing->setProperty("compressedSize", currentCompressedSize + receivedEntry->property("compressedSize").toULongLong());
        return;
    }

    // Find parent entry, creating missing directory Archive::Entry's in the process.
    Archive::Entry *parent = parentFor(receivedEntry, behaviour);
    // Create an Archive::Entry.
    const QStringList path = entryFileName.split(QLatin1Char('/'), QString::SkipEmptyParts);
    Archive::Entry *entry = parent->find(path.last());
    if (entry) {
        entry->copyMetaData(receivedEntry);
        entry->setProperty("fullPath", entryFileName);
    } else {
        QString parentPath = QString(parent->property("fullPath").toString());
        QString childPath = QString(receivedEntry->property("fullPath").toString());
        if (parentPath != "" && childPath.contains(parentPath)) {
            qDebug() << parentPath;
        } else {
            receivedEntry->setProperty("fullPath", parentPath + childPath);
        }

        receivedEntry->setParent(parent);
        if (parent->entries().contains(receivedEntry) == false) {
            insertEntry(receivedEntry, behaviour);
        }
    }
}

void ArchiveModel::slotListEntry(Archive::Entry *entry)
{
    newEntry(entry, DoNotNotifyViews);
}

void ArchiveModel::newEntry(Archive::Entry *receivedEntry, InsertBehaviour behaviour)
{


    if (receivedEntry->fullPath().isEmpty()) {
        qDebug() << "Weird, received empty entry (no filename) - skipping";
        return;
    }

    // If there are no columns registered, then populate columns from entry. If the first entry
    // is a directory we check again for the first file entry to ensure all relevent columms are shown.
    if (m_showColumns.isEmpty() || !m_fileEntryListed) {
//        QList<int> toInsert;

        m_showColumns = {0, 1, 2, 3};//<< toInsert;
        if (behaviour == NotifyViews) {
            beginInsertColumns(QModelIndex(), 0, m_showColumns.size() - 1);
        }

        if (behaviour == NotifyViews) {
            endInsertColumns();
        }

        m_fileEntryListed = !receivedEntry->isDir();
    }

    // #194241: Filenames such as "./file" should be displayed as "file"
    // #241967: Entries called "/" should be ignored
    // #355839: Entries called "//" should be ignored
    QString entryFileName = cleanFileName(receivedEntry->fullPath());
    if (entryFileName.isEmpty()) { // The entry contains only "." or "./"
        return;
    }

    receivedEntry->setProperty("fullPath", entryFileName);

    // For some archive formats (e.g. AppImage and RPM) paths of folders do not
    // contain a trailing slash, so we append it.
    if (receivedEntry->property("isDirectory").toBool() &&
            !receivedEntry->property("fullPath").toString().endsWith(QLatin1Char('/'))) {
        receivedEntry->setProperty("fullPath", QString(receivedEntry->property("fullPath").toString() + QLatin1Char('/')));
        qDebug() << "Trailing slash appended to entry:" << receivedEntry->property("fullPath");
    }

    // Find parent entry, creating missing directory Archive::Entry's in the process.
    Archive::Entry *parent = parentFor(receivedEntry, behaviour);
    //added by hsw for get valid parent begin
    if (m_parent != nullptr && parent != m_parent) {
        parent = m_parent;
    }
    //added end;

    // Create an Archive::Entry.
    const QStringList path = entryFileName.split(QLatin1Char('/'), QString::SkipEmptyParts);
    Archive::Entry *entry = parent->find(path.last());
    if (entry) {
        entry->copyMetaData(receivedEntry);
        //entry->setCompressIndex(receivedEntry->compressIndex());
        entry->setProperty("fullPath", entryFileName);
        /*      if (!entry->isDir()) {
                  insertEntry(entry, behaviour);
              } */
    } else {
        QString parentPath = QString(parent->property("fullPath").toString());
        QString childPath = QString(receivedEntry->property("fullPath").toString());
        if (parentPath != "" && childPath.contains(parentPath)) {
//            qDebug() << parentPath;
        } else {
            receivedEntry->setProperty("fullPath", parentPath + childPath);
        }

        receivedEntry->setParent(parent);
        insertEntry(receivedEntry, behaviour);
    }
}

void ArchiveModel::slotLoadingFinished(KJob *job)
{
    std::sort(m_showColumns.begin(), m_showColumns.end()); // 对列进行排序

    if (!job->error()) {
        qDebug() << "Showing columns: " << m_showColumns;

        m_archive.reset(qobject_cast<LoadJob *>(job)->archive());

        beginResetModel();
        endResetModel();
    }

    emit loadingFinished(job); // 发送加载结束信号

    if (m_tableview && m_showColumns.length() > 0) {
        m_tableview->sortByColumn(0, Qt::AscendingOrder);
        //refresh entry file count
        for (int i = 0 ; i < rowCount(); ++i) {
            QModelIndex  mIndex = index(i, 0);
            Archive::Entry *pEntry = entryForIndex(mIndex);
            if (pEntry && pEntry->isDir()) {
                qulonglong sizeVal = pEntry->property("size").value<qulonglong>();
                if (!sizeVal) {
                    if (ReadOnlyArchiveInterface *pInterface = getPlugin()) {
                        pInterface->RefreshEntryFileCount(pEntry); // 刷新文件夹显示的子文件数目
                    }
                }
            }
        }
    }
}

void ArchiveModel::insertEntry(Archive::Entry *entry, InsertBehaviour behaviour)
{
    Q_ASSERT(entry);
    Archive::Entry *parent = entry->getParent();
    Q_ASSERT(parent);

    // 判断类型以便区分是否需要向列表中插入一行数据
    if (behaviour == NotifyViews) {
        beginInsertRows(indexForEntry(parent), parent->entries().count(), parent->entries().count());
    }

    parent->appendEntry(entry); // 向父节点中插入当前数据
    if (behaviour == NotifyViews) {
        endInsertRows();
    }

    // Save an icon for each newly added entry.
    QMimeDatabase db;
    QIcon icon;
    // 根据类型获取图标
    entry->isDir()
    ? icon = QIcon::fromTheme(db.mimeTypeForName(QStringLiteral("inode/directory")).iconName()).pixmap(24, 24)
             : icon = QIcon::fromTheme(db.mimeTypeForFile(entry->fullPath()).iconName()).pixmap(24, 24);
    if (icon.isNull()) {
        icon = QIcon::fromTheme("empty").pixmap(24, 24);
    }

//    qDebug()<<icon;
    m_entryIcons.insert(entry->fullPath(NoTrailingSlash), icon);
}

void ArchiveModel::appendEntryIcons(const QHash<QString, QIcon> &map)
{
    // 向m_entryIcons添加新的文件名及其类型图标
    QHash<QString, QIcon>::const_iterator iter1 = map.constBegin();
    while (iter1 != map.constEnd()) {
        m_entryIcons.insert(iter1.key(), iter1.value());
        ++iter1;
    }
}

Archive *ArchiveModel::archive() const
{
    return m_archive.data();
}

void ArchiveModel::reset()
{
    //重置相关数据
    m_archive.reset(nullptr);
    s_previousMatch = nullptr;
    s_previousPieces->clear();
    initRootEntry();
    this->mapFilesUpdate.clear();//added by hsw
    // TODO: make sure if it's ok to not have calls to beginRemoveColumns here
    m_showColumns.clear();
    beginResetModel();
    endResetModel();
}

KJob *ArchiveModel::loadArchive(const QString &path, const QString &mimeType, QObject *parent)
{
    reset();

    // 创建加载操作，加载压缩包数据
    auto loadJob = Archive::load(path, mimeType, parent);
    connect(loadJob, &KJob::result, this, &ArchiveModel::slotLoadingFinished);
    connect(loadJob, &Job::newEntry, this, &ArchiveModel::slotListEntry);
    connect(loadJob, &Job::userQuery, this, &ArchiveModel::signalUserQuery);

    setPlugin(loadJob->archiveInterface());
    connect(loadJob->archiveInterface(), &ReadOnlyArchiveInterface::entry, this, &ArchiveModel::slotListEntry);

    emit loadingStarted(); // 发送加载开始信号

    return loadJob;
}

ExtractJob *ArchiveModel::extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDir, const ExtractionOptions &options) const
{
    Q_ASSERT(m_archive);

    if (m_archive->encryptionType() == Archive::Unencrypted) { //没有加密的

    } else {
        //是否启用头部加密,如果启用头部加密，当前用户肯定已经输入正确密码；所以要记录密码，并且将加密状态设置为Archive::Encrypted
        //如果不是头部加密，那就是文件加密了，所以需要将密码设置空字符串，同样加密状态设置为Archive::Encrypted.
        QString psd = m_archive->password();
        bool headerEncrypted = m_archive->encryptionType() == Archive::HeaderEncrypted;
        psd = headerEncrypted ? psd : "";
        m_archive->encrypt(psd, headerEncrypted);
    }

    // 创建解压操作
    ExtractJob *newJob = m_archive->extractFiles(files, destinationDir, options);
    connect(newJob, &ExtractJob::userQuery, this, &ArchiveModel::signalUserQuery);
    return newJob;
}

PreviewJob *ArchiveModel::preview(Archive::Entry *file) const
{
    Q_ASSERT(m_archive);
    PreviewJob *job = m_archive->preview(file); // 预览压缩包文件
    connect(job, &Job::userQuery, this, &ArchiveModel::signalUserQuery);
    return job;
}

OpenJob *ArchiveModel::open(Archive::Entry *file) const
{
    Q_ASSERT(m_archive);
    OpenJob *job = m_archive->open(file); // 打开压缩包文件
    connect(job, &Job::userQuery, this, &ArchiveModel::signalUserQuery);
    return job;
}

OpenWithJob *ArchiveModel::openWith(Archive::Entry *file) const
{
    Q_ASSERT(m_archive);
    OpenWithJob *job = m_archive->openWith(file); // 以...方式打开压缩包文件
    connect(job, &Job::userQuery, this, &ArchiveModel::signalUserQuery);
    return job;
}

AddJob *ArchiveModel::addFiles(QVector<Archive::Entry *> &entries, const Archive::Entry *destination, ReadOnlyArchiveInterface *pIface, const CompressionOptions &options)
{
    if (!m_archive) {
        return nullptr;
    }

    if (!m_archive->isReadOnly()) {
        // 创建添加/压缩操作
        AddJob *job = m_archive->addFiles(entries, destination, pIface, options);
        connect(job, &AddJob::newEntry, this, &ArchiveModel::slotNewEntry);
        connect(job, &AddJob::userQuery, this, &ArchiveModel::signalUserQuery);
        connect(job, &AddJob::addEntry, this, &ArchiveModel::slotAddEntry);

        return job;
    }

    return nullptr;
}

MoveJob *ArchiveModel::moveFiles(QVector<Archive::Entry *> &entries, Archive::Entry *destination, const CompressionOptions &options)
{
    if (!m_archive) {
        return nullptr;
    }

    if (!m_archive->isReadOnly()) {
        // 创建移动操作
        MoveJob *job = m_archive->moveFiles(entries, destination, options);
        connect(job, &MoveJob::newEntry, this, &ArchiveModel::slotNewEntry);
        connect(job, &MoveJob::userQuery, this, &ArchiveModel::signalUserQuery);
        connect(job, &MoveJob::entryRemoved, this, &ArchiveModel::slotEntryRemoved);
        connect(job, &MoveJob::finished, this, &ArchiveModel::slotCleanupEmptyDirs);


        return job;
    }

    return nullptr;
}
CopyJob *ArchiveModel::copyFiles(QVector<Archive::Entry *> &entries, Archive::Entry *destination, const CompressionOptions &options)
{
    if (!m_archive) {
        return nullptr;
    }

    if (!m_archive->isReadOnly()) {
        // 创建拷贝操作
        CopyJob *job = m_archive->copyFiles(entries, destination, options);
        connect(job, &CopyJob::newEntry, this, &ArchiveModel::slotNewEntry);
        connect(job, &CopyJob::userQuery, this, &ArchiveModel::signalUserQuery);

        return job;
    }

    return nullptr;
}

DeleteJob *ArchiveModel::deleteFiles(QVector<Archive::Entry *> entries)
{
    Q_ASSERT(m_archive);
    if (!m_archive->isReadOnly()) {
        // 创建删除操作
        DeleteJob *job = m_archive->deleteFiles(entries);
        connect(job, &DeleteJob::entryRemoved, this, &ArchiveModel::slotEntryRemoved);

        connect(job, &DeleteJob::finished, this, &ArchiveModel::slotCleanupEmptyDirs);

        connect(job, &DeleteJob::userQuery, this, &ArchiveModel::signalUserQuery);
        return job;
    }

    return nullptr;
}

void ArchiveModel::encryptArchive(const QString &password, bool encryptHeader)
{
    if (!m_archive) {
        return;
    }

    m_archive->encrypt(password, encryptHeader);
}

ReadOnlyArchiveInterface *ArchiveModel::getPlugin()
{
    return  m_plugin;
}

void ArchiveModel::slotCleanupEmptyDirs()
{
    QList<QPersistentModelIndex> queue;
    QList<QPersistentModelIndex> nodesToDelete;

    // Add root nodes.
    for (int i = 0; i < rowCount(); ++i) {
        queue.append(QPersistentModelIndex(index(i, 0)));
    }

    // Breadth-first traverse.
    while (!queue.isEmpty()) {
        QPersistentModelIndex node = queue.takeFirst();
        Archive::Entry *entry = entryForIndex(node);

        if (!hasChildren(node)) {
            if (entry->fullPath().isEmpty()) {
                nodesToDelete << node;
            }
        } else {
            for (int i = 0; i < rowCount(node); ++i) {
                queue.append(QPersistentModelIndex(index(i, 0, node)));
            }
        }
    }

    for (const QPersistentModelIndex &node : qAsConst(nodesToDelete)) {
        Archive::Entry *rawEntry = static_cast<Archive::Entry *>(node.internalPointer());
        qDebug() << "Delete with parent entries " << rawEntry->getParent()->entries() << " and row " << rawEntry->row();
        beginRemoveRows(parent(node), rawEntry->row(), rawEntry->row());
        m_entryIcons.remove(rawEntry->getParent()->entries().at(rawEntry->row())->fullPath(NoTrailingSlash));
        rawEntry->getParent()->removeEntryAt(rawEntry->row());
        endRemoveRows();
    }
}


void ArchiveModel::setPlugin(ReadOnlyArchiveInterface *interface)
{
    m_plugin = interface; // 返回插件指针
}


QList<int> ArchiveModel::shownColumns() const
{
    return m_showColumns; // 返回表格的列数
}

QMap<int, QByteArray> ArchiveModel::propertiesMap() const
{
    return m_propertiesMap; // 返回属性列表
}
