#include "libarchiveplugin.h"
#include "queries.h"
#include "common.h"
#include "structs.h"

#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KEncodingProber>

#include <archive_entry.h>
#include <kprocess.h>

LibarchivePlugin::LibarchivePlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
    , m_archiveReadDisk(archive_read_disk_new())
    , m_cachedArchiveEntryCount(0)
    , m_emitNoEntries(false)
    , m_extractedFilesSize(0)
{
    mType = ENUM_PLUGINTYPE::PLUGIN_LIBARCHIVE;
    m_common = new Common(this);
    archive_read_disk_set_standard_lookup(m_archiveReadDisk.data());

    connect(this, &ReadOnlyArchiveInterface::error, this, &LibarchivePlugin::slotRestoreWorkingDir);
    connect(this, &ReadOnlyArchiveInterface::cancelled, this, &LibarchivePlugin::slotRestoreWorkingDir);
}

LibarchivePlugin::~LibarchivePlugin()
{
    for (const auto e : qAsConst(m_emittedEntries)) {
        // Entries might be passed to pending slots, so we just schedule their deletion.
        e->deleteLater();
    }
    m_emittedEntries.clear();

    deleteTempTarPkg(m_tars);
}

bool LibarchivePlugin::list(bool /*isbatch*/)
{
    m_listIndex = 0;
    m_listMap.clear();

    strOldFileName = filename();

    QFileInfo fInfo(filename());
    QString fileName = fInfo.fileName();
    if (fileName.endsWith("tar.bz2") || fileName.endsWith("tar.lzma") || fileName.endsWith("tar.Z")) {
        QString fileName = fInfo.fileName();
        QString tempFilePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QString tempFileName = tempFilePath + QDir::separator() + fileName.left(fileName.size() - fInfo.suffix().size() - 1);

        //QString commandLine = QString("%1%2%3%4").arg("7z x ").arg(filename()).arg(" -aoa -o").arg(tempFilePath);
        QStringList listArgs;
        listArgs << "x" << filename() << "-aoa" << "-o" + tempFilePath;
        QString strProgram = QStandardPaths::findExecutable("7z");
        QProcess cmd;
        cmd.start(strProgram, listArgs);
        if (cmd.waitForFinished(-1)) {
            setFileName(tempFileName);
            if (!m_tars.contains(tempFileName)) {
                m_tars.push_back(tempFileName);
            }
            return list_New();
        }
        return false;
    } else {
        return list_New();
    }
}

bool LibarchivePlugin::addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd)
{
    Q_UNUSED(files)
    Q_UNUSED(destination)
    Q_UNUSED(options)
    Q_UNUSED(numberOfEntriesToAdd)
    return false;
}

bool LibarchivePlugin::moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(destination)
    Q_UNUSED(options)
    return false;
}

bool LibarchivePlugin::copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(destination)
    Q_UNUSED(options)
    return false;
}

bool LibarchivePlugin::deleteFiles(const QVector<Archive::Entry *> &files)
{
    Q_UNUSED(files)
    return false;
}

bool LibarchivePlugin::addComment(const QString &comment)
{
    Q_UNUSED(comment)
    return false;
}

bool LibarchivePlugin::testArchive()
{
    return false;
}

bool LibarchivePlugin::hasBatchExtractionProgress() const
{
    return true;
}

bool LibarchivePlugin::doKill()
{
    return false;
}

bool LibarchivePlugin::extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    emit sigExtractPwdCheckDown();
    calDecompressSize();

    if (!initializeReader()) {
        return false;
    }

    ArchiveWrite writer(archive_write_disk_new());
    if (!writer.data()) {
        return false;
    }

    archive_write_disk_set_options(writer.data(), extractionFlags());

    int totalEntriesCount = 0;
    const bool extractAll = files.isEmpty();//如果是双击解压，则为false;如果是按钮解压，则为true
    if (extractAll) {
        if (!m_cachedArchiveEntryCount) {
            emit progress(0);
            //TODO: once information progress has been implemented, send
            //feedback here that the archive is being read
            m_emitNoEntries = true;
            list();
            m_emitNoEntries = false;
        }
        totalEntriesCount = m_cachedArchiveEntryCount;
    } else {
        totalEntriesCount = files.size();
        this->m_pProgressInfo->resetProgress();
        this->m_pProgressInfo->setTotalSize(files[0]->getSize());//双击解压，设置解压总大小
    }

    this->extractPsdStatus = ReadOnlyArchiveInterface::Default;
    ifReplaceTip = false;
    m_oldWorkingDir = QDir::currentPath();
    QDir::setCurrent(destinationDirectory);
    m_extractDestDir = destinationDirectory;
    // Initialize variables.
    const bool preservePaths = options.preservePaths();
    const bool removeRootNode = options.isDragAndDropEnabled();
    bool overwriteAll = false; // Whether to overwrite all files
    bool skipAll = false; // Whether to skip all files
    bool dontPromptErrors = false; // Whether to prompt for errors
    m_currentExtractedFilesSize = 0;
    int extractedEntriesCount = 0;
    int progressEntryCount = 0;
    struct archive_entry *entry;
    QString fileBeingRenamed;
    // To avoid traversing the entire archive when extracting a limited set of
    // entries, we maintain a list of remaining entries and stop when it's empty.
