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
#include "archivemanager.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "singlejob.h"

#include <QMimeDatabase>
#include <QFileInfo>
#include <QDebug>

Q_DECLARE_METATYPE(KPluginMetaData)

ArchiveManager::ArchiveManager(QObject *parent)
    : QObject(parent)
{

}

ArchiveManager::~ArchiveManager()
{

}

void ArchiveManager::createArchive(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options, bool useLibArchive, bool bBatch)
{
    ReadOnlyArchiveInterface *pInterface = createInterface(strDestination, false, useLibArchive);

    if (bBatch) {       // 批量压缩（多路径）

    } else {            // 单路径压缩
        CreateJob *pCreateJob = new CreateJob(files, pInterface, this);

        // 连接槽函数


        m_pArchiveJob = pCreateJob;
        pCreateJob->start();
    }
}

ReadOnlyArchiveInterface *ArchiveManager::createInterface(const QString &fileName, bool bWrite, bool bUseLibArchive)
{
    QFileInfo fileinfo(fileName);

    const QMimeType mimeType = determineMimeType(fileName);

    QVector<Plugin *> offers;
    if (bWrite) {
        offers = PluginManager::get_instance().preferredWritePluginsFor(mimeType);

        if (bUseLibArchive == true && mimeType.name() == "application/zip") {
            std::sort(offers.begin(), offers.end(), [](Plugin * p1, Plugin * p2) {
                if (p1->metaData().name().contains("Libarchive")) {
                    return true;
                }
                if (p2->metaData().name().contains("Libarchive")) {
                    return false;
                }

                return p1->priority() > p2->priority();
            });
        }
    } else {
        offers = PluginManager::get_instance().preferredPluginsFor(mimeType);
    }

    if (offers.isEmpty()) {
        qDebug() << "Could not find a plugin to handle" << fileName;
        return nullptr;
    }

    //tar.lzo格式 由P7zip插件压缩mimeFromContent为"application/x-7z-compressed"，由Libarchive插件压缩mimeFromContent为"application/x-lzop"
    //删除P7zip插件处理 mimeFromContent为"application/x-lzop" 的情况
    QMimeDatabase db;
    QMimeType mimeFromContent = db.mimeTypeForFile(fileName, QMimeDatabase::MatchContent);
    bool remove7zFlag = "application/x-tzo" == mimeType.name() && "application/x-lzop" == mimeFromContent.name();

    ReadOnlyArchiveInterface *pIface = nullptr;
    for (Plugin *plugin : offers) {
        //删除P7zip插件
        if (remove7zFlag && plugin->metaData().name().contains("P7zip")) {
            offers.removeOne(plugin);
            continue;
        }

        pIface = createInterface(fileName, plugin);
        // Use the first valid plugin, according to the priority sorting.
        if (pIface) {
            break;
        }
    }

    return pIface;
}

ReadOnlyArchiveInterface *ArchiveManager::createInterface(const QString &fileName, Plugin *plugin)
{
    Q_ASSERT(plugin);

    KPluginFactory *factory = KPluginLoader(plugin->metaData().fileName()).factory();
    if (!factory) {
        return nullptr;
    }

    const QVariantList args = {QVariant(QFileInfo(fileName).absoluteFilePath()),
                               QVariant().fromValue(plugin->metaData())
                              };

    ReadOnlyArchiveInterface *iface = factory->create<ReadOnlyArchiveInterface>(nullptr, args);
    return iface;
}

