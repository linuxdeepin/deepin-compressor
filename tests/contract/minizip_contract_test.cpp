// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "legacy_minizip_adapter.h"
#include "minizip_backend.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <chrono>
#include <future>
#include <memory>

namespace {

QString crxSource()
{
    return QStringLiteral(CONTRACT_TEST_SOURCE_DIR)
            + QStringLiteral("/test_sources/crx/test.crx");
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
                new dcarchive::LegacyMinizipAdapter(path, mime));
    return std::unique_ptr<dcarchive::ArchiveBackend>(
            new dcarchive::MinizipBackend(path, mime));
}

class MinizipContractTest : public ::testing::TestWithParam<BackendKind>
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_workspace.isValid());
        m_archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("test.crx"));
        ASSERT_TRUE(QFile::copy(crxSource(), m_archivePath));
        m_backend = createBackend(GetParam(), m_archivePath,
                                  QStringLiteral("application/x-chrome-extension"));
        dcarchive::OpenRequest request;
        request.path = m_archivePath;
        request.mimeType = QStringLiteral("application/x-chrome-extension");
        ASSERT_TRUE(m_backend->open(request).result.ok());
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    dcarchive::ArchiveBackend *backend() { return m_backend.get(); }
    QString targetPath() const { return m_workspace.path(); }

    std::unique_ptr<dcarchive::ArchiveBackend> m_backend;
    dcarchive::FakeObserver m_observer;
    QTemporaryDir m_workspace;
    QString m_archivePath;
};

TEST_P(MinizipContractTest, ListReturnsEntries)
{
    const dcarchive::EntryResult listed = backend()->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    EXPECT_GT(listed.info.entries.size(), 0);

    QStringList names;
    bool hasLocales = false;
    for (const dcarchive::ArchiveEntry &entry : listed.info.entries) {
        names.append(entry.path);
        if (entry.path.startsWith(QStringLiteral("_locales")))
            hasLocales = true;
    }
    EXPECT_TRUE(hasLocales);
}

TEST_P(MinizipContractTest, ExtractWritesEntries)
{
    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = backend()->extractFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const QString messages = QDir(targetPath()).filePath(
            QStringLiteral("_locales/en/messages.json"));
    ASSERT_TRUE(QFileInfo::exists(messages));
    EXPECT_EQ(QFileInfo(messages).size(), 803);
}

TEST_P(MinizipContractTest, UnsupportedCreateReturnsNotSupported)
{
    dcarchive::AddRequest request;
    const dcarchive::VoidResult result = backend()->addFiles(request, m_observer);
    EXPECT_EQ(result.result.error, dcarchive::ArchiveError::NotSupported);
}

class MinizipPauseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(output.isValid());
        ASSERT_TRUE(backend.open({}).result.ok());
        const auto listed = backend.list();
        ASSERT_TRUE(listed.result.ok());
        for (const auto &entry : listed.info.entries) {
            if (entry.path == QStringLiteral("assets/js/background.min.js")) {
                ASSERT_GT(entry.size, 128 * 1024);
                request.targetPath = output.path();
                request.allEntries = false;
                request.entries.append(entry);
                return;
            }
        }
        FAIL() << "Missing multi-chunk CRX fixture entry";
    }

    QString destination() const { return output.filePath(request.entries.first().path); }

    QTemporaryDir output;
    dcarchive::MinizipBackend backend{crxSource(), QStringLiteral("application/x-chrome-extension")};
    dcarchive::ExtractRequest request;
};

class PauseAfterFirstChunk : public dcarchive::FakeObserver
{
public:
    explicit PauseAfterFirstChunk(dcarchive::MinizipBackend &backend) : backend(backend) {}

    void onProgress(const dcarchive::ProgressInfo &progress) override
    {
        dcarchive::FakeObserver::onProgress(progress);
        if (!paused && progress.processedBytes > 0) {
            paused = true;
            backend.pause();
            reached.set_value();
        }
    }

    std::promise<void> reached;

private:
    dcarchive::MinizipBackend &backend;
    bool paused = false;
};

TEST_F(MinizipPauseTest, HonorsPauseBeforeExtraction)
{
    dcarchive::FakeObserver observer;
    backend.pause();
    auto task = std::async(std::launch::async, [&] { return backend.extractFiles(request, observer); });
    EXPECT_EQ(task.wait_for(std::chrono::milliseconds(150)), std::future_status::timeout);
    EXPECT_FALSE(QFileInfo::exists(destination()));
    backend.resume();
    ASSERT_TRUE(task.get().result.ok());
    EXPECT_EQ(QFileInfo(destination()).size(), request.entries.first().size);
}

TEST_F(MinizipPauseTest, PausesBetweenChunksAndResumes)
{
    PauseAfterFirstChunk observer(backend);
    auto paused = observer.reached.get_future();
    auto task = std::async(std::launch::async, [&] { return backend.extractFiles(request, observer); });
    EXPECT_EQ(paused.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    EXPECT_EQ(task.wait_for(std::chrono::milliseconds(150)), std::future_status::timeout);
    EXPECT_FALSE(QFileInfo::exists(destination()));
    backend.resume();
    ASSERT_TRUE(task.get().result.ok());
    EXPECT_EQ(QFileInfo(destination()).size(), request.entries.first().size);
}

TEST_F(MinizipPauseTest, CancelWhilePausedPreservesExistingFile)
{
    const QString path = destination();
    ASSERT_TRUE(QDir().mkpath(QFileInfo(path).path()));
    QFile existing(path);
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    ASSERT_EQ(existing.write("existing"), 8);
    existing.close();
    PauseAfterFirstChunk observer(backend);
    observer.conflictResponse = dcarchive::FileConflictRequest::Response::Overwrite;
    auto paused = observer.reached.get_future();
    auto task = std::async(std::launch::async, [&] { return backend.extractFiles(request, observer); });
    EXPECT_EQ(paused.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    backend.cancel();
    const auto outcome = task.wait_for(std::chrono::seconds(2));
    EXPECT_EQ(outcome, std::future_status::ready);
    if (outcome != std::future_status::ready)
        backend.resume();
    EXPECT_EQ(task.get().result.error, dcarchive::ArchiveError::Cancelled);
    ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
    EXPECT_EQ(existing.readAll(), QByteArray("existing"));
    EXPECT_EQ(QDir(QFileInfo(path).path()).entryList(QDir::Files | QDir::Hidden).size(), 1);
}

INSTANTIATE_TEST_SUITE_P(MinizipBackends,
                         MinizipContractTest,
                         ::testing::Values(BackendKind::Legacy, BackendKind::New),
                         [](const ::testing::TestParamInfo<BackendKind> &info) {
                             return info.param == BackendKind::Legacy ? "legacy" : "new";
                         });

} // namespace
