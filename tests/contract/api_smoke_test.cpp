// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contract_test_base.h"

namespace {

class NullBackend : public dcarchive::ArchiveBackend
{
public:
    dcarchive::BackendCapabilities capabilities() const override { return {}; }
    dcarchive::VoidResult open(const dcarchive::OpenRequest &) override { return {}; }
    dcarchive::EntryResult list() override { return {}; }
    dcarchive::VoidResult testArchive() override { return {}; }
    dcarchive::VoidResult extractFiles(const dcarchive::ExtractRequest &, dcarchive::BackendObserver &) override { return {}; }
    dcarchive::VoidResult addFiles(const dcarchive::AddRequest &, dcarchive::BackendObserver &) override { return {}; }
    dcarchive::VoidResult moveFiles(const dcarchive::MoveRequest &, dcarchive::BackendObserver &) override { return {}; }
    dcarchive::VoidResult copyFiles(const dcarchive::CopyRequest &, dcarchive::BackendObserver &) override { return {}; }
    dcarchive::VoidResult deleteFiles(const dcarchive::DeleteRequest &) override { return {}; }
    dcarchive::VoidResult renameFiles(const dcarchive::RenameRequest &) override { return {}; }
    dcarchive::VoidResult addComment(const QString &) override { return {}; }
    dcarchive::VoidResult updateArchiveData(const dcarchive::UpdateRequest &) override { return {}; }
    void pause() override {}
    void resume() override {}
    void cancel() override {}
};

} // namespace

TEST(ArchiveApiSmoke, DefaultResultIsOk)
{
    dcarchive::ArchiveResult result;
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(result.error, dcarchive::ArchiveError::None);
    EXPECT_FALSE(result.cancelled);
}

TEST(ArchiveApiSmoke, DefaultCapabilitiesAreDisabled)
{
    NullBackend backend;
    const dcarchive::BackendCapabilities caps = backend.capabilities();
    EXPECT_FALSE(caps.list);
    EXPECT_FALSE(caps.extract);
    EXPECT_FALSE(caps.create);
}

TEST(ArchiveApiSmoke, NullBackendListReturnsEmptyInfo)
{
    NullBackend backend;
    dcarchive::EntryResult result = backend.list();
    EXPECT_TRUE(result.result.ok());
    EXPECT_TRUE(result.info.entries.isEmpty());
}

TEST(ArchiveApiSmoke, FakeObserverRecordsEvents)
{
    NullBackend backend;
    dcarchive::FakeObserver observer;
    dcarchive::ProgressInfo progress;
    progress.percentage = 42.0;
    observer.onProgress(progress);
    observer.onCurrentFile(QStringLiteral("a.txt"));

    ASSERT_EQ(observer.progressEvents.size(), 1);
    EXPECT_DOUBLE_EQ(observer.progressEvents.first().percentage, 42.0);
    ASSERT_EQ(observer.currentFiles.size(), 1);
    EXPECT_EQ(observer.currentFiles.first(), QStringLiteral("a.txt"));
}
