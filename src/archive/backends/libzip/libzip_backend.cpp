// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libzip_backend.h"

#include "common.h"
#include "datamanager.h"
#include "queries.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

namespace {

constexpr int kReadBytes = 8192;

QString normalizedName(const QString &raw)
{
    QString name = raw;
    while (name.contains(QLatin1String("../")))
        name.remove(QLatin1String("../"));
    if (name.contains(QLatin1Char('\\')))
        name.replace(QLatin1Char('\\'), QLatin1Char('/'));
    if (name.startsWith(QLatin1Char('/')))
        name.remove(0, 1);
    return name;
}

bool hasLongComponent(const QString &path)
{
    const QStringList parts = path.split(QLatin1Char('/'));
    for (const QString &part : parts) {
        if (NAME_MAX < part.toLocal8Bit().length())
            return true;
    }
    return false;
}

} // namespace

LibzipBackend::LibzipBackend(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
}

LibzipBackend::~LibzipBackend()
{
}

QByteArray LibzipBackend::passwordBytes(const QString &password) const
{
    return password.toUtf8();
}

PluginFinishType LibzipBackend::list()
{
    setPassword(QString());
    m_codeMap.clear();
    DataManager::get_instance().resetArchiveData();

    if (!m_common->isSupportSeek(m_strArchiveName)) {
        m_eErrorType = ET_FileSeekError;
        return PFT_Error;
    }

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_RDONLY, &errcode);
    if (!archive) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    ArchiveData &data = DataManager::get_instance().archiveData();
    QByteArray code;
    m_comment = m_common->trans2uft8(zip_get_archive_comment(archive, nullptr, ZIP_FL_ENC_RAW), code);

    m_archiveCode.clear();
    {
        QByteArray joined;
        const zip_int64_t total = zip_get_num_entries(archive, 0);
        for (zip_int64_t idx = 0; idx < total; ++idx) {
            const char *raw = zip_get_name(archive, zip_uint64_t(idx), ZIP_FL_ENC_RAW);
            if (!raw)
                continue;
            if (!joined.isEmpty())
                joined.append(' ');
            joined.append(raw);
        }
        if (!joined.isEmpty())
            m_archiveCode = m_common->detectEncode(joined, m_strArchiveName);
    }


    const zip_int64_t count = zip_get_num_entries(archive, 0);
    for (zip_int64_t index = 0; index < count; ++index) {
        if (QThread::currentThread()->isInterruptionRequested())
            break;

        zip_stat_t statBuffer;
        if (zip_stat_index(archive, zip_uint64_t(index), ZIP_FL_ENC_RAW, &statBuffer) != 0)
            continue;

        QByteArray entryCode = (!m_archiveCode.isEmpty() && !Common::isUtf8Text(statBuffer.name))
                ? m_archiveCode : QByteArray();
        const QString name = normalizedName(m_common->trans2uft8(statBuffer.name, entryCode));
        m_codeMap[index] = entryCode;

        FileEntry entry;
        entry.iIndex = int(index);
        entry.strFullPath = name;

        const QStringList parts = name.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        entry.strFileName = parts.isEmpty() ? QString() : parts.last();
        entry.isDirectory = name.endsWith(QLatin1Char('/'));

        if (statBuffer.valid & ZIP_STAT_SIZE) {
            if (entry.isDirectory) {
                entry.qSize = 0;
            } else {
                entry.qSize = qlonglong(statBuffer.size);
                data.qSize += qlonglong(statBuffer.size);
                data.qComressSize += qlonglong(statBuffer.comp_size);
            }
        }
        if (statBuffer.valid & ZIP_STAT_MTIME)
            entry.uLastModifiedTime = uint(statBuffer.mtime);

        data.strComment = m_comment;
        handleEntry(entry);
        data.mapFileEntry[name] = entry;
    }

    zip_close(archive);
    return PFT_Nomral;
}

PluginFinishType LibzipBackend::testArchive()
{
    // 旧实现只置工作状态，不做实际校验
    m_workStatus = WT_Test;
    return PFT_Nomral;
}

