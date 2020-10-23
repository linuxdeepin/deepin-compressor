/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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
#include "libarchiveplugin.h"
#include "queries.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QDebug>

#include <archive_entry.h>

LibarchivePlugin::LibarchivePlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{

//    connect(this, &ReadOnlyArchiveInterface::error, this, &LibarchivePlugin::slotRestoreWorkingDir);
//    connect(this, &ReadOnlyArchiveInterface::cancelled, this, &LibarchivePlugin::slotRestoreWorkingDir);
}

LibarchivePlugin::~LibarchivePlugin()
{
    deleteTempTarPkg(m_tars);
}

PluginFinishType LibarchivePlugin::list()
{
    qDebug() << "LibarchivePlugin插件加载压缩包数据";
    PluginFinishType eType;
    m_stArchiveData.reset();

    m_strOldArchiveName = m_strArchiveName; //保存原压缩包名
    QFileInfo fInfo(m_strArchiveName);
    QString fileName = fInfo.fileName();
    //因为tar.bz2、tar.lzma、tar.Z直接list时间较长，所以先用7z解压再list处理
    if (fileName.endsWith("tar.bz2") || fileName.endsWith("tar.lzma") || fileName.endsWith("tar.Z")) {
        QString tempFilePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QString tempFileName = tempFilePath + QDir::separator() + fileName.left(fileName.size() - fInfo.suffix().size() - 1);

        //QString commandLine = QString("%1%2%3%4").arg("7z x ").arg(filename()).arg(" -aoa -o").arg(tempFilePath);
        QStringList listArgs;
        listArgs << "x" << m_strArchiveName << "-aoa" << "-o" + tempFilePath;
        QString strProgram = QStandardPaths::findExecutable("7z");
        QProcess cmd;
        cmd.start(strProgram, listArgs);
        if (cmd.waitForFinished(-1)) {
            m_strArchiveName = tempFileName;
            if (!m_tars.contains(tempFileName)) {
                m_tars.push_back(tempFileName);
            }
            eType = list_New();
        } else {
            eType = PF_Error;
        }
    } else {
        eType = list_New();
    }
    //emit signalFinished(eType);
    return eType;
}

PluginFinishType LibarchivePlugin::testArchive()
{
    return PT_Nomral;
}

