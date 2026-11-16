// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "posix_pty_runner.h"

#include <QCoreApplication>
#include <QTimer>
#include <chrono>
#include <thread>

#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>

using namespace archive;

class PosixPtyRunnerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        runner = std::make_unique<PosixPtyRunner>();
    }
    std::unique_ptr<PosixPtyRunner> runner;
};

TEST_F(PosixPtyRunnerTest, StartAndExit)
{
    ASSERT_TRUE(runner->start("/bin/echo", {"hello"}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(PosixPtyRunnerTest, ReadOutput)
{
    ASSERT_TRUE(runner->start("/bin/echo", {"pty test"}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    QByteArray out = runner->readAllStandardOutput();
    EXPECT_TRUE(out.contains("pty test"));
}

TEST_F(PosixPtyRunnerTest, WriteInput)
{
    // /bin/cat reads from stdin (which is the pty) and echoes back
    ASSERT_TRUE(runner->start("/bin/cat", {}));
    EXPECT_TRUE(runner->isRunning());

    runner->writeInput(QByteArray("test input\n"));

    // Give cat time to process and echo
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    QCoreApplication::processEvents();

    QByteArray out = runner->readAllStandardOutput();
    EXPECT_TRUE(out.contains("test input"));

    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(PosixPtyRunnerTest, IsPasswordPrompt)
{
    EXPECT_TRUE(runner->isPasswordPrompt("Enter password (will not be echoed):"));
    EXPECT_TRUE(runner->isPasswordPrompt("Enter password:"));
    EXPECT_TRUE(runner->isPasswordPrompt("Verify password:"));
    EXPECT_TRUE(runner->isPasswordPrompt("Enter passphrase:"));
    EXPECT_FALSE(runner->isPasswordPrompt("Extracting file.txt"));
    EXPECT_FALSE(runner->isPasswordPrompt("Archive: test.zip"));
}

TEST_F(PosixPtyRunnerTest, CancelRunningProcess)
{
    ASSERT_TRUE(runner->start("/bin/sleep", {"30"}));
    EXPECT_TRUE(runner->isRunning());

    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(PosixPtyRunnerTest, CancelFinishedProcessIsNoop)
{
    ASSERT_TRUE(runner->start("/bin/echo", {"done"}));
    ASSERT_TRUE(runner->waitForFinished(10000));

    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(PosixPtyRunnerTest, TimeoutOnLongProcess)
{
    ASSERT_TRUE(runner->start("/bin/sleep", {"30"}));
    EXPECT_FALSE(runner->waitForFinished(1000));
    EXPECT_TRUE(runner->isRunning());

    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(PosixPtyRunnerTest, PauseAndResume)
{
    ASSERT_TRUE(runner->start("/bin/sleep", {"10"}));
    EXPECT_TRUE(runner->isRunning());

    runner->pause();
    runner->resume();

    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

// Helper: read the state field from /proc/<pid>/status.
static std::string getProcessState(pid_t pid)
{
    std::ifstream f("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("State:", 0) == 0) {
            // Format: "State:	X (description)"
            auto pos = line.find('\t');
            if (pos != std::string::npos && pos + 1 < line.size()) {
                return std::string(1, line[pos + 1]);
            }
        }
    }
    return "";
}

// Verify that pause() and resume() operate on the entire process group,
// not just the direct child.  We start a shell that spawns a grandchild
// (sleep), then pause and check that the grandchild is also stopped.
TEST_F(PosixPtyRunnerTest, PauseResumeAffectsEntireProcessGroup)
{
    // Start a shell that spawns a child sleep process and waits for it.
    // The child writes its PID to stdout so we can inspect its state.
    ASSERT_TRUE(runner->start("/bin/sh", {
        "-c", "sleep 30 & echo $!; wait"
    }));
    EXPECT_TRUE(runner->isRunning());

    // Give the shell time to start the child and echo its PID.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QCoreApplication::processEvents();

    QByteArray out = runner->readAllStandardOutput();
    ASSERT_FALSE(out.isEmpty());

    // Parse the child PID from the output (first line).
    int childPid = 0;
    {
        QByteArray firstLine = out.split('\n').first().trimmed();
        childPid = firstLine.toInt();
    }
    ASSERT_GT(childPid, 0) << "Could not parse child PID from: "
                           << out.toStdString();

    // Verify the child is running before pause.
    std::string stateBefore = getProcessState(childPid);
    EXPECT_EQ(stateBefore, "S")
        << "Child should be sleeping (S) before pause, got: " << stateBefore;

    // Pause the entire process group.  The child should also stop.
    runner->pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::string statePaused = getProcessState(childPid);
    EXPECT_EQ(statePaused, "T")
        << "Child should be stopped (T) after pause, got: " << statePaused;

    // Resume the entire process group.  The child should resume.
    runner->resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::string stateResumed = getProcessState(childPid);
    EXPECT_NE(stateResumed, "T")
        << "Child should not be stopped (T) after resume, got: " << stateResumed;

    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(PosixPtyRunnerTest, StartInvalidProgram)
{
    // forkpty succeeds even for a nonexistent program; execvp fails in the
    // child which calls qFatal/abort.  The parent detects the abnormal exit.
    ASSERT_TRUE(runner->start("/nonexistent/program", {}));
    ASSERT_TRUE(runner->waitForFinished(10000));
    EXPECT_FALSE(runner->isRunning());
}

TEST_F(PosixPtyRunnerTest, DestructorCleansUp)
{
    {
        PosixPtyRunner r;
        ASSERT_TRUE(r.start("/bin/sleep", {"30"}));
        EXPECT_TRUE(r.isRunning());
    }
    SUCCEED();
}

TEST_F(PosixPtyRunnerTest, MasterFdAccessible)
{
    ASSERT_TRUE(runner->start("/bin/echo", {"test"}));
    EXPECT_GE(runner->masterFd(), 0);
    ASSERT_TRUE(runner->waitForFinished(10000));
}

TEST_F(PosixPtyRunnerTest, PasswordCallbackInvokedOnPrompt)
{
    bool callbackCalled = false;
    runner->setPasswordCallback([&callbackCalled]() -> QByteArray {
        callbackCalled = true;
        return QByteArray("secret123\n");
    });

    // Run a real PTY process that outputs a password prompt, reads the
    // response, and echoes it back.  The runner should detect the prompt
    // in the PTY output and invoke the callback automatically.
    ASSERT_TRUE(runner->start("/bin/sh", {
        "-c", "echo 'Enter password:'; read line; echo \"got: $line\""
    }));
    ASSERT_TRUE(runner->waitForFinished(10000));

    EXPECT_TRUE(callbackCalled);

    QByteArray out = runner->readAllStandardOutput();
    EXPECT_TRUE(out.contains("got: secret123"));
}

TEST_F(PosixPtyRunnerTest, SetTimeoutUsedByWaitForFinished)
{
    runner->setTimeout(100);
    ASSERT_TRUE(runner->start("/bin/sleep", {"30"}));
    // waitForFinished() with no args (default -1) should use m_timeoutMs=100
    EXPECT_FALSE(runner->waitForFinished());
    EXPECT_TRUE(runner->isRunning());
    runner->cancel();
    EXPECT_FALSE(runner->isRunning());
}