//    const QStringList fullPaths = entryFullPaths(files);
//    QStringList remainingFiles = entryFullPaths(files);//获取双击的文件名称

    QString extractDst;

    // Iterate through all entries in archive.
    while (!QThread::currentThread()->isInterruptionRequested() && (archive_read_next_header(m_archiveReader.data(), &entry) == ARCHIVE_OK)) {
        if (!extractAll && m_listFileName.isEmpty()) {
            break;
        }

        fileBeingRenamed.clear();
        int index = -1;

        // Retry with renamed entry, fire an overwrite query again
        // if the new entry also exists.
    retry:
        const bool entryIsDir = S_ISDIR(archive_entry_mode(entry));
        // Skip directories if not preserving paths.
        if (!preservePaths && entryIsDir) {
            archive_read_data_skip(m_archiveReader.data());
            continue;
        }

        // entryName is the name inside the archive, full path
        //        QTextCodec *codec = QTextCodec::codecForName(detectEncode(archive_entry_pathname(entry)));
        //        QTextCodec *codecutf8 = QTextCodec::codecForName("utf-8");
        //        QString nameunicode = codec->toUnicode(archive_entry_pathname(entry));
        QString utf8path = m_common->trans2uft8(archive_entry_pathname(entry));
        QString entryName = QDir::fromNativeSeparators(utf8path);

        // Some archive types e.g. AppImage prepend all entries with "./" so remove this part.
        if (entryName.startsWith(QLatin1String("./"))) {
            entryName.remove(0, 2);
        }

        if (entryName.isEmpty()) {
            continue;
        }

        // Static libraries (*.a) contain the two entries "/" and "//".
        // We just skip these to allow extracting this archive type.
        if (entryName == QLatin1String("/") || entryName == QLatin1String("//")) {
            archive_read_data_skip(m_archiveReader.data());
            continue;
        }

        // For now we just can't handle absolute filenames in a tar archive.
        // TODO: find out what to do here!!
        if (entryName.startsWith(QLatin1Char('/'))) {
            return false;
        }

        if (0 == extractedEntriesCount) {
            extractDst = entryName;
            destDirName = entryName;
        } else if (extractDst.isEmpty() == false) {
            if (entryName.startsWith(extractDst + (extractDst.endsWith("/") ? "" : "/")) == false) {
                extractDst.clear();
            }
        }

        // Should the entry be extracted?
        if (extractAll || m_listFileName.contains(entryName) || entryName == fileBeingRenamed) {
            // Find the index of entry.
            if (entryName != fileBeingRenamed) {
                index = m_listFileName.indexOf(entryName);
            }
            if (!extractAll && index == -1) {
                // If entry is not found in files, skip entry.
                continue;
            }

            // entryFI is the fileinfo pointing to where the file will be
            // written from the archive.
            QFileInfo entryFI(entryName);

            const QString fileWithoutPath(entryFI.fileName());
            // If we DON'T preserve paths, we cut the path and set the entryFI
            // fileinfo to the one without the path.
            if (!preservePaths) {
                // Empty filenames (ie dirs) should have been skipped already,
                // so asserting.
                Q_ASSERT(!fileWithoutPath.isEmpty());
                archive_entry_copy_pathname(entry, QFile::encodeName(fileWithoutPath).constData());
                entryFI = QFileInfo(fileWithoutPath);

                // OR, if the file has a rootNode attached, remove it from file path.
            } else if (!extractAll && removeRootNode && entryName != fileBeingRenamed) {
                const QString &rootNode = m_strRootNode/*files.at(index)->rootNode*/;
                if (!rootNode.isEmpty()) {
                    const QString truncatedFilename(entryName.remove(entryName.indexOf(rootNode), rootNode.size()));
                    archive_entry_copy_pathname(entry, QFile::encodeName(truncatedFilename).constData());
                    entryFI = QFileInfo(truncatedFilename);
                }
            } else {
                archive_entry_copy_pathname(entry, entryName.toUtf8().constData());
            }

            // Check if the file about to be written already exists.
            if (!entryIsDir && entryFI.exists()) {
                if (skipAll) {
                    archive_read_data_skip(m_archiveReader.data());
                    archive_entry_clear(entry);
                    continue;
                } else if (!overwriteAll && !skipAll) {
                    OverwriteQuery query(entryName);
                    emit userQuery(&query);
                    query.waitForResponse();

                    if (query.responseCancelled()) {
                        emit cancelled();
                        archive_read_data_skip(m_archiveReader.data());
                        archive_entry_clear(entry);
                        break;
                    } else if (query.responseSkip()) {
                        archive_read_data_skip(m_archiveReader.data());
                        archive_entry_clear(entry);
                        continue;
                    } else if (query.responseAutoSkip()) {
                        archive_read_data_skip(m_archiveReader.data());
                        archive_entry_clear(entry);
                        skipAll = true;
                        continue;
                    } else if (query.responseRename()) {
                        const QString newName(query.newFilename());
                        fileBeingRenamed = newName;
                        archive_entry_copy_pathname(entry, QFile::encodeName(newName).constData());
                        goto retry;
                    } else if (query.responseOverwriteAll()) {
                        overwriteAll = true;
                    }
                }
            }

            // If there is an already existing directory.
            if (entryIsDir && entryFI.exists()) {
                this->ifReplaceTip = true;
                if (entryFI.isWritable()) {
                } else {
                    archive_entry_clear(entry);
                    archive_read_data_skip(m_archiveReader.data());
                    continue;
                }
            }
            //            archiveInterface()->extractPsdStatus = ReadOnlyArchiveInterface::ExtractPsdStatus::Canceled;
            //this->extractPsdStatus;
            // Write the entry header and check return value.
            const int returnCode = archive_write_header(writer.data(), entry);
            switch (returnCode) {
            case ARCHIVE_OK: {
                // If the whole archive is extracted and the total filesize is
                // available, we use partial progress.
                if (extractAll == false) {
                    copyDataFromSource_ArchiveEntry(files[0], m_archiveReader.data(), writer.data(), (m_extractedFilesSize));
                } else {
                    copyDataFromSource(entryName, m_archiveReader.data(), writer.data(), (extractAll && m_extractedFilesSize));
                }
                // qDebug() <<  destinationDirectory + QDir::separator() + entryName;
                QFileDevice::Permissions per = getPermissions(archive_entry_perm(entry));
                if (entryIsDir) {
                    per |= QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
                }
                QFile::setPermissions(destinationDirectory + QDir::separator() + entryName, per);
            }
            break;

            case ARCHIVE_FAILED:

                // If they user previously decided to ignore future errors,
                // don't bother prompting again.
                if (!dontPromptErrors) {
                    // Ask the user if he wants to continue extraction despite an error for this entry.
                    ContinueExtractionQuery query(QLatin1String(archive_error_string(writer.data())),
                                                  entryName);
                    emit userQuery(&query);
                    query.waitForResponse();

                    if (query.responseCancelled()) {
                        emit cancelled();
                        return false;
                    }
                    dontPromptErrors = query.dontAskAgain();
                }
                break;

            case ARCHIVE_FATAL:
                emit error(("Fatal error, extraction aborted."));
                return false;
            default:
                break;
            }

            // If we only partially extract the archive and the number of
            // archive entries is available we use a simple progress based on
            // number of items extracted.
            if (!extractAll && m_cachedArchiveEntryCount) {
                ++progressEntryCount;
                emit progress(static_cast<double>(progressEntryCount) / totalEntriesCount);
                emit progress_filename(entryName);
            }

            extractedEntriesCount++;
            m_listFileName.removeOne(entryName);
        } else {
            // Archive entry not among selected files, skip it.
            archive_read_data_skip(m_archiveReader.data());
        }
    }

    if (extractDst.isEmpty() == false) {
        emit updateDestFileSignal(destinationDirectory + "/" + extractDst);
    }

    slotRestoreWorkingDir();
    return archive_read_close(m_archiveReader.data()) == ARCHIVE_OK;
}

