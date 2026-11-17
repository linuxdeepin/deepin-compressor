// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archive_backend.h"

#include "datamanager.h"
#include "queries.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include <archive_entry.h>

#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

namespace {

// 目标文件的格式与过滤器由后缀决定，与旧实现一致
bool looksLikeZip(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    const QByteArray magic = file.read(4);
    return magic.startsWith("PK\x03\x04") || magic.startsWith("PK\x05\x06");
}

bool configureWriter(struct archive *writer, const QString &path, const QString &mime)
{
    // 已存在的包以内容为准；新建时看 MIME 与后缀
    const bool isZip = looksLikeZip(path) || QFileInfo(path).suffix().compare(QLatin1String("zip"), Qt::CaseInsensitive) == 0
            || (!QFileInfo::exists(path) && mime == QLatin1String("application/zip"));

    if (isZip) {
        if (archive_write_set_format_zip(writer) != ARCHIVE_OK)
            return false;
    } else if (archive_write_set_format_pax_restricted(writer) != ARCHIVE_OK) {
        return false;
    }

    const QString suffix = QFileInfo(path).suffix().toUpper();
    int ret = ARCHIVE_OK;

    if (suffix == QLatin1String("GZ") || suffix == QLatin1String("TGZ"))
        ret = archive_write_add_filter_gzip(writer);
    else if (suffix == QLatin1String("BZ2") || suffix == QLatin1String("TBZ2") || suffix == QLatin1String("TBZ"))
        ret = archive_write_add_filter_bzip2(writer);
    else if (suffix == QLatin1String("XZ") || suffix == QLatin1String("TXZ"))
        ret = archive_write_add_filter_xz(writer);
    else if (suffix == QLatin1String("LZMA"))
        ret = archive_write_add_filter_lzma(writer);
    else if (suffix == QLatin1String("LZ"))
        ret = archive_write_add_filter_lzip(writer);
    else if (suffix == QLatin1String("LZ4"))
        ret = archive_write_add_filter_lz4(writer);
    else if (suffix == QLatin1String("Z"))
        ret = archive_write_add_filter_compress(writer);
    else
        ret = archive_write_add_filter_none(writer);

    return ret == ARCHIVE_OK || ret == ARCHIVE_WARN;
}

QString renamed(const QString &path, const QHash<QString, QString> &renames)
{
    for (auto it = renames.constBegin(); it != renames.constEnd(); ++it) {
        if (path == it.key())
            return it.value();
        if (path.startsWith(it.key()))
            return it.value() + path.mid(it.key().size());
    }
    return path;
}

bool dropped(const QString &path, const QSet<QString> &drop)
{
    for (const QString &item : drop) {
        if (path == item || path.startsWith(item))
            return true;
    }
    return false;
}

} // namespace

bool ArchiveBackend::replaceArchive(const QString &tempPath)
{
    const QByteArray from = QFile::encodeName(tempPath);
    const QByteArray to = QFile::encodeName(m_strArchiveName);
    if (::rename(from.constData(), to.constData()) != 0) {
        QFile::remove(tempPath);
        m_eErrorType = ET_FileWriteError;
        return false;
    }
    return true;
}

bool ArchiveBackend::copyExistingEntries(struct archive *writer, const QSet<QString> &drop,
                                         const QHash<QString, QString> &renames, qint64 totalSize)
{
    if (!openArchive())
        return false;

    QByteArray buffer(64 * 1024, '\0');
    Entry entry;
    while (nextEntry(entry)) {
        if (dropped(entry.file.strFullPath, drop)) {
            archive_read_data_skip(m_reader);
            continue;
        }

        struct archive_entry *header = archive_entry_new();
        const QString newPath = renamed(entry.file.strFullPath, renames);
        archive_entry_set_pathname(header, QFile::encodeName(newPath).constData());
        archive_entry_set_mtime(header, static_cast<time_t>(entry.file.uLastModifiedTime), 0);
        archive_entry_set_perm(header, entry.mode ? static_cast<mode_t>(entry.mode) : 0644);

        if (entry.isLink) {
            archive_entry_set_filetype(header, AE_IFLNK);
            archive_entry_set_symlink(header, QFile::encodeName(entry.linkTarget).constData());
            archive_entry_set_size(header, 0);
        } else if (entry.file.isDirectory) {
            archive_entry_set_filetype(header, AE_IFDIR);
            archive_entry_set_size(header, 0);
        } else {
            archive_entry_set_filetype(header, AE_IFREG);
            archive_entry_set_size(header, entry.file.qSize);
        }

        if (archive_write_header(writer, header) != ARCHIVE_OK) {
            archive_entry_free(header);
            m_eErrorType = ET_FileWriteError;
            return false;
        }

        if (!entry.file.isDirectory && !entry.isLink) {
            la_ssize_t read = 0;
            while ((read = archive_read_data(m_reader, buffer.data(), buffer.size())) > 0) {
                if (archive_write_data(writer, buffer.constData(), static_cast<size_t>(read)) < 0) {
                    archive_entry_free(header);
                    m_eErrorType = ET_FileWriteError;
                    return false;
                }
                m_progressDone += read;
                if (totalSize > 0) {
                    const qint64 percent = m_progressDone * 100 / totalSize;
                    if (percent != m_lastPercent) {
                        m_lastPercent = percent;
                        emit signalprogress(static_cast<double>(percent));
                    }
                }
            }
        }

        archive_entry_free(header);
    }

    closeArchive();
    return true;
}

