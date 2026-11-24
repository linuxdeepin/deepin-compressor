// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "legacy_libarchive_adapter.h"
#include "libarchive_backend.h"
#include "libzip_backend.h"
#include "path_guard.h"

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

QString tarSource()
{
    return testSourceDir() + QStringLiteral("/test_sources/tar/extract/test.tar");
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
                new dcarchive::LegacyLibarchiveAdapter(path, mime));
    return std::unique_ptr<dcarchive::ArchiveBackend>(
            new dcarchive::LibarchiveBackend(path, mime));
}

class LibarchiveContractTest : public ::testing::TestWithParam<BackendKind>
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_workspace.isValid());
        m_archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("test.tar"));
        ASSERT_TRUE(QFile::copy(tarSource(), m_archivePath));
        m_backend = createBackend(GetParam(), m_archivePath, QStringLiteral("application/x-tar"));
        dcarchive::OpenRequest request;
        request.path = m_archivePath;
        request.mimeType = QStringLiteral("application/x-tar");
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

TEST_P(LibarchiveContractTest, ListReturnsEntries)
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

TEST_P(LibarchiveContractTest, ExtractWritesAllEntries)
{
    dcarchive::ExtractRequest request;
    request.targetPath = targetPath();
    request.allEntries = true;

    const dcarchive::VoidResult result = backend()->extractFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();
    EXPECT_TRUE(QFileInfo::exists(QDir(targetPath()).filePath(QStringLiteral("test.txt"))));
    EXPECT_TRUE(QFileInfo::exists(QDir(targetPath()).filePath(QStringLiteral("test1.txt"))));
}

TEST_P(LibarchiveContractTest, AddCreatesArchive)
{
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    const QString archivePath = QDir(m_workspace.path()).filePath(QStringLiteral("created.tar"));
    std::unique_ptr<dcarchive::ArchiveBackend> created =
            createBackend(GetParam(), archivePath, QStringLiteral("application/x-tar"));

    dcarchive::OpenRequest openRequest;
    openRequest.path = archivePath;
    openRequest.mimeType = QStringLiteral("application/x-tar");
    ASSERT_TRUE(created->open(openRequest).result.ok());

    dcarchive::ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("test.txt");
    entry.size = QFileInfo(source).size();
    entry.type = dcarchive::EntryType::File;

    dcarchive::AddRequest request;
    request.archivePath = archivePath;
    request.format = QStringLiteral("tar");
    request.entries.append(entry);

    const dcarchive::VoidResult result = created->addFiles(request, m_observer);
    ASSERT_TRUE(result.result.ok()) << result.result.message.toStdString();

    const dcarchive::EntryResult listed = created->list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_EQ(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().name, QStringLiteral("test.txt"));
}

TEST_P(LibarchiveContractTest, AppendPreservesExistingEntries)
{
    const QString added = QDir(m_workspace.path()).filePath(QStringLiteral("added.txt"));
    QFile file(added);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    ASSERT_EQ(file.write("added"), 5);
    file.close();

    dcarchive::ArchiveEntry entry;
    entry.path = added;
    entry.name = QStringLiteral("added.txt");
    entry.size = 5;
    entry.type = dcarchive::EntryType::File;

    dcarchive::AddRequest request;
    request.archivePath = m_archivePath;
    request.entries.append(entry);
    const dcarchive::VoidResult appended = backend()->addFiles(request, m_observer);
    ASSERT_TRUE(appended.result.ok()) << appended.result.message.toStdString();

    const dcarchive::EntryResult after = backend()->list();
    ASSERT_TRUE(after.result.ok()) << after.result.message.toStdString();
    QStringList names;
    for (const dcarchive::ArchiveEntry &item : after.info.entries)
        names.append(item.name);
    EXPECT_TRUE(names.contains(QStringLiteral("test.txt")));
    EXPECT_TRUE(names.contains(QStringLiteral("test1.txt")));
    EXPECT_TRUE(names.contains(QStringLiteral("added.txt")));
}

