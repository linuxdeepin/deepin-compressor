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

//LibarchivePlugin::LibarchivePlugin(QObject *parent, const QVariantList &args){}

LibarchivePlugin::LibarchivePlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{

//    connect(this, &ReadOnlyArchiveInterface::error, this, &LibarchivePlugin::slotRestoreWorkingDir);
//    connect(this, &ReadOnlyArchiveInterface::cancelled, this, &LibarchivePlugin::slotRestoreWorkingDir);
}

LibarchivePlugin::~LibarchivePlugin()
{

}

bool LibarchivePlugin::list()
{
    return true;
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
