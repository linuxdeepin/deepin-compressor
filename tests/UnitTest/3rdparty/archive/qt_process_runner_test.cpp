// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "qt_process_runner.h"

#include <QCoreApplication>
#include <QTimer>
#include <chrono>
#include <thread>

using namespace archive;

class QtProcessRunnerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        runner = std::make_unique<QtProcessRunner>();
    }
    std::unique_ptr<QtProcessRunner> runner;
};

TEST_F(QtProcessRunnerTest, StartAndExit)
{
    ASSERT_TRUE(runner->start("/bin/echo", {"hello"}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    EXPECT_EQ(runner->exitCode(), 0);
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(QtProcessRunnerTest, ReadStdout)
{
    ASSERT_TRUE(runner->start("/bin/echo", {"hello world"}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    QByteArray out = runner->readAllStandardOutput();
    EXPECT_TRUE(out.contains("hello world"));
}

TEST_F(QtProcessRunnerTest, ExitCodeNonZero)
{
    ASSERT_TRUE(runner->start("/bin/sh", {"-c", "exit 3"}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    EXPECT_EQ(runner->exitCode(), 3);
}

TEST_F(QtProcessRunnerTest, CancelRunningProcess)
{
    ASSERT_TRUE(runner->start("/bin/sleep", {"30"}));
    EXPECT_TRUE(runner->isRunning());

    runner->cancel();
    // After cancel, process should stop
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(QtProcessRunnerTest, CancelFinishedProcessIsNoop)
{
    ASSERT_TRUE(runner->start("/bin/echo", {"done"}));
    ASSERT_TRUE(runner->waitForFinished(10000));

    // Cancelling a finished process should be a safe no-op
    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(QtProcessRunnerTest, TimeoutOnLongProcess)
{
    ASSERT_TRUE(runner->start("/bin/sleep", {"30"}));
    // waitForFinished should time out
    EXPECT_FALSE(runner->waitForFinished(1000));
    EXPECT_TRUE(runner->isRunning());

    runner->cancel();
}

TEST_F(QtProcessRunnerTest, ReadStderr)
{
    ASSERT_TRUE(runner->start("/bin/sh", {"-c", "echo err >&2"}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    QByteArray err = runner->readAllStandardError();
    EXPECT_TRUE(err.contains("err"));
}

TEST_F(QtProcessRunnerTest, PauseAndResume)
{
    ASSERT_TRUE(runner->start("/bin/sleep", {"10"}));
    EXPECT_TRUE(runner->isRunning());

    // Pause and resume — SIGSTOP/SIGCONT
    runner->pause();
    runner->resume();

    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(QtProcessRunnerTest, StartInvalidProgram)
{
    EXPECT_FALSE(runner->start("/nonexistent/program", {}));
}

TEST_F(QtProcessRunnerTest, OutputCallbackReceivesData)
{
    QByteArray collected;
    runner->setOutputCallback([&collected](const QByteArray &data) {
        collected.append(data);
    });

    ASSERT_TRUE(runner->start("/bin/echo", {"callback test"}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    // Allow event processing for signals
    QCoreApplication::processEvents();
    EXPECT_TRUE(collected.contains("callback test"));
}

TEST_F(QtProcessRunnerTest, SetTimeout)
{
    runner->setTimeout(5000);
    EXPECT_EQ(runner->timeout(), 5000);
}

TEST_F(QtProcessRunnerTest, SetTimeoutUsedByWaitForFinished)
{
    runner->setTimeout(100);
    ASSERT_TRUE(runner->start("/bin/sleep", {"30"}));
    // waitForFinished() with no args (default -1) should use m_timeoutMs=100
    EXPECT_FALSE(runner->waitForFinished());
    EXPECT_TRUE(runner->isRunning());
    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(QtProcessRunnerTest, DestructorCleansUp)
{
    {
        QtProcessRunner r;
        ASSERT_TRUE(r.start("/bin/sleep", {"30"}));
        EXPECT_TRUE(r.isRunning());
        // Destructor should cancel and clean up
    }
    // If we get here without hanging, destructor worked
    SUCCEED();
}