bool LibarchivePlugin::initializeReader()
{
    m_archiveReader.reset(archive_read_new());

    if (!(m_archiveReader.data())) {
        emit error(("The archive reader could not be initialized."));
        return false;
    }

    if (archive_read_support_filter_all(m_archiveReader.data()) != ARCHIVE_OK) {
        return false;
    }

    if (archive_read_support_format_all(m_archiveReader.data()) != ARCHIVE_OK) {
        return false;
    }

    if (archive_read_open_filename(m_archiveReader.data(), QFile::encodeName(filename()).constData(), 10240) != ARCHIVE_OK) {
        emit error(("Archive corrupted or insufficient permissions."));
        return false;
    }

    return true;
}

void LibarchivePlugin::setEntryData(/*const */archive_stat &aentry, qlonglong index, const QString &name, bool isMutilFolderFile)
{
    Q_UNUSED(index);
    Q_UNUSED(name);

    Archive::Entry *pCurEntry = new Archive::Entry(/*this*/);

    pCurEntry->setProperty("fullPath", aentry.archive_fullPath);
    pCurEntry->setProperty("owner", aentry.archive_owner);
    pCurEntry->setProperty("group", aentry.archive_group);
    if (!isMutilFolderFile) {
        pCurEntry->setProperty("size", aentry.archive_size);
    } else {
        pCurEntry->setProperty("size", 0);
    }

    pCurEntry->setProperty("isDirectory", aentry.archive_isDirectory);
    pCurEntry->setProperty("link", aentry.archive_link);
    pCurEntry->setProperty("timestamp", aentry.archive_timestamp);

    emit entry(pCurEntry);
    m_emittedEntries << pCurEntry;
}

