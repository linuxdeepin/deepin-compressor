// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "legacy_cli7z_adapter.h"
#include "sevenzip_backend.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <memory>

namespace {

QString testSourceDir()
{
    return QStringLiteral(CONTRACT_TEST_SOURCE_DIR);
}

bool have7z()
{
    return !QStandardPaths::findExecutable(QStringLiteral("7z")).isEmpty();
}

enum class BackendKind {
    Legacy,
    New,
};

std::unique_ptr<dcarchive::ArchiveBackend> createBackend(BackendKind kind,
                                                         const QString &archive,
                                                         const QString &mime,
                                                         const QString &metaData)
{
    if (kind == BackendKind::Legacy)
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyCli7zAdapter(archive, mime, metaData));
    return std::unique_ptr<dcarchive::ArchiveBackend>(
            new dcarchive::SevenZipBackend(archive, mime));
}

bool create7zArchive(const QString &archive, const QString &source)
{
    QProcess process;
    process.start(QStringLiteral("7z"),
                  {QStringLiteral("a"), QStringLiteral("-y"), archive, source});
    process.waitForFinished(60000);
    return process.exitStatus() == QProcess::NormalExit
            && process.exitCode() == 0
            && QFileInfo(archive).size() > 0;
}

class Cli7zContractTest : public ::testing::TestWithParam<BackendKind>
{
protected:
    void SetUp() override
    {
        if (!have7z())
            GTEST_SKIP() << "7z is not available";
        ASSERT_TRUE(m_workspace.isValid());
        m_archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("test.7z"));
        m_sourcePath = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
        ASSERT_TRUE(create7zArchive(m_archivePath, m_sourcePath));
        m_backend = createBackend(GetParam(), m_archivePath,
                                  QStringLiteral("application/x-7z-compressed"), metaDataPath());
        dcarchive::OpenRequest request;
        request.path = m_archivePath;
        request.mimeType = QStringLiteral("application/x-7z-compressed");
        ASSERT_TRUE(m_backend->open(request).result.ok());
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    static QString metaDataPath()
    {
        return QStringLiteral(CONTRACT_TEST_SOURCE_DIR)
                + QStringLiteral("/../../3rdparty/cli7zplugin/kerfuffle_cli7z.json");
    }

    QString targetPath() const { return m_workspace.path(); }

    std::unique_ptr<dcarchive::ArchiveBackend> m_backend;
    dcarchive::FakeObserver m_observer;
    QTemporaryDir m_workspace;
    QString m_archivePath;
    QString m_sourcePath;
};

TEST_P(Cli7zContractTest, ListReturnsEntries)
{
    const dcarchive::EntryResult listed = m_backend->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_GE(listed.info.entries.size(), 1);

    QStringList names;
    for (const dcarchive::ArchiveEntry &entry : listed.info.entries)
        names.append(entry.name);
    EXPECT_TRUE(names.contains(QStringLiteral("test.txt")));
}

TEST_P(Cli7zContractTest, ExtractWritesFile)
{
    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = m_backend->extractFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const QString extracted = QDir(targetPath()).filePath(QStringLiteral("test.txt"));
    ASSERT_TRUE(QFileInfo::exists(extracted));
    EXPECT_EQ(QFileInfo(extracted).size(), QFileInfo(m_sourcePath).size());
}

TEST(SevenZipExtractionTest, SkipConflictPreservesExistingFile)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("test.7z"));
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    ASSERT_TRUE(create7zArchive(archive, source));

    const QString existingPath = workspace.filePath(QStringLiteral("test.txt"));
    QFile existing(existingPath);
    const QByteArray original("keep existing data");
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    ASSERT_EQ(existing.write(original), original.size());
    existing.close();

    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
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

TEST(SevenZipExtractionTest, OverwriteConflictReplacesExistingFile)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("test.7z"));
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    ASSERT_TRUE(create7zArchive(archive, source));

    const QString existingPath = workspace.filePath(QStringLiteral("test.txt"));
    QFile existing(existingPath);
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    ASSERT_EQ(existing.write("old"), 3);
    existing.close();

    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
    ExtractRequest request;
    request.targetPath = workspace.path();
    FakeObserver observer;
    observer.conflictResponse = FileConflictRequest::Response::Overwrite;
    const auto result = backend.extractFiles(request, observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    ASSERT_EQ(observer.conflictRequests.size(), 1);
    ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
    QFile expectedSource(source);
    ASSERT_TRUE(expectedSource.open(QIODevice::ReadOnly));
    EXPECT_EQ(existing.readAll(), expectedSource.readAll());
}

