// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "legacy_libzip_adapter.h"
#include "libarchive_backend.h"
#include "libzip_backend.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>

#include <chrono>
#include <future>
#include <memory>
#include <zip.h>

namespace {

QString testSourceDir()
{
    return QStringLiteral(CONTRACT_TEST_SOURCE_DIR);
}

QString zipSource()
{
    return testSourceDir() + QStringLiteral("/test_sources/zip/extract/test.zip");
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
        return std::unique_ptr<dcarchive::ArchiveBackend>(new dcarchive::LegacyLibzipAdapter(path, mime));
    return std::unique_ptr<dcarchive::ArchiveBackend>(new dcarchive::LibzipBackend(path, mime));
}

class LibzipContractTest : public ::testing::TestWithParam<BackendKind>
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_workspace.isValid());
        m_archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("test.zip"));
        ASSERT_TRUE(QFile::copy(zipSource(), m_archivePath));
        m_backend = createBackend(GetParam(), m_archivePath, QStringLiteral("application/zip"));
        dcarchive::OpenRequest request;
        request.path = m_archivePath;
        request.mimeType = QStringLiteral("application/zip");
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

TEST_P(LibzipContractTest, ListReturnsArchiveEntries)
{
    const dcarchive::EntryResult listed = backend()->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_EQ(listed.info.entries.size(), 2);

    QStringList names;
    for (const dcarchive::ArchiveEntry &entry : listed.info.entries)
        names.append(entry.name);
    EXPECT_TRUE(names.contains(QStringLiteral("test.txt")));
    EXPECT_TRUE(names.contains(QStringLiteral("test1.txt")));
}

TEST_P(LibzipContractTest, ExtractWritesAllEntries)
{
    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = backend()->extractFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const QString testTxt = QDir(targetPath()).filePath(QStringLiteral("test.txt"));
    const QString test1Txt = QDir(targetPath()).filePath(QStringLiteral("test1.txt"));
    EXPECT_TRUE(QFileInfo::exists(testTxt));
    EXPECT_TRUE(QFileInfo::exists(test1Txt));
    EXPECT_EQ(QFileInfo(testTxt).size(), 4);
    EXPECT_EQ(QFileInfo(test1Txt).size(), 4);
}

TEST_P(LibzipContractTest, DeleteRemovesEntry)
{
    const dcarchive::EntryResult listed = backend()->list();
    ASSERT_TRUE(listed.result.ok());

    dcarchive::ArchiveEntry target;
    for (const dcarchive::ArchiveEntry &entry : listed.info.entries) {
        if (entry.name == QStringLiteral("test1.txt"))
            target = entry;
    }
    ASSERT_EQ(target.name, QStringLiteral("test1.txt"));

    dcarchive::DeleteRequest request;
    request.entries.append(target);
    const dcarchive::VoidResult result = backend()->deleteFiles(request);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const dcarchive::EntryResult after = backend()->list();
    ASSERT_TRUE(after.result.ok());
    QStringList names;
    for (const dcarchive::ArchiveEntry &entry : after.info.entries)
        names.append(entry.name);
    EXPECT_FALSE(names.contains(QStringLiteral("test1.txt")));
    EXPECT_TRUE(names.contains(QStringLiteral("test.txt")));
}

TEST_P(LibzipContractTest, RenameChangesEntryName)
{
    const dcarchive::EntryResult listed = backend()->list();
    ASSERT_TRUE(listed.result.ok());

    dcarchive::ArchiveEntry target;
    for (const dcarchive::ArchiveEntry &entry : listed.info.entries) {
        if (entry.name == QStringLiteral("test.txt"))
            target = entry;
    }
    ASSERT_EQ(target.name, QStringLiteral("test.txt"));

    dcarchive::RenameRequest request;
    request.entry = target;
    request.newName = QStringLiteral("renamed.txt");
    const dcarchive::VoidResult result = backend()->renameFiles(request);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const dcarchive::EntryResult after = backend()->list();
    ASSERT_TRUE(after.result.ok());
    QStringList names;
    for (const dcarchive::ArchiveEntry &entry : after.info.entries)
        names.append(entry.name);
    EXPECT_TRUE(names.contains(QStringLiteral("renamed.txt")));
    EXPECT_FALSE(names.contains(QStringLiteral("test.txt")));
}

