// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libzip_backend.h"

#include "datamanager.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

#include <cstring>
#include <sys/stat.h>

void LibzipBackend::progressCallback(zip_t *archive, double progress, void *that)
{
    Q_UNUSED(archive)
    auto *self = static_cast<LibzipBackend *>(that);
    if (self)
        emit self->signalprogress(progress * 100);
}

bool LibzipBackend::writeLocalPath(zip_t *archive, const QString &path, const QString &root)
{
    const QFileInfo info(path);
    const QString name = root + info.fileName();

    if (info.isDir() && !info.isSymLink()) {
        if (zip_dir_add(archive, name.toUtf8().constData(), ZIP_FL_ENC_UTF_8) < 0
                && zip_error_code_zip(zip_get_error(archive)) != ZIP_ER_EXISTS) {
            return false;
        }

        QDirIterator it(path, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString child = it.next();
            const QString childName = name + QLatin1Char('/') + child.mid(path.length() + 1);
            const QFileInfo childInfo(child);

            if (childInfo.isDir() && !childInfo.isSymLink()) {
                if (zip_dir_add(archive, childName.toUtf8().constData(), ZIP_FL_ENC_UTF_8) < 0
                        && zip_error_code_zip(zip_get_error(archive)) != ZIP_ER_EXISTS) {
                    return false;
                }
                continue;
            }

            zip_source_t *source = zip_source_file(archive, QFile::encodeName(child).constData(), 0, -1);
            if (!source)
                return false;
            const zip_int64_t index =
                    zip_file_add(archive, childName.toUtf8().constData(), source, ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE);
            if (index < 0) {
                zip_source_free(source);
                return false;
            }
            if (m_level >= 0)
                zip_set_file_compression(archive, zip_uint64_t(index), ZIP_CM_DEFAULT, zip_uint32_t(m_level));
        }
        return true;
    }

    zip_source_t *source = zip_source_file(archive, QFile::encodeName(path).constData(), 0, -1);
    if (!source)
        return false;

    const zip_int64_t index =
            zip_file_add(archive, name.toUtf8().constData(), source, ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE);
    if (index < 0) {
        zip_source_free(source);
        return false;
    }
    if (m_level >= 0)
        zip_set_file_compression(archive, zip_uint64_t(index), ZIP_CM_DEFAULT, zip_uint32_t(m_level));

    return true;
}

PluginFinishType LibzipBackend::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    m_workStatus = WT_Add;
    m_level = options.iCompressionLevel;

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_CREATE, &errcode);
    if (!archive) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    if (!options.strPassword.isEmpty())
        zip_set_default_password(archive, passwordBytes(options.strPassword).constData());

    bool ok = true;
    for (const FileEntry &file : files) {
        if (!ok)
            break;
        emit signalCurFileName(file.strFullPath);
        ok = writeLocalPath(archive, file.strFullPath, options.strDestination);
    }

    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);

    if (zip_close(archive) != 0) {
        m_eErrorType = ET_FileWriteError;
        ok = false;
    }

    emit signalFinished(ok ? PFT_Nomral : PFT_Error);
    return ok ? PFT_Nomral : PFT_Error;
}

PluginFinishType LibzipBackend::deleteFiles(const QList<FileEntry> &files)
{
    m_workStatus = WT_Delete;

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), 0, &errcode);
    if (!archive) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    bool ok = true;
    for (const FileEntry &file : files) {
        const zip_int64_t index = zip_name_locate(archive, file.strFullPath.toUtf8().constData(), 0);
        if (index < 0) {
            // 目录条目带尾斜杠，去掉再试
            const QString trimmed = file.strFullPath.endsWith(QLatin1Char('/'))
                    ? file.strFullPath.chopped(1)
                    : file.strFullPath;
            const zip_int64_t alt = zip_name_locate(archive, trimmed.toUtf8().constData(), 0);
            if (alt < 0 || zip_delete(archive, zip_uint64_t(alt)) != 0) {
                ok = false;
                continue;
            }
            continue;
        }
        if (zip_delete(archive, zip_uint64_t(index)) != 0)
            ok = false;
    }

    if (zip_close(archive) != 0) {
        m_eErrorType = ET_DeleteError;
        ok = false;
    }

    emit signalFinished(ok ? PFT_Nomral : PFT_Error);
    return ok ? PFT_Nomral : PFT_Error;
}

PluginFinishType LibzipBackend::renameFiles(const QList<FileEntry> &files)
{
    m_workStatus = WT_Rename;

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), 0, &errcode);
    if (!archive) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    bool ok = true;
    for (const FileEntry &file : files) {
        if (file.strAlias.isEmpty()) {
            ok = false;
            continue;
        }

        const zip_int64_t index = zip_name_locate(archive, file.strFullPath.toUtf8().constData(), 0);
        if (index < 0 || zip_file_rename(archive, zip_uint64_t(index),
                                         file.strAlias.toUtf8().constData(), 0) != 0) {
            ok = false;
        }
    }

    if (zip_close(archive) != 0) {
        m_eErrorType = ET_RenameError;
        ok = false;
    }

    emit signalFinished(ok ? PFT_Nomral : PFT_Error);
    return ok ? PFT_Nomral : PFT_Error;
}

PluginFinishType LibzipBackend::addComment(const QString &comment)
{
    setPassword(QString());
    m_workStatus = WT_Comment;

    int errcode = 0;
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_CREATE, &errcode);
    if (!archive)
        return PFT_Error;

    const QByteArray bytes = comment.toUtf8();
    if (zip_set_archive_comment(archive, bytes.constData(), zip_uint16_t(bytes.size())) != 0) {
        zip_close(archive);
        return PFT_Error;
    }

    if (zip_close(archive) != 0) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    m_comment = comment;
    DataManager::get_instance().archiveData().strComment = comment;
    return PFT_Nomral;
}

PluginFinishType LibzipBackend::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    m_workStatus = WT_Move;
    return PFT_Nomral;
}

PluginFinishType LibzipBackend::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    m_workStatus = WT_Copy;
    return PFT_Nomral;
}

PluginFinishType LibzipBackend::updateArchiveData(const UpdateOptions &options)
{
    Q_UNUSED(options)

    // 写操作完成后重新加载包内数据，界面据此刷新
    m_codeMap.clear();
    m_setHasHandlesDirs.clear();
    m_setHasRootDirs.clear();

    return list();
}
