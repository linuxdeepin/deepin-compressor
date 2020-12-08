/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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
#include "archiveinterface.h"
#include "datamanager.h"

#include <QDebug>
#include <sys/stat.h>
#include <QDir>

Q_DECLARE_METATYPE(KPluginMetaData)


ReadOnlyArchiveInterface::ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    Q_ASSERT(args.size() >= 3);
    qDebug() << "Created read-only interface for" << args.first().toString();
    m_strArchiveName = args.first().toString();
    m_metaData = args.at(1).value<KPluginMetaData>();
    m_mimetype = args.at(2).value<QMimeType>();
    m_common = new Common(this);
}

ReadOnlyArchiveInterface::~ReadOnlyArchiveInterface()
{

}

bool ReadOnlyArchiveInterface::waitForFinished()
{
    return m_bWaitForFinished;
}

void ReadOnlyArchiveInterface::setPassword(const QString &strPassword)
{
    m_strPassword = strPassword;
}

ErrorType ReadOnlyArchiveInterface::errorType()
{
    return m_eErrorType;
}

bool ReadOnlyArchiveInterface::doKill()
{
    return false;   // 修改默认为未取消
}

void ReadOnlyArchiveInterface::setWaitForFinishedSignal(bool value)
{
    m_bWaitForFinished = value;
}

QFileDevice::Permissions ReadOnlyArchiveInterface::getPermissions(const mode_t &perm)
{
    QFileDevice::Permissions pers = QFileDevice::Permissions();

    if (perm == 0) {
        pers |= (QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        return pers;
    }

    if (perm & S_IRUSR) {
        pers |= QFileDevice::ReadUser;
    }
    if (perm & S_IWUSR) {
        pers |= QFileDevice::WriteUser;
    }
    if (perm & S_IXUSR) {
        pers |= QFileDevice::ExeUser;
    }

    if (perm & S_IRGRP) {
        pers |= QFileDevice::ReadGroup;
    }
    if (perm & S_IWGRP) {
        pers |= QFileDevice::WriteGroup;
    }
    if (perm & S_IXGRP) {
        pers |= QFileDevice::ExeGroup;
    }

    if (perm & S_IROTH) {
        pers |= QFileDevice::ReadOther;
    }
    if (perm & S_IWOTH) {
        pers |= QFileDevice::WriteOther;
    }
    if (perm & S_IXOTH) {
        pers |= QFileDevice::ExeOther;
    }

    return pers;
}

void ReadOnlyArchiveInterface::handleEntry(const FileEntry &entry)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    if (!entry.strFullPath.contains(QDir::separator()) || (entry.strFullPath.count(QDir::separator()) == 1 && entry.strFullPath.endsWith(QDir::separator()))) {
        if (m_setHasRootDirs.contains(entry.strFullPath)) {
            // 先清除，再追加，保证数据和压缩包数据一致
            for (int i = 0; i < stArchiveData.listRootEntry.count(); ++i) {
                if (stArchiveData.listRootEntry[i].strFullPath == entry.strFullPath) {
                    stArchiveData.listRootEntry.reserve(i);
                    break;
                }
            }
        } else {
            // 获取第一层数据（不包含'/'或者只有末尾一个'/'）
            stArchiveData.listRootEntry.push_back(entry);
            m_setHasRootDirs.insert(entry.strFullPath);
        }
    } else {
        // 多层数据，处理加载时不出现文件夹的情况
        int iIndex = entry.strFullPath.lastIndexOf(QDir::separator());
        QString strDir = entry.strFullPath.left(iIndex + 1);

        // 若此路径未处理过，进行分割处理
        if (!m_setHasHandlesDirs.contains(strDir)) {
            m_setHasHandlesDirs.insert(strDir);
            // 对全路径进行分割，获取所有文件夹名称
            QStringList fileDirs = entry.strFullPath.split(QDir::separator());
            QString folderAppendStr = "";
            for (int i = 0 ; i < fileDirs.size() - 1; ++i) {
                folderAppendStr += fileDirs[i] + QDir::separator();

                // 构建文件数据
                FileEntry entryDir;
                entryDir.strFullPath = folderAppendStr;
                entryDir.strFileName = fileDirs[i];
                entryDir.isDirectory = true;

                // 若第一层数据中未包含此文件夹，写入第一层数据
                if (i == 0 && !m_setHasRootDirs.contains(folderAppendStr)) {
                    stArchiveData.listRootEntry.push_back(entryDir);
                    m_setHasRootDirs.insert(folderAppendStr);
                }

                // 写入数据全集中
                stArchiveData.mapFileEntry[entryDir.strFullPath] = entryDir;
            }
        }
    }
}

bool ReadOnlyArchiveInterface::getHandleCurEntry() const
{
    return m_bHandleCurEntry;
}

//void ReadOnlyArchiveInterface::getFileEntry(QList<FileEntry> &listRootEntry, QMap<QString, FileEntry> &mapEntry)
//{
//    listRootEntry.clear();
//    mapEntry.clear();

//    listRootEntry = m_listRootEntry;
//    mapEntry = m_mapEntry;
//}



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