TEST_P(LibzipContractTest, AddCommentRoundTrips)
{
    const dcarchive::VoidResult result = backend()->addComment(QStringLiteral("hello comment"));
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const dcarchive::EntryResult after = backend()->list();
    ASSERT_TRUE(after.result.ok());
    EXPECT_EQ(after.info.comment, QStringLiteral("hello comment"));
}

TEST(LibzipCommentTest, PublishesProgressWhileSavingComment)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString path = workspace.filePath(QStringLiteral("comment.zip"));
    ASSERT_TRUE(QFile::copy(zipSource(), path));
    dcarchive::LibzipBackend zip(path, QStringLiteral("application/zip"));
    dcarchive::ArchiveBackend &backend = zip;
    dcarchive::FakeObserver observer;
    ASSERT_TRUE(backend.open({}).result.ok());
    ASSERT_TRUE(backend.addComment(QStringLiteral("updated comment"), observer).result.ok());
    ASSERT_GE(observer.progressEvents.size(), 2);
    EXPECT_EQ(observer.progressEvents.first().percentage, 0.0);
    EXPECT_GT(observer.progressEvents.last().percentage, 0.0);
    EXPECT_EQ(backend.list().info.comment, QStringLiteral("updated comment"));
}

TEST_P(LibzipContractTest, AddCreatesArchive)
{
    const QString source = testSourceDir() + QStringLiteral("/test_sources/zip/compress/test.txt");
    const QString archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("created.zip"));
    std::unique_ptr<dcarchive::ArchiveBackend> created =
            createBackend(GetParam(), archivePath, QStringLiteral("application/zip"));

    dcarchive::OpenRequest openRequest;
    openRequest.path = archivePath;
    openRequest.mimeType = QStringLiteral("application/zip");
    ASSERT_TRUE(created->open(openRequest).result.ok());

    dcarchive::ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("test.txt");
    entry.size = QFileInfo(source).size();
    entry.type = dcarchive::EntryType::File;

    dcarchive::AddRequest request;
    request.archivePath = archivePath;
    request.format = QStringLiteral("zip");
    request.compressionLevel = 6;
    request.entries.append(entry);

    const dcarchive::VoidResult result = created->addFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const dcarchive::EntryResult listed = created->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_EQ(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().name, QStringLiteral("test.txt"));
}

TEST(LibzipDestinationTest, DirectoryAppendAndNestedExtractPreserveDestination)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString sourceDir = workspace.filePath(QStringLiteral("folder"));
    ASSERT_TRUE(QDir().mkpath(sourceDir));
    QFile child(sourceDir + QStringLiteral("/child.txt"));
    ASSERT_TRUE(child.open(QIODevice::WriteOnly));
    ASSERT_EQ(child.write("child"), 5);
    child.close();

    const QString archivePath = workspace.filePath(QStringLiteral("nested.zip"));
    LibzipBackend writer(archivePath, QStringLiteral("application/zip"));
    ArchiveEntry entry;
    entry.path = sourceDir;
    entry.name = QStringLiteral("folder");
    entry.type = EntryType::Directory;
    AddRequest add;
    add.destinationPath = QStringLiteral("dest");
    add.entries.append(entry);
    FakeObserver observer;
    ASSERT_TRUE(writer.addFiles(add, observer).result.ok());

    LibzipBackend reader(archivePath, QStringLiteral("application/zip"));
    const auto listed = reader.list();
    ASSERT_TRUE(listed.result.ok());
    QStringList paths;
    for (const ArchiveEntry &item : listed.info.entries)
        paths.append(item.path);
    EXPECT_TRUE(paths.contains(QStringLiteral("dest/folder/")));
    EXPECT_TRUE(paths.contains(QStringLiteral("dest/folder/child.txt")));

    ExtractRequest extract;
    extract.targetPath = workspace.filePath(QStringLiteral("target"));
    extract.destinationPrefix = QStringLiteral("dest/");
    extract.allEntries = false;
    for (const ArchiveEntry &item : listed.info.entries) {
        if (item.type == EntryType::Directory && item.path == QStringLiteral("dest/folder/"))
            extract.entries.append(item);
    }
    ASSERT_EQ(extract.entries.size(), 1);
    ASSERT_TRUE(reader.extractFiles(extract, observer).result.ok());
    EXPECT_TRUE(QFileInfo::exists(workspace.filePath(QStringLiteral("target/folder/child.txt"))));
}