Archive::Entry *LibarchivePlugin::setEntryDataA(/*const */archive_stat &aentry, qlonglong index, const QString &name)
{
    Q_UNUSED(index);
    Q_UNUSED(name);
    Archive::Entry *pCurEntry = new Archive::Entry(this);

    pCurEntry->setProperty("fullPath", aentry.archive_fullPath);
    pCurEntry->setProperty("owner", aentry.archive_owner);
    pCurEntry->setProperty("group", aentry.archive_group);
    pCurEntry->setProperty("size", aentry.archive_size);
    pCurEntry->setProperty("isDirectory", aentry.archive_isDirectory);
    pCurEntry->setProperty("link", aentry.archive_link);
    pCurEntry->setProperty("timestamp", aentry.archive_timestamp);

    return pCurEntry;
}

void LibarchivePlugin::setEntryVal(/*const */archive_stat &aentry, int &index, const QString &name, QString &dirRecord)
{
    Q_UNUSED(dirRecord);

    if ((name.endsWith("/") && name.count("/") == 1) || (name.count("/") == 0)) {
        setEntryData(aentry, index, name);
    }
    /*if (dirRecord.isEmpty()) {
        if (name.endsWith("/") && name.count("/") == 1) {
            setEntryData(aentry, index, name);
            m_SigDirRecord = name;
            ++index;
        } else  if (name.endsWith("/") && name.count("/") > 1) {
            if (!m_SigDirRecord.isEmpty() && name.left(m_SigDirRecord.size()) == m_SigDirRecord) {
                setEntryData(aentry, index, name);
                ++index;
                return;
            }

            //Create FileFolder
            QStringList fileDirs = name.split("/");
            QString folderAppendStr = "";
            for (int i = 0 ; i < fileDirs.size() - 1; ++i) {
                folderAppendStr += fileDirs[i] + "/";
                setEntryData(aentry, index, folderAppendStr);
                m_listMap.insert(folderAppendStr, qMakePair(aentry, -1));
            }

            ++index;
            m_DirRecord = name;
        } else if (name.count("/") == 0) {
            setEntryData(aentry, index, name);
            ++index;
        } else if (!name.endsWith("/") && name.count("/") >= 1) {
            if (!m_SigDirRecord.isEmpty() && (name.left(m_SigDirRecord.size()) == m_SigDirRecord)) {
                return;
            } else if (!m_DirRecord.isEmpty() && (name.left(m_DirRecord.size()) == m_DirRecord)) {
                return;
            }

            //Create FileFolder and file
            QStringList fileDirs = name.split("/");
            QString folderAppendStr = "";
            for (int i = 0 ; i <  fileDirs.size() ; ++i) {
                if (i < fileDirs.size() - 1) {
                    folderAppendStr.append(fileDirs[i]).append("/");
                    setEntryData(aentry, index, folderAppendStr, true);
                    m_listMap.insert(folderAppendStr, qMakePair(aentry, -1));
                } else {
                    folderAppendStr.append(fileDirs[i]);
                }
            }

            ++index;
        }
    } else {
        m_DirRecord = "";
        setEntryVal(aentry, index, name, m_DirRecord);
    }*/
}

