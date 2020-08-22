#include "readwritelibarchiveplugin.h"

//#include <KLocalizedString>
//#include "kpluginfactory.h"

#include <QDirIterator>
#include <QSaveFile>
#include <QThread>

#include <archive_entry.h>

// 300M
#define MB300 314572800 /*(300*1024*1024)*/

//K_PLUGIN_CLASS_WITH_JSON(ReadWriteLibarchivePlugin, "kerfuffle_libarchive.json")

ReadWriteLibarchivePluginFactory::ReadWriteLibarchivePluginFactory()
{
    registerPlugin<ReadWriteLibarchivePlugin>();
}
ReadWriteLibarchivePluginFactory::~ReadWriteLibarchivePluginFactory()
{

}

ReadWriteLibarchivePlugin::ReadWriteLibarchivePlugin(QObject *parent, const QVariantList &args)
    : LibarchivePlugin(parent, args)
{
    mType = ENUM_PLUGINTYPE::PLUGIN_READWRITE_LIBARCHIVE;
}

ReadWriteLibarchivePlugin::~ReadWriteLibarchivePlugin()
{
}

bool ReadWriteLibarchivePlugin::addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd)
{
    const bool creatingNewFile = !QFileInfo::exists(filename());
    if (destination == nullptr) {
        m_numberOfEntries = 0;
    } else {
        qint64 count = 0;
        Archive::Entry *pParentEntry = destination->getParent();
        if (pParentEntry == nullptr) {
            destination->calEntriesCount(count);
            m_numberOfEntries = (uint)count;//
        } else {
            while (pParentEntry->getParent() != nullptr) {
                pParentEntry = pParentEntry->getParent();
            }
            pParentEntry->calEntriesCount(count);
            m_numberOfEntries = (uint)count;//
        }
    }

    //          总数量     = 原有的归档数量      + 新添加的文件数量
    const uint totalCount = m_numberOfEntries + numberOfEntriesToAdd;

    m_writtenFiles.clear();

    if (!creatingNewFile && !initializeReader()) {
        return false;
    }

    if (!initializeWriter(creatingNewFile, options)) {
        return false;
    }

    // First write the new files.
    uint addedEntries = 0;
    qint64 sizeOfAdd = 0;
    files[0]->calAllSize(sizeOfAdd);
    bool bInternalDuty = false;
    if (sizeOfAdd > MB300 && files[0]->isDir() == false) {//如果大于300M
//        bool bInternalDuty  = totalCount < 6;//如果总文件数量小于6个，那么启动细分进度
        bInternalDuty = true;
    }

    // Recreate destination directory structure.
    QString destinationPath = (destination == nullptr) ? QString() : destination->fullPath();

    for (Archive::Entry *selectedFile : files) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        emit progress_filename(selectedFile->fullPath());

        FileProgressInfo info;

        if (bInternalDuty) {
            info.fileProgressStart = static_cast<float>(addedEntries) / (static_cast<float>(totalCount));//记录当前进度值
            info.fileProgressProportion = (float)1.0 / totalCount;//设定内度百分比范围,1表示对当前这一个压缩包进行内部进度细分分析
        }

        if (!writeFileFromEntry(selectedFile->fullPath(), destinationPath, selectedFile, info, bInternalDuty)) {
            finish(false);
            return false;
        }

        addedEntries++;
        emit progress(float(addedEntries) / float(totalCount));
        //qDebug() << "front========" << "addedEntries:" << addedEntries << "totalCount:" << totalCount;
        // For directories, write all subfiles/folders.
        const QString &fullPath = selectedFile->fullPath();
        if (QFileInfo(fullPath).isDir()) {
            QDirIterator it(fullPath,
                            QDir::AllEntries | QDir::Readable |
                            QDir::Hidden | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);
            QString firstDirFullPath = fullPath;
            QString externalPath = QFileInfo(fullPath).path() + QDir::separator();

            while (!QThread::currentThread()->isInterruptionRequested() && it.hasNext()) {
                QString path = it.next();

                emit progress_filename(it.filePath());

                if ((it.fileName() == QLatin1String("..")) ||
                        (it.fileName() == QLatin1String("."))) {
                    continue;
                }

                bInternalDuty = false;
                FileProgressInfo info;
                if (it.fileInfo().size() > MB300 && it.fileInfo().isDir() == false) {//如果不是文件夹，且大小超过300M，则执行内部进度分析
                    bInternalDuty = true;
                }
                if (bInternalDuty) {
                    info.fileProgressStart = (float)addedEntries / totalCount; //记录当前进度值
                    info.fileProgressProportion = (float)1.0 / totalCount;//设定内部百分比范围；1表示对当前这一个压缩包进行内部进度细分分析
                }

                if (!writeFileTodestination(path, destinationPath, externalPath, info, bInternalDuty)) {
                    finish(false);
                    return false;
                }

                addedEntries++;

                if (bInternalDuty == false) {//如果不启动内部进度细分分析
                    double percent = float(addedEntries) / totalCount;
                    //qDebug() << "back=======percent:" << percent;
                    emit progress(percent);
                }
            }
            //qDebug() << "back========" << "addedEntries:" << addedEntries << "totalCount:" << totalCount;
        }

    }

    bool isSuccessful = true;
    // If we have old archive entries.
    if (!creatingNewFile) {
        m_filesPaths = m_writtenFiles;
        isSuccessful = processOldEntries_Add(addedEntries, Add, totalCount);
    }

    finish(isSuccessful);
    return isSuccessful;
}

