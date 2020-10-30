#ifndef ARCHIVEFORMAT_H
#define ARCHIVEFORMAT_H

#include "kpluginmetadata.h"
#include "commonstruct.h"

#include <QMimeType>
#include <QVariantMap>

class ArchiveFormat
{
public:
    explicit ArchiveFormat();
    explicit ArchiveFormat(const QMimeType &mimeType,
                           EncryptionType encryptionType,
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
    EncryptionType encryptionType() const;

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
    EncryptionType m_encryptionType = Unencrypted;
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