TEST(SevenZipExtractionTest, SkipInsideDirectoryDoesNotOverwriteDescendant)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString sourceRoot = workspace.filePath(QStringLiteral("source"));
    ASSERT_TRUE(QDir().mkpath(sourceRoot + QStringLiteral("/folder")));
    QFile first(sourceRoot + QStringLiteral("/folder/first.txt"));
    QFile second(sourceRoot + QStringLiteral("/folder/second.txt"));
    ASSERT_TRUE(first.open(QIODevice::WriteOnly));
    ASSERT_EQ(first.write("new-first"), 9);
    first.close();
    ASSERT_TRUE(second.open(QIODevice::WriteOnly));
    ASSERT_EQ(second.write("new-second"), 10);
    second.close();
    const QString archive = workspace.filePath(QStringLiteral("directory.7z"));
    ASSERT_TRUE(create7zArchive(archive, sourceRoot + QStringLiteral("/folder")));

    const QString target = workspace.filePath(QStringLiteral("target"));
    ASSERT_TRUE(QDir().mkpath(target + QStringLiteral("/folder")));
    QFile existing(target + QStringLiteral("/folder/first.txt"));
    const QByteArray original("original-first");
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    ASSERT_EQ(existing.write(original), original.size());
    existing.close();

    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
    ExtractRequest request;
    request.targetPath = target;
    FakeObserver observer;
    observer.conflictResponse = FileConflictRequest::Response::Skip;
    const auto result = backend.extractFiles(request, observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    ASSERT_EQ(observer.conflictRequests.size(), 1);
    ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
    EXPECT_EQ(existing.readAll(), original);
    QFile extractedSecond(target + QStringLiteral("/folder/second.txt"));
    ASSERT_TRUE(extractedSecond.open(QIODevice::ReadOnly));
    EXPECT_EQ(extractedSecond.readAll(), QByteArray("new-second"));
}

TEST(SevenZipCreationTest, DestinationPathIsPreserved)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString source = workspace.filePath(QStringLiteral("item.txt"));
    QFile input(source);
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write("payload"), 7);
    input.close();

    const QString archive = workspace.filePath(QStringLiteral("nested.7z"));
    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
    ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("item.txt");
    entry.type = EntryType::File;
    AddRequest add;
    add.entries.append(entry);
    add.destinationPath = QStringLiteral("nested");
    FakeObserver observer;
    ASSERT_TRUE(backend.addFiles(add, observer).result.ok());
    const auto listed = backend.list();
    ASSERT_TRUE(listed.result.ok());
    ASSERT_EQ(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().path, QStringLiteral("nested/item.txt"));
}

TEST(SevenZipCreationTest, Tar7zContainsInnerTar)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    const QString archive = workspace.filePath(QStringLiteral("payload.tar.7z"));
    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
    ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("test.txt");
    entry.type = EntryType::File;
    AddRequest add;
    add.entries.append(entry);
    add.tar7z = true;
    FakeObserver observer;
    const auto created = backend.addFiles(add, observer);
    ASSERT_TRUE(created.result.ok()) << created.result.message.toStdString();

    const auto listed = backend.list();
    ASSERT_TRUE(listed.result.ok());
    ASSERT_EQ(listed.info.entries.size(), 1);
    EXPECT_TRUE(listed.info.entries.first().path.endsWith(QStringLiteral(".tar")));
}

TEST(SevenZipCreationTest, Tar7zHandlesNestedDirectories)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString sourceDir = workspace.filePath(QStringLiteral("source"));
    ASSERT_TRUE(QDir().mkpath(sourceDir + QStringLiteral("/one/two")));
    QFile child(sourceDir + QStringLiteral("/one/two/file.txt"));
    ASSERT_TRUE(child.open(QIODevice::WriteOnly));
    ASSERT_EQ(child.write("nested"), 6);
    child.close();
    ArchiveEntry entry;
    entry.path = sourceDir + QStringLiteral("/one");
    entry.name = QStringLiteral("one");
    entry.type = EntryType::Directory;
    AddRequest add;
    add.entries.append(entry);
    add.tar7z = true;
    const QString archive = workspace.filePath(QStringLiteral("nested.tar.7z"));
    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
    FakeObserver observer;
    const auto result = backend.addFiles(add, observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    const auto listed = backend.list();
    ASSERT_TRUE(listed.result.ok());
    ASSERT_EQ(listed.info.entries.size(), 1);
}