PluginFinishType LibarchivePlugin::extractFiles(const QVector<FileEntry> &files, const ExtractionOptions &options)
{
    if (!initializeReader()) {
        return PF_Error;
    }

    ArchiveWrite writer(archive_write_disk_new());
    if (!writer.data()) {
        return PF_Error;
    }

    // 选择要保留的属性
    archive_write_disk_set_options(writer.data(), extractionFlags());

    int totalEntriesCount = 0;
    const bool extractAll = files.isEmpty();//如果是双击解压，则为false;如果是按钮解压，则为true
    if (extractAll) {  //全部解压
        if (!m_ArchiveEntryCount) {
//            emit progress(0);
//            //TODO: once information progress has been implemented, send
//            //feedback here that the archive is being read
//            m_emitNoEntries = true;
//            list();
//            m_emitNoEntries = false;
        }
        totalEntriesCount = m_ArchiveEntryCount;
    } else { //部分解压、提取 todo
//        totalEntriesCount = files.size();
//        this->m_pProgressInfo->resetProgress();
//        this->m_pProgressInfo->setTotalSize(files[0]->getSize());//双击解压，设置解压总大小
    }


//    this->extractPsdStatus = ReadOnlyArchiveInterface::Default;
//    ifReplaceTip = false; //默认没有替换提示

    //更改应用工作目录
    HandleWorkingDir handleWorkingDir(&m_oldWorkingDir);
    handleWorkingDir.change(options.strTargetPath);
    m_extractDestDir = options.strTargetPath;
    // Initialize variables.
//    const bool preservePaths = options.preservePaths(); //是否保留路径
//    const bool removeRootNode = options.isDragAndDropEnabled();

//    bool dontPromptErrors = false; // Whether to prompt for errors
//    m_currentExtractedFilesSize = 0;
    int extractedEntriesCount = 0; //记录已经解压的文件数量
    int progressEntryCount = 0;
    struct archive_entry *entry;
    QString fileBeingRenamed;
    QString extractDst;
    qlonglong qExtractSize = 0; //记录已经解压的文件大小

    // Iterate through all entries in archive.
    while (!QThread::currentThread()->isInterruptionRequested() && (archive_read_next_header(m_archiveReader.data(), &entry) == ARCHIVE_OK)) {
        // 空压缩包直接跳过
        if (!extractAll && 0 == m_ArchiveEntryCount) {
            break;
        }

        fileBeingRenamed.clear();
        int index = -1;

//        // Retry with renamed entry, fire an overwrite query again
//        // if the new entry also exists.
//    retry:
        const bool entryIsDir = S_ISDIR(archive_entry_mode(entry));
//        // Skip directories if not preserving paths.
//        // 如果不保留路径，直接跳过目录
//        if (!preservePaths && entryIsDir) { //preservePaths一直是true，只解压出文件，不要目录
//            archive_read_data_skip(m_archiveReader.data());
//            continue;
//        }

//        QString utf8path = m_common->trans2uft8(archive_entry_pathname(entry));
//        QString entryName = QDir::fromNativeSeparators(utf8path);
        QString entryName = m_common->trans2uft8(archive_entry_pathname(entry));

        // Some archive types e.g. AppImage prepend all entries with "./" so remove this part.
        // 移除"./"开头的，例如rpm包
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
        //目前，无法处理tar归档文件中的绝对文件名
        if (entryName.startsWith(QLatin1Char('/'))) {
            return PF_Error;
        }

        if (0 == extractedEntriesCount) {
            extractDst = entryName;
//            destDirName = entryName;
        } else if (extractDst.isEmpty() == false) {
            if (entryName.startsWith(extractDst + (extractDst.endsWith("/") ? "" : "/")) == false) {
                extractDst.clear();
            }
        }


//        // Should the entry be extracted?
//        if (extractAll || m_listFileName.contains(entryName) || entryName == fileBeingRenamed) {
//            // Find the index of entry.
//            if (entryName != fileBeingRenamed) {
//                index = m_listFileName.indexOf(entryName);
//            }
//            if (!extractAll && index == -1) {
//                // If entry is not found in files, skip entry.
//                continue;
//            }

        // entryFI is the fileinfo pointing to where the file will be
        // written from the archive.
        QFileInfo entryFI(entryName);
        emit signalCurFileName(entryName); // 发送当前正在解压的文件名

        const QString fileWithoutPath(entryFI.fileName());
//            // If we DON'T preserve paths, we cut the path and set the entryFI
//            // fileinfo to the one without the path.
//            if (!preservePaths) {
//                // Empty filenames (ie dirs) should have been skipped already,
//                // so asserting.
//                Q_ASSERT(!fileWithoutPath.isEmpty());
//                archive_entry_copy_pathname(entry, QFile::encodeName(fileWithoutPath).constData());
//                entryFI = QFileInfo(fileWithoutPath);

//                // OR, if the file has a rootNode attached, remove it from file path.
//            } else if (!extractAll && removeRootNode && entryName != fileBeingRenamed) { //提取
//                const QString &rootNode = m_strRootNode/*files.at(index)->rootNode*/;
//                if (!rootNode.isEmpty()) {
//                    const QString truncatedFilename(entryName.remove(entryName.indexOf(rootNode), rootNode.size()));
//                    archive_entry_copy_pathname(entry, QFile::encodeName(truncatedFilename).constData());
//                    entryFI = QFileInfo(truncatedFilename);
//                }
//            } else {
        archive_entry_copy_pathname(entry, entryName.toUtf8().constData());
//            }

//        }

        // Check if the file about to be written already exists.
        if (!entryIsDir && entryFI.exists()) {
            if (m_bSkipAll) {
                archive_read_data_skip(m_archiveReader.data());
                archive_entry_clear(entry);
                continue;
            } else if (!m_bOverwriteAll) {
                OverwriteQuery query(entryName);
                emit signalQuery(&query);
                query.waitForResponse();

                if (query.responseCancelled()) { //取消结束
                    emit signalCancel();
                    archive_read_data_skip(m_archiveReader.data());
                    archive_entry_clear(entry);
                    m_eErrorType = ET_UserCancelOpertion;
                    return PF_Cancel;
                } else if (query.responseSkip()) { //跳过
                    archive_read_data_skip(m_archiveReader.data());
                    archive_entry_clear(entry);
                    m_eErrorType = ET_NoError;
                    continue;
                } else if (query.responseSkipAll()) { //全部跳过
                    archive_read_data_skip(m_archiveReader.data());
                    archive_entry_clear(entry);
                    m_bSkipAll = true;
                    m_eErrorType = ET_NoError;
                    continue;
                } else if (query.responseOverwriteAll()) { //全部覆盖
                    m_bOverwriteAll = true;
                }
            }
        }

        // If there is an already existing directory.
        if (entryIsDir && entryFI.exists()) {
//            this->ifReplaceTip = true;
            if (entryFI.isWritable()) {
            } else {
                archive_entry_clear(entry);
                archive_read_data_skip(m_archiveReader.data());
                continue;
            }
        }

        // Write the entry header and check return value.
        const int returnCode = archive_write_header(writer.data(), entry);
        switch (returnCode) {
        case ARCHIVE_OK: {
            // If the whole archive is extracted and the total filesize is
            // available, we use partial progress.
            if (extractAll == false) {
//                copyDataFromSource_ArchiveEntry(files[0], m_archiveReader.data(), writer.data(), (m_extractedFilesSize));
            } else {
                copyDataFromSource(entryName, m_archiveReader.data(), writer.data(), (extractAll && options.qSize));
            }

//            // qDebug() <<  destinationDirectory + QDir::separator() + entryName;
//            // 文件权限设置
            QFileDevice::Permissions per = getPermissions(archive_entry_perm(entry));
            if (entryIsDir) {
                per |= QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
            }
            QFile::setPermissions(/*destinationDirectory + QDir::separator() + */entryName, per);
        }
        break;

        case ARCHIVE_FAILED:
            emit error(("Filed error, extraction aborted."));
            return PF_Error;
//            // If they user previously decided to ignore future errors,
//            // don't bother prompting again.
//            if (!dontPromptErrors) {
//                // Ask the user if he wants to continue extraction despite an error for this entry.
//                ContinueExtractionQuery query(QLatin1String(archive_error_string(writer.data())),
//                                              entryName);
//                emit userQuery(&query);
//                query.waitForResponse();

//                if (query.responseCancelled()) {
//                    emit cancelled();
//                    return false;
//                }
//                dontPromptErrors = query.dontAskAgain();
//            }
            break;

        case ARCHIVE_FATAL:
            emit error(("Fatal error, extraction aborted."));
            return PF_Error;
        default:
            break;
        }

        // If we only partially extract the archive and the number of
        // archive entries is available we use a simple progress based on
        // number of items extracted.
//        if (!extractAll && m_cachedArchiveEntryCount) {
//            ++progressEntryCount;
//            emit progress(static_cast<double>(progressEntryCount) / totalEntriesCount);
//            emit progress_filename(entryName);
//        }

        extractedEntriesCount++;
//        m_listFileName.removeOne(entryName);
//    } else {
//        // Archive entry not among selected files, skip it.
//        archive_read_data_skip(m_archiveReader.data());
//    }







    }

//    if (extractDst.isEmpty() == false) {
//        emit updateDestFileSignal(destinationDirectory + "/" + extractDst);
//    }

//    slotRestoreWorkingDir();
//return archive_read_close(m_archiveReader.data()) == ARCHIVE_OK;



    if (archive_read_close(m_archiveReader.data()) == ARCHIVE_OK) {
        return PT_Nomral;
    } else {
        return PF_Error;
    }



}