bool ReadWriteLibarchivePlugin::moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options)
{
    Q_UNUSED(options);


    if (!initializeReader()) {
        return false;
    }

    if (!initializeWriter()) {
        return false;
    }

    // Copy old elements from previous archive to new archive.
    uint movedEntries = 0;
    m_filesPaths = entryFullPaths(files);
    m_entriesWithoutChildren = entriesWithoutChildren(files).count();
    m_destination = destination;
    const bool isSuccessful = processOldEntries(movedEntries, Move, m_numberOfEntries);
    if (isSuccessful) {
    } else {
    }

    finish(isSuccessful);
    return isSuccessful;
}

bool ReadWriteLibarchivePlugin::copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options)
{
    Q_UNUSED(options);


    if (!initializeReader()) {
        return false;
    }

    if (!initializeWriter()) {
        return false;
    }

    // Copy old elements from previous archive to new archive.
    uint copiedEntries = 0;
    m_filesPaths = entryFullPaths(files);
    m_destination = destination;
    const bool isSuccessful = processOldEntries(copiedEntries, Copy, m_numberOfEntries);
    if (isSuccessful) {
    } else {
    }

    finish(isSuccessful);
    return isSuccessful;
}

bool ReadWriteLibarchivePlugin::deleteFiles(const QVector<Archive::Entry *> &files)
{
    if (files.length() == 0) {
        return false;
    }
    if (!initializeReader()) {
        return false;
    }

    if (!initializeWriter()) {
        return false;
    }

    // Copy old elements from previous archive to new archive.
    uint deletedEntries = 0;
//    m_filesPaths = entryFullPaths(files);
    qint64 count = 0;
    //Archive::Entry *pFirstEntry = files[0];

    m_filesPaths.clear();
    foreach (Archive::Entry *entry, files) {
        QStringList listTempPath;
        entry->getAllNodesFullPath(listTempPath);
        m_filesPaths << listTempPath;

        qint64 tempCount = 0;
        Archive::Entry *pRootEntry = entry;
        while (pRootEntry->getParent() != nullptr) {
            pRootEntry = pRootEntry->getParent();
        }

        pRootEntry->calEntriesCount(tempCount);
        count += tempCount;
    }
    //pFirstEntry->getAllNodesFullPath(m_filesPaths);

    //    while (pFirstEntry->getParent() != nullptr) {
    //        pFirstEntry = pFirstEntry->getParent();
    //    }
    //    Archive::Entry *pRootEntry = pFirstEntry;
    //    pRootEntry->calEntriesCount(count);
    m_numberOfEntries = (uint)count - m_filesPaths.length();
//    const bool isSuccessful = processOldEntries(deletedEntries, Delete, m_numberOfEntries);
    const bool isSuccessful = deleteEntry(deletedEntries, m_numberOfEntries);
    //    if (isSuccessful) {
    //        emit entryRemoved(files[0]->fullPath());
    //    }

    finish(isSuccessful);
    return isSuccessful;
}

