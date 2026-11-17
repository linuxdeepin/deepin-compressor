// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTRACT_TEST_H
#define CONTRACT_TEST_H

#include "archiveinterface.h"
#include "datamanager.h"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <string>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QProcess>
#include <QEventLoop>
#include <QTimer>

// ---------------------------------------------------------------------------
// Factory type aliases — a self-developed backend only needs to provide
// a callable matching one of these signatures to be covered by the
// unified contract test suite.
// ---------------------------------------------------------------------------
using ReadOnlyBackendFactory =
    std::function<ReadOnlyArchiveInterface *(const QString &archivePath)>;
using ReadWriteBackendFactory =
    std::function<ReadWriteArchiveInterface *(const QString &archivePath)>;

// ---------------------------------------------------------------------------
// Build QVariantList from a real archive path using QMimeDatabase to detect
// the MIME type by content — the same pattern the individual plugin tests use.
// ---------------------------------------------------------------------------
inline QVariantList makePluginArgs(const QString &archivePath)
{
    KPluginMetaData metaData;
    QMimeDatabase db;
    const QMimeType mimeFromContent = QFileInfo::exists(archivePath)
        ? db.mimeTypeForFile(archivePath, QMimeDatabase::MatchContent)
        : db.mimeTypeForFile(archivePath, QMimeDatabase::MatchExtension);
    QVariantList args;
    args << QVariant(archivePath)
         << QVariant::fromValue(metaData)
         << QVariant::fromValue(mimeFromContent);
    return args;
}

// ---------------------------------------------------------------------------
// SignalTracker — captures signal emissions from ReadOnlyArchiveInterface
// without requiring QtTest's QSignalSpy (which is unavailable in this env).
// ---------------------------------------------------------------------------
class SignalTracker : public QObject
{
public:
    int finishedCount = 0;
    int progressCount = 0;
    int curFileNameCount = 0;
    int cancelCount = 0;
    int queryCount = 0;
    PluginFinishType lastFinishType = PFT_Nomral;
    double lastProgress = -1.0;
    QString lastCurFileName;

    void connect(ReadOnlyArchiveInterface *iface)
    {
        QObject::connect(iface, &ReadOnlyArchiveInterface::signalFinished,
                         this, [this](PluginFinishType t) {
                             finishedCount++;
                             lastFinishType = t;
                         });
        QObject::connect(iface, &ReadOnlyArchiveInterface::signalprogress,
                         this, [this](double p) {
                             progressCount++;
                             lastProgress = p;
                         });
        QObject::connect(iface, &ReadOnlyArchiveInterface::signalCurFileName,
                         this, [this](const QString &n) {
                             curFileNameCount++;
                             lastCurFileName = n;
                         });
        QObject::connect(iface, &ReadOnlyArchiveInterface::signalCancel,
                         this, [this]() {
                             cancelCount++;
                         });
        QObject::connect(iface, &ReadOnlyArchiveInterface::signalQuery,
                         this, [this](Query *) {
                             queryCount++;
                         });
    }

    void reset()
    {
        finishedCount = 0;
        progressCount = 0;
        curFileNameCount = 0;
        cancelCount = 0;
        queryCount = 0;
        lastFinishType = PFT_Nomral;
        lastProgress = -1.0;
        lastCurFileName.clear();
    }
};

