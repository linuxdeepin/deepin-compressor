// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "legacy_clirar_adapter.h"
#include "rar_backend.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <memory>

namespace {

QString rarSource()
{
    return QStringLiteral(CONTRACT_TEST_SOURCE_DIR)
            + QStringLiteral("/../FuzzyTest/libfuzzer/libfuzzer_in/archives/common/rar/small_archive.rar");
}

enum class BackendKind {
    Legacy,
    New,
};

std::unique_ptr<dcarchive::ArchiveBackend> createBackend(BackendKind kind,
                                                         const QString &path,
                                                         const QString &mime)
{
    if (kind == BackendKind::Legacy) {
        const QString metaData = QStringLiteral(CONTRACT_TEST_SOURCE_DIR)
                + QStringLiteral("/../../3rdparty/clirarplugin/kerfuffle_clirar.json");
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyCliRarAdapter(path, mime, metaData));
    }
    return std::unique_ptr<dcarchive::ArchiveBackend>(
            new dcarchive::RarBackend(path, mime));
}

class CliRarContractTest : public ::testing::TestWithParam<BackendKind>
{
protected:
    void SetUp() override
    {
        if (QStandardPaths::findExecutable(QStringLiteral("unrar")).isEmpty())
            GTEST_SKIP() << "unrar is not available";
        ASSERT_TRUE(m_workspace.isValid());
        m_archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("test.rar"));
        ASSERT_TRUE(QFile::copy(rarSource(), m_archivePath));
        m_backend = createBackend(GetParam(), m_archivePath,
                                  QStringLiteral("application/vnd.rar"));
        dcarchive::OpenRequest request;
        request.path = m_archivePath;
        request.mimeType = QStringLiteral("application/vnd.rar");
        ASSERT_TRUE(m_backend->open(request).result.ok());
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    QString targetPath() const { return m_workspace.path(); }

    std::unique_ptr<dcarchive::ArchiveBackend> m_backend;
    dcarchive::FakeObserver m_observer;
    QTemporaryDir m_workspace;
    QString m_archivePath;
};

TEST_P(CliRarContractTest, ListReturnsEntries)
{
    const dcarchive::EntryResult listed = m_backend->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_GE(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().name, QStringLiteral("limerick"));
    EXPECT_EQ(listed.info.entries.first().size, 191);
}

TEST_P(CliRarContractTest, ExtractWritesFile)
{
    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = m_backend->extractFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const QString extracted = QDir(targetPath()).filePath(QStringLiteral("limerick"));
    ASSERT_TRUE(QFileInfo::exists(extracted));
    EXPECT_EQ(QFileInfo(extracted).size(), 191);
}

TEST(RarExtractionTest, SkipConflictPreservesExistingFile)
{
    if (QStandardPaths::findExecutable(QStringLiteral("unrar")).isEmpty())
        GTEST_SKIP() << "unrar is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("test.rar"));
    ASSERT_TRUE(QFile::copy(rarSource(), archive));
    const QString existingPath = workspace.filePath(QStringLiteral("limerick"));
    QFile existing(existingPath);
    const QByteArray original("keep existing data");
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    ASSERT_EQ(existing.write(original), original.size());
    existing.close();

    RarBackend backend(archive, QStringLiteral("application/vnd.rar"));
    ExtractRequest request;
    request.targetPath = workspace.path();
    FakeObserver observer;
    observer.conflictResponse = FileConflictRequest::Response::Skip;
    const auto result = backend.extractFiles(request, observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    ASSERT_EQ(observer.conflictRequests.size(), 1);
    ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
    EXPECT_EQ(existing.readAll(), original);
}

TEST(RarPasswordTest, EncryptedMultipartArchiveListsAndExtracts)
{
    const QString rar = QStandardPaths::findExecutable(QStringLiteral("rar"));
    if (rar.isEmpty() || QStandardPaths::findExecutable(QStringLiteral("unrar")).isEmpty())
        GTEST_SKIP() << "rar and unrar are required";

    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString fileName = QStringLiteral("\u5f52\u6863 payload.txt");
    QFile input(workspace.filePath(fileName));
    const QByteArray expected(32 * 1024, 'x');
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write(expected), expected.size());
    input.close();

    const QString password = QStringLiteral("rar-test-password");
    QProcess creator;
    creator.setWorkingDirectory(workspace.path());
    creator.start(rar, {QStringLiteral("a"), QStringLiteral("-cfg-"), QStringLiteral("-m0"),
                        QStringLiteral("-v4k"), QStringLiteral("-hp") + password,
                        QStringLiteral("sample.rar"), fileName});
    ASSERT_TRUE(creator.waitForFinished(30000));
    ASSERT_EQ(creator.exitCode(), 0) << creator.readAllStandardError().constData();

    const QStringList volumes = QDir(workspace.path()).entryList({QStringLiteral("sample.part*.rar")},
                                                                QDir::Files, QDir::Name);
    ASSERT_GT(volumes.size(), 1);
    RarBackend reader(workspace.filePath(volumes.first()), QStringLiteral("application/vnd.rar"));
    EXPECT_EQ(reader.list().result.error, ArchiveError::PasswordRequired);
    OpenRequest open;
    open.password = QStringLiteral("wrong-password");
    ASSERT_TRUE(reader.open(open).result.ok());
    EXPECT_EQ(reader.list().result.error, ArchiveError::WrongPassword);

    ExtractRequest extract;
    extract.targetPath = workspace.filePath(QStringLiteral("out"));
    ASSERT_TRUE(QDir().mkpath(extract.targetPath));
    FakeObserver observer;
    EXPECT_EQ(reader.extractFiles(extract, observer).result.error, ArchiveError::Cancelled);
    EXPECT_FALSE(QFileInfo::exists(QDir(extract.targetPath).filePath(fileName)));
    observer.reset();
    observer.passwordResponse = PasswordRequest::Response::Provide;
    observer.passwordValue = password;
    const auto extracted = reader.extractFiles(extract, observer);
    ASSERT_TRUE(extracted.result.ok()) << extracted.result.message.toStdString();
    EXPECT_EQ(observer.passwordRequests.size(), 1);

    const auto list = reader.list();
    ASSERT_TRUE(list.result.ok()) << list.result.message.toStdString();
    ASSERT_EQ(list.info.entries.size(), 1);
    EXPECT_TRUE(list.info.encrypted);
    EXPECT_EQ(list.info.entries.first().path, fileName);
    EXPECT_EQ(list.info.totalSize, expected.size());
    QFile output(QDir(extract.targetPath).filePath(fileName));
    ASSERT_TRUE(output.open(QIODevice::ReadOnly));
    EXPECT_EQ(output.readAll(), expected);
}

