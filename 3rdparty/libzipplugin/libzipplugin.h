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
#ifndef LIBZIPPLUGIN_H
#define LIBZIPPLUGIN_H

#include "kpluginfactory.h"
#include "archiveinterface.h"

#include <QObject>


class LibzipPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libzip.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibzipPluginFactory();
    ~LibzipPluginFactory() override;
};

class LibzipPlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibzipPlugin(QObject *parent, const QVariantList &args);
    ~LibzipPlugin() override;


    // ReadOnlyArchiveInterface interface
public:
    bool list() override;
    bool testArchive() override;
    bool extractFiles(const QVector<FileEntry> &files, const QString &destinationDirectory, const ExtractionOptions &options) override;

    // ReadWriteArchiveInterface interface
public:
    bool addFiles(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options) override;
    bool moveFiles(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options) override;
    bool copyFiles(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options) override;
    bool deleteFiles(const QVector<FileEntry> &files) override;
    bool addComment(const QString &comment) override;
};

#endif // LIBZIPPLUGIN_H