/*void LibarchivePlugin::createEntry(const QString &externalPath, archive_entry *aentry)
{
    Archive::Entry *pCurEntry = new Archive::Entry();
//    QTextCodec *codec = QTextCodec::codecForName(detectEncode(archive_entry_pathname(aentry)));
//    QTextCodec *codecutf8 = QTextCodec::codecForName("utf-8");
//    QString nameunicode = codec->toUnicode(archive_entry_pathname(aentry));
    QString utf8path = trans2uft8(archive_entry_pathname(aentry));

    pCurEntry->setProperty("fullPath", QDir::fromNativeSeparators(utf8path));


    const QString owner = QString::fromLatin1(archive_entry_uname(aentry));
    if (!owner.isEmpty()) {
        pCurEntry->setProperty("owner", owner);
    }

    const QString group = QString::fromLatin1(archive_entry_gname(aentry));
    if (!group.isEmpty()) {
        pCurEntry->setProperty("group", group);
    }

    pCurEntry->compressedSizeIsSet = false;
    pCurEntry->setProperty("size", (qlonglong)archive_entry_size(aentry));
    pCurEntry->setProperty("isDirectory", S_ISDIR(archive_entry_mode(aentry)));

    if (archive_entry_symlink(aentry)) {
        pCurEntry->setProperty("link", QLatin1String(archive_entry_symlink(aentry)));
    }

    auto time = static_cast<uint>(archive_entry_mtime(aentry));
    pCurEntry->setProperty("timestamp", QDateTime::fromTime_t(time));

    if (pCurEntry->isDir()) {
        pCurEntry->setIsDirectory(true);
        QHash<QString, QIcon> *map = new QHash<QString, QIcon>();
        Archive::CreateEntry(pCurEntry->fullPath(), pCurEntry, externalPath, map);
//        m_model->appendEntryIcons(*map);
        delete map;
        map = nullptr;
    }

    emit entry(pCurEntry);
    m_emittedEntries << pCurEntry;
}*/

void LibarchivePlugin::emitEntryForIndex(archive_entry *aentry, qlonglong index)
{
//    archive_stat *entry = nullptr;
//    archive_stat entry;
    m_archiveEntryStat.archive_fullPath = m_common->trans2uft8(archive_entry_pathname(aentry));
    m_archiveEntryStat.archive_owner = QString::fromLatin1(archive_entry_uname(aentry));
    m_archiveEntryStat.archive_group = QString::fromLatin1(archive_entry_gname(aentry));
    if (archive_entry_symlink(aentry)) {
        m_archiveEntryStat.archive_link = QLatin1String(archive_entry_symlink(aentry));
    }

    m_archiveEntryStat.archive_timestamp = QDateTime::fromTime_t(static_cast<uint>(archive_entry_mtime(aentry)));
    m_archiveEntryStat.archive_size = static_cast<qlonglong>(archive_entry_size(aentry));
    m_archiveEntryStat.archive_isDirectory = S_ISDIR(archive_entry_mode(aentry));

    setEntryVal(m_archiveEntryStat, m_indexCount, m_archiveEntryStat.archive_fullPath, m_DirRecord);

    if (m_listMap.find(m_archiveEntryStat.archive_fullPath) == m_listMap.end()) {
        m_listMap.insert(m_archiveEntryStat.archive_fullPath, qMakePair(m_archiveEntryStat, index));
    }
}