bool ReadWriteLibarchivePlugin::initializeWriter(const bool creatingNewFile, const CompressionOptions &options)
{
    m_tempFile.setFileName(filename());
    if (!m_tempFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        emit error(("Failed to create a temporary file for writing data."));
        return false;
    }

    m_archiveWriter.reset(archive_write_new());
    if (!(m_archiveWriter.data())) {
        emit error(("The archive writer could not be initialized."));
        return false;
    }

    QString mimeTypeName = mimetype().name();

    // pax_restricted is the libarchive default, let's go with that.
    if (mimeTypeName == "application/zip") {
        archive_write_set_format_zip(m_archiveWriter.data());
    } else {
        archive_write_set_format_pax_restricted(m_archiveWriter.data());
    }


    if (creatingNewFile) {
        if (!initializeNewFileWriterFilters(options)) {
            return false;
        }
    } else {
        if (!initializeWriterFilters()) {
            return false;
        }
    }

    if (archive_write_open_fd(m_archiveWriter.data(), m_tempFile.handle()) != ARCHIVE_OK) {
        emit error(("Could not open the archive for writing entries."));
        return false;
    }

    return true;
}

bool ReadWriteLibarchivePlugin::initializeWriterFilters()
{
    int ret;
    bool requiresExecutable = false;
    switch (archive_filter_code(m_archiveReader.data(), 0)) {
    case ARCHIVE_FILTER_GZIP:
        ret = archive_write_add_filter_gzip(m_archiveWriter.data());
        break;
    case ARCHIVE_FILTER_BZIP2:
        ret = archive_write_add_filter_bzip2(m_archiveWriter.data());
        break;
    case ARCHIVE_FILTER_XZ:
        ret = archive_write_add_filter_xz(m_archiveWriter.data());
        break;
    case ARCHIVE_FILTER_LZMA:
        ret = archive_write_add_filter_lzma(m_archiveWriter.data());
        break;
    case ARCHIVE_FILTER_COMPRESS:
        ret = archive_write_add_filter_compress(m_archiveWriter.data());
        break;
    case ARCHIVE_FILTER_LZIP:
        ret = archive_write_add_filter_lzip(m_archiveWriter.data());
        break;
    case ARCHIVE_FILTER_LZOP:
        ret = archive_write_add_filter_lzop(m_archiveWriter.data());
        requiresExecutable = true;   //add
        break;
    case ARCHIVE_FILTER_LRZIP:
        ret = archive_write_add_filter_lrzip(m_archiveWriter.data());
        requiresExecutable = true;
        break;
    case ARCHIVE_FILTER_LZ4:
        ret = archive_write_add_filter_lz4(m_archiveWriter.data());
        break;
#ifdef HAVE_ZSTD_SUPPORT
    case ARCHIVE_FILTER_ZSTD:
        ret = archive_write_add_filter_zstd(m_archiveWriter.data());
        break;
#endif
    case ARCHIVE_FILTER_NONE:
        ret = archive_write_add_filter_none(m_archiveWriter.data());
        break;
    default:
        return false;
    }

    // Libarchive emits a warning for lrzip due to using external executable.
    if ((requiresExecutable && ret != ARCHIVE_WARN) ||
            (!requiresExecutable && ret != ARCHIVE_OK)) {
        emit error(("Could not set the compression method."));
        return false;
    }

    return true;
}

