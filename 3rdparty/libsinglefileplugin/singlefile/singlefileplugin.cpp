/*
 * Copyright (c) 2009  Raphael Kubo da Costa <rakuco@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "singlefileplugin.h"
#include "queries.h"
#include "datamanager.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

#include <KFilterDev>
//#include <KLocalizedString>

LibSingleFileInterface::LibSingleFileInterface(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{
}

LibSingleFileInterface::~LibSingleFileInterface()
{
}

PluginFinishType LibSingleFileInterface::list()
{
    DataManager::get_instance().resetArchiveData();

    FileEntry entry;
    entry.strFullPath = uncompressedFileName();
    entry.strFileName = entry.strFullPath;
    entry.qSize = QFileInfo(m_strArchiveName).size();

    DataManager::get_instance().archiveData().listRootEntry.push_back(entry);
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

    return PFT_Nomral;
}

PluginFinishType LibSingleFileInterface::testArchive()
{
    m_workStatus = WT_Test;
    return PFT_Nomral;
}

PluginFinishType LibSingleFileInterface::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)

    QString strFileName = uncompressedFileName();       // 获取文件名

    // 若自动创建文件夹，祛除以文件名结尾的字符串，方式解压时文件名包含“xx.xx“，导致解压失败
    QString outputFileName = options.strTargetPath;
    if (outputFileName.endsWith(uncompressedFileName()))
        outputFileName.chop(strFileName.length());

    // 解压路径
    if (!outputFileName.endsWith(QLatin1Char('/'))) {
        outputFileName += QLatin1Char('/');
    }

    // 判断解压路径是否存在，不存在则创建文件夹
    if (QDir().exists(outputFileName) == false)
        QDir().mkpath(outputFileName);

    outputFileName += uncompressedFileName();   // 完整文件路径

    // 对重复文件进行询问判断
    if (QFile::exists(outputFileName)) {
        OverwriteQuery query(outputFileName);

        emit signalQuery(&query);
        query.waitForResponse();

        if (query.responseCancelled()) {
            emit signalCancel();
            return PFT_Cancel;
        } else if (query.responseSkip()) {
            return PFT_Cancel;
        } else if (query.responseSkipAll()) {
            m_bSkipAll = true;
            return PFT_Cancel;
        }  else if (query.responseOverwriteAll()) {
            m_bOverwriteAll = true;
        }
    }

    qDebug() << "Extracting to" << outputFileName;


    // 写文件
    QFile outputFile(outputFileName);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    // 打开压缩设备，写入数据
    KCompressionDevice *device = new KCompressionDevice(m_strArchiveName, KFilterDev::compressionTypeForMimeType(m_mimeType));
    if (!device) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    device->open(QIODevice::ReadOnly);  // 以只读方式打开

    qint64 bytesRead;
    QByteArray dataChunk(1024 * 16, '\0'); // 16Kb

    // 写数据
    while (true) {
        bytesRead = device->read(dataChunk.data(), dataChunk.size());

        if (bytesRead == -1) {
            m_eErrorType = ET_FileWriteError;
            break;
        } else if (bytesRead == 0) {
            break;
        }

        outputFile.write(dataChunk.data(), bytesRead);
    }

    outputFile.close();
    device->close();
    delete device;

    return PFT_Nomral;
}

void LibSingleFileInterface::pauseOperation()
{

}

void LibSingleFileInterface::continueOperation()
{

}

bool LibSingleFileInterface::doKill()
{
    return false;
}

const QString LibSingleFileInterface::uncompressedFileName() const
{
    QString uncompressedName(QFileInfo(m_strArchiveName).fileName());

    // Bug 252701: For .svgz just remove the terminal "z".
    if (uncompressedName.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)) {
        uncompressedName.chop(1);
        return uncompressedName;
    }

    for (const QString &extension : qAsConst(m_possibleExtensions)) {

        if (uncompressedName.endsWith(extension, Qt::CaseInsensitive)) {
            uncompressedName.chop(extension.size());
            return uncompressedName;
        }
    }

    return uncompressedName + QStringLiteral(".uncompressed");
}
