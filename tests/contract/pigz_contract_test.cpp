// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "legacy_pigz_adapter.h"
#include "libarchive_backend.h"
#include "pigz_backend.h"

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QThread>

#include <memory>

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
                new dcarchive::LegacyPigzAdapter(path, mime));
    return std::unique_ptr<dcarchive::ArchiveBackend>(
            new dcarchive::PigzBackend(path, mime));
}

bool waitForNonEmptyFile(const QString &path, int timeoutMs)
{
    const int step = 50;
    for (int waited = 0; waited < timeoutMs; waited += step) {
        QCoreApplication::processEvents();
        if (QFileInfo(path).exists() && QFileInfo(path).size() > 0)
            return true;
        QThread::msleep(step);
    }
    return false;
}

class PigzContractTest : public ::testing::TestWithParam<BackendKind>
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_workspace.isValid());
        m_archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("created.tar.gz"));
        m_backend = createBackend(GetParam(), m_archivePath,
                                  QStringLiteral("application/x-compressed-tar"));
        dcarchive::OpenRequest request;
        request.path = m_archivePath;
        request.mimeType = QStringLiteral("application/x-compressed-tar");
        ASSERT_TRUE(m_backend->open(request).result.ok());
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    dcarchive::ArchiveBackend *backend() { return m_backend.get(); }

    std::unique_ptr<dcarchive::ArchiveBackend> m_backend;
    dcarchive::FakeObserver m_observer;
    QTemporaryDir m_workspace;
    QString m_archivePath;
};

TEST_P(PigzContractTest, AddCreatesTarGzArchive)
{
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    dcarchive::ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("test.txt");
    entry.size = QFileInfo(source).size();
    entry.type = dcarchive::EntryType::File;

    dcarchive::AddRequest request;
    request.archivePath = m_archivePath;
    request.format = QStringLiteral("tar.gz");
    request.compressionLevel = 6;
    request.cpuThreads = 1;
    request.entries.append(entry);

    const dcarchive::VoidResult result = backend()->addFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    ASSERT_TRUE(waitForNonEmptyFile(m_archivePath, 15000));

    dcarchive::LibarchiveBackend verifier(m_archivePath,
                                          QStringLiteral("application/x-compressed-tar"));
    const dcarchive::EntryResult listed = verifier.list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_GE(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().name, QStringLiteral("test.txt"));
}

TEST(PigzWriteTest, TarFailureLeavesExistingArchiveIntact)
{
    if (QStandardPaths::findExecutable(QStringLiteral("pigz")).isEmpty())
        GTEST_SKIP() << "pigz is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString path = workspace.filePath(QStringLiteral("archive.tar.gz"));
    const QString mime = QStringLiteral("application/x-compressed-tar");
    PigzBackend writer(path, mime);
    FakeObserver observer;

    ArchiveEntry source;
    source.path = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    AddRequest add;
    add.entries.append(source);
    ASSERT_TRUE(writer.addFiles(add, observer).result.ok());

    QFile archive(path);
    ASSERT_TRUE(archive.open(QIODevice::ReadOnly));
    const QByteArray original = archive.readAll();
    ASSERT_FALSE(original.isEmpty());
    archive.close();

    QFile replacement(workspace.filePath(QStringLiteral("replacement.txt")));
    ASSERT_TRUE(replacement.open(QIODevice::WriteOnly));
    ASSERT_EQ(replacement.write("replacement"), 11);
    replacement.close();
    add.entries.first().path = replacement.fileName();
    ArchiveEntry missing;
    missing.path = workspace.filePath(QStringLiteral("missing.txt"));
    add.entries.append(missing);
    observer.reset();
    EXPECT_FALSE(writer.addFiles(add, observer).result.ok());
    ASSERT_EQ(observer.finishes.size(), 1);
    EXPECT_FALSE(observer.finishes.first().result.ok());
    ASSERT_TRUE(archive.open(QIODevice::ReadOnly));
    EXPECT_EQ(archive.readAll(), original);
}

TEST(PigzWriteTest, TemporaryArchiveIsNotPackedWhenOutputIsInsideSource)
{
    if (QStandardPaths::findExecutable(QStringLiteral("pigz")).isEmpty())
        GTEST_SKIP() << "pigz is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString sourcePath = workspace.filePath(QStringLiteral("source"));
    ASSERT_TRUE(QDir().mkpath(sourcePath));
    const QString path = QDir(sourcePath).filePath(QStringLiteral("archive.tar.gz"));
    QFile input(QDir(sourcePath).filePath(QStringLiteral("test.txt")));
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write("payload"), 7);
    input.close();

    const QString mime = QStringLiteral("application/x-compressed-tar");
    PigzBackend writer(path, mime);
    ArchiveEntry entry;
    entry.path = sourcePath;
    AddRequest add;
    add.entries.append(entry);
    FakeObserver observer;
    const auto created = writer.addFiles(add, observer);
    ASSERT_TRUE(created.result.ok()) << created.result.message.toStdString();

    LibarchiveBackend reader(path, mime);
    const auto listed = reader.list();
    ASSERT_TRUE(listed.result.ok());
    ASSERT_EQ(listed.info.entries.size(), 2);
    EXPECT_EQ(listed.info.entries.last().path, QStringLiteral("source/test.txt"));
}

INSTANTIATE_TEST_SUITE_P(PigzBackends,
                         PigzContractTest,
                         ::testing::Values(BackendKind::Legacy, BackendKind::New),
                         [](const ::testing::TestParamInfo<BackendKind> &info) {
                             return info.param == BackendKind::Legacy ? "legacy" : "new";
                         });

} // namespace