TEST_P(LibarchiveContractTest, DeleteRemovesEntry)
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
}

TEST_P(LibarchiveContractTest, RenameChangesEntryName)
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
}

TEST(TarWriterTest, ExternalLzopFilterSupportsCreationAndUpdates)
{
    const QString lzop = QStandardPaths::findExecutable(QStringLiteral("lzop"));
    if (lzop.isEmpty())
        GTEST_SKIP() << "lzop is not installed";

    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QDir root(workspace.path());
    const QString path = root.filePath(QStringLiteral("data.tar.lzo"));
    const QString mime = QStringLiteral("application/x-tzo");
    dcarchive::LibarchiveBackend backend(path, mime);
    dcarchive::FakeObserver observer;

    dcarchive::OpenRequest open;
    open.path = path;
    open.mimeType = mime;
    ASSERT_TRUE(backend.open(open).result.ok());

    const QByteArray data("external lzop filter test\n");
    for (const QString &name : {QStringLiteral("first.txt"), QStringLiteral("second.txt")}) {
        QFile file(root.filePath(name));
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(file.write(data), data.size());
        file.close();

        dcarchive::ArchiveEntry entry;
        entry.path = file.fileName();
        entry.name = name;
        entry.size = data.size();
        entry.type = dcarchive::EntryType::File;
        dcarchive::AddRequest add;
        add.archivePath = path;
        add.entries.append(entry);
        const auto added = backend.addFiles(add, observer);
        ASSERT_TRUE(added.result.ok()) << added.result.message.toStdString();
        ASSERT_EQ(QProcess::execute(lzop, {QStringLiteral("-t"), path}), 0);
    }

    const auto listed = backend.list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_EQ(listed.info.entries.size(), 2);
    ASSERT_EQ(listed.info.entries[0].name, QStringLiteral("first.txt"));
    ASSERT_EQ(listed.info.entries[1].name, QStringLiteral("second.txt"));

    dcarchive::RenameRequest rename;
    rename.entry = listed.info.entries[1];
    rename.newName = QStringLiteral("renamed.txt");
    ASSERT_TRUE(backend.renameFiles(rename).result.ok());
    ASSERT_EQ(QProcess::execute(lzop, {QStringLiteral("-t"), path}), 0);

    const QString output = QStringLiteral("output");
    ASSERT_TRUE(root.mkdir(output));
    dcarchive::ExtractRequest extract;
    extract.targetPath = root.filePath(output);
    extract.allEntries = true;
    const auto extracted = backend.extractFiles(extract, observer);
    ASSERT_TRUE(extracted.result.ok()) << extracted.result.message.toStdString();
    EXPECT_FALSE(QFile::exists(QDir(extract.targetPath).filePath(QStringLiteral("second.txt"))));
    for (const QString &name : {QStringLiteral("first.txt"), rename.newName}) {
        QFile file(QDir(extract.targetPath).filePath(name));
        ASSERT_TRUE(file.open(QIODevice::ReadOnly));
        EXPECT_EQ(file.readAll(), data);
    }
}

TEST(ExtractionPathTest, RootFastPathDoesNotPermitNestedSymlinks)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QDir parent(workspace.path());
    ASSERT_TRUE(parent.mkdir(QStringLiteral("output")));
    ASSERT_TRUE(parent.mkdir(QStringLiteral("separate")));
    const QString root = parent.filePath(QStringLiteral("output"));
    const QString separate = parent.filePath(QStringLiteral("separate"));
    const QString linked = QDir(root).filePath(QStringLiteral("linked"));
    ASSERT_TRUE(QFile::link(separate, linked));
    EXPECT_TRUE(dcarchive::ensureDirectoryWithinRoot(root, root));
    EXPECT_TRUE(dcarchive::ensureParentWithinRoot(root, QDir(root).filePath(QStringLiteral("ordinary/file.txt"))));
    EXPECT_FALSE(dcarchive::ensureParentWithinRoot(root, linked + QStringLiteral("/nested/file.txt")));
    EXPECT_FALSE(QFileInfo::exists(QDir(separate).filePath(QStringLiteral("nested"))));
}