bool ReadWriteLibarchivePlugin::initializeNewFileWriterFilters(const CompressionOptions &options)
{
    int ret = ARCHIVE_OK;
    bool requiresExecutable = false;
    if (filename().right(2).toUpper() == QLatin1String("GZ")) {
        ret = archive_write_add_filter_gzip(m_archiveWriter.data());
    } else if (filename().right(3).toUpper() == QLatin1String("BZ2")) {
        ret = archive_write_add_filter_bzip2(m_archiveWriter.data());
    } else if (filename().right(2).toUpper() == QLatin1String("XZ")) {
        ret = archive_write_add_filter_xz(m_archiveWriter.data());
    } else if (filename().right(4).toUpper() == QLatin1String("LZMA")) {
        ret = archive_write_add_filter_lzma(m_archiveWriter.data());
    } else if (filename().right(2).toUpper() == QLatin1String(".Z")) {
        ret = archive_write_add_filter_compress(m_archiveWriter.data());
    } else if (filename().right(2).toUpper() == QLatin1String("LZ")) {
        ret = archive_write_add_filter_lzip(m_archiveWriter.data());
    } else if (filename().right(3).toUpper() == QLatin1String("LZO")) {
        ret = archive_write_add_filter_lzop(m_archiveWriter.data());
        requiresExecutable = true;
    } else if (filename().right(3).toUpper() == QLatin1String("LRZ")) {
        ret = archive_write_add_filter_lrzip(m_archiveWriter.data());
        requiresExecutable = true;
    } else if (filename().right(3).toUpper() == QLatin1String("LZ4")) {
        ret = archive_write_add_filter_lz4(m_archiveWriter.data());
    } else if (filename().right(3).toUpper() == QLatin1String("TAR")) {
        ret = archive_write_add_filter_none(m_archiveWriter.data());
    } else if (filename().right(3).toUpper() == QLatin1String("GZIP")) {
        ret = archive_write_add_filter_gzip(m_archiveWriter.data());
    }

    // Libarchive emits a warning for lrzip due to using external executable.
    if ((requiresExecutable && ret != ARCHIVE_WARN) ||
            (!requiresExecutable && ret != ARCHIVE_OK)) {
        emit error(("Could not set the compression method."));
        return false;
    }

    // Set compression level if passed in CompressionOptions.
    if (options.isCompressionLevelSet()) {
        if (filename().right(3).toUpper() == QLatin1String("ZIP")) {
            ret = archive_write_set_options(m_archiveWriter.data(), QString("compression-level=" + QString::number(options.compressionLevel())).toUtf8().constData());
        } else {
            ret = archive_write_set_filter_option(m_archiveWriter.data(), nullptr, "compression-level", QString::number(options.compressionLevel()).toUtf8().constData());
        }


        if (ret != ARCHIVE_OK) {
            emit error(("Could not set the compression level."));
            return false;
        }
    }

    if (false == password().isEmpty()) {
        archive_write_set_options(m_archiveWriter.data(), "encryption=aes256");
        archive_write_set_passphrase(m_archiveWriter.data(), password().toUtf8().constData());
    }


    return true;
}

void ReadWriteLibarchivePlugin::finish(const bool isSuccessful)
{
    if (!isSuccessful || QThread::currentThread()->isInterruptionRequested()) {
        archive_write_fail(m_archiveWriter.data());
        m_tempFile.cancelWriting();
    } else {
        // archive_write_close() needs to be called before calling QSaveFile::commit(),
        // otherwise the latter will close() the file descriptor m_archiveWriter is still working on.
        // TODO: We need to abstract this code better so that we only deal with one
        // object that manages both QSaveFile and ArchiveWriter.
        archive_write_close(m_archiveWriter.data());
        m_tempFile.commit();
    }
}

