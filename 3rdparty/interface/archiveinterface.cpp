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

#include <QDebug>
#include <sys/stat.h>

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

void ReadOnlyArchiveInterface::getArchiveData(ArchiveData &stArchiveData)
{
    stArchiveData = m_stArchiveData;
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