TEST(TarExtractionTest, SkipAllConflictsStillExtractsMissingFiles)
{
    using namespace dcarchive;
    QTemporaryDir target;
    ASSERT_TRUE(target.isValid());
    QFile existing(target.filePath(QStringLiteral("test.txt")));
    const QByteArray original("existing data");
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    ASSERT_EQ(existing.write(original), original.size());
    existing.close();

    LibarchiveBackend reader(tarSource(), QStringLiteral("application/x-tar"));
    ExtractRequest extract;
    extract.targetPath = target.path();
    extract.overwrite = OverwritePolicy::SkipAll;
    FakeObserver observer;
    ASSERT_TRUE(reader.extractFiles(extract, observer).result.ok());
    EXPECT_TRUE(observer.conflictRequests.isEmpty());
    EXPECT_TRUE(QFileInfo::exists(target.filePath(QStringLiteral("test1.txt"))));
    ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
    EXPECT_EQ(existing.readAll(), original);
}

TEST(TarExtractionTest, CancelPreservesDestination)
{
    using namespace dcarchive;
    QTemporaryDir target;
    ASSERT_TRUE(target.isValid());
    QFile existing(target.filePath(QStringLiteral("test.txt")));
    const QByteArray original("existing data");
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    ASSERT_EQ(existing.write(original), original.size());
    existing.close();

    LibarchiveBackend reader(tarSource(), QStringLiteral("application/x-tar"));
    class CancellingObserver : public BackendObserver
    {
    public:
        explicit CancellingObserver(ArchiveBackend &backend) : m_backend(backend) {}
        void onFileConflict(FileConflictRequest &request) override
        {
            request.response = FileConflictRequest::Response::Overwrite;
        }
        void onProgress(const ProgressInfo &progress) override
        {
            if (progress.processedBytes > 0) {
                requestedCancel = true;
                m_backend.cancel();
            }
        }

        bool requestedCancel = false;

    private:
        ArchiveBackend &m_backend;
    } observer(reader);
    ExtractRequest extract;
    extract.targetPath = target.path();
    extract.overwrite = OverwritePolicy::OverwriteAll;
    const auto listed = reader.list();
    ASSERT_TRUE(listed.result.ok());
    extract.allEntries = false;
    for (const ArchiveEntry &entry : listed.info.entries) {
        if (entry.name == QStringLiteral("test.txt"))
            extract.entries.append(entry);
    }
    ASSERT_EQ(extract.entries.size(), 1);
    EXPECT_EQ(reader.extractFiles(extract, observer).result.error, ArchiveError::Cancelled);
    EXPECT_TRUE(observer.requestedCancel);
    ASSERT_TRUE(existing.open(QIODevice::ReadOnly));
    EXPECT_EQ(existing.readAll(), original);
}

TEST(LibarchiveCreationTest, ZipExtensionProducesReadableZip)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    const QString archivePath = workspace.filePath(QStringLiteral("created.zip"));

    LibarchiveBackend writer(archivePath, QStringLiteral("application/zip"));
    ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("test.txt");
    entry.type = EntryType::File;
    AddRequest add;
    add.entries.append(entry);
    FakeObserver observer;
    const auto created = writer.addFiles(add, observer);
    ASSERT_TRUE(created.result.ok()) << created.result.message.toStdString();

    LibzipBackend reader(archivePath, QStringLiteral("application/zip"));
    const auto listed = reader.list();
    ASSERT_TRUE(listed.result.ok()) << listed.result.message.toStdString();
    ASSERT_EQ(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().name, QStringLiteral("test.txt"));
}