TEST(EncryptedZipTest, PasswordRetriesAndCancelPreservesExistingFile)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString path = workspace.filePath(QStringLiteral("encrypted.zip"));
    const QString mime = QStringLiteral("application/zip");
    ArchiveEntry entry;
    entry.path = testSourceDir() + QStringLiteral("/test_sources/zip/compress/test.txt");
    entry.name = QStringLiteral("test.txt");
    AddRequest add;
    add.entries.append(entry);
    add.password = QStringLiteral("zip-test-password");
    FakeObserver observer;
    LibzipBackend writer(path, mime);
    ASSERT_TRUE(writer.addFiles(add, observer).result.ok());

    ExtractRequest extract;
    extract.targetPath = workspace.path();
    QFile output(workspace.filePath(entry.name));
    const QByteArray original("existing contents");
    ASSERT_TRUE(output.open(QIODevice::WriteOnly));
    ASSERT_EQ(output.write(original), original.size());
    output.close();

    LibzipBackend reader(path, mime);
    OpenRequest open;
    open.password = QStringLiteral("wrong-password");
    ASSERT_TRUE(reader.open(open).result.ok());
    observer.conflictResponse = FileConflictRequest::Response::Overwrite;
    EXPECT_EQ(reader.extractFiles(extract, observer).result.error, ArchiveError::Cancelled);
    ASSERT_TRUE(output.open(QIODevice::ReadOnly));
    EXPECT_EQ(output.readAll(), original);
    output.close();

    observer.reset();
    observer.passwordResponse = PasswordRequest::Response::Provide;
    observer.passwordValue = add.password;
    ASSERT_TRUE(reader.extractFiles(extract, observer).result.ok());
    EXPECT_EQ(observer.passwordRequests.size(), 1);
    QFile source(entry.path);
    ASSERT_TRUE(source.open(QIODevice::ReadOnly));
    ASSERT_TRUE(output.open(QIODevice::ReadOnly));
    EXPECT_EQ(output.readAll(), source.readAll());
}