PluginFinishType LibarchivePlugin::addFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PF_Error;
}

PluginFinishType LibarchivePlugin::moveFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    return PF_Error;
}

PluginFinishType LibarchivePlugin::copyFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    return PF_Error;
}

PluginFinishType LibarchivePlugin::deleteFiles(const QVector<FileEntry> &files)
{
    return PF_Error;
}

PluginFinishType LibarchivePlugin::addComment(const QString &comment)
{
    return PF_Error;
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

    if (archive_read_open_filename(m_archiveReader.data(), QFile::encodeName(m_strArchiveName).constData(), 10240) != ARCHIVE_OK) {
        emit error(("Archive corrupted or insufficient permissions."));
        return false;
    }

    return true;
}

void LibarchivePlugin::copyDataFromSource(const QString &filename, struct archive *source, struct archive *dest,  bool partialprogress)
{
    char buff[10240]; //缓存大小
//    qlonglong size = 0;
    auto readBytes = archive_read_data(source, buff, sizeof(buff)); //读压缩包数据到buff
    while (readBytes > 0 && !QThread::currentThread()->isInterruptionRequested()) {
//        if (m_isPause) { //解压暂停
//            sleep(1);
//            //            qDebug() << "pause";
//            continue;
//        }

        archive_write_data(dest, buff, static_cast<size_t>(readBytes)); //写数据
        if (archive_errno(dest) != ARCHIVE_OK) {
            return;
        }

//        if (partialprogress) {
//            size += readBytes;
        // 解压百分比进度
        m_currentExtractedFilesSize += readBytes;
        emit signalprogress((double(readBytes + m_currentExtractedFilesSize)) / m_extractedFilesSize * 100);
//            emit progress(static_cast<double>(size + m_currentExtractedFilesSize) / m_extractedFilesSize);
//            emit progress_filename(filename);
//        }

        readBytes = archive_read_data(source, buff, sizeof(buff));
    }
//    if (partialprogress) {
//        m_currentExtractedFilesSize  += size;
//    }
}