bool ReadWriteLibarchivePlugin::processOldEntries(uint &entriesCounter, OperationMode mode, uint totalCount)
{
    const uint newEntries = entriesCounter;
    entriesCounter = 0;
    uint iteratedEntries = 0;

    // Create a map that contains old path as key and new path as value.
    QMap<QString, QString> pathMap;
    if (mode == Move || mode == Copy) {
        m_filesPaths.sort();
        QStringList resultList = entryPathsFromDestination(m_filesPaths, m_destination, m_entriesWithoutChildren);
        const int listSize = m_filesPaths.count();
        Q_ASSERT(listSize == resultList.count());
        for (int i = 0; i < listSize; ++i) {
            pathMap.insert(m_filesPaths.at(i), resultList.at(i));
        }
    }

    struct archive_entry *entry;
    while (!QThread::currentThread()->isInterruptionRequested() && archive_read_next_header(m_archiveReader.data(), &entry) == ARCHIVE_OK) {
        const QString file = QFile::decodeName(archive_entry_pathname(entry));

        if (mode == Move || mode == Copy) {
            const QString newPathname = pathMap.value(file);
            if (!newPathname.isEmpty()) {
                if (mode == Copy) {
                    // Write the old entry.
                    if (!writeEntry(entry)) {
                        return false;
                    }
                } else {
                    emit entryRemoved(file);
                }

                entriesCounter++;
                iteratedEntries--;

                // Change entry path.
                archive_entry_set_pathname(entry, newPathname.toUtf8().constData());
                emitEntryFromArchiveEntry(entry);
            }
        } else if (m_filesPaths.contains(file)) {
            archive_read_data_skip(m_archiveReader.data());
            switch (mode) {
            case Delete:
                entriesCounter++;
                emit entryRemoved(file);
                emit progress(float(newEntries + entriesCounter + iteratedEntries) / float(totalCount));
                break;

            case Add:
                // When overwriting entries, we need to decrement the counter manually,
                // because entry was emitted.
                m_numberOfEntries--;
                break;

            default:
                Q_ASSERT(false);
            }
            continue;
        }

        // Write old entries.
        if (writeEntry(entry)) {
            if (mode == Add) {
                entriesCounter++;
            } else if (mode == Move || mode == Copy) {
                iteratedEntries++;
            } else if (mode == Delete) {
                iteratedEntries++;
            }
        } else {
            return false;
        }
        emit progress(float(newEntries + entriesCounter + iteratedEntries) / float(totalCount));
    }

    return !QThread::currentThread()->isInterruptionRequested();
}

bool ReadWriteLibarchivePlugin::processOldEntries_Add(uint &entriesCounter, ReadWriteArchiveInterface::OperationMode mode, uint totalCount)
{
    const uint newEntries = entriesCounter;
    entriesCounter = 0;

    // Create a map that contains old path as key and new path as value.
    QMap<QString, QString> pathMap;
    struct archive_entry *entry;
    while (!QThread::currentThread()->isInterruptionRequested() && archive_read_next_header(m_archiveReader.data(), &entry) == ARCHIVE_OK) {
        const QString file = QFile::decodeName(archive_entry_pathname(entry));
        emit progress_filename(file);
        if (m_filesPaths.contains(file)) {
            archive_read_data_skip(m_archiveReader.data());
            switch (mode) {
            case Add:
                // When overwriting entries, we need to decrement the counter manually,
                // because entry was emitted.
                m_numberOfEntries--;
                break;
            default:
                Q_ASSERT(false);
            }
            continue;
        }

        // Write old entries.
        FileProgressInfo info;
        float entrySize = archive_entry_size(entry);
        bool bInternalDuty = false;
        if (entrySize > MB300) { //如果大于300M
            bInternalDuty = true;
            info.fileProgressStart = (newEntries + entriesCounter) * 1.0 / (totalCount); //记录当前进度值
            info.fileProgressProportion = (float)1.0 / totalCount;//设定内度百分比范围,1表示对当前这一个压缩包进行内部进度细分分析
        }


//        if (writeEntry(entry)) {
        if (writeEntry_Add(entry, info, bInternalDuty)) {
            if (mode == Add) {
                entriesCounter++;
            }
        } else {
            return false;
        }

        if (bInternalDuty == false) {
            double percent = float(newEntries + entriesCounter) / totalCount;
            //qDebug() << "add========" << "newEntries:" << newEntries << ",entriesCounter:" << entriesCounter << ",totalCount:" << totalCount << ",percent:" << percent;
            emit progress(percent);
        }

    }

    return !QThread::currentThread()->isInterruptionRequested();
}