ErrorType LibzipBackend::extractEntry(zip_t *archive, zip_int64_t index,
                                      const ExtractionOptions &options,
                                      qlonglong &extracted, QString &fileName)
{
    zip_stat_t statBuffer;
    if (zip_stat_index(archive, zip_uint64_t(index), ZIP_FL_ENC_RAW, &statBuffer) != 0)
        return ET_FileReadError;

    QByteArray code;
    fileName = normalizedName(m_common->trans2uft8(statBuffer.name, code));

    const QString originName = fileName;
    QString longKey;
    bool truncated = false;

    if (!m_bLnfs) {
        QString filePath;
        QString leaf = fileName;
        const int slash = fileName.lastIndexOf(QLatin1Char('/'));
        if (slash >= 0) {
            filePath = fileName.left(slash);
            leaf = fileName.mid(slash + 1);
        }

        // 目录级长名：交给契约层统一处理，保持与旧实现一致
        const QString longDir =
                m_common->handleLongNameforPath(filePath, fileName, m_mapLongDirName, m_mapRealDirValue);
        if (!longDir.isEmpty()) {
            filePath = longDir.endsWith(QLatin1Char('/')) ? longDir.chopped(1) : longDir;
            if (fileName.endsWith(QLatin1Char('/')))
                fileName = longDir;
            else if (NAME_MAX >= leaf.toLocal8Bit().length())
                fileName = longDir + leaf;
            if (!m_mapLongDirName.isEmpty())
                truncated = true;
        }

        if (NAME_MAX < leaf.toLocal8Bit().length() && !leaf.endsWith(QLatin1Char('/'))) {
            const QString base = leaf.left(TRUNCATION_FILE_LONG);
            longKey = filePath + QLatin1Char('/') + base;
            if (m_mapLongName.value(longKey) >= LONGFILE_SAME_FILES)
                return ET_LongNameError;

            truncated = true;
            QString suffix = QFileInfo(leaf).completeSuffix();
            if (suffix.length() > 10) {
                suffix = QFileInfo(leaf).suffix();
                if (suffix.length() > 10)
                    suffix = suffix.right(10);
            }

            const QString newName = base
                    + QStringLiteral("(%1)").arg(m_mapLongName.value(longKey) + 1, LONGFILE_SUFFIX_FieldWidth, 10, QChar('0'))
                    + QLatin1Char('.') + suffix;
            fileName = slash >= 0 ? filePath + QLatin1Char('/') + newName : newName;
        }
    }

    if (!options.strDestination.isEmpty())
        fileName.remove(0, options.strDestination.size());

    // 截断过的条目，界面仍显示包内的原始名字
    emit signalCurFileName(truncated ? originName : fileName);

    const bool isDirectory = fileName.endsWith(QLatin1Char('/'));

    if (!QDir().exists(options.strTargetPath))
        QDir().mkpath(options.strTargetPath);

    const QString destPath = options.strTargetPath + QLatin1Char('/') + fileName;
    const QString cleanTarget = QDir::cleanPath(QDir(options.strTargetPath).absolutePath());
    const QString cleanDest = QDir::cleanPath(QDir(destPath).absolutePath());
    if (cleanDest != cleanTarget && !cleanDest.startsWith(cleanTarget + QLatin1Char('/')))
        return ET_FileWriteError;

    QFile file(destPath);

    QString parentDir;
    if (isDirectory) {
        QDir parent = QFileInfo(destPath).dir();
        parent.cdUp();
        parentDir = parent.path();
    } else {
        parentDir = QFileInfo(destPath).path();
    }
    const bool restoreParentMtime = (parentDir != options.strTargetPath);
    time_t parentMtime = 0;
    if (restoreParentMtime)
        parentMtime = QFileInfo(parentDir).lastModified().toMSecsSinceEpoch() / 1000;

    zip_uint8_t opsys = 0;
    zip_uint32_t attributes = 0;
    zip_file_get_external_attributes(archive, zip_uint64_t(index), 0, &opsys, &attributes);

    mode_t mode = 0;
    if (opsys == ZIP_OPSYS_UNIX)
        mode = attributes >> 16;

    const bool isLink = S_ISLNK(mode);
    QFileDevice::Permissions permissions = getPermissions(mode);

    if (isDirectory && !isLink) {
        if (hasLongComponent(fileName))
            return ET_LongNameError;

        if (!QDir().exists(destPath))
            QDir().mkpath(destPath);
        permissions = permissions | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
    } else if (isLink) {
        zip_file_t *zipFile = zip_fopen_index(archive, zip_uint64_t(index), 0);
        if (!zipFile)
            return ET_FileOpenError;

        char buffer[kReadBytes] = {0};
        const zip_int64_t readBytes = zip_fread(zipFile, buffer, kReadBytes);
        if (readBytes > 0) {
            const QString target = QString::fromLocal8Bit(buffer, int(readBytes));
            QFile::link(target, destPath);
        }
        file.close();
        zip_fclose(zipFile);
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

        zip_file_t *zipFile = zip_fopen_index(archive, zip_uint64_t(index), 0);
        if (!zipFile) {
            const int zipError = zip_error_code_zip(zip_get_error(archive));
            if (zipError == ZIP_ER_WRONGPASSWD)
                return ET_WrongPassword;
            if (zipError == ZIP_ER_NOPASSWD)
                return ET_NeedPassword;
            return ET_FileOpenError;
        }

        if (!file.open(QIODevice::WriteOnly)) {
            zip_fclose(zipFile);
            if (hasLongComponent(destPath))
                return ET_LongNameError;
            emit signalFileWriteErrorName(QFileInfo(destPath).fileName());
            return ET_FileWriteError;
        }

        QDataStream out(&file);
        double scale = 1;
        if (!options.bExistList && statBuffer.size > 0)
            scale = double(statBuffer.comp_size) / double(statBuffer.size);

        zip_int64_t sum = 0;
        char buffer[kReadBytes];
        while (sum != zip_int64_t(statBuffer.size)) {
            if (QThread::currentThread()->isInterruptionRequested())
                break;

            while (m_bPause)
                sleep(1);

            const zip_int64_t readBytes = zip_fread(zipFile, buffer, kReadBytes);
            if (readBytes < 0) {
                file.close();
                zip_fclose(zipFile);
                emit signalFileWriteErrorName(QFileInfo(destPath).fileName());
                return ET_FileWriteError;
            }

            if (out.writeRawData(buffer, int(readBytes)) != readBytes) {
                file.close();
                zip_fclose(zipFile);
                if (isInsufficientDiskSpace(options.strTargetPath, static_cast<qint64>(readBytes)))
                    return ET_InsufficientDiskSpace;
                emit signalFileWriteErrorName(QFileInfo(destPath).fileName());
                return ET_FileWriteError;
            }

            sum += readBytes;
            extracted += static_cast<qlonglong>(readBytes * scale);
            emit signalprogress(double(extracted) * m_scaleSize);
        }

        file.close();
        zip_fclose(zipFile);
    }

    if (!isLink) {
        file.setPermissions(permissions);

        utimbuf times;
        times.actime = time_t(statBuffer.mtime);
        times.modtime = time_t(statBuffer.mtime);
        utime(QFile::encodeName(destPath).constData(), &times);

        if (restoreParentMtime) {
            times.modtime = parentMtime;
            utime(QFile::encodeName(parentDir).constData(), &times);
        }
    }

    if (!longKey.isEmpty()) {
        m_mapLongName[longKey]++;
        m_handledLongName = true;
    } else if (truncated) {
        m_handledLongName = true;
    }

    return ET_NoError;
}

