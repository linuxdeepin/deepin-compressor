// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archive_registry.h"
#include "clipzip_backend.h"
#include "libarchive_backend.h"
#include "libzip_backend.h"
#include "pigz_backend.h"
#include "rar_backend.h"
#include "sevenzip_backend.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <gtest/gtest.h>

TEST(ArchiveRegistryTest, SelectsReadBackends)
{
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.zip"), QStringLiteral("application/zip"), false), nullptr);
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.7z"), QStringLiteral("application/x-7z-compressed"), false), nullptr);
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.rar"), QStringLiteral("application/vnd.rar"), false), nullptr);
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.tar.gz"), QStringLiteral("application/x-compressed-tar"), false), nullptr);
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.gz"), QStringLiteral("application/gzip"), false), nullptr);
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.crx"), QStringLiteral("application/x-chrome-extension"), false), nullptr);
}

TEST(ArchiveRegistryTest, SelectsWriteBackends)
{
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.zip"), QStringLiteral("application/zip"), true), nullptr);
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.tar.gz"), QStringLiteral("application/x-compressed-tar"), true), nullptr);
    EXPECT_NE(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.7z"), QStringLiteral("application/x-7z-compressed"), true), nullptr);
}

TEST(ArchiveRegistryTest, ReturnsNullForUnknownFormat)
{
    EXPECT_EQ(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.unknown"), QStringLiteral("application/octet-stream"), false), nullptr);
}

TEST(ArchiveRegistryTest, SelectsOperationSpecificBackends)
{
    auto createZip = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.zip"), QStringLiteral("application/zip"),
            dcarchive::ArchiveOperation::Create);
    ASSERT_NE(createZip, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::ClipzipBackend *>(createZip.get()), nullptr);

    auto appendZip = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.zip"), QStringLiteral("application/zip"),
            dcarchive::ArchiveOperation::Append);
    ASSERT_NE(appendZip, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::LibzipBackend *>(appendZip.get()), nullptr);

    auto modifyZip = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.zip"), QStringLiteral("application/zip"),
            dcarchive::ArchiveOperation::Modify);
    ASSERT_NE(modifyZip, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::LibzipBackend *>(modifyZip.get()), nullptr);

    auto appendTar = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.tar"), QStringLiteral("application/x-tar"),
            dcarchive::ArchiveOperation::Append);
    ASSERT_NE(appendTar, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::LibarchiveBackend *>(appendTar.get()), nullptr);
}

TEST(ArchiveRegistryTest, SelectsExplicitBackendPreferences)
{
    auto sevenZip = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.zip"), QStringLiteral("application/zip"),
            dcarchive::ArchiveOperation::Read, dcarchive::BackendPreference::SevenZip);
    ASSERT_NE(sevenZip, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::SevenZipBackend *>(sevenZip.get()), nullptr);

    auto libarchive = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.zip"), QStringLiteral("application/zip"),
            dcarchive::ArchiveOperation::Create, dcarchive::BackendPreference::Libarchive);
    ASSERT_NE(libarchive, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::LibarchiveBackend *>(libarchive.get()), nullptr);

    auto libzip = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.zip"), QStringLiteral("application/zip"),
            dcarchive::ArchiveOperation::Create, dcarchive::BackendPreference::Libzip);
    ASSERT_NE(libzip, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::LibzipBackend *>(libzip.get()), nullptr);

    auto pigz = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.tar.gz"), QStringLiteral("application/x-compressed-tar"),
            dcarchive::ArchiveOperation::Create, dcarchive::BackendPreference::Pigz);
    ASSERT_NE(pigz, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::PigzBackend *>(pigz.get()), nullptr);
}

TEST(ArchiveRegistryTest, SelectsGenericLibarchiveFormatsForRead)
{
    const QVector<QPair<QString, QString>> formats{
        {QStringLiteral("a.deb"), QStringLiteral("application/vnd.debian.binary-package")},
        {QStringLiteral("a.rpm"), QStringLiteral("application/x-rpm")},
        {QStringLiteral("a.cab"), QStringLiteral("application/vnd.ms-cab-compressed")},
        {QStringLiteral("a.xar"), QStringLiteral("application/x-xar")},
        {QStringLiteral("a.cpio"), QStringLiteral("application/x-cpio")},
        {QStringLiteral("a.tar.lzma"), QStringLiteral("application/x-lzma-compressed-tar")},
        {QStringLiteral("a.tar.lz4"), QStringLiteral("application/x-lz4-compressed-tar")},
        {QStringLiteral("a.tar.zst"), QStringLiteral("application/x-zstd-compressed-tar")},
    };
    for (const auto &format : formats) {
        auto backend = dcarchive::ArchiveRegistry::create(
                format.first, format.second, dcarchive::ArchiveOperation::Read);
        ASSERT_NE(backend, nullptr) << format.first.toStdString();
        EXPECT_NE(dynamic_cast<dcarchive::LibarchiveBackend *>(backend.get()), nullptr)
                << format.first.toStdString();
    }
}

TEST(ArchiveRegistryTest, RecognizesZipAndSevenZipVolumes)
{
    auto splitZip = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.zip.001"), QStringLiteral("application/zip"),
            dcarchive::ArchiveOperation::Read);
    ASSERT_NE(splitZip, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::SevenZipBackend *>(splitZip.get()), nullptr);

    auto split7z = dcarchive::ArchiveRegistry::create(
            QStringLiteral("a.7z.002"), QStringLiteral("application/x-7z-compressed"),
            dcarchive::ArchiveOperation::Read);
    ASSERT_NE(split7z, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::SevenZipBackend *>(split7z.get()), nullptr);

    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    QFile firstPart(QDir(workspace.path()).filePath(QStringLiteral("a.z01")));
    ASSERT_TRUE(firstPart.open(QIODevice::WriteOnly));
    firstPart.close();
    auto legacySplitZip = dcarchive::ArchiveRegistry::create(
            QDir(workspace.path()).filePath(QStringLiteral("a.zip")),
            QStringLiteral("application/zip"), dcarchive::ArchiveOperation::Read);
    ASSERT_NE(legacySplitZip, nullptr);
    EXPECT_NE(dynamic_cast<dcarchive::SevenZipBackend *>(legacySplitZip.get()), nullptr);
}

TEST(ArchiveRegistryTest, RejectsCreationForReadOnlyFormats)
{
    EXPECT_EQ(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.deb"), QStringLiteral("application/vnd.debian.binary-package"),
                      dcarchive::ArchiveOperation::Create), nullptr);
    EXPECT_EQ(dcarchive::ArchiveRegistry::create(
                      QStringLiteral("a.iso"), QStringLiteral("application/x-iso9660-image"),
                      dcarchive::ArchiveOperation::Create), nullptr);
}
