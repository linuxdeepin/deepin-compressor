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
#include "archiveformat.h"

#include <QJsonArray>

ArchiveFormat::ArchiveFormat()
{
}

ArchiveFormat::ArchiveFormat(const QMimeType &mimeType,
                             EncryptionType encryptionType,
                             int minCompLevel,
                             int maxCompLevel,
                             int defaultCompLevel,
                             bool supportsWriteComment,
                             bool supportsTesting,
                             bool supportsMultiVolume,
                             const QVariantMap &compressionMethods,
                             const QString &defaultCompressionMethod,
                             const QStringList &encryptionMethods,
                             const QString &defaultEncryptionMethod)
    : m_mimeType(mimeType)
    , m_encryptionType(encryptionType)
    , m_minCompressionLevel(minCompLevel)
    , m_maxCompressionLevel(maxCompLevel)
    , m_defaultCompressionLevel(defaultCompLevel)
    , m_supportsWriteComment(supportsWriteComment)
    , m_supportsTesting(supportsTesting)
    , m_supportsMultiVolume(supportsMultiVolume)
    , m_compressionMethods(compressionMethods)
    , m_defaultCompressionMethod(defaultCompressionMethod)
    , m_encryptionMethods(encryptionMethods)
    , m_defaultEncryptionMethod(defaultEncryptionMethod)
{
}

ArchiveFormat ArchiveFormat::fromMetadata(const QMimeType &mimeType, const KPluginMetaData &metadata)
{
    const QJsonObject json = metadata.rawData();
    const QStringList mimeTypes = metadata.mimeTypes();
    for (const QString &mime : mimeTypes) {
        if (mimeType.name() != mime) {
            continue;
        }

        const QJsonObject formatProps = json[mime].toObject();

        int minCompLevel = formatProps[QStringLiteral("CompressionLevelMin")].toInt();
        int maxCompLevel = formatProps[QStringLiteral("CompressionLevelMax")].toInt();
        int defaultCompLevel = formatProps[QStringLiteral("CompressionLevelDefault")].toInt();

        bool supportsWriteComment = formatProps[QStringLiteral("SupportsWriteComment")].toBool();
        bool supportsTesting = formatProps[QStringLiteral("SupportsTesting")].toBool();
        bool supportsMultiVolume = formatProps[QStringLiteral("SupportsMultiVolume")].toBool();

        QVariantMap compressionMethods = formatProps[QStringLiteral("CompressionMethods")].toObject().toVariantMap();
        QString defaultCompMethod = formatProps[QStringLiteral("CompressionMethodDefault")].toString();

        // We use a QStringList instead of QVariantMap for encryption methods, to
        // allow arbitrary ordering of the items.
        QStringList encryptionMethods;
        const QJsonArray array = formatProps[QStringLiteral("EncryptionMethods")].toArray();
        for (const QJsonValue &value : array) {
            encryptionMethods.append(value.toString());
        }

        QString defaultEncMethod = formatProps[QStringLiteral("EncryptionMethodDefault")].toString();

        EncryptionType encType = Unencrypted;
        if (formatProps[QStringLiteral("HeaderEncryption")].toBool()) {
            encType = HeaderEncrypted;
        } else if (formatProps[QStringLiteral("Encryption")].toBool()) {
            encType = Encrypted;
        }

        return ArchiveFormat(mimeType,
                             encType,
                             minCompLevel,
                             maxCompLevel,
                             defaultCompLevel,
                             supportsWriteComment,
                             supportsTesting,
                             supportsMultiVolume,
                             compressionMethods,
                             defaultCompMethod,
                             encryptionMethods,
                             defaultEncMethod);
    }

    return ArchiveFormat();
}

bool ArchiveFormat::isValid() const
{
    return m_mimeType.isValid();
}

EncryptionType ArchiveFormat::encryptionType() const
{
    return m_encryptionType;
}

int ArchiveFormat::minCompressionLevel() const
{
    return m_minCompressionLevel;
}

int ArchiveFormat::maxCompressionLevel() const
{
    return m_maxCompressionLevel;
}

int ArchiveFormat::defaultCompressionLevel() const
{
    return m_defaultCompressionLevel;
}

bool ArchiveFormat::supportsWriteComment() const
{
    return m_supportsWriteComment;
}

bool ArchiveFormat::supportsTesting() const
{
    return m_supportsTesting;
}

bool ArchiveFormat::supportsMultiVolume() const
{
    return m_supportsMultiVolume;
}

QVariantMap ArchiveFormat::compressionMethods() const
{
    return m_compressionMethods;
}

QString ArchiveFormat::defaultCompressionMethod() const
{
    return m_defaultCompressionMethod;
}

QStringList ArchiveFormat::encryptionMethods() const
{
    return m_encryptionMethods;
}

QString ArchiveFormat::defaultEncryptionMethod() const
{
    return m_defaultEncryptionMethod;
}
