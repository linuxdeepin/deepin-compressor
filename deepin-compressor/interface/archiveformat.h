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
#ifndef ARCHIVEFORMAT_H
#define ARCHIVEFORMAT_H

#include "archive_manager.h"

#include "kpluginmetadata.h"



class  ArchiveFormat
{
public:
    explicit ArchiveFormat();
    explicit ArchiveFormat(const QMimeType &mimeType,
                           Archive::EncryptionType encryptionType,
                           int minCompLevel,
                           int maxCompLevel,
                           int defaultCompLevel,
                           bool supportsWriteComment,
                           bool supportsTesting,
                           bool suppportsMultiVolume,
                           const QVariantMap &compressionMethods,
                           const QString &defaultCompressionMethod,
                           const QStringList &encryptionMethods,
                           const QString &defaultEncryptionMethod);

    static ArchiveFormat fromMetadata(const QMimeType &mimeType, const KPluginMetaData &metadata);
    bool isValid() const;
    Archive::EncryptionType encryptionType() const;

    int minCompressionLevel() const;
    int maxCompressionLevel() const;
    int defaultCompressionLevel() const;
    bool supportsWriteComment() const;
    bool supportsTesting() const;
    bool supportsMultiVolume() const;
    QVariantMap compressionMethods() const;
    QString defaultCompressionMethod() const;
    QStringList encryptionMethods() const;
    QString defaultEncryptionMethod() const;

private:
    QMimeType m_mimeType;
    Archive::EncryptionType m_encryptionType = Archive::Unencrypted;
    int m_minCompressionLevel = -1;
    int m_maxCompressionLevel = 0;
    int m_defaultCompressionLevel = 0;
    bool m_supportsWriteComment = false;
    bool m_supportsTesting = false;
    bool m_supportsMultiVolume = false;
    QVariantMap m_compressionMethods;
    QString m_defaultCompressionMethod;
    QStringList m_encryptionMethods;
    QString m_defaultEncryptionMethod;
};



#endif // ARCHIVEFORMAT_H