PluginFinishType LibarchivePlugin::list_New()
{
    if (!initializeReader()) {
        return PF_Error;
    }

    QString compMethod = convertCompressionName(QString::fromUtf8(archive_filter_name(m_archiveReader.data(), 0)));
    if (!compMethod.isEmpty()) {
//        emit compressionMethodFound(compMethod);
    }

    m_ArchiveEntryCount = 0;
    m_extractedFilesSize = 0;
    m_numberOfEntries = 0;
    m_stArchiveData.qComressSize = QFileInfo(m_strArchiveName).size(); // 压缩包大小

    struct archive_entry *aentry;
    int result = ARCHIVE_RETRY;

    while (!QThread::currentThread()->isInterruptionRequested() && (result = archive_read_next_header(m_archiveReader.data(), &aentry)) == ARCHIVE_OK) {
//        if (!m_emitNoEntries) {
        emitEntryForIndex(aentry);
//        m_listIndex++;
//        }

        m_extractedFilesSize += static_cast<qlonglong>(archive_entry_size(aentry));

//        emit progress(static_cast<double>(archive_filter_bytes(m_archiveReader.data(), -1)) / compressedArchiveSize);

        m_ArchiveEntryCount++;

        //The call to `archive_read_data_skip()` here is not actually necessary, since libarchive will invoke it automatically
        //if you request the next header without reading the data for the last entry.
//        archive_read_data_skip(m_archiveReader.data());
    }

    return PT_Nomral;
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

void LibarchivePlugin::emitEntryForIndex(archive_entry *aentry)
{
    FileEntry m_archiveEntryStat; //压缩包内文件数据
    // 文件名
    m_archiveEntryStat.strFullPath = m_common->trans2uft8(archive_entry_pathname(aentry));

    // 文件名
    const QStringList pieces = m_archiveEntryStat.strFullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    m_archiveEntryStat.strFileName = pieces.last();

    // 是否为文件夹
    m_archiveEntryStat.isDirectory = S_ISDIR(archive_entry_mode(aentry));

    // 文件真实大小（文件夹显示项）
    if (m_archiveEntryStat.isDirectory) {
        m_archiveEntryStat.qSize = 0;
    } else {
        m_archiveEntryStat.qSize = static_cast<qlonglong>(archive_entry_size(aentry));
    }

    // 文件最后修改时间
    m_archiveEntryStat.uLastModifiedTime = static_cast<uint>(archive_entry_mtime(aentry));

    // 获取第一层数据
    if (!m_archiveEntryStat.strFullPath.contains(QDir::separator())
            || (m_archiveEntryStat.strFullPath.endsWith(QDir::separator()) && m_archiveEntryStat.strFullPath.count(QDir::separator()) == 1)) {
        m_stArchiveData.listRootEntry.push_back(m_archiveEntryStat);
    }
    // 压缩包原始大小
    m_stArchiveData.qSize += m_archiveEntryStat.qSize;
    // 构建压缩包数据map
    m_stArchiveData.mapFileEntry.insert(m_archiveEntryStat.strFullPath, m_archiveEntryStat);
}

void LibarchivePlugin::deleteTempTarPkg(const QStringList &tars)
{
    if (tars.size() > 0) {
        for (const QString &tar : tars) {
            QFile::remove(tar);
        }
    }
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


HandleWorkingDir::HandleWorkingDir(QString *oldWorkingDir)
    : m_oldWorkingDir(oldWorkingDir)
{

}

void HandleWorkingDir::change(const QString &newWorkingDir)
{
    *m_oldWorkingDir = QDir::currentPath();
    QDir::setCurrent(newWorkingDir);
}

HandleWorkingDir::~HandleWorkingDir()
{
    if (!m_oldWorkingDir->isEmpty() && QDir::setCurrent(*m_oldWorkingDir)) {
        m_oldWorkingDir->clear();
    }
}