TEST(LibarchiveDestinationTest, AddAndExtractNestedDestination)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString source = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    const QString archivePath = workspace.filePath(QStringLiteral("nested.tar"));
    LibarchiveBackend writer(archivePath, QStringLiteral("application/x-tar"));
    ArchiveEntry entry;
    entry.path = source;
    entry.name = QStringLiteral("test.txt");
    entry.type = EntryType::File;
    AddRequest add;
    add.destinationPath = QStringLiteral("root/sub");
    add.entries.append(entry);
    FakeObserver observer;
    ASSERT_TRUE(writer.addFiles(add, observer).result.ok());

    LibarchiveBackend reader(archivePath, QStringLiteral("application/x-tar"));
    const auto listed = reader.list();
    ASSERT_TRUE(listed.result.ok());
    ASSERT_EQ(listed.info.entries.size(), 1);
    EXPECT_EQ(listed.info.entries.first().path, QStringLiteral("root/sub/test.txt"));

    ExtractRequest extract;
    extract.targetPath = workspace.filePath(QStringLiteral("target"));
    extract.destinationPrefix = QStringLiteral("root/");
    extract.allEntries = false;
    extract.entries.append(listed.info.entries.first());
    ASSERT_TRUE(reader.extractFiles(extract, observer).result.ok());
    EXPECT_TRUE(QFileInfo::exists(workspace.filePath(QStringLiteral("target/sub/test.txt"))));
}

TEST(LibarchiveRebuildTest, CorruptSourceDoesNotReplaceArchive)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archivePath = workspace.filePath(QStringLiteral("broken.tar"));
    QFile archive(archivePath);
    ASSERT_TRUE(archive.open(QIODevice::WriteOnly));
    QByteArray payload(1536, 'x');
    ASSERT_EQ(archive.write(payload), payload.size());
    archive.close();
    QFile original(archivePath);
    ASSERT_TRUE(original.open(QIODevice::ReadOnly));
    const QByteArray expected = original.readAll();
    original.close();

    LibarchiveBackend backend(archivePath, QStringLiteral("application/x-tar"));
    DeleteRequest request;
    ArchiveEntry entry;
    entry.index = 0;
    entry.path = QStringLiteral("missing.txt");
    request.entries.append(entry);
    EXPECT_FALSE(backend.deleteFiles(request).result.ok());
    ASSERT_TRUE(original.open(QIODevice::ReadOnly));
    EXPECT_EQ(original.readAll(), expected);
}

TEST(LibarchiveAppendTest, PreservesUserSidecarFiles)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archivePath = workspace.filePath(QStringLiteral("archive.tar"));
    const QString firstPath = workspace.filePath(QStringLiteral("first.txt"));
    const QString secondPath = workspace.filePath(QStringLiteral("second.txt"));
    QFile first(firstPath);
    QFile second(secondPath);
    ASSERT_TRUE(first.open(QIODevice::WriteOnly));
    ASSERT_EQ(first.write("first"), 5);
    first.close();
    ASSERT_TRUE(second.open(QIODevice::WriteOnly));
    ASSERT_EQ(second.write("second"), 6);
    second.close();

    LibarchiveBackend backend(archivePath, QStringLiteral("application/x-tar"));
    FakeObserver observer;
    ArchiveEntry firstEntry;
    firstEntry.path = firstPath;
    firstEntry.name = QStringLiteral("first.txt");
    AddRequest create;
    create.entries.append(firstEntry);
    ASSERT_TRUE(backend.addFiles(create, observer).result.ok());

    const QString backupPath = archivePath + QStringLiteral(".bak");
    const QString temporaryPath = archivePath + QStringLiteral(".tmp-add");
    QFile backup(backupPath);
    QFile temporary(temporaryPath);
    ASSERT_TRUE(backup.open(QIODevice::WriteOnly));
    ASSERT_EQ(backup.write("user-backup"), 11);
    backup.close();
    ASSERT_TRUE(temporary.open(QIODevice::WriteOnly));
    ASSERT_EQ(temporary.write("user-temporary"), 14);
    temporary.close();

    ArchiveEntry secondEntry;
    secondEntry.path = secondPath;
    secondEntry.name = QStringLiteral("second.txt");
    AddRequest append;
    append.entries.append(secondEntry);
    ASSERT_TRUE(backend.addFiles(append, observer).result.ok());
    ASSERT_TRUE(backup.open(QIODevice::ReadOnly));
    EXPECT_EQ(backup.readAll(), QByteArray("user-backup"));
    ASSERT_TRUE(temporary.open(QIODevice::ReadOnly));
    EXPECT_EQ(temporary.readAll(), QByteArray("user-temporary"));
}

