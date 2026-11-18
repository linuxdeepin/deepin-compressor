// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "minizip_backend.h"

#include "common.h"
#include "datamanager.h"
#include "queries.h"

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include <cerrno>
#include <unistd.h>

namespace {

constexpr int kMaxFilename = 4096;
constexpr int kBufferSize = 1024;

bool isLongName(const QString &path)
{
    const QStringList parts = path.split(QLatin1Char('/'));
    for (const QString &part : parts) {
        if (NAME_MAX < part.toLocal8Bit().length())
            return true;
    }
    return false;
}

} // namespace

MinizipBackend::MinizipBackend(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{
}

MinizipBackend::~MinizipBackend()
{
}

namespace {

// 扫全包原始名字拼接后探测一次
QByteArray archiveEncoding(unzFile zipfile, const QString &archiveName)
{
    QByteArray joined;
    if (unzGoToFirstFile(zipfile) != UNZ_OK)
        return QByteArray();
    do {
        char filename[kMaxFilename] = {0};
        if (unzGetCurrentFileInfo(zipfile, nullptr, filename, kMaxFilename, nullptr, 0, nullptr, 0) == UNZ_OK) {
            if (!joined.isEmpty())
                joined.append(' ');
            joined.append(filename);
        }
    } while (unzGoToNextFile(zipfile) == UNZ_OK);
    unzGoToFirstFile(zipfile);

    if (joined.isEmpty())
        return QByteArray();
    Common common;
    return common.detectEncode(joined, archiveName);
}

} // namespace

bool MinizipBackend::loadEntry(unzFile zipfile, FileEntry &entry)
{
    unz_file_info fileInfo;
    char filename[kMaxFilename] = {0};
    if (unzGetCurrentFileInfo(zipfile, &fileInfo, filename, kMaxFilename, nullptr, 0, nullptr, 0) != UNZ_OK)
        return false;

    QByteArray code = (!m_archiveCode.isEmpty() && !Common::isUtf8Text(filename))
            ? m_archiveCode : QByteArray();
    const QString name = m_common->trans2uft8(filename, code);

    entry.reset();
    entry.strFullPath = name;
    const QStringList pieces = name.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    entry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
    entry.isDirectory = name.endsWith(QLatin1Char('/'));

    if (entry.isDirectory) {
        entry.qSize = 0;
    } else {
        entry.qSize = static_cast<qlonglong>(fileInfo.uncompressed_size);
        DataManager::get_instance().archiveData().qSize += entry.qSize;
        DataManager::get_instance().archiveData().qComressSize += static_cast<qlonglong>(fileInfo.compressed_size);
    }

    QDateTime datetime;
    datetime.setDate(QDate(int(fileInfo.tmu_date.tm_year), int(fileInfo.tmu_date.tm_mon + 1),
                           int(fileInfo.tmu_date.tm_mday)));
    datetime.setTime(QTime(int(fileInfo.tmu_date.tm_hour), int(fileInfo.tmu_date.tm_min),
                           int(fileInfo.tmu_date.tm_sec)));
    entry.uLastModifiedTime = static_cast<uint>(datetime.toSecsSinceEpoch());

    return true;
}

PluginFinishType MinizipBackend::list()
{
    DataManager::get_instance().resetArchiveData();

    unzFile zipfile = unzOpen(QFile::encodeName(m_strArchiveName).constData());
    if (!zipfile) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    unz_global_info globalInfo;
    if (unzGetGlobalInfo(zipfile, &globalInfo) != UNZ_OK) {
        unzClose(zipfile);
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    m_archiveCode = archiveEncoding(zipfile, m_strArchiveName);

    for (uLong i = 0; i < globalInfo.number_entry; ++i) {
        if (QThread::currentThread()->isInterruptionRequested())
            break;

        FileEntry entry;
        if (!loadEntry(zipfile, entry)) {
            unzClose(zipfile);
            m_eErrorType = ET_ArchiveDamaged;
            return PFT_Error;
        }

        handleEntry(entry);
        DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

        if ((i + 1) < globalInfo.number_entry)
            unzGoToNextFile(zipfile);
    }

    unzClose(zipfile);
    return PFT_Nomral;
}

PluginFinishType MinizipBackend::testArchive()
{
    m_workStatus = WT_Test;
    return PFT_Nomral;
}

ErrorType MinizipBackend::extractEntry(unzFile zipfile, const unz_file_info &fileInfo,
                                       const ExtractionOptions &options, QString fileName)
{
    if (!options.strDestination.isEmpty())
        fileName.remove(0, options.strDestination.size());

    if (fileName.contains(QLatin1Char('\\')))
        fileName.replace(QLatin1Char('\\'), QLatin1Char('/'));

    const bool isDirectory = fileName.endsWith(QLatin1Char('/'));

    QStringList safeParts;
    for (const QString &part : fileName.split(QLatin1Char('/'), Qt::SkipEmptyParts)) {
        if (part == QLatin1String("..")) {
            if (!safeParts.isEmpty())
                safeParts.removeLast();
        } else if (part != QLatin1String(".")) {
            safeParts.append(part);
        }
    }
    fileName = safeParts.join(QLatin1Char('/'));
    if (isDirectory && !fileName.isEmpty() && !fileName.endsWith(QLatin1Char('/')))
        fileName += QLatin1Char('/');

    emit signalCurFileName(fileName);

    if (!QDir().exists(options.strTargetPath))
        QDir().mkpath(options.strTargetPath);

    const QString destPath = options.strTargetPath + QLatin1Char('/') + fileName;
    const QString cleanTarget = QDir::cleanPath(QDir(options.strTargetPath).absolutePath());
    const QString cleanDest = QDir::cleanPath(QDir(destPath).absolutePath());
    if (cleanDest != cleanTarget
            && !cleanDest.startsWith(cleanTarget + QLatin1Char('/'))) {
        return ET_FileWriteError;
    }

    QFile file(destPath);

    if (isDirectory) {
        if (isLongName(fileName))
            return ET_LongNameError;
        QDir().mkpath(destPath);
    } else {
        if (file.exists()) {
            if (m_bSkipAll)
                return ET_NoError;

            if (!m_bOverwriteAll) {
                OverwriteQuery query(destPath);
                emit signalQuery(&query);
                query.waitForResponse();

                if (query.responseCancelled()) {
                    emit signalCancel();
                    return ET_UserCancelOpertion;
                }
                if (query.responseSkip())
                    return ET_NoError;
                if (query.responseSkipAll()) {
                    m_bSkipAll = true;
                    return ET_NoError;
                }
                if (query.responseOverwriteAll())
                    m_bOverwriteAll = true;
            }
        }

        if (file.exists() && !file.isWritable()) {
            file.remove();
            file.setFileName(destPath);
            file.setPermissions(QFileDevice::WriteUser);
        }

        if (!QDir().exists(QFileInfo(destPath).path()))
            QDir().mkpath(QFileInfo(destPath).path());

        if (!file.open(QIODevice::WriteOnly)) {
            if (isLongName(destPath))
                return ET_LongNameError;
            emit signalFileWriteErrorName(QFileInfo(destPath).fileName());
            return ET_FileOpenError;
        }

        if (unzOpenCurrentFile(zipfile) != UNZ_OK)
            return ET_ArchiveDamaged;

        char buffer[kBufferSize];
        uLong written = 0;
        QDataStream out(&file);

        while (written != fileInfo.uncompressed_size) {
            if (QThread::currentThread()->isInterruptionRequested())
                break;

            while (m_bPause)
                sleep(1);

            const int readBytes = unzReadCurrentFile(zipfile, buffer, kBufferSize);
            if (readBytes < 0) {
                file.close();
                emit signalFileWriteErrorName(QFileInfo(destPath).fileName());
                return ET_FileWriteError;
            }

            if (out.writeRawData(buffer, readBytes) != readBytes) {
                file.close();
                if (isInsufficientDiskSpace(options.strTargetPath, static_cast<qint64>(readBytes)))
                    return ET_InsufficientDiskSpace;
                emit signalFileWriteErrorName(QFileInfo(destPath).fileName());
                return ET_FileWriteError;
            }

            written += static_cast<uLong>(readBytes);
            // 进度：右键快捷解压按压缩比例折算，否则按原始大小
            if (!options.bExistList) {
                const double ratio = fileInfo.uncompressed_size > 0
                        ? double(fileInfo.compressed_size) / double(fileInfo.uncompressed_size)
                        : 0;
                m_extractedSize += static_cast<qlonglong>(readBytes * ratio);
                if (options.qComressSize > 0)
                    emit signalprogress(double(m_extractedSize) / options.qComressSize * 100);
            } else {
                m_extractedSize += readBytes;
                if (options.qSize > 0)
                    emit signalprogress(double(m_extractedSize) / options.qSize * 100);
            }
        }

        file.close();
    }

    unzCloseCurrentFile(zipfile);
    return ET_NoError;
}

PluginFinishType MinizipBackend::extractFiles(const QList<FileEntry> &files,
                                              const ExtractionOptions &options)
{
    m_workStatus = WT_Extract;
    m_bOverwriteAll = false;
    m_bSkipAll = false;
    m_extractedSize = 0;

    unzFile zipfile = unzOpen(QFile::encodeName(m_strArchiveName).constData());
    if (!zipfile) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    unz_global_info globalInfo;
    if (unzGetGlobalInfo(zipfile, &globalInfo) != UNZ_OK) {
        unzClose(zipfile);
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    m_archiveCode = archiveEncoding(zipfile, m_strArchiveName);

    if (options.bAllExtract) {
        for (uLong i = 0; i < globalInfo.number_entry; ++i) {
            if (QThread::currentThread()->isInterruptionRequested())
                break;

            unz_file_info fileInfo;
            char filename[kMaxFilename] = {0};
            if (unzGetCurrentFileInfo(zipfile, &fileInfo, filename, kMaxFilename, nullptr, 0, nullptr, 0) != UNZ_OK) {
                unzClose(zipfile);
                m_eErrorType = ET_ArchiveDamaged;
                return PFT_Error;
            }

            QByteArray code;
            const QString name = m_common->trans2uft8(filename, code);

            m_eErrorType = extractEntry(zipfile, fileInfo, options, name);

            if (!options.bExistList && i == 0) {
                FileEntry root;
                root.strFullPath = name;
                DataManager::get_instance().archiveData().listRootEntry << root;
            }

            if (m_eErrorType == ET_NoError) {
                if ((i + 1) < globalInfo.number_entry)
                    unzGoToNextFile(zipfile);
            } else if (m_eErrorType == ET_UserCancelOpertion) {
                unzClose(zipfile);
                return PFT_Cancel;
            } else {
                unzClose(zipfile);
                return PFT_Error;
            }
        }
    } else {
        for (const FileEntry &file : files) {
            if (QThread::currentThread()->isInterruptionRequested())
                break;

            QByteArray code;
            const QString target = m_common->trans2uft8(file.strFullPath.toUtf8().constData(), code);
            if (unzLocateFile(zipfile, target.toLatin1().constData(), 0) != UNZ_OK)
                continue;

            unz_file_info fileInfo;
            char filename[kMaxFilename] = {0};
            if (unzGetCurrentFileInfo(zipfile, &fileInfo, filename, kMaxFilename, nullptr, 0, nullptr, 0) != UNZ_OK) {
                unzClose(zipfile);
                m_eErrorType = ET_ArchiveDamaged;
                return PFT_Error;
            }

            m_eErrorType = extractEntry(zipfile, fileInfo, options, file.strFullPath);
            if (m_eErrorType == ET_UserCancelOpertion) {
                unzClose(zipfile);
                return PFT_Cancel;
            }
            if (m_eErrorType != ET_NoError) {
                unzClose(zipfile);
                return PFT_Error;
            }
        }
    }

    unzClose(zipfile);
    return PFT_Nomral;
}

void MinizipBackend::pauseOperation()
{
    m_bPause = true;
}

void MinizipBackend::continueOperation()
{
    m_bPause = false;
}

bool MinizipBackend::doKill()
{
    m_bPause = false;
    return false;
}
