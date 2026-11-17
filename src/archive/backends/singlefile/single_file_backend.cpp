// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "single_file_backend.h"

#include "datamanager.h"
#include "queries.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include <linux/limits.h>
#include <unistd.h>

namespace {

constexpr int kChunkSize = 16 * 1024;

} // namespace

SingleFileBackend::SingleFileBackend(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{
    const QString mime = m_mimetype.name();

    if (mime == QLatin1String("application/gzip") || mime == QLatin1String("application/x-gzip")) {
        m_codec = Codec::Gzip;
        m_suffixes = {QStringLiteral(".gz"), QStringLiteral(".svgz")};
    } else if (mime == QLatin1String("application/x-bzip") || mime == QLatin1String("application/x-bzip2")) {
        m_codec = Codec::Bzip2;
        m_suffixes = {QStringLiteral(".bz2")};
    } else if (mime == QLatin1String("application/x-xz")) {
        m_codec = Codec::Xz;
        m_suffixes = {QStringLiteral(".xz")};
    } else if (mime == QLatin1String("application/x-lzma")) {
        m_codec = Codec::Lzma;
        m_suffixes = {QStringLiteral(".lzma")};
    } else {
        m_codec = Codec::Gzip;
        m_suffixes = {QStringLiteral(".gz")};
    }
}

SingleFileBackend::~SingleFileBackend()
{
}

QString SingleFileBackend::uncompressedFileName() const
{
    QString name = QFileInfo(m_strArchiveName).fileName();

    if (name.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)) {
        name.chop(1);
        return name;
    }

    for (const QString &suffix : m_suffixes) {
        if (name.endsWith(suffix, Qt::CaseInsensitive)) {
            name.chop(suffix.size());
            return name;
        }
    }

    return name + QStringLiteral(".uncompressed");
}

PluginFinishType SingleFileBackend::list()
{
    DataManager::get_instance().resetArchiveData();
    ArchiveData &data = DataManager::get_instance().archiveData();

    FileEntry entry;
    entry.strFullPath = uncompressedFileName();
    entry.strFileName = entry.strFullPath;
    entry.qSize = QFileInfo(m_strArchiveName).size();
    entry.uLastModifiedTime =
            static_cast<uint>(QFileInfo(m_strArchiveName).lastModified().toSecsSinceEpoch());

    data.qSize = entry.qSize;
    data.qComressSize = entry.qSize;
    data.listRootEntry.push_back(entry);
    data.mapFileEntry.insert(entry.strFullPath, entry);

    return PFT_Nomral;
}

PluginFinishType SingleFileBackend::testArchive()
{
    m_workStatus = WT_Test;
    return PFT_Nomral;
}

PluginFinishType SingleFileBackend::extractFiles(const QList<FileEntry> &files,
                                                 const ExtractionOptions &options)
{
    Q_UNUSED(files)

    const QString fileName = uncompressedFileName();

    QString outputPath = options.strTargetPath;
    if (outputPath.endsWith(fileName))
        outputPath.chop(fileName.length());
    if (!outputPath.endsWith(QLatin1Char('/')))
        outputPath += QLatin1Char('/');
    if (!QDir().exists(outputPath))
        QDir().mkpath(outputPath);
    outputPath += fileName;

    if (!options.bExistList) {
        FileEntry entry;
        entry.strFullPath = fileName;
        DataManager::get_instance().archiveData().listRootEntry << entry;
    }

    QFile output(outputPath);
    if (output.exists()) {
        OverwriteQuery query(outputPath);
        emit signalQuery(&query);
        query.waitForResponse();

        if (query.responseCancelled()) {
            emit signalCancel();
            return PFT_Cancel;
        }
        if (query.responseSkip() || query.responseSkipAll()) {
            m_bSkipAll = query.responseSkipAll();
            return PFT_Cancel;
        }
        if (query.responseOverwriteAll())
            m_bOverwriteAll = true;
    }

    if (!output.open(QIODevice::WriteOnly)) {
        const QStringList parts = outputPath.split(QLatin1Char('/'));
        for (const QString &part : parts) {
            if (NAME_MAX < part.toLocal8Bit().length()) {
                m_eErrorType = ET_LongNameError;
                return PFT_Error;
            }
        }
        emit signalFileWriteErrorName(QFileInfo(outputPath).fileName());
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    Decoder decoder;
    if (!decoder.open(m_strArchiveName, m_codec)) {
        emit signalFileWriteErrorName(QFileInfo(outputPath).fileName());
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    m_extractedSize = 0;
    emit signalCurFileName(fileName);

    QByteArray chunk(kChunkSize, '\0');
    while (true) {
        if (QThread::currentThread()->isInterruptionRequested())
            break;

        if (m_bPause) {
            QThread::sleep(1);
            continue;
        }

        const qint64 bytesRead = decoder.read(chunk.data(), chunk.size());

        m_extractedSize += bytesRead;
        if (options.qSize > 0)
            emit signalprogress(static_cast<double>(m_extractedSize) / options.qSize * 100);
        else
            emit signalprogress(0);

        if (bytesRead < 0) {
            emit signalFileWriteErrorName(QFileInfo(outputPath).fileName());
            m_eErrorType = ET_FileWriteError;
            break;
        }
        if (bytesRead == 0)
            break;

        if (output.write(chunk.constData(), bytesRead) != bytesRead) {
            if (isInsufficientDiskSpace(options.strTargetPath, bytesRead))
                m_eErrorType = ET_InsufficientDiskSpace;
            break;
        }
    }

    output.close();
    decoder.close();

    return m_eErrorType == ET_NoError ? PFT_Nomral : PFT_Error;
}

void SingleFileBackend::pauseOperation()
{
    m_bPause = true;
}

void SingleFileBackend::continueOperation()
{
    m_bPause = false;
}

bool SingleFileBackend::doKill()
{
    m_bPause = false;
    return false;
}