bool ArchiveBackend::appendLocalPath(struct archive *writer, const QString &path,
                                     const QString &destination, qint64 totalSize)
{
    QByteArray buffer(64 * 1024, '\0');

    const auto addOne = [&](const QString &source, const QString &name) {
        const QFileInfo info(source);
        struct stat st;
        if (::lstat(QFile::encodeName(source).constData(), &st) != 0)
            return false;

        struct archive_entry *header = archive_entry_new();
        const QString entryName = destination + name;
        archive_entry_set_pathname(header, QFile::encodeName(entryName).constData());
        archive_entry_set_mtime(header, info.lastModified().toSecsSinceEpoch(), 0);
        archive_entry_set_perm(header, st.st_mode & 07777);

        bool ok = true;
        if (S_ISLNK(st.st_mode)) {
            archive_entry_set_filetype(header, AE_IFLNK);
            archive_entry_set_symlink(header, QFile::encodeName(info.symLinkTarget()).constData());
            archive_entry_set_size(header, 0);
            ok = (archive_write_header(writer, header) == ARCHIVE_OK);
        } else if (S_ISDIR(st.st_mode)) {
            archive_entry_set_filetype(header, AE_IFDIR);
            archive_entry_set_size(header, 0);
            ok = (archive_write_header(writer, header) == ARCHIVE_OK);
        } else {
            QFile file(source);
            if (!file.open(QIODevice::ReadOnly)) {
                archive_entry_free(header);
                return false;
            }
            archive_entry_set_filetype(header, AE_IFREG);
            archive_entry_set_size(header, st.st_size);
            ok = (archive_write_header(writer, header) == ARCHIVE_OK);
            la_ssize_t read = 0;
            while (ok && (read = file.read(buffer.data(), buffer.size())) > 0) {
                ok = archive_write_data(writer, buffer.constData(), static_cast<size_t>(read)) >= 0;
                m_progressDone += read;
                if (totalSize > 0) {
                    const qint64 percent = m_progressDone * 100 / totalSize;
                    if (percent != m_lastPercent) {
                        m_lastPercent = percent;
                        emit signalprogress(static_cast<double>(percent));
                    }
                }
            }
        }

        archive_entry_free(header);
        return ok;
    };

    emit signalCurFileName(QFileInfo(path).fileName());

    const QString baseName = QFileInfo(path).fileName();
    if (!addOne(path, baseName))
        return false;

    if (QFileInfo(path).isDir() && !QFileInfo(path).isSymLink()) {
        QDirIterator it(path, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString child = it.next();
            const QString name = baseName + child.mid(path.length());
            emit signalCurFileName(child);
            if (!addOne(child, name))
                return false;
        }
    }

    return true;
}

bool ArchiveBackend::rebuildArchive(const QString &outPath, const QSet<QString> &drop,
                                    const QHash<QString, QString> &renames,
                                    const QStringList &inputs, const QString &destination,
                                    qint64 totalSize)
{
    m_progressDone = 0;
    m_lastPercent = -1;

    struct archive *writer = archive_write_new();
    if (!writer) {
        m_eErrorType = ET_FileWriteError;
        return false;
    }

    if (!configureWriter(writer, m_strArchiveName, m_mimetype.name())) {
        archive_write_free(writer);
        m_eErrorType = ET_FileWriteError;
        return false;
    }

    if (archive_write_open_filename(writer, QFile::encodeName(outPath).constData()) != ARCHIVE_OK) {
        archive_write_free(writer);
        m_eErrorType = ET_FileWriteError;
        return false;
    }

    bool ok = true;
    if (QFileInfo::exists(m_strArchiveName)) {
        ok = copyExistingEntries(writer, drop, renames, totalSize);
    }
    for (int i = 0; ok && i < inputs.size(); ++i)
        ok = appendLocalPath(writer, inputs.at(i), destination, totalSize);

    const bool closed = (archive_write_close(writer) == ARCHIVE_OK);
    archive_write_free(writer);

    if (!ok || !closed) {
        QFile::remove(outPath);
        m_eErrorType = ET_FileWriteError;
        return false;
    }

    return true;
}