bool ReadWriteLibarchivePlugin::deleteEntry(uint &entriesCounter, uint totalCount)
{
    const uint newEntries = entriesCounter;
    // entriesCounter = 0;
    uint iteratedEntries = 0;

    // Create a map that contains old path as key and new path as value.
    QMap<QString, QString> pathMap;

    struct archive_entry *entry;
    int count = archive_filter_count(m_archiveReader.data());
    while (!QThread::currentThread()->isInterruptionRequested() && archive_read_next_header(m_archiveReader.data(), &entry) == ARCHIVE_OK) {
        const QString file = QFile::decodeName(archive_entry_pathname(entry));
        if (m_filesPaths.contains(file)) {
            archive_read_data_skip(m_archiveReader.data());
            //entriesCounter++;
            m_filesPaths.removeOne(file);
            emit entryRemoved(file);
        } else {
            // Write old entries.
            if (writeEntry(entry)) {
                iteratedEntries++;
                double percent = float(newEntries + /*entriesCounter + */iteratedEntries) / float(totalCount);
                //qDebug() << "==========deleteEntry:percent:" << percent;
                emit progress(percent);
            } else {
                return false;
            }
        }
    }

    return !QThread::currentThread()->isInterruptionRequested();
}

bool ReadWriteLibarchivePlugin::writeEntry(struct archive_entry *entry)
{
    const int returnCode = archive_write_header(m_archiveWriter.data(), entry);
    switch (returnCode) {
    case ARCHIVE_OK:
        // If the whole archive is extracted and the total filesize is
        // available, we use partial progress.
        copyDataFromSource(QLatin1String(archive_entry_pathname(entry)), m_archiveReader.data(), m_archiveWriter.data(), false);
        break;
    case ARCHIVE_FAILED:
    case ARCHIVE_FATAL:
        emit error(("Could not compress entry, operation aborted."));
        return false;
    default:
        break;
    }

    return true;
}

bool ReadWriteLibarchivePlugin::writeEntry_Add(archive_entry *entry, FileProgressInfo &info, bool bInternalDuty)
{
    const int returnCode = archive_write_header(m_archiveWriter.data(), entry);
    switch (returnCode) {
    case ARCHIVE_OK:
        // If the whole archive is extracted and the total filesize is
        // available, we use partial progress.
        copyDataFromSourceAdd(QLatin1String(archive_entry_pathname(entry)), m_archiveReader.data(), m_archiveWriter.data(), entry, info, bInternalDuty);
//        copyDataFromSource(QLatin1String(archive_entry_pathname(entry)), m_archiveReader.data(), m_archiveWriter.data(), false);
        break;
    case ARCHIVE_FAILED:
    case ARCHIVE_FATAL:
        emit error(("Could not compress entry, operation aborted."));
        return false;
    default:
        break;
    }

    return true;
}