TEST(EncryptedZipTest, AesZipAuthenticatesBeforeReplacingDestination)
{
    using namespace dcarchive;
    for (const auto &sample : {QStringLiteral("-mx=0"), QStringLiteral("-mx=5"), QStringLiteral("pzip-utf8")}) {
        SCOPED_TRACE(sample.toStdString());
        QTemporaryDir workspace;
        ASSERT_TRUE(workspace.isValid());
        const bool utf8 = sample == QStringLiteral("pzip-utf8");
        const QString password = utf8 ? QStringLiteral("\u6211\u4eec") : QStringLiteral("test-password");
        const QByteArray payload = utf8 ? QByteArray("archive regression sample\n")
                                        : QByteArray("AES ZIP regression sample\n").repeated(512);
        const QString name = QStringLiteral("payload.txt");
        const QString path = workspace.filePath(QStringLiteral("aes.zip"));
        if (utf8) {
            // One-entry AE-2 fixture produced by pzip with a UTF-8 password.
            const QByteArray fixture = QByteArray::fromBase64(
                "UEsDBBQAAQBjAHtUd10AAAAAOwAAABoAAAALABQAcGF5bG9hZC50eHQBmQcAAgBBRQMIAFVUBQAB"
                "iqYDa2Lff3zUZaqkR4Q1zsZvBgLUP4tBuyNzdHsj8hp5jdpl3UCY3/gU4WwpKqcfK9COKD8J3Oif"
                "t+j5YU/4UEsBAhQDFAABAGMAe1R3XQAAAAA7AAAAGgAAAAsAFAAAAAAAAAAAALSBAAAAAHBheWxv"
                "YWQudHh0VVQFAAGKpgNrAZkHAAIAQUUDCABQSwUGAAAAAAEAAQBNAAAAeAAAAAAA");
            QFile file(path);
            ASSERT_TRUE(file.open(QIODevice::WriteOnly));
            ASSERT_EQ(file.write(fixture), fixture.size());
        } else {
            QFile source(workspace.filePath(name));
            ASSERT_TRUE(source.open(QIODevice::WriteOnly));
            ASSERT_EQ(source.write(payload), payload.size());
            source.close();

            QProcess tool;
            tool.setWorkingDirectory(workspace.path());
            tool.start(QStringLiteral("7z"), {QStringLiteral("a"), QStringLiteral("-tzip"),
                       QStringLiteral("-mem=AES256"), QStringLiteral("-p") + password, sample, path, name});
            ASSERT_TRUE(tool.waitForFinished());
            ASSERT_EQ(tool.exitCode(), 0) << tool.readAllStandardError().constData();
        }

        QTemporaryDir target;
        ASSERT_TRUE(target.isValid());
        QFile existing(target.filePath(name));
        const QByteArray original("keep existing destination");
        const auto replaceWith = [&](const QByteArray &contents) {
            ASSERT_TRUE(existing.open(QIODevice::WriteOnly | QIODevice::Truncate));
            ASSERT_EQ(existing.write(contents), contents.size());
            existing.close();
        };
        const auto verify = [&](const QByteArray &contents) {
            ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
            EXPECT_EQ(existing.readAll(), contents);
            existing.close();
            EXPECT_EQ(QDir(target.path()).entryList(QDir::Files | QDir::Hidden), QStringList{name});
        };
        replaceWith(original);
        ExtractRequest request;
        request.targetPath = target.path();
        request.overwrite = OverwritePolicy::OverwriteAll;
        const QString mime = QStringLiteral("application/zip");
        LibzipBackend reader(path, mime);
        const auto listed = reader.list();
        ASSERT_TRUE(listed.result.ok());
        ASSERT_EQ(listed.info.entries.size(), 1);
        request.allEntries = false;
        request.entries.append(listed.info.entries.first());
        OpenRequest options;
        options.password = QStringLiteral("incorrect");
        ASSERT_TRUE(reader.open(options).result.ok());
        FakeObserver observer;
        EXPECT_EQ(reader.extractFiles(request, observer).result.error, ArchiveError::Cancelled);
        verify(original);
        observer.reset();
        observer.passwordResponse = PasswordRequest::Response::Provide;
        observer.passwordValue = password;
        const auto valid = reader.extractFiles(request, observer).result;
        EXPECT_TRUE(valid.ok()) << valid.message.toStdString();
        EXPECT_EQ(observer.passwordRequests.size(), 1);
        verify(payload);

        // The authentication tag is the last ten bytes of the local entry's data.
        // The next record in this one-entry 7z fixture is its central directory.
        QFile damaged(path);
        ASSERT_TRUE(damaged.open(QIODevice::ReadWrite));
        const QByteArray bytes = damaged.readAll();
        const auto central = bytes.lastIndexOf(QByteArray("PK\x01\x02", 4));
        ASSERT_GT(central, 10);
        ASSERT_TRUE(damaged.seek(central - 1));
        const char tampered = bytes.at(central - 1) ^ 1;
        ASSERT_EQ(damaged.write(&tampered, 1), 1);
        damaged.close();
        replaceWith(original);
        options.password = password;
        ASSERT_TRUE(reader.open(options).result.ok());
        observer.reset();
        EXPECT_EQ(reader.extractFiles(request, observer).result.error, ArchiveError::CorruptArchive);
        EXPECT_TRUE(observer.passwordRequests.isEmpty());
        verify(original);
    }
}

