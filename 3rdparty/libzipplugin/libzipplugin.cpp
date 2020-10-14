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
#include "libzipplugin.h"

#include <QDebug>


LibzipPluginFactory::LibzipPluginFactory()
{
    registerPlugin<LibzipPlugin>();
}

LibzipPluginFactory::~LibzipPluginFactory()
{

}



LibzipPlugin::LibzipPlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
    qDebug() << "LibzipPlugin";
    m_ePlugintype = PT_Libzip;
}

LibzipPlugin::~LibzipPlugin()
{

}

bool LibzipPlugin::list()
{
    return true;
}

bool LibzipPlugin::testArchive()
{
    return true;
}

bool LibzipPlugin::extractFiles(const QVector<FileEntry> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    return true;
}

bool LibzipPlugin::addFiles(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options)
{
    return true;
}

bool LibzipPlugin::moveFiles(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options)
{
    return true;
}

bool LibzipPlugin::copyFiles(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options)
{
    return true;
}

bool LibzipPlugin::deleteFiles(const QVector<FileEntry> &files)
{
    return true;
}

bool LibzipPlugin::addComment(const QString &comment)
{
    return true;
}

