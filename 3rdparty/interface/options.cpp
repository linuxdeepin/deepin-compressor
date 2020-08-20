/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#include "options.h"
#include "structs.h"

bool Options::encryptedArchiveHint() const
{
    return m_encryptedArchiveHint;
}

void Options::setEncryptedArchiveHint(bool encrypted)
{
    m_encryptedArchiveHint = encrypted;
}

bool ExtractionOptions::preservePaths() const
{
    return m_preservePaths;
}

void ExtractionOptions::setPreservePaths(bool preservePaths)
{
    m_preservePaths = preservePaths;
}

bool ExtractionOptions::isDragAndDropEnabled() const
{
    return m_dragAndDrop;
}

void ExtractionOptions::setDragAndDropEnabled(bool enabled)
{
    m_dragAndDrop = enabled;
}

bool ExtractionOptions::alwaysUseTempDir() const
{
    return m_alwaysUseTempDir;
}

void ExtractionOptions::setAlwaysUseTempDir(bool alwaysUseTempDir)
{
    m_alwaysUseTempDir = alwaysUseTempDir;
}

void ExtractionOptions::setBatchExtract(bool status)
{
    m_isBatch = status;
}

bool ExtractionOptions::isAutoCreatDir() const
{
    return m_isAutoCreatDir;
}

void ExtractionOptions::setAutoCreatDir(bool bAutoCreatDir)
{
    m_isAutoCreatDir = bAutoCreatDir;
}

bool ExtractionOptions::isRightMenuExtractHere() const
{
    return  m_isRightMenuExtractHere;
}

void ExtractionOptions::setRightMenuExtractHere(bool rightMenuExtractHere)
{
    m_isRightMenuExtractHere = rightMenuExtractHere;
}

bool ExtractionOptions::isBatchExtract()
{
    return m_isBatch;
}

bool CompressionOptions::isCompressionLevelSet() const
{
    return compressionLevel() != -1;
}

bool CompressionOptions::isVolumeSizeSet() const
{
    return volumeSize() > 0;
}

int CompressionOptions::compressionLevel() const
{
    return m_compressionLevel;
}

void CompressionOptions::setCompressionLevel(int level)
{
    m_compressionLevel = level;
}

ulong CompressionOptions::volumeSize() const
{
    return m_volumeSize;
}

void CompressionOptions::setVolumeSize(ulong size)
{
    m_volumeSize = size;
}

QString CompressionOptions::compressionMethod() const
{
    return m_compressionMethod;
}

void CompressionOptions::setCompressionMethod(const QString &method)
{
    m_compressionMethod = method;
}

QString CompressionOptions::encryptionMethod() const
{
    return m_encryptionMethod;
}

void CompressionOptions::setEncryptionMethod(const QString &method)
{
    m_encryptionMethod = method;
}

QString CompressionOptions::globalWorkDir() const
{
    return m_globalWorkDir;
}

void CompressionOptions::setGlobalWorkDir(const QString &workDir)
{
    m_globalWorkDir = workDir;
}

bool CompressionOptions::isTar7z() const
{
    return m_isTar7z;
}

void CompressionOptions::setIsTar7z(bool bTar7z)
{
    m_isTar7z = bTar7z;
}

qint64 CompressionOptions::getfilesSize() const
{
    return m_filesSize;
}

void CompressionOptions::setFilesSize(const qint64 &filesSize)
{
    m_filesSize = filesSize / 1024 / 1024;
    if (m_filesSize < 1) {
        m_filesSize = 1;
    }
}

QDebug operator<<(QDebug d, const CompressionOptions &options)
{
    d.nospace() << "(encryption hint: " << options.encryptedArchiveHint();
    if (!options.compressionMethod().isEmpty()) {
        d.nospace() << ", compression method: " << options.compressionMethod();
    }
    if (!options.encryptionMethod().isEmpty()) {
        d.nospace() << ", encryption method: " << options.encryptionMethod();
    }
    if (!options.globalWorkDir().isEmpty()) {
        d.nospace() << ", global work dir: " << options.globalWorkDir();
    }
    d.nospace() << ", compression level: " << options.compressionLevel();
    d.nospace() << ", volume size: " << options.volumeSize();
    d.nospace() << ")";
    return d.space();
}

QDebug operator<<(QDebug d, const ExtractionOptions &options)
{
    d.nospace() << "(encryption hint: " << options.encryptedArchiveHint();
    d.nospace() << ", preserve paths: " << options.preservePaths();
    d.nospace() << ", drag and drop: " << options.isDragAndDropEnabled();
    d.nospace() << ", always temp dir: " << options.alwaysUseTempDir();
    d.nospace() << ")";
    return d.space();
}