TEST(EncryptedZipTest, MixedEntriesExtractWithExactNames)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    QTemporaryDir target;
    ASSERT_TRUE(workspace.isValid());
    ASSERT_TRUE(target.isValid());
    const QString path = workspace.filePath(QStringLiteral("mixed.zip"));
    const QString password = QStringLiteral("test-password");
    const QStringList names{QStringLiteral("clear.txt"), QStringLiteral("first.txt"),
                            QStringLiteral("\u6587\u4ef6.txt")};
    for (int i = 0; i < names.size(); ++i) {
        QFile file(workspace.filePath(names.at(i)));
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        const QByteArray contents(100 + i, 'a' + i);
        ASSERT_EQ(file.write(contents), contents.size());
    }
    const auto add = [&](QStringList args) {
        QProcess tool;
        tool.setWorkingDirectory(workspace.path());
        tool.start(QStringLiteral("7z"), args);
        return tool.waitForFinished() && tool.exitCode() == 0;
    };
    ASSERT_TRUE(add({QStringLiteral("a"), QStringLiteral("-tzip"), path, names.at(0)}));
    ASSERT_TRUE(add({QStringLiteral("a"), QStringLiteral("-tzip"), QStringLiteral("-mcu=on"),
                     QStringLiteral("-mem=AES256"), QStringLiteral("-p") + password,
                     path, names.at(1), names.at(2)}));
    LibzipBackend reader(path, QStringLiteral("application/zip"));
    ExtractRequest request;
    request.targetPath = target.path();
    FakeObserver observer;
    observer.passwordResponse = PasswordRequest::Response::Provide;
    observer.passwordValue = password;
    const auto result = reader.extractFiles(request, observer).result;
    ASSERT_TRUE(result.ok()) << result.message.toStdString();
    EXPECT_EQ(observer.passwordRequests.size(), 1);
    for (int i = 0; i < names.size(); ++i) {
        QFile file(target.filePath(names.at(i)));
        ASSERT_TRUE(file.open(QIODevice::ReadOnly));
        EXPECT_EQ(file.readAll(), QByteArray(100 + i, 'a' + i));
    }
}

TEST(LibzipExtractionTest, SkipAllConflictsStillExtractsMissingFiles)
{
    using namespace dcarchive;
    for (const auto policy : {OverwritePolicy::Ask, OverwritePolicy::SkipAll, OverwritePolicy::OverwriteAll}) {
        SCOPED_TRACE(static_cast<int>(policy));
        QTemporaryDir target;
        ASSERT_TRUE(target.isValid());
        QFile existing(target.filePath(QStringLiteral("test.txt")));
        const QByteArray original("existing data");
        ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
        ASSERT_EQ(existing.write(original), original.size());
        existing.close();

        LibzipBackend reader(zipSource(), QStringLiteral("application/zip"));
        ExtractRequest extract;
        extract.targetPath = target.path();
        extract.overwrite = policy;
        FakeObserver observer;
        observer.conflictResponse = FileConflictRequest::Response::SkipAll;
        ASSERT_TRUE(reader.extractFiles(extract, observer).result.ok());
        EXPECT_EQ(observer.conflictRequests.size(), policy == OverwritePolicy::Ask ? 1 : 0);
        EXPECT_EQ(QFileInfo(target.filePath(QStringLiteral("test1.txt"))).size(), 4);
        ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
        const QByteArray actual = existing.readAll();
        if (policy == OverwritePolicy::OverwriteAll)
            EXPECT_EQ(actual.size(), 4);
        else
            EXPECT_EQ(actual, original);
    }
}

