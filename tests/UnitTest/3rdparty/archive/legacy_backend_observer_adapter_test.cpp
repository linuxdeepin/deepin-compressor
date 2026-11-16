// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "legacy_backend_observer_adapter.h"
#include "archive_types.h"

using namespace archive;

class LegacyBackendObserverAdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adapter = std::make_unique<LegacyBackendObserverAdapter>();
    }
    std::unique_ptr<LegacyBackendObserverAdapter> adapter;
};

TEST_F(LegacyBackendObserverAdapterTest, OnProgress)
{
    ProgressInfo info;
    info.percentage = 50.0;
    info.processedSize = 100;
    info.totalSize = 200;

    adapter->onProgress(info);
    EXPECT_DOUBLE_EQ(adapter->lastProgress(), 50.0);
}

TEST_F(LegacyBackendObserverAdapterTest, OnCurrentFile)
{
    adapter->onCurrentFile("/tmp/test.txt");
    EXPECT_EQ(adapter->lastCurrentFile().toStdString(), "/tmp/test.txt");
}

TEST_F(LegacyBackendObserverAdapterTest, OnPasswordRequiredNoHandler)
{
    PasswordRequest req;
    req.archivePath = "/tmp/test.zip";

    adapter->onPasswordRequired(req);
    EXPECT_TRUE(adapter->wasPasswordRequested());
    EXPECT_TRUE(req.password.isEmpty());
}

TEST_F(LegacyBackendObserverAdapterTest, OnPasswordRequiredWithHandler)
{
    adapter->setPasswordHandler([](const PasswordRequest &req) -> QString {
        return QStringLiteral("mypassword");
    });

    PasswordRequest req;
    adapter->onPasswordRequired(req);
    EXPECT_TRUE(adapter->wasPasswordRequested());
    EXPECT_EQ(req.password.toStdString(), "mypassword");
    EXPECT_FALSE(req.cancelled);
}

TEST_F(LegacyBackendObserverAdapterTest, OnPasswordRequiredHandlerReturnsEmpty)
{
    adapter->setPasswordHandler([](const PasswordRequest &) -> QString {
        return QString();
    });

    PasswordRequest req;
    adapter->onPasswordRequired(req);
    EXPECT_TRUE(req.cancelled);
}

TEST_F(LegacyBackendObserverAdapterTest, OnFileConflictNoHandler)
{
    FileConflictRequest req;
    req.path = "/tmp/overwrite.txt";

    adapter->onFileConflict(req);
    EXPECT_TRUE(adapter->wasFileConflict());
    EXPECT_EQ(req.action, FileConflictRequest::Cancel);
}

TEST_F(LegacyBackendObserverAdapterTest, OnFileConflictWithHandler)
{
    adapter->setConflictHandler([](const FileConflictRequest &) -> FileConflictRequest::Action {
        return FileConflictRequest::OverwriteAll;
    });

    FileConflictRequest req;
    adapter->onFileConflict(req);
    EXPECT_TRUE(adapter->wasFileConflict());
    EXPECT_EQ(req.action, FileConflictRequest::OverwriteAll);
}

TEST_F(LegacyBackendObserverAdapterTest, OnWarning)
{
    WarningInfo warning;
    warning.message = "Test warning";
    warning.details = "Some details";

    adapter->onWarning(warning);
    EXPECT_EQ(adapter->warnings().size(), 1);
    EXPECT_EQ(adapter->warnings().first().message.toStdString(), "Test warning");
}

TEST_F(LegacyBackendObserverAdapterTest, OnFinished)
{
    FinishInfo finish;
    finish.success = true;
    finish.code = ErrorCode::NoError;
    finish.message = "Done";

    adapter->onFinished(finish);
    EXPECT_TRUE(adapter->isFinished());
    EXPECT_TRUE(adapter->finishInfo().success);
    EXPECT_EQ(adapter->finishInfo().code, ErrorCode::NoError);
}

TEST_F(LegacyBackendObserverAdapterTest, MultipleWarningsAccumulate)
{
    for (int i = 0; i < 3; ++i) {
        WarningInfo w;
        w.message = QString("Warning %1").arg(i);
        adapter->onWarning(w);
    }
    EXPECT_EQ(adapter->warnings().size(), 3);
}

TEST_F(LegacyBackendObserverAdapterTest, MultipleProgressUpdates)
{
    for (double p = 0.0; p <= 100.0; p += 25.0) {
        ProgressInfo info;
        info.percentage = p;
        adapter->onProgress(info);
    }
    EXPECT_DOUBLE_EQ(adapter->lastProgress(), 100.0);
}