bool ReadWriteLibarchivePlugin::writeFileFromEntry(const QString &relativeName, const QString destination, Archive::Entry *pEntry, const FileProgressInfo &info, bool bInternalDuty)
{
    //如果是文件夹，采用软链接的形式
    QString newFilePath = relativeName;
    QString absoluteDestinationPath = "";
    if (QFileInfo(relativeName).isDir()) {
        m_extractTempDir.reset(new QTemporaryDir());
        absoluteDestinationPath = m_extractTempDir->path() + QDir::separator() + destination;
        QDir dir;
        dir.mkpath(absoluteDestinationPath);//创建临时文件夹
        QString newFilePath = absoluteDestinationPath + pEntry->name();
        if (QFile::link(relativeName, newFilePath)) {
            qDebug() << "Symlink's created:" << destination << relativeName;
        } else {
            qDebug() << "Can't create symlink" << destination << relativeName;
            return false;
        }
    }

//    QFileInfo fileInfo(relativeName);
    QFileInfo fileInfo(newFilePath);
    const QString absoluteFilename = fileInfo.isSymLink() ? fileInfo.symLinkTarget() : fileInfo.absoluteFilePath();
    const QString destinationFilename = destination + fileInfo.fileName();

    // #253059: Even if we use archive_read_disk_entry_from_file,
    //          libarchive may have been compiled without HAVE_LSTAT,
    //          or something may have caused it to follow symlinks, in
    //          which case stat() will be called. To avoid this, we
    //          call lstat() ourselves.
    struct stat st;
    lstat(QFile::encodeName(absoluteFilename).constData(), &st); // krazy:exclude=syscalls

    struct archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, QFile::encodeName(destinationFilename).constData());
    archive_entry_copy_sourcepath(entry, QFile::encodeName(absoluteFilename).constData());
    archive_read_disk_entry_from_file(m_archiveReadDisk.data(), entry, -1, &st);

    const auto returnCode = archive_write_header(m_archiveWriter.data(), entry);
    if (returnCode == ARCHIVE_OK) {
        // If the whole archive is extracted and the total filesize is
        // available, we use partial progress.
        copyData(absoluteFilename, m_archiveWriter.data(), info, bInternalDuty);
        if (QFileInfo(relativeName).isDir()) {//clean temp path;
            QDir::cleanPath(absoluteDestinationPath);
        }
    } else {
        if (QFileInfo(relativeName).isDir()) {
            QDir::cleanPath(absoluteDestinationPath);
        }
        emit error(("Could not compress entry."));

        archive_entry_free(entry);

        return false;
    }

    if (QThread::currentThread()->isInterruptionRequested()) {
        archive_entry_free(entry);
        return false;
    }

    m_writtenFiles.push_back(destinationFilename);

//    emitEntryFromArchiveEntry(entry);//这句不需要添加，因为在MainWindow::addArchive函数中已经建立了Archive::Entry在ArchiveModel的树

    archive_entry_free(entry);

    return true;
}

bool ReadWriteLibarchivePlugin::writeFileTodestination(const QString &sourceFileFullPath, const QString &destination, const QString &externalPath,  const FileProgressInfo &info, bool partialprogress)
{
    //如果是文件夹，采用软链接的形式
    QString newFilePath = sourceFileFullPath;
    QFileInfo sourceFileInfo(sourceFileFullPath);
    QString absoluteDestinationPath = "";
    if (sourceFileInfo.isDir()) {
        m_extractTempDir.reset(new QTemporaryDir());
        absoluteDestinationPath = m_extractTempDir->path() + QDir::separator() + destination;
        QDir dir;
        dir.mkpath(absoluteDestinationPath);
        QString newFilePath = absoluteDestinationPath + sourceFileInfo.fileName();
        if (QFile::link(sourceFileFullPath, newFilePath)) {
            qDebug() << "Symlink's created:" << destination << sourceFileFullPath;
        } else {
            qDebug() << "Can't create symlink" << destination << sourceFileFullPath;
            return false;
        }
    }

//    QFileInfo fileInfo(relativeName);
    QFileInfo fileInfo(newFilePath);
    QString absoluteFilename = fileInfo.isSymLink() ? fileInfo.symLinkTarget() : fileInfo.absoluteFilePath();
    QString destinationFilename = absoluteFilename;
    destinationFilename = destination + destinationFilename.remove(externalPath);

    // #253059: Even if we use archive_read_disk_entry_from_file,
    //          libarchive may have been compiled without HAVE_LSTAT,
    //          or something may have caused it to follow symlinks, in
    //          which case stat() will be called. To avoid this, we
    //          call lstat() ourselves.
    struct stat st;
    lstat(QFile::encodeName(absoluteFilename).constData(), &st); // krazy:exclude=syscalls

    struct archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, QFile::encodeName(destinationFilename).constData());
    archive_entry_copy_sourcepath(entry, QFile::encodeName(absoluteFilename).constData());
    archive_read_disk_entry_from_file(m_archiveReadDisk.data(), entry, -1, &st);

    const auto returnCode = archive_write_header(m_archiveWriter.data(), entry);
    if (returnCode == ARCHIVE_OK) {
        // If the whole archive is extracted and the total filesize is
        // available, we use partial progress.
        copyData(absoluteFilename, m_archiveWriter.data(), info, partialprogress);
        if (sourceFileInfo.isDir()) {
            QDir::cleanPath(absoluteDestinationPath);
        }
    } else {

        emit error(("Could not compress entry."));

        archive_entry_free(entry);

        if (sourceFileInfo.isDir()) {
            QDir::cleanPath(absoluteDestinationPath);
        }
        return false;
    }

    if (QThread::currentThread()->isInterruptionRequested()) {
        archive_entry_free(entry);
        return false;
    }

    m_writtenFiles.push_back(destinationFilename);