PluginFinishType ArchiveBackend::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    QStringList inputs;
    for (const FileEntry &file : files)
        inputs << file.strFullPath;
    if (inputs.isEmpty()) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    const QString tempPath = m_strArchiveName + QStringLiteral(".tmp-%1").arg(QCoreApplication::applicationPid());
    QFile::remove(tempPath);

    const bool ok = rebuildArchive(tempPath, {}, {}, inputs, options.strDestination, options.qTotalSize)
            && replaceArchive(tempPath);

    emit signalFinished(ok ? PFT_Nomral : PFT_Error);
    return ok ? PFT_Nomral : PFT_Error;
}

PluginFinishType ArchiveBackend::deleteFiles(const QList<FileEntry> &files)
{
    if (files.isEmpty())
        return PFT_Error;

    QSet<QString> drop;
    for (const FileEntry &file : files)
        drop.insert(file.strFullPath);

    const QString tempPath = m_strArchiveName + QStringLiteral(".tmp-%1").arg(QCoreApplication::applicationPid());
    QFile::remove(tempPath);

    const bool ok = rebuildArchive(tempPath, drop, {}, {}, {}, 0) && replaceArchive(tempPath);

    emit signalFinished(ok ? PFT_Nomral : PFT_Error);
    return ok ? PFT_Nomral : PFT_Error;
}

PluginFinishType ArchiveBackend::renameFiles(const QList<FileEntry> &files)
{
    if (files.isEmpty())
        return PFT_Error;

    QHash<QString, QString> renames;
    for (const FileEntry &file : files) {
        QString alias = file.strAlias;
        if (alias.isEmpty())
            continue;
        const QString parent = QFileInfo(file.strFullPath.endsWith(QLatin1Char('/'))
                                                 ? file.strFullPath.chopped(1)
                                                 : file.strFullPath).path();
        if (!parent.isEmpty() && parent != QLatin1String("."))
            alias = parent + QLatin1Char('/') + alias;
        renames.insert(file.strFullPath, alias);
    }
    if (renames.isEmpty())
        return PFT_Error;

    const QString tempPath = m_strArchiveName + QStringLiteral(".tmp-%1").arg(QCoreApplication::applicationPid());
    QFile::remove(tempPath);

    const bool ok = rebuildArchive(tempPath, {}, renames, {}, {}, 0) && replaceArchive(tempPath);

    emit signalFinished(ok ? PFT_Nomral : PFT_Error);
    return ok ? PFT_Nomral : PFT_Error;
}

PluginFinishType ArchiveBackend::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PFT_Error;
}

PluginFinishType ArchiveBackend::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PFT_Error;
}

PluginFinishType ArchiveBackend::addComment(const QString &comment)
{
    Q_UNUSED(comment)
    return PFT_Error;
}

