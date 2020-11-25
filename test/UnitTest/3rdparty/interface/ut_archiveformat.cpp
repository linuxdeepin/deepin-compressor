#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include <QModelIndex>
#include "archiveformat.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <jobs.h>
TEST(ArchiveFormat_ArchiveFormat_UT, ArchiveFormat_ArchiveFormat_UT001)
{
    ArchiveFormat *archive_format = nullptr;
    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    ASSERT_EQ(archive_format, nullptr);
}


TEST(ArchiveFormat_ArchiveFormat_UT, ArchiveFormat_ArchiveFormat_UT002)
{
    QMimeDatabase db;
    QMimeType m_mimeType = db.mimeTypeForFile("../test/UnitTest/regress/new.zip");


    KPluginMetaData m_metaData("../../3rdparty/clizipplugin/kerfuffle_clizip.json");
    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);

}

TEST(ArchiveFormat_isValid_UT, ArchiveFormat_isValid_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.isValid();
}

TEST(ArchiveFormat_encryptionType_UT, ArchiveFormat_encryptionType_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.encryptionType();
}

TEST(ArchiveFormat_minCompressionLevel_UT, ArchiveFormat_minCompressionLevel_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.minCompressionLevel();
}

TEST(ArchiveFormat_maxCompressionLevel_UT, ArchiveFormat_maxCompressionLevel_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.maxCompressionLevel();
}

TEST(ArchiveFormat_defaultCompressionLevel_UT, ArchiveFormat_defaultCompressionLevel_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.defaultCompressionLevel();
}

TEST(ArchiveFormat_supportsWriteComment_UT, ArchiveFormat_supportsWriteComment_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.supportsWriteComment();
}

TEST(ArchiveFormat_supportsTesting_UT, ArchiveFormat_supportsTesting_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.supportsTesting();
}


TEST(ArchiveFormat_supportsMultiVolume_UT, ArchiveFormat_supportsMultiVolume_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.supportsMultiVolume();
}


TEST(ArchiveFormat_compressionMethods_UT, ArchiveFormat_compressionMethods_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.compressionMethods();
}


TEST(ArchiveFormat_defaultCompressionMethod_UT, ArchiveFormat_defaultCompressionMethod_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.defaultCompressionMethod();
}

TEST(ArchiveFormat_encryptionMethods_UT, ArchiveFormat_encryptionMethods_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.encryptionMethods();
}

TEST(ArchiveFormat_defaultEncryptionMethod_UT, ArchiveFormat_defaultEncryptionMethod_UT001)
{
    QMimeType m_mimeType;
    KPluginMetaData m_metaData;

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);
    format.defaultEncryptionMethod();
}


