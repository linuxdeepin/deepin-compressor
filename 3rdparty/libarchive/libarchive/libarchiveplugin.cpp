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
#include "datamanager.h"

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
    m_bHandleCurEntry = true; //提取使用选中文件
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
    DataManager::get_instance().archiveData().reset();

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
            eType = PFT_Error;
        }
    } else {
        eType = list_New();
    }
    //emit signalFinished(eType);
    return eType;
}

PluginFinishType LibarchivePlugin::testArchive()
{
    return PFT_Nomral;
}

PluginFinishType LibarchivePlugin::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    if (!initializeReader()) {
        return PFT_Error;
    }

    ArchiveWrite writer(archive_write_disk_new());
    if (!writer.data()) {
        return PFT_Error;
    }

    // 选择要保留的属性
    archive_write_disk_set_options(writer.data(), extractionFlags());

    bool isAtRootPath = false; //是否提取首层目录下的文件
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
//        totalEntriesCount = m_ArchiveEntryCount;
    } else { //部分解压、提取
        const QString &rootNode = options.strDestination;
        if (rootNode.isEmpty() || "/" == rootNode) {
            isAtRootPath = true;
        }
    }

    // 判断解压路径是否存在，不存在则创建文件夹
    if (QDir().exists(options.strTargetPath) == false) {
        if (!QDir().mkpath(options.strTargetPath)) {
            qDebug() << "Failed to create extractDestDir";
            return PFT_Error;
        }
    }

    // 更改应用工作目录，结束时自动恢复原来路径
    HandleWorkingDir handleWorkingDir(&m_oldWorkingDir);
    handleWorkingDir.change(options.strTargetPath);
    m_extractDestDir = options.strTargetPath;

    m_currentExtractedFilesSize = 0;
    int extractedEntriesCount = 0; //记录已经解压的文件数量

    struct archive_entry *entry = nullptr;

    QString extractDst;


    // Iterate through all entries in archive.
    while (!QThread::currentThread()->isInterruptionRequested() && (archive_read_next_header(m_archiveReader.data(), &entry) == ARCHIVE_OK)) {
        // 空压缩包直接跳过
        if (!extractAll && 0 == m_ArchiveEntryCount) {
            break;
        }

        const bool entryIsDir = S_ISDIR(archive_entry_mode(entry)); //该条entry是否是文件夹

        QByteArray strCode;
        QString entryName = m_common->trans2uft8(archive_entry_pathname(entry), strCode); //该条entry在压缩包内文件名(全路径)

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
        // 目前，无法处理tar归档文件中的绝对文件名
        if (entryName.startsWith(QLatin1Char('/'))) {
            return PFT_Error;
        }


        // Should the entry be extracted?
        // 解压:不需要过滤
        // 提取:过滤选中的文件以及选中目录下所有文件
        if (!extractAll) {
            bool flag = false;
            foreach (auto tmp, qAsConst(files)) {
                if (tmp.isDirectory) {
                    if (entryName.startsWith(tmp.strFullPath)) {
                        flag = true;
                        break;
                    }
                } else {
                    if (0 == entryName.compare(tmp.strFullPath)) {
                        flag = true;
                        break;
                    }
                }
            }

            // If entry is not found, skip entry.
            if (!flag) {
                continue;
            }
        }

        // entryFI is the fileinfo pointing to where the file will be
        // written from the archive.
        QFileInfo entryFI(entryName);

        // If the file has a rootNode attached, remove it from file path.
        // 提取操作不需要保留上层目录
        if (!extractAll && !options.strDestination.isEmpty()) { //提取
            const QString &rootNode = options.strDestination;
            if (isAtRootPath) { //提取首层目录下文件
                archive_entry_copy_pathname(entry, entryName.toUtf8().constData());
            } else {
                const QString truncatedFilename(entryName.remove(entryName.indexOf(rootNode), rootNode.size()));
                archive_entry_copy_pathname(entry, QFile::encodeName(truncatedFilename).constData());
                entryFI = QFileInfo(truncatedFilename);
            }
        } else {
            archive_entry_copy_pathname(entry, entryName.toUtf8().constData());
        }
        emit signalCurFileName(entryName); // 发送当前正在解压的文件名

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
                    return PFT_Cancel;
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
            if (entryFI.isWritable()) {
            } else {
                archive_entry_clear(entry);
                archive_read_data_skip(m_archiveReader.data());
                continue;
            }
        }

        // Write the entry header and check return value.
        const int returnCode = archive_write_header(writer.data(), entry); //创建文件
        switch (returnCode) {
        case ARCHIVE_OK: {
            // If the whole archive is extracted and the total filesize is
            // available, we use partial progress.
            if (extractAll == false) {
                copyDataFromSource_ArchiveEntry(entryName, m_archiveReader.data(), writer.data(), options.qSize);
            } else {
                copyDataFromSource(entryName, m_archiveReader.data(), writer.data());
            }

            // qDebug() <<  destinationDirectory + QDir::separator() + entryName;
            // 文件权限设置
            QFileDevice::Permissions per = getPermissions(archive_entry_perm(entry));
            if (entryIsDir) {
                per |= QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
            }
            QFile::setPermissions(/*destinationDirectory + QDir::separator() + */entryName, per);
        }
        break;

        case ARCHIVE_FAILED:
            emit error(("Filed error, extraction aborted."));
            return PFT_Error;

        case ARCHIVE_FATAL:
            emit error(("Fatal error, extraction aborted."));
            return PFT_Error;

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
        return PFT_Nomral;
    } else {
        return PFT_Error;
    }



}