//    emitEntryFromArchiveEntry(entry);//屏蔽by hsw 20200528

    archive_entry_free(entry);

    return true;
}

// TODO: if we merge this with copyData(), we can pass more data
//       such as an fd to archive_read_disk_entry_from_file()
bool ReadWriteLibarchivePlugin::writeFile(const QString &relativeName, const QString &destination, const FileProgressInfo &info, bool partialprogress)
{

//    const QString filePath = file->fullPath();
////            const QString newFilePath = absoluteDestinationPath + file->fullPath(NoTrailingSlash);
//    const QString newFilePath = absoluteDestinationPath + file->name();
//    if (QFile::link(filePath, newFilePath)) {
//        qDebug() << "Symlink's created:" << filePath << newFilePath;
//    } else {
//        qDebug() << "Can't create symlink" << filePath << newFilePath;
//        emit finished(false);
//        return false;
//    }
    m_extractTempDir.reset(new QTemporaryDir());
    const QString absoluteDestinationPath = m_extractTempDir->path() + QDir::separator() + destination;
    QDir dir;
    dir.mkpath(absoluteDestinationPath);

    if (QFile::link(destination, relativeName)) {
        qDebug() << "Symlink's created:" << destination << relativeName;
    } else {
        qDebug() << "Can't create symlink" << destination << relativeName;
        return false;
    }

    QFileInfo fileInfo(relativeName);
    const QString absoluteFilename = fileInfo.isSymLink() ? fileInfo.symLinkTarget() : fileInfo.absoluteFilePath();
    const QString destinationFilename = destination + relativeName;

    // #253059: Even if we use archive_read_disk_entry_from_file,
    //          libarchive may have been compiled without HAVE_LSTAT,
    //          or something may have caused it to follow symlinks, in
    //          which case stat() will be called. To avoid this, we
    //          call lstat() ourselves.
    struct stat st;
    lstat(QFile::encodeName(absoluteFilename).constData(), &st); // krazy:exclude=syscalls

    struct archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, QFile::encodeName(destinationFilename).constData());
    archive_entry_copy_sourcepath(entry, QFile::encodeName(absoluteFilename).constData());
    archive_read_disk_entry_from_file(m_archiveReadDisk.data(), entry, -1, &st);

    const auto returnCode = archive_write_header(m_archiveWriter.data(), entry);
    if (returnCode == ARCHIVE_OK) {
        // If the whole archive is extracted and the total filesize is
        // available, we use partial progress.
        copyData(absoluteFilename, m_archiveWriter.data(), info, partialprogress);
    } else {

        emit error(("Could not compress entry."));

        archive_entry_free(entry);

        return false;
    }

    if (QThread::currentThread()->isInterruptionRequested()) {
        archive_entry_free(entry);
        return false;
    }

    m_writtenFiles.push_back(destinationFilename);

    emitEntryFromArchiveEntry(entry);

    archive_entry_free(entry);

    return true;
}

//#include "readwritelibarchiveplugin.moc"
