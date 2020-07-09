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

// Used to speed up the loading of large archives.
static Archive::Entry *s_previousMatch = nullptr;
Q_GLOBAL_STATIC(QStringList, s_previousPieces)

ArchiveModel::ArchiveModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_numberOfFiles(0)
    , m_numberOfFolders(0)
    , m_fileEntryListed(false)
    , m_plugin(nullptr)
{
    initRootEntry();

    m_mimetype = new MimeTypeDisplayManager(this);
    // Mappings between column indexes and entry properties.
    m_propertiesMap = {
        { FullPath, "fullPath" },
        { Timestamp, "timestamp" },
        { Type, "type" },
        { Size, "size" },

    };
}

QVariant ArchiveModel::data(const QModelIndex &index, int role) const
{
    if ((1 == index.row() || 0 == index.row()) && 0 == index.column()) {
        if (m_ppathindex && *m_ppathindex > 0) {
            //m_tableview->setRowHeight(0, ArchiveModelDefine::gTableHeight * 2);
            emit sigShowLabel();
        } else {
            //m_tableview->setRowHeight(0, ArchiveModelDefine::gTableHeight);
        }
    }

//    if (0 != index.row()) {
//        m_tableview->setRowHeight(index.row(), ArchiveModelDefine::gTableHeight);
//    }

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
                QMimeType mimetype = determineMimeType(entry->name());
                return m_mimetype->displayName(mimetype.name());
            }
            case Size:
                if (entry->isDir()) {
                    uint dirs;
                    uint files;
                    entry->countChildren(dirs, files);
                    if (archive()->fileName().endsWith(".zip") || archive()->fileName().endsWith(".jar")) {
                        return QString::number(entry->property("size").toLongLong()) + " " + tr("item(s)") + "    ";
                    } else {
                        return QString::number(dirs + files) + " " + tr("item(s)") + "    ";//KIO::itemsSummaryString(dirs + files, files, dirs, 0, false);
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
                QIcon::Mode mode = (filesToMove.contains(e->fullPath())) ? QIcon::Disabled : QIcon::Normal;
                return m_entryIcons.value(e->fullPath(NoTrailingSlash)).pixmap(24, 24, mode);
            }
            return QVariant();
        case Qt::TextAlignmentRole:
            //if (m_showColumns.at(index.column()) == Size) {
            //return QVariant(Qt::AlignRight | Qt::AlignVCenter);
            //} else {
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        //}
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
    const Archive::Entry *parentEntry = index.isValid()
                                        ? static_cast<Archive::Entry *>(index.internalPointer())
                                        : m_rootEntry.data();
    return parentEntry->isDir();
}

void ArchiveModel::setPathIndex(int *index)
{
    m_ppathindex = index;
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

//void ArchiveModel::sort(int column, Qt::SortOrder order)
//{
//    class ProjectInfoSort
//    {
//    public:
//        ProjectInfoSort(int column, Qt::SortOrder order)
//            : mColumn(column)
//            , mSortOrder(order)
//        {}

//        bool operator()(const Archive::Entry& info1, const Archive::Entry& info2)
//        {
//            bool is_less = false;
//            switch ( mColumn )
//            {
//            case Id:

//                break;
//            default:
//                break;
//            }

//            if ( mSortOrder == Qt::DescendingOrder )
//            {
//                is_less = !is_less;
//            }

//            return is_less;
//        }
//    private:
//        int                 mColumn;
//        Qt::SortOrder       mSortOrder;
//    };

//    ProjectInfoSort sorter(column, order);
//    qSort(mProjectInfo.begin(), mProjectInfo.end(), sorter);


//}

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

    Archive::Entry *entry = m_rootEntry->findByPath(entryFileName.split(QLatin1Char('/'), QString::SkipEmptyParts));
    if (entry) {
        Archive::Entry *parent = entry->getParent();
        QModelIndex index = indexForEntry(entry);
        Q_UNUSED(index)

        beginRemoveRows(indexForEntry(parent), entry->row(), entry->row());
        m_entryIcons.remove(parent->entries().at(entry->row())->fullPath(NoTrailingSlash));
        parent->removeEntryAt(entry->row());
        endRemoveRows();
    }
}