PluginFinishType LibarchivePlugin::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::deleteFiles(const QList<FileEntry> &files)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::addComment(const QString &comment)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::updateArchiveData(const UpdateOptions &options)
{
    return PFT_Nomral;
}

void LibarchivePlugin::pauseOperation()
{
    m_bPause = true;
}

void LibarchivePlugin::continueOperation()
{
    m_bPause = false;
}

bool LibarchivePlugin::doKill()
{
    m_bPause = false;
    return false;
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

void LibarchivePlugin::copyDataFromSource(const QString &filename, struct archive *source, struct archive *dest)
{
    char buff[10240]; //缓存大小
    auto readBytes = archive_read_data(source, buff, sizeof(buff)); //读压缩包数据到buff

    while (readBytes > 0 && !QThread::currentThread()->isInterruptionRequested()) {
        if (m_bPause) { //解压暂停
            sleep(1);
//            qDebug() << "pause";
            continue;
        }

        archive_write_data(dest, buff, static_cast<size_t>(readBytes)); //写数据
        if (archive_errno(dest) != ARCHIVE_OK) {
            return;
        }


        // 解压百分比进度
        m_currentExtractedFilesSize += readBytes;
        emit signalprogress((double(m_currentExtractedFilesSize)) / DataManager::get_instance().archiveData().qSize * 100);

        readBytes = archive_read_data(source, buff, sizeof(buff));
    }
}

void LibarchivePlugin::copyDataFromSource_ArchiveEntry(const QString &filename, archive *source, archive *dest, qint64 extractFileSize)
{
    char buff[10240];
    auto readBytes = archive_read_data(source, buff, sizeof(buff)); //读压缩包数据到buff

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

        // 解压百分比进度
        m_currentExtractedFilesSize += readBytes;
        emit signalprogress((double(m_currentExtractedFilesSize)) / extractFileSize * 100);

        readBytes = archive_read_data(source, buff, sizeof(buff));
    }
}

PluginFinishType LibarchivePlugin::list_New()
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();

    if (!initializeReader()) {
        return PFT_Error;
    }

    QString compMethod = convertCompressionName(QString::fromUtf8(archive_filter_name(m_archiveReader.data(), 0)));
    if (!compMethod.isEmpty()) {
//        emit compressionMethodFound(compMethod);
    }

    m_ArchiveEntryCount = 0;
    m_numberOfEntries = 0;
    stArchiveData.qComressSize = QFileInfo(m_strArchiveName).size(); // 压缩包大小

    struct archive_entry *aentry;
    int result = ARCHIVE_RETRY;

    while (!QThread::currentThread()->isInterruptionRequested() && (result = archive_read_next_header(m_archiveReader.data(), &aentry)) == ARCHIVE_OK) {
//        if (!m_emitNoEntries) {
        emitEntryForIndex(aentry);
//        m_listIndex++;
//        }

        m_ArchiveEntryCount++;

        //The call to `archive_read_data_skip()` here is not actually necessary, since libarchive will invoke it automatically
        //if you request the next header without reading the data for the last entry.
//        archive_read_data_skip(m_archiveReader.data());
    }

    return PFT_Nomral;
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
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    FileEntry m_archiveEntryStat; //压缩包内文件数据
    // 文件名
    QByteArray strCode;
    m_archiveEntryStat.strFullPath = m_common->trans2uft8(archive_entry_pathname(aentry), strCode);

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
        stArchiveData.listRootEntry.push_back(m_archiveEntryStat);
    }
    // 压缩包原始大小
    stArchiveData.qSize += m_archiveEntryStat.qSize;
    // 构建压缩包数据map
    stArchiveData.mapFileEntry.insert(m_archiveEntryStat.strFullPath, m_archiveEntryStat);
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