TEST(LibarchiveExtractionTest, SelectingOnlyDirectoryExtractsDescendants)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString sourceDir = workspace.filePath(QStringLiteral("folder"));
    ASSERT_TRUE(QDir().mkpath(sourceDir + QStringLiteral("/nested")));
    QFile child(sourceDir + QStringLiteral("/nested/child.txt"));
    ASSERT_TRUE(child.open(QIODevice::WriteOnly));
    ASSERT_EQ(child.write("child"), 5);
    child.close();
    const QString archivePath = workspace.filePath(QStringLiteral("directory.tar"));
    LibarchiveBackend writer(archivePath, QStringLiteral("application/x-tar"));
    FakeObserver observer;
    ArchiveEntry entry;
    entry.path = sourceDir;
    entry.name = QStringLiteral("folder");
    entry.type = EntryType::Directory;
    AddRequest add;
    add.entries.append(entry);
    ASSERT_TRUE(writer.addFiles(add, observer).result.ok());

    LibarchiveBackend reader(archivePath, QStringLiteral("application/x-tar"));
    const auto listed = reader.list();
    ASSERT_TRUE(listed.result.ok());
    ArchiveEntry directory;
    for (const ArchiveEntry &item : listed.info.entries) {
        if (item.path == QStringLiteral("folder/"))
            directory = item;
    }
    ASSERT_FALSE(directory.path.isEmpty());
    ExtractRequest extract;
    extract.targetPath = workspace.filePath(QStringLiteral("target"));
    extract.allEntries = false;
    extract.entries.append(directory);
    ASSERT_TRUE(reader.extractFiles(extract, observer).result.ok());
    EXPECT_TRUE(QFileInfo::exists(workspace.filePath(QStringLiteral("target/folder/nested/child.txt"))));
}

TEST(LibarchiveReadTest, CorruptionIsReported)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archivePath = workspace.filePath(QStringLiteral("broken.tar"));
    QFile broken(archivePath);
    ASSERT_TRUE(broken.open(QIODevice::WriteOnly));
    ASSERT_EQ(broken.write("not a tar archive"), 17);
    broken.close();

    LibarchiveBackend backend(archivePath, QStringLiteral("application/x-tar"));
    EXPECT_EQ(backend.list().result.error, ArchiveError::CorruptArchive);
    EXPECT_EQ(backend.testArchive().result.error, ArchiveError::CorruptArchive);
}

TEST(PathGuardTest, RejectsEscapingSymlinkTargets)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString root = workspace.path();
    EXPECT_TRUE(dcarchive::symlinkTargetWithinRoot(
            root, root + QStringLiteral("/dir/link"), QStringLiteral("../target")));
    EXPECT_FALSE(dcarchive::symlinkTargetWithinRoot(
            root, root + QStringLiteral("/dir/link"), QStringLiteral("../../outside")));
    EXPECT_FALSE(dcarchive::symlinkTargetWithinRoot(
            root, root + QStringLiteral("/dir/link"), QStringLiteral("/etc/passwd")));
}

INSTANTIATE_TEST_SUITE_P(LibarchiveBackends,
                         LibarchiveContractTest,
                         ::testing::Values(BackendKind::Legacy, BackendKind::New),
                         [](const ::testing::TestParamInfo<BackendKind> &info) {
                             return info.param == BackendKind::Legacy ? "legacy" : "new";
                         });

} // namespace