void ArchiveModel::slotUserQuery(Query *query)
{
    query->execute();
}

void ArchiveModel::slotNewEntry(Archive::Entry *entry)
{
    newEntry(entry, NotifyViews);
}

void ArchiveModel::slotListEntry(Archive::Entry *entry)
{
    newEntry(entry, DoNotNotifyViews);
}

void ArchiveModel::newEntry(Archive::Entry *receivedEntry, InsertBehaviour behaviour)
{
//    qDebug()<<receivedEntry->name();
//    qDebug()<<receivedEntry->name().toLocal8Bit();
//    qDebug()<<Utils::detectEncode(receivedEntry->name().toLocal8Bit());
//    QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
//    QTextCodec* codec = QTextCodec::codecForName(Utils::detectEncode(receivedEntry->name().toLocal8Bit()));

//    QString strUnicode= codec->toUnicode(receivedEntry->name().toLocal8Bit().data());
//    qDebug()<<"unicode:"<<strUnicode;
//    QByteArray ByteUtf8= utf8Codec->fromUnicode(strUnicode);
//    char *utf8code = ByteUtf8.data();
//    qDebug()<<ByteUtf8;

//    for (int mib : QTextCodec::availableMibs()) {
//        QTextCodec *codec = QTextCodec::codecForMib(mib);
//        QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
//        qDebug()<<codec->name();

//        QString strUnicode= codec->toUnicode("\xC1\xB6\xBC\xB1\xBE\xEE");
//        qDebug()<<"unicode:"<<strUnicode;
//        QByteArray ByteUtf8= utf8Codec->fromUnicode(strUnicode);
//        char *utf8code = ByteUtf8.data();
//        qDebug()<<utf8code;
//    }

//    QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
//    QTextCodec* gb2312Codec = QTextCodec::codecForName("EUC-KR");

//    QByteArray tempdata = "\xC1\xB6\xBC\xB1\xBE\xEE";
//    qDebug()<<Utils::detectEncode(tempdata);

//    QString strUnicode= gb2312Codec->toUnicode(tempdata.data());
//    qDebug()<<strUnicode;
//    QByteArray ByteUtf8= utf8Codec->fromUnicode(strUnicode);

//    char * strGb2312= ByteUtf8.data();
//    qDebug()<<strGb2312;

    if (receivedEntry->fullPath().isEmpty()) {
        qDebug() << "Weird, received empty entry (no filename) - skipping";
        return;
    }

    // If there are no columns registered, then populate columns from entry. If the first entry
    // is a directory we check again for the first file entry to ensure all relevent columms are shown.
    if (m_showColumns.isEmpty() || !m_fileEntryListed) {
//        QList<int> toInsert;

//        const auto size = receivedEntry->property("size").toULongLong();
//        const auto compressedSize = receivedEntry->property("compressedSize").toULongLong();
//        for (auto i = m_propertiesMap.begin(); i != m_propertiesMap.end(); ++i) {
//            // Singlefile plugin doesn't report the uncompressed size.
//            if (i.key() == Size && size == 0 && compressedSize > 0) {
//                continue;
//            }
//            if (!receivedEntry->property(i.value().constData()).toString().isEmpty()) {
//                if (i.key() != CompressedSize || receivedEntry->compressedSizeIsSet) {
//                    if (!m_showColumns.contains(i.key())) {
//                        toInsert << i.key();
//                    }
//                }
//            }
//        }

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
    // Archive::Entry *existing = m_rootEntry->findByPath(entryFileName.split(QLatin1Char('/')));
    // if (existing) {
    //     existing->setProperty("fullPath", entryFileName);
    //     // Multi-volume files are repeated at least in RAR archives.
    //     // In that case, we need to sum the compressed size for each volume
    //     qulonglong currentCompressedSize = existing->property("compressedSize").toULongLong();
    //     existing->setProperty("compressedSize", currentCompressedSize + receivedEntry->property("compressedSize").toULongLong());
    //     return;
    // }

    // Find parent entry, creating missing directory Archive::Entry's in the process.
    Archive::Entry *parent = parentFor(receivedEntry, behaviour);

    // Create an Archive::Entry.
    const QStringList path = entryFileName.split(QLatin1Char('/'), QString::SkipEmptyParts);
    Archive::Entry *entry = parent->find(path.last());
    if (entry) {
        entry->copyMetaData(receivedEntry);
        entry->setCompressIndex(receivedEntry->compressIndex());
        entry->setProperty("fullPath", entryFileName);
        /*      if (!entry->isDir()) {
                  insertEntry(entry, behaviour);
              } */
    } else {
        receivedEntry->setParent(parent);
        insertEntry(receivedEntry, behaviour);
    }
}

void ArchiveModel::slotLoadingFinished(KJob *job)
{
    std::sort(m_showColumns.begin(), m_showColumns.end());

    if (!job->error()) {

        qDebug() << "Showing columns: " << m_showColumns;

        m_archive.reset(qobject_cast<LoadJob *>(job)->archive());

        beginResetModel();
        endResetModel();
    }

    emit loadingFinished(job);

    if (m_showColumns.size() > 0) {
        m_tableview->sortByColumn(0, Qt::AscendingOrder);
    }

    //refresh entry file count
    for (int i = 0 ; i < rowCount(); ++i) {
        QModelIndex  mIndex = index(i, 0);
        Archive::Entry *pEntry = entryForIndex(mIndex);
        if (pEntry && pEntry->isDir()) {
            qlonglong sizeVal = pEntry->property("size").value<qulonglong>();
            if (!sizeVal) {
                if (ReadOnlyArchiveInterface *pInterface = getPlugin()) {
                    pInterface->RefreshEntryFileCount(pEntry);
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
    if (behaviour == NotifyViews) {
        beginInsertRows(indexForEntry(parent), parent->entries().count(), parent->entries().count());
    }
    parent->appendEntry(entry);
    if (behaviour == NotifyViews) {
        endInsertRows();
    }

    // Save an icon for each newly added entry.
    QMimeDatabase db;
    QIcon icon;
    entry->isDir()
    ? icon = QIcon::fromTheme(db.mimeTypeForName(QStringLiteral("inode/directory")).iconName()).pixmap(24, 24)
             : icon = QIcon::fromTheme(db.mimeTypeForFile(entry->fullPath()).iconName()).pixmap(24, 24);
    if (icon.isNull()) {
        icon = QIcon::fromTheme("empty").pixmap(24, 24);
    }

//    qDebug()<<icon;
    m_entryIcons.insert(entry->fullPath(NoTrailingSlash), icon);
}

Archive *ArchiveModel::archive() const
{
    return m_archive.data();
}

void ArchiveModel::reset()
{
    m_archive.reset(nullptr);
    s_previousMatch = nullptr;
    s_previousPieces->clear();
    initRootEntry();

    // TODO: make sure if it's ok to not have calls to beginRemoveColumns here
    m_showColumns.clear();
    beginResetModel();
    endResetModel();
}

void ArchiveModel::createEmptyArchive(const QString &path, const QString &mimeType, QObject *parent)
{
    reset();
    m_archive.reset(Archive::createEmpty(path, mimeType, parent));
}

KJob *ArchiveModel::loadArchive(const QString &path, const QString &mimeType, QObject *parent)
{
    reset();

    auto loadJob = Archive::load(path, mimeType, parent);
    connect(loadJob, &KJob::result, this, &ArchiveModel::slotLoadingFinished);
    connect(loadJob, &Job::newEntry, this, &ArchiveModel::slotListEntry);
    connect(loadJob, &Job::userQuery, this, &ArchiveModel::slotUserQuery);

    setPlugin(loadJob->archiveInterface());
    connect(loadJob->archiveInterface(), &ReadOnlyArchiveInterface::entry, this, &ArchiveModel::slotListEntry);

    emit loadingStarted();

    return loadJob;
}

ExtractJob *ArchiveModel::extractFile(Archive::Entry *file, const QString &destinationDir, const ExtractionOptions &options) const
{
    QVector<Archive::Entry *> files({file});
    return extractFiles(files, destinationDir, options);
}

ExtractJob *ArchiveModel::extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDir, const ExtractionOptions &options) const
{
    Q_ASSERT(m_archive);
    QString psd = m_archive->password();
    if (m_archive->encryptionType() == Archive::Unencrypted) { //没有加密的

    } else {
        //是否启用头部加密,如果启用头部加密，当前用户肯定已经输入正确密码；所以要记录密码，并且将加密状态设置为Archive::Encrypted
        //如果不是头部加密，那就是文件加密了，所以需要将密码设置空字符串，同样加密状态设置为Archive::Encrypted.
        bool headerEncrypted = m_archive->encryptionType() == Archive::HeaderEncrypted;
        psd = headerEncrypted ? psd : "";
        m_archive->encrypt(psd, headerEncrypted);
    }

    ExtractJob *newJob = m_archive->extractFiles(files, destinationDir, options);
    connect(newJob, &ExtractJob::userQuery, this, &ArchiveModel::slotUserQuery);
    return newJob;
}

PreviewJob *ArchiveModel::preview(Archive::Entry *file) const
{
    Q_ASSERT(m_archive);
    PreviewJob *job = m_archive->preview(file);
    connect(job, &Job::userQuery, this, &ArchiveModel::slotUserQuery);
    return job;
}

OpenJob *ArchiveModel::open(Archive::Entry *file) const
{
    Q_ASSERT(m_archive);
    OpenJob *job = m_archive->open(file);
    connect(job, &Job::userQuery, this, &ArchiveModel::slotUserQuery);
    return job;
}

OpenWithJob *ArchiveModel::openWith(Archive::Entry *file) const
{
    Q_ASSERT(m_archive);
    OpenWithJob *job = m_archive->openWith(file);
    connect(job, &Job::userQuery, this, &ArchiveModel::slotUserQuery);
    return job;
}

AddJob *ArchiveModel::addFiles(QVector<Archive::Entry *> &entries, const Archive::Entry *destination, const CompressionOptions &options)
{
    if (!m_archive) {
        return nullptr;
    }

    if (!m_archive->isReadOnly()) {
        AddJob *job = m_archive->addFiles(entries, destination, options);
        connect(job, &AddJob::newEntry, this, &ArchiveModel::slotNewEntry);
        connect(job, &AddJob::userQuery, this, &ArchiveModel::slotUserQuery);

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
        MoveJob *job = m_archive->moveFiles(entries, destination, options);
        connect(job, &MoveJob::newEntry, this, &ArchiveModel::slotNewEntry);
        connect(job, &MoveJob::userQuery, this, &ArchiveModel::slotUserQuery);
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
        CopyJob *job = m_archive->copyFiles(entries, destination, options);
        connect(job, &CopyJob::newEntry, this, &ArchiveModel::slotNewEntry);
        connect(job, &CopyJob::userQuery, this, &ArchiveModel::slotUserQuery);

        return job;
    }

    return nullptr;
}

DeleteJob *ArchiveModel::deleteFiles(QVector<Archive::Entry *> entries)
{
    Q_ASSERT(m_archive);
    if (!m_archive->isReadOnly()) {
        DeleteJob *job = m_archive->deleteFiles(entries);
        connect(job, &DeleteJob::entryRemoved, this, &ArchiveModel::slotEntryRemoved);

        connect(job, &DeleteJob::finished, this, &ArchiveModel::slotCleanupEmptyDirs);

        connect(job, &DeleteJob::userQuery, this, &ArchiveModel::slotUserQuery);
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

bool ArchiveModel::conflictingEntries(QList<const Archive::Entry *> &conflictingEntries, const QStringList &entries, bool allowMerging) const
{
    bool error = false;

    // We can't accept destination as an argument, because it can be a new entry path for renaming.
    const Archive::Entry *destination;
    {
        QStringList destinationParts = entries.first().split(QLatin1Char('/'), QString::SkipEmptyParts);
        destinationParts.removeLast();
        if (destinationParts.count() > 0) {
            destination = m_rootEntry->findByPath(destinationParts);
        } else {
            destination = m_rootEntry.data();
        }
    }

    const Archive::Entry *lastDirEntry = destination;
    QString skippedDirPath;

    for (const QString &entry : entries) {
        if (skippedDirPath.count() > 0 && entry.startsWith(skippedDirPath)) {
            continue;
        } else {
            skippedDirPath.clear();
        }

        while (!entry.startsWith(lastDirEntry->fullPath())) {
            lastDirEntry = lastDirEntry->getParent();
        }

        bool isDir = entry.right(1) == QLatin1String("/");
        const Archive::Entry *archiveEntry = lastDirEntry->find(entry.split(QLatin1Char('/'), QString::SkipEmptyParts).last());

        if (archiveEntry != nullptr) {
            if (archiveEntry->isDir() != isDir || !allowMerging) {
                if (isDir) {
                    skippedDirPath = lastDirEntry->fullPath();
                }

                if (!error) {
                    conflictingEntries.clear();
                    error = true;
                }
                conflictingEntries << archiveEntry;
            } else {
                if (isDir) {
                    lastDirEntry = archiveEntry;
                } else if (!error) {
                    conflictingEntries << archiveEntry;
                }
            }
        } else if (isDir) {
            skippedDirPath = entry;
        }
    }

    return error;
}

bool ArchiveModel::hasDuplicatedEntries(const QStringList &entries)
{
    QStringList tempList;
    for (const QString &entry : entries) {
        if (tempList.contains(entry)) {
            return true;
        }

        tempList << entry;
    }

    return false;
}

QMap<QString, Archive::Entry *> ArchiveModel::entryMap(const QVector<Archive::Entry *> &entries)
{
    QMap<QString, Archive::Entry *> map;
    for (Archive::Entry *entry : entries) {
        map.insert(entry->fullPath(), entry);
    }

    return map;
}

const QHash<QString, QIcon> ArchiveModel::entryIcons() const
{
    return m_entryIcons;
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

void ArchiveModel::countEntriesAndSize()
{
    // This function is used to count the number of folders/files and
    // the total compressed size. This is needed for PropertiesDialog
    // to update the corresponding values after adding/deleting files.

    // When ArchiveModel has been properly fixed, this code can likely
    // be removed.

    m_numberOfFiles = 0;
    m_numberOfFolders = 0;
    m_uncompressedSize = 0;

    QElapsedTimer timer;
    timer.start();

    traverseAndCountDirNode(m_rootEntry.data());

    qDebug() << "Time to count entries and size:" << timer.elapsed() << "ms";
}

void ArchiveModel::traverseAndCountDirNode(Archive::Entry *dir)
{
    const auto entries = dir->entries();
    for (Archive::Entry *entry : entries) {
        if (entry->isDir()) {
            traverseAndCountDirNode(entry);
            m_numberOfFolders++;
        } else {
            m_numberOfFiles++;
            m_uncompressedSize += entry->property("size").toULongLong();
        }
    }
}

void ArchiveModel::setPlugin(ReadOnlyArchiveInterface *interface)
{
    m_plugin = interface;
}

qulonglong ArchiveModel::numberOfFiles() const
{
    return m_numberOfFiles;
}

qulonglong ArchiveModel::numberOfFolders() const
{
    return m_numberOfFolders;
}

qulonglong ArchiveModel::uncompressedSize() const
{
    return m_uncompressedSize;
}

QList<int> ArchiveModel::shownColumns() const
{
    return m_showColumns;
}

QMap<int, QByteArray> ArchiveModel::propertiesMap() const
{
    return m_propertiesMap;
}
