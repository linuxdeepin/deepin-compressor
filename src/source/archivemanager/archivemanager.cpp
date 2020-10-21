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
    if (m_pArchiveJob != nullptr) {
        delete m_pArchiveJob;
        m_pArchiveJob = nullptr;
    }

    if (m_pInterface != nullptr) {
        delete m_pInterface;
        m_pInterface = nullptr;
    }

}

ArchiveJob *ArchiveManager::archiveJob()
{
    return m_pArchiveJob;
}

void ArchiveManager::createArchive(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options, bool useLibArchive, bool bBatch)
{
    ReadOnlyArchiveInterface *pInterface = createInterface(strDestination, true, useLibArchive);

    if (bBatch) {       // 批量压缩（多路径）
        CreateJob *pCreateJob = new CreateJob(files, pInterface, options, this);

        // 连接槽函数
        connect(pCreateJob, &CreateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);


        m_pArchiveJob = pCreateJob;
        pCreateJob->start();
    } else {            // 单路径压缩
        CreateJob *pCreateJob = new CreateJob(files, pInterface, options, this);

        // 连接槽函数
        connect(pCreateJob, &CreateJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
        connect(pCreateJob, &CreateJob::signalprogress, this, &ArchiveManager::signalprogress);
        connect(pCreateJob, &CreateJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);


        m_pArchiveJob = pCreateJob;
        pCreateJob->start();
    }
}

void ArchiveManager::loadArchive(const QString &strArchiveName)
{
    m_pInterface = createInterface(strArchiveName);

    LoadJob *pLoadJob = new LoadJob(m_pInterface);

    // 连接槽函数
    connect(pLoadJob, &LoadJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);

    m_pArchiveJob = pLoadJob;
    pLoadJob->start();
}

void ArchiveManager::getLoadArchiveData(ArchiveData &stArchiveData)
{
    if (m_pInterface != nullptr) {
        m_pInterface->getArchiveData(stArchiveData);
    }
}

void ArchiveManager::extractArchive(const QVector<FileEntry> &files, const QString &strArchiveName, const ExtractionOptions &options)
{
    if (m_pInterface == nullptr) {
        m_pInterface = createInterface(strArchiveName);
    }

    ExtractJob *pExtractJob = new ExtractJob(files, m_pInterface, options);

    // 连接槽函数
    connect(pExtractJob, &ExtractJob::signalJobFinshed, this, &ArchiveManager::slotJobFinished);
    connect(pExtractJob, &ExtractJob::signalprogress, this, &ArchiveManager::signalprogress);
    connect(pExtractJob, &ExtractJob::signalCurFileName, this, &ArchiveManager::signalCurFileName);
    connect(pExtractJob, &ExtractJob::signalQuery, this, &ArchiveManager::signalQuery);


    pExtractJob->start();
    m_pArchiveJob = pExtractJob;
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

        pIface = createInterface(fileName, mimeType, plugin);
        // Use the first valid plugin, according to the priority sorting.
        if (pIface) {
            break;
        }
    }

    return pIface;
}

ReadOnlyArchiveInterface *ArchiveManager::createInterface(const QString &fileName, const QMimeType &mimeType, Plugin *plugin)
{
    Q_ASSERT(plugin);

    KPluginFactory *factory = KPluginLoader(plugin->metaData().fileName()).factory();
    if (!factory) {
        return nullptr;
    }

    const QVariantList args = {QVariant(QFileInfo(fileName).absoluteFilePath()),
                               QVariant().fromValue(plugin->metaData()),
                               QVariant::fromValue(mimeType)
                              };

    ReadOnlyArchiveInterface *iface = factory->create<ReadOnlyArchiveInterface>(nullptr, args);
    return iface;
}

void ArchiveManager::slotJobFinished()
{
    emit signalJobFinished();

    if (m_pArchiveJob != nullptr) {
        m_pArchiveJob->deleteLater();
        m_pArchiveJob = nullptr;
    }
}
