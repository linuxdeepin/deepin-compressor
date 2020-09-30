#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include <QModelIndex>
#include "options.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <jobs.h>
TEST(Options_setEncryptedArchiveHint_UT, Options_setEncryptedArchiveHint_UT001)
{
    Options options;
    options.setEncryptedArchiveHint(true);
}

TEST(Options_encryptedArchiveHint_UT, Options_encryptedArchiveHint_UT001)
{
    Options options;
    options.encryptedArchiveHint();
}



TEST(ExtractionOptions_preservePaths_UT, ExtractionOptions_preservePaths_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.preservePaths();
}

TEST(ExtractionOptions_setPreservePaths_UT, ExtractionOptions_setPreservePaths_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.setPreservePaths(true);
}

TEST(ExtractionOptions_isDragAndDropEnabled_UT, ExtractionOptions_isDragAndDropEnabled_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.isDragAndDropEnabled();
}

TEST(ExtractionOptions_setDragAndDropEnabled_UT, ExtractionOptions_setDragAndDropEnabled_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.setDragAndDropEnabled(true);
}

TEST(ExtractionOptions_alwaysUseTempDir_UT, ExtractionOptions_alwaysUseTempDir_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.alwaysUseTempDir();
}

TEST(ExtractionOptions_setAlwaysUseTempDir_UT, ExtractionOptions_setAlwaysUseTempDir_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.setAlwaysUseTempDir(true);
}

TEST(ExtractionOptions_setBatchExtract_UT, ExtractionOptions_setBatchExtract_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.setBatchExtract(true);
}

TEST(ExtractionOptions_isAutoCreatDir_UT, ExtractionOptions_isAutoCreatDir_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.isAutoCreatDir();
}

TEST(ExtractionOptions_setAutoCreatDir_UT, ExtractionOptions_setAutoCreatDir_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.setAutoCreatDir(true);
}

TEST(ExtractionOptions_isRightMenuExtractHere_UT, ExtractionOptions_isRightMenuExtractHere_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.isRightMenuExtractHere();
}

TEST(ExtractionOptions_setRightMenuExtractHere_UT, ExtractionOptions_setRightMenuExtractHere_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.setRightMenuExtractHere(true);
}

TEST(ExtractionOptions_isBatchExtract_UT, ExtractionOptions_isBatchExtract_UT001)
{
    ExtractionOptions extractionOptions;
    extractionOptions.isBatchExtract();
}

TEST(CompressionOptions_isCompressionLevelSet_UT, CompressionOptions_isCompressionLevelSet_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.isCompressionLevelSet();
}


TEST(CompressionOptions_isVolumeSizeSet_UT, CompressionOptions_isVolumeSizeSet_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.isVolumeSizeSet();
}

TEST(CompressionOptions_compressionLevel_UT, CompressionOptions_compressionLevel_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.compressionLevel();
}

TEST(CompressionOptions_setCompressionLevel_UT, CompressionOptions_setCompressionLevel_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setCompressionLevel(10);
}


TEST(CompressionOptions_volumeSize_UT, CompressionOptions_volumeSize_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.volumeSize();
}

TEST(CompressionOptions_setVolumeSize_UT, CompressionOptions_setVolumeSize_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setVolumeSize(10);
}

TEST(CompressionOptions_compressionMethod_UT, CompressionOptions_compressionMethod_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.compressionMethod();
}


TEST(CompressionOptions_setCompressionMethod_UT, CompressionOptions_setCompressionMethod_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setCompressionMethod("time");
}

TEST(CompressionOptions_encryptionMethod_UT, CompressionOptions_encryptionMethod_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.encryptionMethod();
}


TEST(CompressionOptions_setEncryptionMethod_UT, CompressionOptions_setEncryptionMethod_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setEncryptionMethod("str");
}

TEST(CompressionOptions_globalWorkDir_UT, CompressionOptions_globalWorkDir_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.globalWorkDir();
}

TEST(CompressionOptions_setGlobalWorkDir_UT, CompressionOptions_setGlobalWorkDir_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setGlobalWorkDir("tfd");
}


TEST(CompressionOptions_isTar7z_UT, CompressionOptions_isTar7z_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.isTar7z();
}


TEST(CompressionOptions_setIsTar7z_UT, CompressionOptions_setIsTar7z_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setIsTar7z(true);
}

TEST(CompressionOptions_getfilesSize_UT, CompressionOptions_getfilesSize_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.getfilesSize();
}

TEST(CompressionOptions_setFilesSize_UT, CompressionOptions_setFilesSize_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setFilesSize(100);
}


TEST(CompressionOptions_operator_UT, CompressionOptions_operator_UT001)
{
    CompressionOptions CompressionOptions;
    CompressionOptions.setCompressionMethod("tt");
    CompressionOptions.setEncryptionMethod("gg");
    CompressionOptions.setGlobalWorkDir("ff");
    qDebug()<<CompressionOptions;
}


TEST(ExtractionOptions_operator_UT, ExtractionOptions_operator_UT001)
{
    ExtractionOptions CompressionOptions;
    qDebug()<<CompressionOptions;
}