PluginFinishType ArchiveBackend::updateArchiveData(const UpdateOptions &options)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    QString rootEntry;

    foreach (FileEntry entry, options.listEntry) {
        if (options.eType == UpdateOptions::Delete ) { // 删除，更新压缩包数据
            if (entry.isDirectory) { // 删除文件夹
                // 在map中查找该文件夹下的文件并删除
                QMap<QString, FileEntry>::iterator itor = stArchiveData.mapFileEntry.begin();
                while (itor != stArchiveData.mapFileEntry.end()) {
                    if (itor->strFullPath.startsWith(entry.strFullPath)) {
                        if (!itor->isDirectory) {
                            stArchiveData.qSize -= itor->qSize; // 更新压缩包内文件原始总大小
                        }
                        itor = stArchiveData.mapFileEntry.erase(itor);
                    } else {
                        ++itor;
                    }
                }

                // 更新文件夹第一层的数据
                if (entry.strFullPath.endsWith(QLatin1Char('/')) && entry.strFullPath.count(QLatin1Char('/')) == 1) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            } else { // 删除文件
                stArchiveData.qSize -= entry.qSize; // 更新压缩包内文件原始总大小
                stArchiveData.mapFileEntry.remove(entry.strFullPath); //在map中删除该文件
                // 更新文件夹第一层的数据
                if (!entry.strFullPath.contains(QLatin1Char('/'))) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            }
        } else if (options.eType == UpdateOptions::Rename) { // 重命名，更新压缩包数据
            QMap<QString, FileEntry> tmpMapFileEntry;
            QString strAlias;
            if (entry.isDirectory) { // 重命名文件夹
                // 在map中查找该文件夹下的文件并重命名
                QMap<QString, FileEntry>::iterator itor = stArchiveData.mapFileEntry.begin();
                while (itor != stArchiveData.mapFileEntry.end()) {
                    if (itor->strFullPath.startsWith(entry.strFullPath)) {
                        QString strPath = QFileInfo(entry.strFullPath.left(entry.strFullPath.length() - 1)).path();
                        if(strPath == "."){
                            strAlias = entry.strAlias + QDir::separator();
                        } else {
                            strAlias = strPath + QDir::separator() + entry.strAlias + QDir::separator();
                        }
                        strAlias = strAlias + QString(itor->strFullPath).right(QString(itor->strFullPath).length()-entry.strFullPath.length());
                        FileEntry tmpEntry = itor.value();
                        tmpEntry.strFullPath = strAlias;
                        if(tmpEntry.isDirectory) {
                            tmpEntry.strFileName = QFileInfo(strAlias.left(strAlias.length() - 1)).fileName();
                        } else {
                            tmpEntry.strFileName = QFileInfo(strAlias).fileName();
                        }
                        tmpEntry.strAlias.clear();
                        tmpMapFileEntry.insert(strAlias, tmpEntry);
                        itor = stArchiveData.mapFileEntry.erase(itor);
                    } else {
                        ++itor;
                    }
                }
                if(!tmpMapFileEntry.isEmpty()) {
                    for (QString strFullPath: tmpMapFileEntry.keys()) {
                        stArchiveData.mapFileEntry.insert(strFullPath, tmpMapFileEntry.value(strFullPath));
                    }
                }
                // 更新文件夹第一层的数据
                if (entry.strFullPath.endsWith(QLatin1Char('/')) && entry.strFullPath.count(QLatin1Char('/')) == 1) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            strAlias = entry.strAlias + QDir::separator();
                            entry.strFullPath = strAlias;
                            entry.strFileName = strAlias;
//                            entry.strAlias.clear();
                            stArchiveData.listRootEntry.append(entry);
                            break;
                        }
                    }
                }
            } else { // 重命名文件
//                stArchiveData.qSize -= entry.qSize; // 更新压缩包内文件原始总大小
                stArchiveData.mapFileEntry.remove(entry.strFullPath); //在map中重命名该文件
                QString strPath = QFileInfo(entry.strFullPath).path();
                if(strPath == "." || strPath.isEmpty() || strPath.isNull()) {
                    strAlias = entry.strAlias;
                } else {
                    strAlias = strPath + QDir::separator() + entry.strAlias;
                }
                FileEntry tmpEntry = entry;
                tmpEntry.strFullPath = strAlias;
                stArchiveData.mapFileEntry.insert(strAlias, tmpEntry);
                // 更新文件夹第一层的数据
                if (!entry.strFullPath.contains(QLatin1Char('/'))) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry重命名
                            stArchiveData.listRootEntry.removeAt(i);
                            strAlias = entry.strAlias;
                            entry.strFullPath = strAlias;
                            entry.strFileName = strAlias;
                            stArchiveData.listRootEntry.append(entry);
                            break;
                        }
                    }
                }
            }
        } else if (options.eType == UpdateOptions::Add) { // 追加，更新压缩包数据
            QString destinationPath = options.strParentPath; // 在压缩包中追加的路径
            QFileInfo file(entry.strFullPath); // 注意此时entry.strFullPath如果是是文件夹还没有'/'
            if (rootEntry.isEmpty()) { // 获取所有追加文件的父目录
                rootEntry = file.filePath().left(file.filePath().size() - file.fileName().size());
            }

            entry.strFullPath = destinationPath + entry.strFullPath.remove(rootEntry); // entry在压缩包中的全路径
            if (file.isDir()) { // 文件夹
                entry.strFullPath = entry.strFullPath + QDir::separator(); // 手动添加'/'
                //entry.qSize = QDir(entry.strFullPath).entryInfoList().count(); // 获取文件夹大小为遍历文件夹获取文件夹下子文件的数目
            } else {
//                entry.qSize = file.size(); // 文件大小
                // 更新压缩包内文件原始总大小
                stArchiveData.qSize -= stArchiveData.mapFileEntry.value(entry.strFullPath).qSize;
                stArchiveData.qSize += entry.qSize;
            }

            // 判断是否追加到第一层数据
            if (destinationPath.isEmpty() && ((entry.strFullPath.count('/') == 1 && entry.strFullPath.endsWith('/')) || entry.strFullPath.count('/') == 0)) {
                for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                    if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一层数据中找到entry，不添加数据
                        stArchiveData.listRootEntry.removeAt(i);
                        break;
                    }
                }

                // 在第一层数据中没有找到entry，在第一层数据中添加entry
                stArchiveData.listRootEntry.push_back(entry);
            }

            // 更新压缩包map
            stArchiveData.mapFileEntry.insert(entry.strFullPath, entry);

        }
    }

    stArchiveData.qComressSize = QFileInfo(m_strArchiveName).size(); // 更新压缩包大小

    return PFT_Nomral;
}
