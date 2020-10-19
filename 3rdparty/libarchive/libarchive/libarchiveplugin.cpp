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

bool LibarchivePlugin::list()
{
    qDebug() << "LibarchivePlugin插件加载压缩包数据";
    bool ret = false;
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
            ret = list_New();
        } else {
            ret = false;
        }
    } else {
        ret = list_New();
    }
    emit signalFinished(ret);
    return ret;
}

bool LibarchivePlugin::testArchive()
{
    return true;
}

bool LibarchivePlugin::extractFiles(const QVector<FileEntry> &files, const ExtractionOptions &options)
{
    return true;
}

bool LibarchivePlugin::addFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return false;
}

bool LibarchivePlugin::moveFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    return false;
}

bool LibarchivePlugin::copyFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    return false;
}

bool LibarchivePlugin::deleteFiles(const QVector<FileEntry> &files)
{
    return false;
}

bool LibarchivePlugin::addComment(const QString &comment)
{
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

bool LibarchivePlugin::list_New()
{
    if (!initializeReader()) {
        return false;
    }

    QString compMethod = convertCompressionName(QString::fromUtf8(archive_filter_name(m_archiveReader.data(), 0)));
    if (!compMethod.isEmpty()) {
//        emit compressionMethodFound(compMethod);
    }

    m_ArchiveEntryCount = 0;
    m_extractedFilesSize = 0;
    m_numberOfEntries = 0;
    auto compressedArchiveSize = QFileInfo(m_strArchiveName).size();

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

    return true;
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