qint64 LibarchivePlugin::extractSize(const QVector<Archive::Entry *> &files)
{
    m_listFileName.clear();

    qint64 qExtractSize = 0;
    for (Archive::Entry *e : files) {
        QString strPath = e->fullPath();
        m_strRootNode = e->rootNode;
        auto iter = m_listMap.find(strPath);
        for (; iter != m_listMap.end();) {
            if (!iter.key().startsWith(strPath)) {
                break;
            } else {
                if (!iter.key().endsWith("/")) {
                    qExtractSize += iter.value().first.archive_size;
                }

                m_listFileName << iter.value().first.archive_fullPath;
                ++iter;
                if (!strPath.endsWith(QDir::separator())) {
                    break;
                }
            }
        }
    }

//    std::sort(m_listExtractIndex.begin(), m_listExtractIndex.end());

    return qExtractSize;
}

void LibarchivePlugin::emitEntryFromArchiveEntry(struct archive_entry *aentry)
{
    Archive::Entry *pCurEntry = new Archive::Entry();
    //    QTextCodec *codec = QTextCodec::codecForName(detectEncode(archive_entry_pathname(aentry)));
    //    QTextCodec *codecutf8 = QTextCodec::codecForName("utf-8");
    //    QString nameunicode = codec->toUnicode(archive_entry_pathname(aentry));
    QString utf8path = m_common->trans2uft8(archive_entry_pathname(aentry));

    pCurEntry->setProperty("fullPath", QDir::fromNativeSeparators(utf8path));

    const QString owner = QString::fromLatin1(archive_entry_uname(aentry));
    if (!owner.isEmpty()) {
        pCurEntry->setProperty("owner", owner);
    }

    const QString group = QString::fromLatin1(archive_entry_gname(aentry));
    if (!group.isEmpty()) {
        pCurEntry->setProperty("group", group);
    }

    pCurEntry->compressedSizeIsSet = false;
    pCurEntry->setProperty("size", static_cast<qlonglong>(archive_entry_size(aentry)));
    pCurEntry->setProperty("isDirectory", S_ISDIR(archive_entry_mode(aentry)));

    if (archive_entry_symlink(aentry)) {
        pCurEntry->setProperty("link", QLatin1String(archive_entry_symlink(aentry)));
    }

    auto time = static_cast<uint>(archive_entry_mtime(aentry));
    pCurEntry->setProperty("timestamp", QDateTime::fromTime_t(time));

    emit entry(pCurEntry);
    m_emittedEntries << pCurEntry;
}

int LibarchivePlugin::extractionFlags() const
{
    int result = ARCHIVE_EXTRACT_TIME;
    result |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;

    // TODO: Don't use arksettings here
    /*if ( ArkSettings::preservePerms() )
    {
        result &= ARCHIVE_EXTRACT_PERM;
    }

    if ( !ArkSettings::extractOverwrite() )
    {
        result &= ARCHIVE_EXTRACT_NO_OVERWRITE;
    }*/

    return result;
}

void LibarchivePlugin::copyData(const QString &filename, struct archive *dest, const FileProgressInfo &info, bool bInternalDuty)
{
    m_currentExtractedFilesSize = 0;
    char buff[10240];
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    static int pastProgress = -1;

    pastProgress = -1;

    float fileSize = static_cast<float>(file.size());//filesize in the disk

    auto readBytes = file.read(buff, sizeof(buff));
    while (readBytes > 0 && !QThread::currentThread()->isInterruptionRequested()) {
        if (m_isPause) { //压缩暂停
            sleep(1);
            //            qDebug() << "pause";
            continue;
        }

        archive_write_data(dest, buff, static_cast<size_t>(readBytes));
        if (archive_errno(dest) != ARCHIVE_OK) {
            file.close();
            return;
        }

        if (bInternalDuty) {
            m_currentExtractedFilesSize += readBytes;
            float currentProgress = (static_cast<float>(m_currentExtractedFilesSize) / fileSize) * info.fileProgressProportion + info.fileProgressStart;//根据内容写入比例，加上上次的进度值
            if (static_cast<int>(100 * currentProgress) != pastProgress) {
                emit progress(static_cast<double>(currentProgress));
                pastProgress = static_cast<int>(100 * currentProgress);
            }
            //emit progress_filename(file.fileName());
        }

        readBytes = file.read(buff, sizeof(buff));
    }

    file.close();
}

