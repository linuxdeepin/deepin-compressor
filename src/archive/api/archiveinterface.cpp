// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archiveinterface.h"
#include "datamanager.h"
#include "ddlog.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStorageInfo>

#include <sys/stat.h>

Q_DECLARE_METATYPE(KPluginMetaData)

ReadOnlyArchiveInterface::ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    if (args.count() == 3) {
        m_strArchiveName = args.at(0).toString();
        m_metaData = args.at(1).value<KPluginMetaData>();
        m_mimetype = args.at(2).value<CustomMimeType>();
    }

    m_common = new Common(this);
}

ReadOnlyArchiveInterface::~ReadOnlyArchiveInterface()
{
}

bool ReadOnlyArchiveInterface::waitForFinished()
{
    return m_bWaitForFinished;
}

void ReadOnlyArchiveInterface::setWaitForFinishedSignal(bool value)
{
    m_bWaitForFinished = value;
}

void ReadOnlyArchiveInterface::setPassword(const QString &strPassword)
{
    m_strPassword = strPassword;
}

QString ReadOnlyArchiveInterface::getPassword()
{
    return m_strPassword;
}

ErrorType ReadOnlyArchiveInterface::errorType()
{
    return m_eErrorType;
}

bool ReadOnlyArchiveInterface::doKill()
{
    return false;
}

bool ReadOnlyArchiveInterface::status()
{
    return m_bPause;
}

QFileDevice::Permissions ReadOnlyArchiveInterface::getPermissions(const mode_t &perm)
{
    QFileDevice::Permissions pers;

    // 0 与 0644 都按无效值处理（zip 外部属性的历史问题）
    if (perm == 0 || perm == 0644) {
        pers |= QFileDevice::ReadUser | QFileDevice::WriteUser
              | QFileDevice::ReadGroup | QFileDevice::ReadOther;
        return pers;
    }

    if (perm & S_IRUSR)
        pers |= QFileDevice::ReadUser;
    if (perm & S_IWUSR)
        pers |= QFileDevice::WriteUser;
    if (perm & S_IXUSR)
        pers |= QFileDevice::ExeUser;

    if (perm & S_IRGRP)
        pers |= QFileDevice::ReadGroup;
    if (perm & S_IWGRP)
        pers |= QFileDevice::WriteGroup;
    if (perm & S_IXGRP)
        pers |= QFileDevice::ExeGroup;

    if (perm & S_IROTH)
        pers |= QFileDevice::ReadOther;
    if (perm & S_IWOTH)
        pers |= QFileDevice::WriteOther;
    if (perm & S_IXOTH)
        pers |= QFileDevice::ExeOther;

    return pers;
}

void ReadOnlyArchiveInterface::handleEntry(const FileEntry &entry)
{
    ArchiveData &data = DataManager::get_instance().archiveData();
    const QChar sep = QDir::separator();

    const bool isRootLevel = !entry.strFullPath.contains(sep)
            || (entry.strFullPath.count(sep) == 1 && entry.strFullPath.endsWith(sep));

    if (isRootLevel) {
        if (m_setHasRootDirs.contains(entry.strFullPath)) {
            // 同一路径出现多次时以最后一次为准
            for (int i = 0; i < data.listRootEntry.size(); ++i) {
                if (data.listRootEntry.at(i).strFullPath == entry.strFullPath) {
                    data.listRootEntry[i] = entry;
                    break;
                }
            }
        } else {
            data.listRootEntry.push_back(entry);
            m_setHasRootDirs.insert(entry.strFullPath);
        }
        return;
    }

    const int lastSep = entry.strFullPath.lastIndexOf(sep);
    const QString parentDir = entry.strFullPath.left(lastSep + 1);
    if (m_setHasHandlesDirs.contains(parentDir))
        return;

    m_setHasHandlesDirs.insert(parentDir);

    // 压缩包未显式列出目录项时，按路径补出缺失的每一级目录
    const QStringList parts = entry.strFullPath.split(sep);
    QString dirPath;
    for (int i = 0; i < parts.size() - 1; ++i) {
        dirPath += parts.at(i) + sep;

        FileEntry dirEntry;
        dirEntry.strFullPath = dirPath;
        dirEntry.strFileName = parts.at(i);
        dirEntry.isDirectory = true;

        if (i == 0 && !m_setHasRootDirs.contains(dirPath)) {
            data.listRootEntry.push_back(dirEntry);
            m_setHasRootDirs.insert(dirPath);
        }

        if (!data.mapFileEntry.contains(dirPath))
            data.mapFileEntry.insert(dirPath, dirEntry);
    }
}

bool ReadOnlyArchiveInterface::isInsufficientDiskSpace(QString diskPath, qint64 standard)
{
    QStorageInfo storage(QFileInfo(diskPath).absolutePath());
    qCDebug(ifaceLog) << "available disk space" << diskPath << storage.bytesAvailable();
    return storage.isValid() && storage.bytesAvailable() < standard;
}

ReadWriteArchiveInterface::ReadWriteArchiveInterface(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{
}

ReadWriteArchiveInterface::~ReadWriteArchiveInterface()
{
}

QString ReadWriteArchiveInterface::getArchiveName()
{
    return m_strArchiveName;
}
