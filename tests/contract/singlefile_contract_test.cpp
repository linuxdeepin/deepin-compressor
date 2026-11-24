// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "legacy_singlefile_adapter.h"
#include "singlefile_backend.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <chrono>
#include <future>
#include <memory>
#include <vector>

namespace {

const QByteArray kExpectedContent = "testdfkogjkdsgnl sdnglksdjjkf ";

enum class BackendKind {
    Legacy,
    New,
};

struct SingleFileCase {
    QString path;
    QString mime;
    QString expectedName;
    QString label;
    BackendKind kind;
};

std::vector<SingleFileCase> singleFileCases()
{
    const QString base = QStringLiteral(CONTRACT_TEST_SOURCE_DIR);
    return {
        {base + QStringLiteral("/test_sources/gz/test.gz"),
         QStringLiteral("application/gzip"), QStringLiteral("test"), QStringLiteral("gz_legacy"), BackendKind::Legacy},
        {base + QStringLiteral("/test_sources/gz/test.gz"),
         QStringLiteral("application/gzip"), QStringLiteral("test"), QStringLiteral("gz_new"), BackendKind::New},
        {base + QStringLiteral("/test_sources/bz2/test.txt.bz2"),
         QStringLiteral("application/x-bzip"), QStringLiteral("test.txt"), QStringLiteral("bz2_legacy"), BackendKind::Legacy},
        {base + QStringLiteral("/test_sources/bz2/test.txt.bz2"),
         QStringLiteral("application/x-bzip"), QStringLiteral("test.txt"), QStringLiteral("bz2_new"), BackendKind::New},
        {base + QStringLiteral("/test_sources/xz/test.xz"),
         QStringLiteral("application/x-xz"), QStringLiteral("test"), QStringLiteral("xz_legacy"), BackendKind::Legacy},
        {base + QStringLiteral("/test_sources/xz/test.xz"),
         QStringLiteral("application/x-xz"), QStringLiteral("test"), QStringLiteral("xz_new"), BackendKind::New},
    };
}

class LegacySingleFileContractTest : public ::testing::TestWithParam<SingleFileCase>
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_workspace.isValid());
        if (GetParam().kind == BackendKind::Legacy) {
            m_backend.reset(new dcarchive::LegacySingleFileAdapter(GetParam().path, GetParam().mime));
        } else {
            m_backend.reset(new dcarchive::SingleFileBackend(GetParam().path, GetParam().mime));
        }
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    dcarchive::ArchiveBackend *backend() { return m_backend.get(); }

    QString targetPath() const { return m_workspace.path(); }

    QString expectedFilePath() const
    {
        return QDir(targetPath()).filePath(GetParam().expectedName);
    }

    void writeExistingFile()
    {
        QFile file(expectedFilePath());
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(file.write("existing"), 8);
        file.close();
    }

    QByteArray readOutputFile()
    {
        QFile file(expectedFilePath());
        if (!file.open(QIODevice::ReadOnly))
            return {};
        return file.readAll();
    }

    std::unique_ptr<dcarchive::ArchiveBackend> m_backend;
    dcarchive::FakeObserver m_observer;
    QTemporaryDir m_workspace;
};

TEST_P(LegacySingleFileContractTest, ListReturnsSingleVirtualEntry)
{
    dcarchive::OpenRequest openRequest;
    openRequest.path = GetParam().path;
    openRequest.mimeType = GetParam().mime;
    ASSERT_TRUE(backend()->open(openRequest).result.ok());

    const dcarchive::EntryResult listed = backend()->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_EQ(listed.info.entries.size(), 1);

    const dcarchive::ArchiveEntry &entry = listed.info.entries.first();
    EXPECT_EQ(entry.name, GetParam().expectedName);
    EXPECT_EQ(entry.type, dcarchive::EntryType::File);
    EXPECT_GT(entry.size, 0);
    EXPECT_GT(listed.info.totalSize, 0);
}