class ExtractionOutputTest : public ::testing::TestWithParam<bool>
{
protected:
    void SetUp() override
    {
        using namespace dcarchive;
        ASSERT_TRUE(workspace.isValid());
        ASSERT_TRUE(target.isValid());
        QFile source(workspace.filePath(QStringLiteral("payload.txt")));
        ASSERT_TRUE(source.open(QIODevice::WriteOnly));
        payload = QByteArray(128 * 1024, 'x');
        ASSERT_EQ(source.write(payload), payload.size());
        source.close();
        const QString path = workspace.filePath(GetParam() ? QStringLiteral("sample.zip") : QStringLiteral("sample.tar.gz"));
        reader.reset(GetParam() ? static_cast<ArchiveBackend *>(new LibzipBackend(path, QStringLiteral("application/zip")))
                                : static_cast<ArchiveBackend *>(new LibarchiveBackend(path, QStringLiteral("application/x-compressed-tar"))));
        AddRequest add;
        add.archivePath = path;
        add.format = GetParam() ? QStringLiteral("zip") : QStringLiteral("tar.gz");
        ArchiveEntry entry;
        entry.path = source.fileName();
        entry.name = QStringLiteral("payload.txt");
        add.entries.append(entry);
        FakeObserver observer;
        ASSERT_TRUE(reader->addFiles(add, observer).result.ok());
        ASSERT_TRUE(reader->open({}).result.ok());
        request.targetPath = target.path();
        request.overwrite = OverwritePolicy::OverwriteAll;
        destination = target.filePath(entry.name);
        QFile original(destination);
        ASSERT_TRUE(original.open(QIODevice::WriteOnly));
        ASSERT_EQ(original.write("original"), 8);
        original.close();
        ASSERT_TRUE(original.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    }

    QByteArray readDestination() const
    {
        QFile file(destination);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }

    QStringList outputEntries() const
    {
        return QDir(target.path()).entryList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    }

    QTemporaryDir workspace;
    QTemporaryDir target;
    QString destination;
    QByteArray payload;
    dcarchive::ExtractRequest request;
    std::unique_ptr<dcarchive::ArchiveBackend> reader;
};

TEST_P(ExtractionOutputTest, ReplacesContentsAndPreservesMode)
{
    using namespace dcarchive;
    const auto mode = QFile::permissions(destination);
    FakeObserver observer;
    ASSERT_TRUE(reader->extractFiles(request, observer).result.ok());
    EXPECT_EQ(readDestination(), payload);
    EXPECT_EQ(QFile::permissions(destination), mode);
    EXPECT_EQ(outputEntries(), QStringList{QStringLiteral("payload.txt")});
}

TEST_P(ExtractionOutputTest, PauseBeforeExtractionWaitsForResume)
{
    dcarchive::FakeObserver observer;
    reader->pause();
    auto extraction = std::async(std::launch::async, [&] {
        return reader->extractFiles(request, observer);
    });
    EXPECT_EQ(extraction.wait_for(std::chrono::milliseconds(250)), std::future_status::timeout);
    EXPECT_EQ(readDestination(), QByteArray("original"));
    reader->resume();
    ASSERT_TRUE(extraction.get().result.ok());
    EXPECT_EQ(readDestination(), payload);
}

namespace {
class CancelAfterWriteObserver final : public dcarchive::BackendObserver
{
public:
    explicit CancelAfterWriteObserver(dcarchive::ArchiveBackend &backend) : m_backend(backend) {}
    void onProgress(const dcarchive::ProgressInfo &info) override
    {
        if (info.processedBytes > 0)
            m_backend.cancel();
    }

private:
    dcarchive::ArchiveBackend &m_backend;
};
} // namespace

TEST_P(ExtractionOutputTest, CancelAfterWritingPreservesExistingContents)
{
    using namespace dcarchive;
    request.keepIncompleteOnCancel = true;
    CancelAfterWriteObserver observer(*reader);
    EXPECT_EQ(reader->extractFiles(request, observer).result.error, ArchiveError::Cancelled);
    EXPECT_EQ(readDestination(), QByteArray("original"));
    EXPECT_EQ(outputEntries(), QStringList{QStringLiteral("payload.txt")});
}

TEST_P(ExtractionOutputTest, CancelOnlyKeepsNewPartialZipWhenRequested)
{
    using namespace dcarchive;
    ASSERT_TRUE(QFile::remove(destination));
    CancelAfterWriteObserver observer(*reader);
    EXPECT_EQ(reader->extractFiles(request, observer).result.error, ArchiveError::Cancelled);
    EXPECT_TRUE(outputEntries().isEmpty());

    ASSERT_TRUE(reader->open({}).result.ok());
    request.keepIncompleteOnCancel = true;
    const auto cancelled = reader->extractFiles(request, observer).result;
    ASSERT_EQ(cancelled.error, ArchiveError::Cancelled);
    if (GetParam()) {
        EXPECT_EQ(outputEntries(), QStringList{QStringLiteral("payload.txt")});
        const auto partial = readDestination();
        ASSERT_FALSE(partial.isEmpty());
        ASSERT_LT(partial.size(), payload.size());
        EXPECT_EQ(partial, payload.left(partial.size()));
    } else {
        EXPECT_TRUE(outputEntries().isEmpty());
    }
}

TEST_P(ExtractionOutputTest, ReplacingDirectoryDoesNotAlterIt)
{
    using namespace dcarchive;
    ASSERT_TRUE(QFile::remove(destination));
    ASSERT_TRUE(QDir().mkdir(destination));
    QFile sentinel(QDir(destination).filePath(QStringLiteral("keep.txt")));
    ASSERT_TRUE(sentinel.open(QIODevice::WriteOnly));
    ASSERT_EQ(sentinel.write("keep"), 4);
    sentinel.close();
    FakeObserver observer;
    EXPECT_EQ(reader->extractFiles(request, observer).result.error, ArchiveError::PermissionDenied);
    ASSERT_TRUE(sentinel.open(QIODevice::ReadOnly));
    EXPECT_EQ(sentinel.readAll(), QByteArray("keep"));
    EXPECT_EQ(outputEntries(), QStringList{QStringLiteral("payload.txt")});
}

TEST_P(ExtractionOutputTest, DoesNotWriteThroughDestinationSymlink)
{
    using namespace dcarchive;
    const QString external = workspace.filePath(QStringLiteral("protected.txt"));
    ASSERT_TRUE(QFile::rename(destination, external));
    ASSERT_TRUE(QFile::link(external, destination));
    FakeObserver observer;
    EXPECT_EQ(reader->extractFiles(request, observer).result.error, ArchiveError::PermissionDenied);
    EXPECT_TRUE(QFileInfo(destination).isSymLink());
    EXPECT_EQ(readDestination(), QByteArray("original"));
    EXPECT_EQ(outputEntries(), QStringList{QStringLiteral("payload.txt")});
}

TEST_P(ExtractionOutputTest, ReadFailureDoesNotReplaceExistingContents)
{
    using namespace dcarchive;
    const QString archivePath = workspace.filePath(QStringLiteral("damaged.zip"));
    const QByteArray encodedPath = QFile::encodeName(archivePath);
    zip_t *archive = zip_open(encodedPath.constData(), ZIP_CREATE | ZIP_TRUNCATE, nullptr);
    ASSERT_NE(archive, nullptr);
    zip_source_t *source = zip_source_buffer(archive, payload.constData(), payload.size(), 0);
    ASSERT_NE(source, nullptr);
    const auto index = zip_file_add(archive, "payload.txt", source, 0);
    ASSERT_GE(index, 0);
    ASSERT_EQ(zip_set_file_compression(archive, index, ZIP_CM_STORE, 0), 0);
    ASSERT_EQ(zip_close(archive), 0);

    // Invalidate the stored entry data without modifying the directory or CRC.
    QFile damaged(archivePath);
    ASSERT_TRUE(damaged.open(QIODevice::ReadWrite));
    const auto offset = damaged.readAll().indexOf(payload);
    ASSERT_GE(offset, 0);
    ASSERT_TRUE(damaged.seek(offset + payload.size() - 1));
    ASSERT_EQ(damaged.write("y", 1), 1);
    damaged.close();

    const QString mime = QStringLiteral("application/zip");
    if (GetParam())
        reader.reset(new LibzipBackend(archivePath, mime));
    else
        reader.reset(new LibarchiveBackend(archivePath, mime));
    FakeObserver observer;
    EXPECT_EQ(reader->extractFiles(request, observer).result.error, ArchiveError::CorruptArchive);
    EXPECT_EQ(readDestination(), QByteArray("original"));
    EXPECT_EQ(outputEntries(), QStringList{QStringLiteral("payload.txt")});
}

INSTANTIATE_TEST_SUITE_P(ZipAndTarGz, ExtractionOutputTest, ::testing::Bool());

INSTANTIATE_TEST_SUITE_P(LibzipBackends,
                         LibzipContractTest,
                         ::testing::Values(BackendKind::Legacy, BackendKind::New),
                         [](const ::testing::TestParamInfo<BackendKind> &info) {
                             return info.param == BackendKind::Legacy ? "legacy" : "new";
                         });

} // namespace