TEST(SevenZipExtractionTest, DestinationPrefixIsStrippedFromOutput)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString sourceRoot = workspace.filePath(QStringLiteral("source"));
    ASSERT_TRUE(QDir().mkpath(sourceRoot + QStringLiteral("/folder")));
    QFile source(sourceRoot + QStringLiteral("/folder/item.txt"));
    ASSERT_TRUE(source.open(QIODevice::WriteOnly));
    ASSERT_EQ(source.write("payload"), 7);
    source.close();
    const QString archive = workspace.filePath(QStringLiteral("nested.7z"));
    ASSERT_TRUE(create7zArchive(archive, sourceRoot + QStringLiteral("/folder")));

    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
    ExtractRequest request;
    request.targetPath = workspace.filePath(QStringLiteral("target"));
    request.destinationPrefix = QStringLiteral("folder/");
    FakeObserver observer;
    const auto result = backend.extractFiles(request, observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    EXPECT_TRUE(QFileInfo::exists(workspace.filePath(QStringLiteral("target/item.txt"))));
    EXPECT_FALSE(QFileInfo::exists(workspace.filePath(QStringLiteral("target/folder/item.txt"))));
}

TEST(SevenZipExtractionTest, LargeEntryListIsChunked)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString sourceRoot = workspace.filePath(QStringLiteral("source"));
    ASSERT_TRUE(QDir().mkpath(sourceRoot));
    for (int index = 0; index < 300; ++index) {
        QFile file(sourceRoot + QStringLiteral("/file-%1.txt").arg(index));
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(file.write("x"), 1);
    }
    const QString archive = workspace.filePath(QStringLiteral("many.7z"));
    QProcess process;
    process.setWorkingDirectory(sourceRoot);
    process.start(QStringLiteral("7z"),
                  {QStringLiteral("a"), archive, QStringLiteral(".")});
    ASSERT_TRUE(process.waitForFinished(60000));
    ASSERT_EQ(process.exitCode(), 0);

    SevenZipBackend backend(archive, QStringLiteral("application/x-7z-compressed"));
    ExtractRequest request;
    request.targetPath = workspace.filePath(QStringLiteral("target"));
    FakeObserver observer;
    const auto result = backend.extractFiles(request, observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    EXPECT_EQ(QDir(request.targetPath).entryList(QDir::Files).size(), 300);
}

TEST_P(Cli7zContractTest, AddCreatesArchive)
{
    const QString archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("created.7z"));
    std::unique_ptr<dcarchive::ArchiveBackend> created =
            createBackend(GetParam(), archivePath,
                          QStringLiteral("application/x-7z-compressed"), metaDataPath());
    dcarchive::OpenRequest openRequest;
    openRequest.path = archivePath;
    openRequest.mimeType = QStringLiteral("application/x-7z-compressed");
    ASSERT_TRUE(created->open(openRequest).result.ok());

    dcarchive::ArchiveEntry entry;
    entry.path = m_sourcePath;
    entry.name = QStringLiteral("test.txt");
    entry.size = QFileInfo(m_sourcePath).size();
    entry.type = dcarchive::EntryType::File;

    dcarchive::AddRequest request;
    request.archivePath = archivePath;
    request.format = QStringLiteral("7z");
    request.compressionLevel = 6;
    request.cpuThreads = 1;
    request.entries.append(entry);

    const dcarchive::VoidResult result = created->addFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    EXPECT_GT(QFileInfo(archivePath).size(), 0);

    const dcarchive::EntryResult listed = created->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_GE(listed.info.entries.size(), 1);
}