// ---------------------------------------------------------------------------
// Wait for signalFinished on async backends (CLI plugins that call
// setWaitForFinishedSignal(true)).  Uses QEventLoop with a timeout so the
// test never hangs.
// ---------------------------------------------------------------------------
inline void waitForFinishedSignal(ReadOnlyArchiveInterface *iface, int timeoutMs = 15000)
{
    if (!iface->waitForFinished())
        return; // synchronous backend — nothing to wait for

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(iface, &ReadOnlyArchiveInterface::signalFinished,
                     &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();
    timer.stop();
}

// Run an operation and, for asynchronous backends, wait for the final
// signalFinished value instead of relying on the initial return value.
inline PluginFinishType runInterfaceOp(ReadOnlyArchiveInterface *iface,
                                       bool isAsync,
                                       std::function<PluginFinishType()> op)
{
    if (!isAsync)
        return op();

    bool finished = false;
    PluginFinishType finalType = PFT_Error;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    const auto connection = QObject::connect(
        iface, &ReadOnlyArchiveInterface::signalFinished, &loop,
        [&finished, &finalType, &loop](PluginFinishType type) {
            finished = true;
            finalType = type;
            loop.quit();
        });

    const PluginFinishType initialType = op();
    if (!finished) {
        timer.start(15000);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        loop.exec();
        timer.stop();
    }
    QObject::disconnect(connection);

    Q_UNUSED(initialType);
    return finished ? finalType : PFT_Error;
}

// ---------------------------------------------------------------------------
// ReadOnlyBackendCase — declares a read-only backend factory along with
// capability flags so the contract test only exercises supported operations.
// ---------------------------------------------------------------------------
struct ReadOnlyBackendCase {
    std::string name;
    ReadOnlyBackendFactory factory;
    QString fixtureKind;          // zip, tar, 7z, rar, gz, bz2, xz
    QString archivePath;          // relative fixture path
    bool isAsync = false;                  // CLI backends: list/extract are async
    bool supportsList = true;
    bool supportsTestArchive = true;
    bool supportsExtractFiles = true;
    bool canTestCorrupt = false;          // testArchive() detects corrupt archives
    bool canTestPasswordArchive = false;  // testArchive() detects password-protected archives
    bool canExtractWrongPassword = false; // extractFiles() with wrong password returns error
};

// ---------------------------------------------------------------------------
// ReadWriteBackendCase — declares a read-write backend factory along with
// capability flags so the contract test only exercises supported operations.
// ---------------------------------------------------------------------------
struct ReadWriteBackendCase {
    std::string name;
    ReadWriteBackendFactory factory;
    QString fixtureKind;
    QString archivePath;
    bool isAsync = false;
    bool supportsAddFiles = false;
    bool supportsDeleteFiles = false;
    bool supportsRenameFiles = false;
    bool supportsAddComment = false;
    bool supportsUpdateArchiveData = true;
    bool unsupportedAddCommentReturnsError = true;
};

// ---------------------------------------------------------------------------
// Test fixture helpers — create real archives on disk using system tools,
// clean up automatically via QTemporaryDir.  All QProcess calls are checked
// for successful start, completion, and exit code.
// ---------------------------------------------------------------------------
class ContractTestFixture
{
public:
    ContractTestFixture()
    {
        m_tempDir = new QTemporaryDir();
        m_tempDir->setAutoRemove(true);
    }

    ~ContractTestFixture()
    {
        delete m_tempDir;
    }

    QString tempPath() const { return m_tempDir->path(); }

    // Create a plain text file with known content.
    QString makeFile(const QString &relativePath, const QString &content = "hello world")
    {
        QString fullPath = m_tempDir->filePath(relativePath);
        if (!QDir().mkpath(QFileInfo(fullPath).absolutePath())) {
            ADD_FAILURE() << "Failed to create directory for: " << fullPath.toStdString();
            return {};
        }
        QFile f(fullPath);
        if (!f.open(QIODevice::WriteOnly)) {
            ADD_FAILURE() << "Failed to open file for writing: " << fullPath.toStdString();
            return {};
        }
        if (f.write(content.toUtf8()) != content.toUtf8().size()) {
            ADD_FAILURE() << "Failed to write fixture file: " << fullPath.toStdString();
            return {};
        }
        f.close();
        return fullPath;
    }

    // Create a nested directory structure: dir/sub/deep.txt
    QString makeNestedDir(const QString &relativeBase)
    {
        QString base = m_tempDir->filePath(relativeBase);
        if (!QDir().mkpath(base + "/sub")) {
            ADD_FAILURE() << "Failed to create nested fixture directory: "
                          << base.toStdString();
            return {};
        }
        QFile f(base + "/sub/deep.txt");
        if (!f.open(QIODevice::WriteOnly)) {
            ADD_FAILURE() << "Failed to open nested fixture file";
            return {};
        }
        if (f.write("nested content") != 14) {
            ADD_FAILURE() << "Failed to write nested fixture file";
            return {};
        }
        f.close();
        return base;
    }

    // Run a command and assert success — checked start, finish, and exit code.
    bool runChecked(const QString &program, const QStringList &args,
                    const QString &workDir = QString())
    {
        QProcess proc;
        if (!workDir.isEmpty())
            proc.setWorkingDirectory(workDir);
        proc.start(program, args);
        if (!proc.waitForStarted(5000)) {
            ADD_FAILURE() << "QProcess failed to start: " << program.toStdString()
                          << " error=" << proc.error();
            return false;
        }
        if (!proc.waitForFinished(30000)) {
            ADD_FAILURE() << "QProcess timed out: " << program.toStdString();
            proc.kill();
            proc.waitForFinished(3000);
            return false;
        }
        if (proc.exitCode() != 0) {
            ADD_FAILURE() << "QProcess exited with code " << proc.exitCode()
                          << ": " << program.toStdString()
                          << " stderr=" << proc.readAllStandardError().toStdString();
            return false;
        }
        return true;
    }

    // Create a normal zip archive containing file1.txt and dir/file2.txt
    QString makeZipArchive(const QString &relativePath)
    {
        QString fullPath = m_tempDir->filePath(relativePath);
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        // Create source files
        if (makeFile("zip_src/file1.txt", "file1 content").isEmpty())
            return {};
        if (makeNestedDir("zip_src/dir").isEmpty())
            return {};
        // makeNestedDir creates zip_src/dir/sub/deep.txt, we need dir/file2.txt
        if (makeFile("zip_src/dir/file2.txt", "file2 content").isEmpty())
            return {};

        if (!runChecked("zip", QStringList() << "-r" << fullPath << "file1.txt" << "dir",
                        m_tempDir->filePath("zip_src"))) {
            return {};
        }
        return fullPath;
    }

    // Create a normal tar archive
    QString makeTarArchive(const QString &relativePath)
    {
        QString fullPath = m_tempDir->filePath(relativePath);
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        if (makeFile("tar_src/test.txt", "test content").isEmpty()
            || makeFile("tar_src/test1.txt", "test1 content").isEmpty()) {
            return {};
        }

        if (!runChecked("tar", QStringList() << "cf" << fullPath << "test.txt" << "test1.txt",
                        m_tempDir->filePath("tar_src"))) {
            return {};
        }
        return fullPath;
    }

    // Copy a checked-in fixture into the temporary workspace.
    QString copyExisting(const QString &source, const QString &relativeDest)
    {
        if (!QFile::exists(source)) {
            ADD_FAILURE() << "Fixture does not exist: " << source.toStdString();
            return {};
        }

        QString dest = m_tempDir->filePath(relativeDest);
        QDir().mkpath(QFileInfo(dest).absolutePath());
        if (!QFile::copy(source, dest)) {
            ADD_FAILURE() << "Failed to copy fixture: " << source.toStdString()
                          << " -> " << dest.toStdString();
            return {};
        }
        return dest;
    }

    // Create the fixture appropriate for the backend from a relative spec.
    QString makeArchive(const QString &kind, const QString &relativePath)
    {
        if (kind == "zip")
            return makeZipArchive(relativePath);
        if (kind == "tar")
            return makeTarArchive(relativePath);
        if (kind == "7z")
            return make7zArchive(relativePath);
        if (kind == "rar")
            return copyExisting(
                QString(_SOURCEDIR) +
                    "/tests/FuzzyTest/libfuzzer/libfuzzer_in/archives/common/rar/small_archive.rar",
                relativePath);
        if (kind == "gz")
            return copyExisting(
                QString(_UTSOURCEDIR) + "/test_sources/gz/test.gz", relativePath);
        if (kind == "bz2")
            return copyExisting(
                QString(_UTSOURCEDIR) + "/test_sources/bz2/test.txt.bz2", relativePath);
        if (kind == "xz")
            return copyExisting(
                QString(_UTSOURCEDIR) + "/test_sources/xz/test.xz", relativePath);

        ADD_FAILURE() << "Unknown fixture kind: " << kind.toStdString();
        return {};
    }

    // Create a 7z archive for CLI backend tests
    QString make7zArchive(const QString &relativePath)
    {
        QString fullPath = m_tempDir->filePath(relativePath);
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        if (makeFile("7z_src/file1.txt", "7z file1 content").isEmpty()
            || makeFile("7z_src/file2.txt", "7z file2 content").isEmpty()) {
            return {};
        }

        if (!runChecked("7z", QStringList() << "a" << fullPath
                         << "file1.txt" << "file2.txt",
                        m_tempDir->filePath("7z_src"))) {
            return {};
        }
        return fullPath;
    }

    // Create a password-protected zip archive
    QString makePasswordZip(const QString &relativePath, const QString &password = "secret")
    {
        QString fullPath = m_tempDir->filePath(relativePath);
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        if (makeFile("pw_src/file1.txt", "password protected content").isEmpty())
            return {};

        if (!runChecked("7z", QStringList() << "a" << ("-p" + password)
                         << fullPath << "file1.txt",
                        m_tempDir->filePath("pw_src"))) {
            return {};
        }
        return fullPath;
    }

    // Create a corrupted archive by overwriting the header with garbage AND
    // truncating the tail. Overwriting the start corrupts tar headers so
    // archive_read_next_header() fails; truncating the tail removes the zip
    // central directory so zip_open() fails. Both operations are applied so
    // the test works regardless of archive format.
    QString makeCorruptArchive(const QString &relativePath, const QString &sourceArchive)
    {
        QString fullPath = m_tempDir->filePath(relativePath);
        QDir().mkpath(QFileInfo(fullPath).absolutePath());
        if (!QFile::copy(sourceArchive, fullPath)) {
            ADD_FAILURE() << "Failed to copy corrupt fixture source: "
                          << sourceArchive.toStdString();
            return {};
        }
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadWrite)) {
            ADD_FAILURE() << "Failed to open corrupt fixture: " << fullPath.toStdString();
            return {};
        }
        qint64 garbageSize = qMin(file.size(), qint64(256));
        QByteArray garbage(garbageSize, char(0xFF));
        if (file.write(garbage) != garbageSize) {
            ADD_FAILURE() << "Failed to corrupt fixture: " << fullPath.toStdString();
            return {};
        }
        file.close();
        qint64 halfSize = QFileInfo(fullPath).size() / 2;
        if (!QFile::resize(fullPath, halfSize)) {
            ADD_FAILURE() << "Failed to truncate corrupt fixture: " << fullPath.toStdString();
            return {};
        }
        return fullPath;
    }

    // Copy a file to the temp dir with a new name
    QString copyToTemp(const QString &source, const QString &relativeDest)
    {
        QString dest = m_tempDir->filePath(relativeDest);
        QDir().mkpath(QFileInfo(dest).absolutePath());
        if (!QFile::copy(source, dest)) {
            ADD_FAILURE() << "Failed to copy file: " << source.toStdString()
                          << " -> " << dest.toStdString();
            return {};
        }
        return dest;
    }