TEST(RarWriteTest, CreateAppendRenameCommentAndDelete)
{
    if (QStandardPaths::findExecutable(QStringLiteral("rar")).isEmpty()
            || QStandardPaths::findExecutable(QStringLiteral("unrar")).isEmpty())
        GTEST_SKIP() << "rar and unrar are required";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString path = workspace.filePath(QStringLiteral("archive.rar"));
    RarBackend backend(path, QStringLiteral("application/vnd.rar"));
    ASSERT_TRUE(backend.capabilities().create);
    const QStringList names{QStringLiteral("\u6d4b\u8bd5 first.txt"), QStringLiteral("second.txt")};
    FakeObserver observer;
    for (const QString &name : names) {
        QFile source(workspace.filePath(name));
        ASSERT_TRUE(source.open(QIODevice::WriteOnly));
        ASSERT_EQ(source.write("payload"), 7);
        source.close();
        ArchiveEntry entry;
        entry.path = source.fileName();
        AddRequest add;
        add.entries.append(entry);
        const auto added = backend.addFiles(add, observer);
        ASSERT_TRUE(added.result.ok()) << added.result.message.toStdString();
    }
    RenameRequest rename;
    rename.entry.path = names.first();
    rename.newName = QStringLiteral("renamed.txt");
    ASSERT_TRUE(backend.renameFiles(rename).result.ok());
    const QString comment = QStringLiteral("RAR contract comment");
    ASSERT_TRUE(backend.addComment(comment).result.ok());
    const auto listed = backend.list();
    ASSERT_TRUE(listed.result.ok());
    ASSERT_EQ(listed.info.entries.size(), 2);
    EXPECT_EQ(listed.info.comment, comment);
    QStringList paths;
    for (const ArchiveEntry &entry : listed.info.entries)
        paths.append(entry.path);
    EXPECT_TRUE(paths.contains(rename.newName));
    EXPECT_TRUE(paths.contains(names.last()));

    DeleteRequest remove;
    ArchiveEntry target;
    target.path = names.last();
    remove.entries.append(target);
    ASSERT_TRUE(backend.deleteFiles(remove).result.ok());
    const auto afterDelete = backend.list();
    ASSERT_TRUE(afterDelete.result.ok());
    ASSERT_EQ(afterDelete.info.entries.size(), 1);
    EXPECT_EQ(afterDelete.info.entries.first().path, rename.newName);
    EXPECT_TRUE(backend.testArchive().result.ok());
}

INSTANTIATE_TEST_SUITE_P(CliRarBackends,
                         CliRarContractTest,
                         ::testing::Values(BackendKind::Legacy, BackendKind::New),
                         [](const ::testing::TestParamInfo<BackendKind> &info) {
                             return info.param == BackendKind::Legacy ? "legacy" : "new";
                         });

} // namespace