TEST(SevenZipOptionsTest, WrongDataPasswordFinishesInsteadOfAskingAgain)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir work;
    ASSERT_TRUE(work.isValid());
    const QString mime = QStringLiteral("application/x-7z-compressed");
    const QString archive = work.filePath(QStringLiteral("encrypted.7z"));
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    AddRequest add;
    ArchiveEntry entry;
    entry.path = source;
    add.entries.append(entry);
    add.password = QStringLiteral("correct-password");
    SevenZipBackend writer(archive, mime);
    FakeObserver createObserver;
    ASSERT_TRUE(writer.addFiles(add, createObserver).result.ok());

    class PasswordObserver : public FakeObserver
    {
    public:
        void onPasswordRequired(PasswordRequest &request) override
        {
            passwordRequests.append(request);
            if (passwordRequests.size() == 1) {
                request.response = PasswordRequest::Response::Provide;
                request.password = QStringLiteral("wrong-password");
            }
        }
    } observer;
    SevenZipBackend reader(archive, mime);
    ASSERT_TRUE(reader.list().result.ok());
    ExtractRequest extract;
    extract.targetPath = work.filePath(QStringLiteral("failed"));
    const auto failed = reader.extractFiles(extract, observer).result;
    EXPECT_EQ(failed.error, ArchiveError::WrongPassword);
    EXPECT_FALSE(failed.cancelled);
    EXPECT_EQ(observer.passwordRequests.size(), 1);
    EXPECT_FALSE(QFileInfo::exists(QDir(extract.targetPath).filePath(QStringLiteral("test.txt"))));

    OpenRequest open;
    open.password = add.password;
    ASSERT_TRUE(reader.open(open).result.ok());
    extract.targetPath = work.filePath(QStringLiteral("retry"));
    ASSERT_TRUE(reader.extractFiles(extract, observer).result.ok());
    QFile original(source);
    QFile restored(QDir(extract.targetPath).filePath(QStringLiteral("test.txt")));
    ASSERT_TRUE(original.open(QIODevice::ReadOnly));
    ASSERT_TRUE(restored.open(QIODevice::ReadOnly));
    EXPECT_EQ(restored.readAll(), original.readAll());
}

TEST(SevenZipOptionsTest, EncryptedHeaderAndSplitVolumeRoundTrip)
{
    if (!have7z())
        GTEST_SKIP() << "7z is not available";
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString path = workspace.filePath(QStringLiteral("split.7z"));
    const QString mime = QStringLiteral("application/x-7z-compressed");
    const QString fileName = QStringLiteral("payload.txt");
    QFile input(workspace.filePath(fileName));
    const QByteArray payload(32 * 1024, 'x');
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write(payload), payload.size());
    input.close();

    ArchiveEntry entry;
    entry.path = input.fileName();
    AddRequest add;
    add.entries.append(entry);
    add.compressionLevel = 0;
    add.cpuThreads = 1;
    add.password = QStringLiteral("7z-test-password");
    add.encryptList = true;
    add.splitSize = 4096;
    SevenZipBackend writer(path, mime);
    FakeObserver observer;
    ASSERT_TRUE(writer.addFiles(add, observer).result.ok());
    const QString firstVolume = path + QStringLiteral(".001");
    ASSERT_EQ(QFileInfo(firstVolume).size(), add.splitSize);
    ASSERT_TRUE(QFileInfo::exists(path + QStringLiteral(".002")));

    SevenZipBackend reader(firstVolume, mime);
    EXPECT_EQ(reader.list().result.error, ArchiveError::PasswordRequired);
    OpenRequest open;
    open.password = QStringLiteral("wrong-password");
    ASSERT_TRUE(reader.open(open).result.ok());
    EXPECT_EQ(reader.list().result.error, ArchiveError::WrongPassword);

    ExtractRequest extract;
    extract.targetPath = workspace.filePath(QStringLiteral("out"));
    ASSERT_TRUE(QDir().mkpath(extract.targetPath));
    EXPECT_EQ(reader.extractFiles(extract, observer).result.error, ArchiveError::Cancelled);
    observer.reset();
    observer.passwordResponse = PasswordRequest::Response::Provide;
    observer.passwordValue = add.password;
    const auto extracted = reader.extractFiles(extract, observer);
    ASSERT_TRUE(extracted.result.ok()) << extracted.result.message.toStdString();
    EXPECT_EQ(observer.passwordRequests.size(), 1);

    const auto listed = reader.list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_EQ(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().path, fileName);
    QFile output(QDir(extract.targetPath).filePath(fileName));
    ASSERT_TRUE(output.open(QIODevice::ReadOnly));
    EXPECT_EQ(output.readAll(), payload);
}

INSTANTIATE_TEST_SUITE_P(Cli7zBackends,
                         Cli7zContractTest,
                         ::testing::Values(BackendKind::Legacy, BackendKind::New),
                         [](const ::testing::TestParamInfo<BackendKind> &info) {
                             return info.param == BackendKind::Legacy ? "legacy" : "new";
                         });

} // namespace