private:
    QTemporaryDir *m_tempDir;
};

// ---------------------------------------------------------------------------
// ReadOnly contract fixture — parametrised on ReadOnlyBackendCase
// ---------------------------------------------------------------------------
class ReadOnlyContractTest
    : public ::testing::TestWithParam<ReadOnlyBackendCase>
{
protected:
    void SetUp() override
    {
        const auto &param = GetParam();
        m_fixture = std::make_unique<ContractTestFixture>();
        m_archivePath = m_fixture->makeArchive(param.fixtureKind, param.archivePath);
        ASSERT_FALSE(m_archivePath.isEmpty());
        m_iface = std::unique_ptr<ReadOnlyArchiveInterface>(
            param.factory(m_archivePath));
        ASSERT_NE(m_iface, nullptr);
    }

    void TearDown() override
    {
        m_iface.reset();
        m_fixture.reset();
        DataManager::get_instance().resetArchiveData();
    }

    // Helper: run an async operation and wait for signalFinished if needed.
    PluginFinishType runOp(std::function<PluginFinishType()> op)
    {
        return runInterfaceOp(m_iface.get(), GetParam().isAsync, op);
    }

    std::unique_ptr<ReadOnlyArchiveInterface> m_iface;
    std::unique_ptr<ContractTestFixture> m_fixture;
    QString m_archivePath;
};