PluginFinishType LibzipBackend::extractFiles(const QList<FileEntry> &files,
                                             const ExtractionOptions &options)
{
    setPassword(QString());
    m_workStatus = WT_Extract;
    m_bOverwriteAll = false;
    m_bSkipAll = false;

    if (!m_common->isSupportSeek(m_strArchiveName)) {
        m_eErrorType = ET_FileSeekError;
        return PFT_Error;
    }

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_RDONLY, &errcode);
    if (!archive) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    m_bLnfs = m_common->isSubpathOfLnfs(options.strTargetPath);

    m_scaleSize = options.bExistList
            ? (options.qSize > 0 ? 100.0 / options.qSize : 0)
            : (options.qComressSize > 0 ? 100.0 / options.qComressSize : 0);

    QList<zip_int64_t> indexes;
    if (options.bAllExtract) {
        const zip_int64_t count = zip_get_num_entries(archive, 0);
        for (zip_int64_t i = 0; i < count; ++i)
            indexes << i;
    } else {
        for (const FileEntry &file : files) {
            const zip_int64_t index = zip_name_locate(archive, file.strFullPath.toUtf8().constData(), 0);
            if (index >= 0)
                indexes << index;
        }
    }

    qlonglong extracted = 0;
    zip_int64_t lastPasswordIndex = -1;
    m_handledLongName = false;

    for (int i = 0; i < indexes.size(); ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            m_bCancel = false;
            break;
        }

        QString fileName;
        m_eErrorType = extractEntry(archive, indexes.at(i), options, extracted, fileName);


        if (!options.bExistList && i == 0) {
            FileEntry root;
            root.strFullPath = fileName;
            DataManager::get_instance().archiveData().listRootEntry << root;
        }

        if (m_eErrorType == ET_NoError)
            continue;

        if (m_eErrorType == ET_UserCancelOpertion) {
            zip_close(archive);
            return PFT_Cancel;
        }

        if (m_eErrorType == ET_WrongPassword || m_eErrorType == ET_NeedPassword) {
            if (lastPasswordIndex == indexes.at(i)) {
                zip_close(archive);
                setPassword(QString());
                return PFT_Error;
            }

            PasswordNeededQuery query(fileName);
            emit signalQuery(&query);
            query.waitForResponse();

            if (query.responseCancelled()) {
                setPassword(QString());
                zip_close(archive);
                return PFT_Error;
            }

            setPassword(query.password());
            zip_set_default_password(archive, passwordBytes(m_strPassword).constData());
            lastPasswordIndex = indexes.at(i);
            --i;
            continue;
        }

        zip_close(archive);
        return PFT_Error;
    }

    zip_close(archive);

    if (m_handledLongName)
        m_eErrorType = ET_LongNameError;

    return PFT_Nomral;
}

void LibzipBackend::pauseOperation()
{
    m_bPause = true;
}

void LibzipBackend::continueOperation()
{
    m_bPause = false;
}

bool LibzipBackend::doKill()
{
    m_bPause = false;
    return false;
}