void LibarchivePlugin::copyDataFromSource(const QString &filename, struct archive *source, struct archive *dest,  bool partialprogress)
{
    char buff[10240];
    qlonglong size = 0;
    auto readBytes = archive_read_data(source, buff, sizeof(buff));
    while (readBytes > 0 && !QThread::currentThread()->isInterruptionRequested()) {
        if (m_isPause) { //解压暂停
            sleep(1);
            //            qDebug() << "pause";
            continue;
        }

        archive_write_data(dest, buff, static_cast<size_t>(readBytes));
        if (archive_errno(dest) != ARCHIVE_OK) {
            return;
        }

        if (partialprogress) {
            size += readBytes;
            emit progress(static_cast<double>(size + m_currentExtractedFilesSize) / m_extractedFilesSize);
            emit progress_filename(filename);
        }

        readBytes = archive_read_data(source, buff, sizeof(buff));
    }
    if (partialprogress) {
        m_currentExtractedFilesSize  += size;
    }
}

void LibarchivePlugin::copyDataFromSource_ArchiveEntry(Archive::Entry *pSourceEntry, archive *source, archive *dest, bool bInternalDuty)
{
    QString fileName = pSourceEntry->name();
    qint64 fullSize = pSourceEntry->getSize();
    char buff[10240];
    qlonglong size = 0;
    auto readBytes = archive_read_data(source, buff, sizeof(buff));
    while (readBytes > 0 && !QThread::currentThread()->isInterruptionRequested()) {
        //         TODO:提取暂停，暂时不支持
        //        if (m_isPause) {
        //            sleep(1);
        //            qDebug() << "pause";
        //            continue;
        //        }

        archive_write_data(dest, buff, static_cast<size_t>(readBytes));
        if (archive_errno(dest) != ARCHIVE_OK) {
            return;
        }

        if (bInternalDuty) {
            size += readBytes;
            emit progress(size * 1.0 / fullSize);
            emit progress_filename(fileName);
        }

        readBytes = archive_read_data(source, buff, sizeof(buff));
    }
    if (bInternalDuty) {
        m_currentExtractedFilesSize  += size;
    }
}

void LibarchivePlugin::copyDataFromSourceAdd(const QString &/*filename*/, archive *source, archive *dest, archive_entry *sourceEntry, FileProgressInfo &info, bool bInternalDuty)
{
    char buff[10240];
    m_currentExtractedFilesSize = 0;
    float entrySize = archive_entry_size(sourceEntry);
    auto readBytes = archive_read_data(source, buff, sizeof(buff));
    while (readBytes > 0 && !QThread::currentThread()->isInterruptionRequested()) {
        archive_write_data(dest, buff, static_cast<size_t>(readBytes));
        if (archive_errno(dest) != ARCHIVE_OK) {
            return;
        }

        if (bInternalDuty) {
            m_currentExtractedFilesSize += readBytes;
            float currentProgress = (static_cast<float>(m_currentExtractedFilesSize) / entrySize) * info.fileProgressProportion + info.fileProgressStart;//根据内容写入比例，加上上次的进度值
            emit progress(static_cast<double>(currentProgress));
            //emit progress_filename(file.fileName());
        }
        readBytes = archive_read_data(source, buff, sizeof(buff));
    }

}

void LibarchivePlugin::slotRestoreWorkingDir()
{
    if (m_oldWorkingDir.isEmpty()) {
        return;
    }

    if (!QDir::setCurrent(m_oldWorkingDir)) {
    } else {
        m_oldWorkingDir.clear();
    }

    if (this->extractPsdStatus == ReadOnlyArchiveInterface::Canceled) {
        qDebug() << "=====点击了取消";
        if (this->ifReplaceTip == true) {
            return;
        }
        if (this->m_extractDestDir == "" || this->destDirName == "") {
            return;
        } else {
            QString fullPath = m_extractDestDir + "/" + destDirName;
            QFileInfo fileInfo(fullPath);
            if (fileInfo.exists()) {
                ReadWriteArchiveInterface::clearPath(fullPath);
            }
        }

    }
}

QString LibarchivePlugin::convertCompressionName(const QString &method)
{
    if (method == QLatin1String("gzip")) {
        return QStringLiteral("GZip");
    } else if (method == QLatin1String("bzip2")) {
        return QStringLiteral("BZip2");
    } else if (method == QLatin1String("xz")) {
        return QStringLiteral("XZ");
    } else if (method == QLatin1String("compress (.Z)")) {
        return QStringLiteral("Compress");
    } else if (method == QLatin1String("lrzip")) {
        return QStringLiteral("LRZip");
    } else if (method == QLatin1String("lzip")) {
        return QStringLiteral("LZip");
    } else if (method == QLatin1String("lz4")) {
        return QStringLiteral("LZ4");
    } else if (method == QLatin1String("lzop")) {
        return QStringLiteral("lzop");
    } else if (method == QLatin1String("lzma")) {
        return QStringLiteral("LZMA");
    } else if (method == QLatin1String("zstd")) {
        return QStringLiteral("Zstandard");
    }
    return QString();
}