// ---------------------------------------------------------------------------
// ReadWrite contract fixture — parametrised on ReadWriteBackendCase
// ---------------------------------------------------------------------------
class ReadWriteContractTest
    : public ::testing::TestWithParam<ReadWriteBackendCase>
{
protected:
    void SetUp() override
    {
        const auto &param = GetParam();
        m_fixture = std::make_unique<ContractTestFixture>();
        m_archivePath = m_fixture->makeArchive(param.fixtureKind, param.archivePath);
        ASSERT_FALSE(m_archivePath.isEmpty());
        m_iface = std::unique_ptr<ReadWriteArchiveInterface>(
            param.factory(m_archivePath));
        ASSERT_NE(m_iface, nullptr);
    }

    void TearDown() override
    {
        m_iface.reset();
        m_fixture.reset();
        DataManager::get_instance().resetArchiveData();
    }

    PluginFinishType runOp(std::function<PluginFinishType()> op)
    {
        return runInterfaceOp(m_iface.get(), GetParam().isAsync, op);
    }

    PluginFinishType runOpOn(ReadWriteArchiveInterface *iface,
                             std::function<PluginFinishType()> op)
    {
        return runInterfaceOp(iface, GetParam().isAsync, op);
    }

    std::unique_ptr<ReadWriteArchiveInterface> m_iface;
    std::unique_ptr<ContractTestFixture> m_fixture;
    QString m_archivePath;
};

#endif // CONTRACT_TEST_H
