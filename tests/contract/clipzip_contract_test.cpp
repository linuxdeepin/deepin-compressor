// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipzip_backend.h"
#include "fakes.h"
#include "legacy_clipzip_adapter.h"
#include "libzip_backend.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <memory>
#include <sys/stat.h>

namespace {

QString testSourceDir()
{
    return QStringLiteral(CONTRACT_TEST_SOURCE_DIR);
}

enum class BackendKind {
    Legacy,
    New,
};

std::unique_ptr<dcarchive::ArchiveBackend> createBackend(BackendKind kind,
                                                         const QString &path,
                                                         const QString &mime)
{
    if (kind == BackendKind::Legacy)
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyClipzipAdapter(path, mime));
    return std::unique_ptr<dcarchive::ArchiveBackend>(
            new dcarchive::ClipzipBackend(path, mime));
}

class ClipzipContractTest : public ::testing::TestWithParam<BackendKind>
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_workspace.isValid());
        m_archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("created.zip"));
        m_backend = createBackend(GetParam(), m_archivePath, QStringLiteral("application/zip"));
        dcarchive::OpenRequest request;
        request.path = m_archivePath;
        request.mimeType = QStringLiteral("application/zip");
        ASSERT_TRUE(m_backend->open(request).result.ok());
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    std::unique_ptr<dcarchive::ArchiveBackend> m_backend;
    dcarchive::FakeObserver m_observer;
    QTemporaryDir m_workspace;
    QString m_archivePath;
};

TEST_P(ClipzipContractTest, AddCreatesZip)
{
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    dcarchive::ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("test.txt");
    entry.size = QFileInfo(source).size();
    entry.type = dcarchive::EntryType::File;

    dcarchive::AddRequest request;
    request.archivePath = m_archivePath;
    request.format = QStringLiteral("zip");
    request.compressionLevel = 6;
    request.cpuThreads = 1;
    request.entries.append(entry);

    const dcarchive::VoidResult result = m_backend->addFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    ASSERT_TRUE(QFileInfo::exists(m_archivePath));

    dcarchive::LibzipBackend verifier(m_archivePath, QStringLiteral("application/zip"));
    const dcarchive::EntryResult listed = verifier.list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_GE(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().name, QStringLiteral("test.txt"));
}

TEST(ClipzipRoundTripTest, ReportsDiskFullSeparatelyFromOpenFailure)
{
#ifndef DC_BUILT_PZIP
    GTEST_SKIP() << "pzip is not built on this platform";
#else
    using namespace dcarchive;
    struct stat device;
    if (::stat("/dev/full", &device) != 0 || !S_ISCHR(device.st_mode))
        GTEST_SKIP() << "/dev/full is not available";
    QTemporaryDir work;
    ASSERT_TRUE(work.isValid());
    const QString mime = QStringLiteral("application/zip");
    AddRequest request;
    ArchiveEntry entry;
    entry.path = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    request.entries.append(entry);
    FakeObserver observer;
    ClipzipBackend fullDevice(QStringLiteral("/dev/full"), mime);
    EXPECT_EQ(fullDevice.addFiles(request, observer).result.error, ArchiveError::DiskFull);
    ClipzipBackend missingParent(work.filePath(QStringLiteral("missing/file.zip")), mime);
    const auto failed = missingParent.addFiles(request, observer).result;
    EXPECT_FALSE(failed.ok());
    EXPECT_NE(failed.error, ArchiveError::DiskFull);
#endif
}

TEST(ClipzipRoundTripTest, SmallFilesRoundTripAtAllCompressionLevels)
{
#ifndef DC_BUILT_PZIP
    GTEST_SKIP() << "pzip is not built on this platform";
#else
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString mime = QStringLiteral("application/zip");
    ArchiveEntry entry;
    entry.path = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    entry.name = QStringLiteral("test.txt");
    entry.size = QFileInfo(entry.path).size();
    QFile source(entry.path);
    ASSERT_TRUE(source.open(QIODevice::ReadOnly));
    const QByteArray expected = source.readAll();

    for (int level = 0; level <= 9; ++level) {
        for (bool encrypted : {false, true}) {
            SCOPED_TRACE(::testing::Message() << "level=" << level << " encrypted=" << encrypted);
            QTemporaryDir sample(workspace.filePath(QStringLiteral("sample-XXXXXX")));
            ASSERT_TRUE(sample.isValid());
            const QString archive = sample.filePath(QStringLiteral("created.zip"));
            ClipzipBackend writer(archive, mime);
            FakeObserver observer;
            AddRequest add;
            add.entries.append(entry);
            add.compressionLevel = level;
            if (encrypted)
                add.password = QStringLiteral("pzip-\u5bc6\u7801");
            const auto created = writer.addFiles(add, observer);
            ASSERT_TRUE(created.result.ok()) << created.result.message.toStdString();

            LibzipBackend reader(archive, mime);
            ExtractRequest extract;
            extract.targetPath = sample.path();
            if (encrypted) {
                EXPECT_EQ(reader.extractFiles(extract, observer).result.error, ArchiveError::Cancelled);
                EXPECT_FALSE(QFileInfo::exists(sample.filePath(entry.name)));
                observer.reset();
                observer.passwordResponse = PasswordRequest::Response::Provide;
                observer.passwordValue = add.password;
            }
            const auto extracted = reader.extractFiles(extract, observer);
            ASSERT_TRUE(extracted.result.ok()) << extracted.result.message.toStdString();
            EXPECT_EQ(observer.passwordRequests.size(), encrypted ? 1 : 0);
            QFile output(sample.filePath(entry.name));
            ASSERT_TRUE(output.open(QIODevice::ReadOnly));
            EXPECT_EQ(output.readAll(), expected);
        }
    }
#endif
}

INSTANTIATE_TEST_SUITE_P(ClipzipBackends,
                         ClipzipContractTest,
                         ::testing::Values(BackendKind::Legacy, BackendKind::New),
                         [](const ::testing::TestParamInfo<BackendKind> &info) {
                             return info.param == BackendKind::Legacy ? "legacy" : "new";
                         });

} // namespace