bool LibarchivePlugin::list_New(bool /*isbatch*/)
{
    if (!initializeReader()) {
        return false;
    }

    QString compMethod = convertCompressionName(QString::fromUtf8(archive_filter_name(m_archiveReader.data(), 0)));
    if (!compMethod.isEmpty()) {
        emit compressionMethodFound(compMethod);
    }

    m_cachedArchiveEntryCount = 0;
    m_extractedFilesSize = 0;
    m_numberOfEntries = 0;
    auto compressedArchiveSize = QFileInfo(filename()).size();

    struct archive_entry *aentry;
    int result = ARCHIVE_RETRY;

    while (!QThread::currentThread()->isInterruptionRequested() && (result = archive_read_next_header(m_archiveReader.data(), &aentry)) == ARCHIVE_OK) {
        if (!m_emitNoEntries) {
            emitEntryForIndex(aentry, m_listIndex);
            m_listIndex++;
        }

        m_extractedFilesSize += static_cast<qlonglong>(archive_entry_size(aentry));

        emit progress(static_cast<double>(archive_filter_bytes(m_archiveReader.data(), -1)) / compressedArchiveSize);

        m_cachedArchiveEntryCount++;
        archive_read_data_skip(m_archiveReader.data());
    }

    if (result != ARCHIVE_EOF) {
        return false;
    }

    setFileName(strOldFileName);
    return archive_read_close(m_archiveReader.data()) == ARCHIVE_OK;
}

void LibarchivePlugin::deleteTempTarPkg(const QStringList &tars)
{
    if (tars.size() > 0) {
        Q_FOREACH (const QString &tar, tars) {
            QProcess p;
            QString command = "rm";
            QStringList args;
            args.append("-fr");
            args.append(tar);
            p.execute(command, args);
            p.waitForFinished(-1);
        }
    }
}

qlonglong LibarchivePlugin::calDecompressSize()
{
    m_cachedArchiveEntryCount = 0;
    m_extractedFilesSize = 0;

    if (!initializeReader()) {
        return m_extractedFilesSize;
    }

    struct archive_entry *aentry;

    while (!QThread::currentThread()->isInterruptionRequested() && (archive_read_next_header(m_archiveReader.data(), &aentry) == ARCHIVE_OK)) {

        m_extractedFilesSize += (qlonglong)archive_entry_size(aentry);
        m_cachedArchiveEntryCount++;
        archive_read_data_skip(m_archiveReader.data());
    }

    archive_read_close(m_archiveReader.data()) == ARCHIVE_OK;

    return m_extractedFilesSize;
}

void LibarchivePlugin::cleanIfCanceled()
{

}

void LibarchivePlugin::watchFileList(QStringList */*strList*/)
{

}

void LibarchivePlugin::showEntryListFirstLevel(const QString &directory)
{
    if (directory.isEmpty()) return;
    auto iter = m_listMap.find(directory);
    for (; iter != m_listMap.end() ;) {
        if (iter.key().left(directory.size()) != directory) {
            break;
        } else {
            QString chopStr = iter.key().right(iter.key().size() - directory.size());
            if (!chopStr.isEmpty()) {
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    Archive::Entry *fileEntry = setEntryDataA(iter.value().first, iter.value().second, iter.key());
                    RefreshEntryFileCount(fileEntry);
                    emit entry(fileEntry);
                    m_emittedEntries << fileEntry;
                }
            }

            ++iter;
        }
    }
}

void LibarchivePlugin::RefreshEntryFileCount(Archive::Entry *file)
{
    if (!file || !file->isDir()) return;
    qulonglong count = 0;
    auto iter = m_listMap.find(file->fullPath());
    for (; iter != m_listMap.end();) {
        if (!iter.key().startsWith(file->fullPath())) {
            break;
        } else {
            if (iter.key().size() > file->fullPath().size()) {
                QString chopStr = iter.key().right(iter.key().size() - file->fullPath().size());
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    ++count;
                }
            }

            ++iter;
        }

        file->setProperty("size", count);
    }
}