TEST_P(LegacySingleFileContractTest, TestArchiveReturnsOkWithoutValidation)
{
    dcarchive::OpenRequest openRequest;
    openRequest.path = GetParam().path;
    openRequest.mimeType = GetParam().mime;
    ASSERT_TRUE(backend()->open(openRequest).result.ok());

    const dcarchive::VoidResult result = backend()->testArchive();
    EXPECT_TRUE(result.result.ok());
    EXPECT_EQ(result.result.error, dcarchive::ArchiveError::None);
}

TEST_P(LegacySingleFileContractTest, ExtractWritesDecompressedFile)
{
    dcarchive::OpenRequest openRequest;
    openRequest.path = GetParam().path;
    openRequest.mimeType = GetParam().mime;
    ASSERT_TRUE(backend()->open(openRequest).result.ok());

    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = backend()->extractFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    EXPECT_EQ(readOutputFile(), kExpectedContent);
    EXPECT_FALSE(m_observer.finishes.isEmpty());
}

TEST_P(LegacySingleFileContractTest, ExtractSkipKeepsExistingFile)
{
    dcarchive::OpenRequest openRequest;
    openRequest.path = GetParam().path;
    openRequest.mimeType = GetParam().mime;
    ASSERT_TRUE(backend()->open(openRequest).result.ok());

    writeExistingFile();
    m_observer.conflictResponse = dcarchive::FileConflictRequest::Response::Skip;

    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = backend()->extractFiles(request, m_observer);
    EXPECT_TRUE(result.result.cancelled);
    EXPECT_EQ(result.result.error, dcarchive::ArchiveError::Cancelled);

    QFile file(expectedFilePath());
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    EXPECT_EQ(file.readAll(), QByteArray("existing"));
}

TEST_P(LegacySingleFileContractTest, ExtractOverwriteReplacesExistingFile)
{
    dcarchive::OpenRequest openRequest;
    openRequest.path = GetParam().path;
    openRequest.mimeType = GetParam().mime;
    ASSERT_TRUE(backend()->open(openRequest).result.ok());

    writeExistingFile();
    m_observer.conflictResponse = dcarchive::FileConflictRequest::Response::Overwrite;

    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = backend()->extractFiles(request, m_observer);
    EXPECT_TRUE(result.result.ok()) << result.result.message.toStdString();
    EXPECT_EQ(readOutputFile(), kExpectedContent);
}

TEST_P(LegacySingleFileContractTest, UnsupportedCreateReturnsNotSupported)
{
    dcarchive::OpenRequest openRequest;
    openRequest.path = GetParam().path;
    openRequest.mimeType = GetParam().mime;
    ASSERT_TRUE(backend()->open(openRequest).result.ok());

    dcarchive::AddRequest request;
    dcarchive::VoidResult result = backend()->addFiles(request, m_observer);
    EXPECT_EQ(result.result.error, dcarchive::ArchiveError::NotSupported);
}

TEST(SingleFileExtractionTest, WaitsForResumeWhenPausedBeforeDecoding)
{
    for (const auto &format : singleFileCases()) {
        if (format.kind != BackendKind::New)
            continue;
        SCOPED_TRACE(format.label.toStdString());
        QTemporaryDir output;
        ASSERT_TRUE(output.isValid());
        dcarchive::SingleFileBackend backend(format.path, format.mime);
        ASSERT_TRUE(backend.open({}).result.ok());
        dcarchive::ExtractRequest request;
        request.targetPath = output.path();
        dcarchive::FakeObserver observer;
        backend.pause();
        auto running = std::async(std::launch::async, [&] {
            return backend.extractFiles(request, observer);
        });
        EXPECT_EQ(running.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout);
        backend.resume();
        ASSERT_TRUE(running.get().result.ok());
        QFile restored(output.filePath(format.expectedName));
        ASSERT_TRUE(restored.open(QIODevice::ReadOnly));
        EXPECT_EQ(restored.readAll(), kExpectedContent);
    }
}

INSTANTIATE_TEST_SUITE_P(SingleFileBackends,
                         LegacySingleFileContractTest,
                         ::testing::ValuesIn(singleFileCases()),
                         [](const ::testing::TestParamInfo<SingleFileCase> &info) {
                             return info.param.label.toStdString();
                         });

} // namespace
