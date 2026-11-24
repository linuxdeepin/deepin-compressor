// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archiveviewstate.h"
#include "archive_session.h"
#include "archivequeries.h"
#include "fakes.h"
#include "sevenzip_backend.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <algorithm>
#include <chrono>
#include <future>
#include <random>

#include <memory>

namespace {

QString testSourceDir()
{
    return QStringLiteral(CONTRACT_TEST_SOURCE_DIR);
}

TEST(ArchiveSessionTest, JarAndCompressedTarRoundTrip)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString entryName = QStringLiteral("payload.txt");
    const QByteArray payload("archive format regression\n");
    QFile input(workspace.filePath(entryName));
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write(payload), payload.size());
    input.close();

    struct Format {
        QString suffix;
        QString mime;
        QByteArray magic;
    };
    const Format formats[] = {
#ifdef DC_BUILT_PZIP
        {QStringLiteral("jar"), QStringLiteral("application/java-archive"), QByteArray::fromHex("504b0304")},
#endif
        {QStringLiteral("tar.Z"), QStringLiteral("application/x-tarz"), QByteArray::fromHex("1f9d")},
    };
    for (const auto &format : formats) {
        SCOPED_TRACE(format.suffix.toStdString());
        const QString path = workspace.filePath(QStringLiteral("archive.") + format.suffix);
        std::unique_ptr<dcarchive::ArchiveSession> writer(dcarchive::ArchiveSession::create(path, format.mime, true));
        ASSERT_NE(writer, nullptr);
        FileEntry source;
        source.strFullPath = input.fileName();
        CompressOptions options;
        options.bCreateNew = true;
        ASSERT_EQ(writer->addFiles({source}, options), PFT_Nomral);

        QFile archive(path);
        ASSERT_TRUE(archive.open(QIODevice::ReadOnly));
        EXPECT_EQ(archive.read(format.magic.size()), format.magic);
        archive.close();

        std::unique_ptr<dcarchive::ArchiveSession> reader(dcarchive::ArchiveSession::create(path, format.mime, false));
        ASSERT_NE(reader, nullptr);
        ExtractionOptions extract;
        extract.strTargetPath = workspace.filePath(format.suffix);
        extract.bExistList = false;
        ASSERT_EQ(reader->extractFiles({}, extract), PFT_Nomral);
        QFile restored(QDir(extract.strTargetPath).filePath(entryName));
        ASSERT_TRUE(restored.open(QIODevice::ReadOnly));
        EXPECT_EQ(restored.readAll(), payload);
    }
}

TEST(ArchiveSessionTest, CompressionReportsProgressBeforePublishing)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    QFile input(workspace.filePath(QStringLiteral("payload.bin")));
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    std::mt19937 generator(42);
    QByteArray block(64 * 1024, '\0');
    for (int index = 0; index < 256; ++index) {
        for (char &byte : block)
            byte = static_cast<char>(generator());
        ASSERT_EQ(input.write(block), block.size());
    }
    const qint64 size = input.size();
    input.close();

    struct Format {
        const char *suffix;
        const char *mime;
        dcarchive::BackendPreference preference;
        const char *tool;
        int level;
    };
    using dcarchive::BackendPreference;
    const Format formats[] = {
#ifdef DC_BUILT_PZIP
        {"zip", "application/zip", BackendPreference::Auto, nullptr, 6},
        {"zip", "application/zip", BackendPreference::Auto, nullptr, 0},
#endif
        {"zip", "application/zip", BackendPreference::Libzip, nullptr, 6},
        {"tar", "application/x-tar", BackendPreference::Auto, nullptr, 6},
        {"tar.gz", "application/x-compressed-tar", BackendPreference::Auto, "pigz", 6},
        {"7z", "application/x-7z-compressed", BackendPreference::Auto, "7z", 9},
        {"rar", "application/vnd.rar", BackendPreference::Auto, "rar", 3},
    };
    for (const auto &format : formats) {
        SCOPED_TRACE(format.suffix);
        if (format.tool && QStandardPaths::findExecutable(QString::fromLatin1(format.tool)).isEmpty())
            continue;
        QTemporaryDir output(workspace.filePath(QStringLiteral("output-XXXXXX")));
        ASSERT_TRUE(output.isValid());
        const QString path = output.filePath(QStringLiteral("archive.") + QString::fromLatin1(format.suffix));
        std::unique_ptr<dcarchive::ArchiveSession> session(dcarchive::ArchiveSession::create(
                path, QString::fromLatin1(format.mime), true, format.preference));
        ASSERT_NE(session, nullptr);
        QList<double> updates;
        QObject::connect(session.get(), &dcarchive::ArchiveSession::signalprogress, [&](double percentage) {
            EXPECT_EQ(QFileInfo::exists(path), percentage == 100.0);
            updates.append(percentage);
        });
        FileEntry entry;
        entry.strFullPath = input.fileName();
        CompressOptions options;
        options.bCreateNew = true;
        options.qTotalSize = size;
        options.iCompressionLevel = format.level;
        ASSERT_EQ(session->addFiles({entry}, options), PFT_Nomral);
        ASSERT_GE(updates.size(), 2);
        EXPECT_GT(updates.first(), 0.0);
        EXPECT_TRUE(std::is_sorted(updates.cbegin(), updates.cend()));
        EXPECT_EQ(updates.last(), 100.0);
    }
}

TEST(ArchiveSessionTest, CompressionPausesUntilResumedOrCancelled)
{
    using namespace dcarchive;
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    QFile input(workspace.filePath(QStringLiteral("payload.bin")));
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_TRUE(input.resize(8 * 1024 * 1024));
    input.close();

    const auto checkPause = [&](const QString &extension, const QString &mime,
                                BackendPreference preference = BackendPreference::Auto) {
        SCOPED_TRACE(extension.toStdString());
        for (bool cancel : {false, true}) {
            SCOPED_TRACE(cancel);
            QTemporaryDir output(workspace.filePath(QStringLiteral("pause-XXXXXX")));
            ASSERT_TRUE(output.isValid());
            const QString path = output.filePath(QStringLiteral("archive.") + extension);
            auto backend = ArchiveRegistry::create(path, mime, ArchiveOperation::Create, preference);
            ASSERT_NE(backend, nullptr);
            ASSERT_TRUE(backend->open({}).result.ok());

            FakeObserver observer;
            AddRequest request;
            ArchiveEntry source;
            source.path = input.fileName();
            request.entries.append(source);
            request.cpuThreads = 1;
            backend->pause();
            auto running = std::async(std::launch::async, [&] {
                return backend->addFiles(request, observer).result;
            });
            EXPECT_TRUE(running.wait_for(std::chrono::milliseconds(300)) == std::future_status::timeout);
            if (cancel)
                backend->cancel();
            else
                backend->resume();
            const auto result = running.get();
            if (cancel)
                EXPECT_EQ(result.error, ArchiveError::Cancelled);
            else
                EXPECT_TRUE(result.ok()) << result.message.toStdString();
        }
    };
#ifdef DC_BUILT_PZIP
    checkPause(QStringLiteral("zip"), QStringLiteral("application/zip"));
#endif
    if (!QStandardPaths::findExecutable(QStringLiteral("pigz")).isEmpty())
        checkPause(QStringLiteral("tar.gz"), QStringLiteral("application/x-compressed-tar"));
    checkPause(QStringLiteral("tar"), QStringLiteral("application/x-tar"));
    checkPause(QStringLiteral("zip"), QStringLiteral("application/zip"), BackendPreference::Libzip);
}

TEST(ArchiveSessionTest, ZipListAndExtract)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());

    const QString archive = testSourceDir() + QStringLiteral("/test_sources/zip/extract/test.zip");
    std::unique_ptr<dcarchive::ArchiveSession> bridge(
            dcarchive::ArchiveSession::create(archive, QStringLiteral("application/zip"), false));
    ASSERT_NE(bridge, nullptr);

    EXPECT_EQ(bridge->list(), PFT_Nomral);
    const ArchiveData &data = ArchiveViewState::instance().archiveData();
    EXPECT_GT(data.listRootEntry.size(), 0);

    ExtractionOptions options;
    options.strTargetPath = workspace.path();
    options.bAllExtract = true;
    options.bExistList = true;
    EXPECT_EQ(bridge->extractFiles({}, options), PFT_Nomral);
    EXPECT_TRUE(QFileInfo::exists(QDir(workspace.path()).filePath(QStringLiteral("test.txt"))));
}

TEST(ArchiveSessionTest, ReportsNameWhenExtractCannotCreateOutput)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = testSourceDir() + QStringLiteral("/test_sources/zip/extract/test.zip");
    std::unique_ptr<dcarchive::ArchiveSession> session(dcarchive::ArchiveSession::create(
            archive, QStringLiteral("application/zip"), false));
    ASSERT_NE(session, nullptr);
    ASSERT_EQ(session->list(), PFT_Nomral);
    QString failureName;
    QObject::connect(session.get(), &dcarchive::ArchiveSession::signalFileWriteErrorName,
                     [&](const QString &name) { failureName = name; });

    ExtractionOptions options;
    options.bAllExtract = true;
    options.bExistList = true;
    options.strTargetPath = workspace.filePath(QStringLiteral("unavailable"));
    QFile obstruction(options.strTargetPath);
    ASSERT_TRUE(obstruction.open(QIODevice::WriteOnly));
    obstruction.close();
    EXPECT_EQ(session->extractFiles({}, options), PFT_Error);
    EXPECT_EQ(failureName, QStringLiteral("unavailable"));
}

TEST(ArchiveSessionTest, TarListAndExtract)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());

    const QString archive = testSourceDir() + QStringLiteral("/test_sources/tar/extract/test.tar");
    std::unique_ptr<dcarchive::ArchiveSession> bridge(
            dcarchive::ArchiveSession::create(archive, QStringLiteral("application/x-tar"), false));
    ASSERT_NE(bridge, nullptr);

    EXPECT_EQ(bridge->list(), PFT_Nomral);
    const ArchiveData &data = ArchiveViewState::instance().archiveData();
    EXPECT_GT(data.listRootEntry.size(), 0);

    ExtractionOptions options;
    options.strTargetPath = workspace.path();
    options.bAllExtract = true;
    options.bExistList = true;
    EXPECT_EQ(bridge->extractFiles({}, options), PFT_Nomral);
    EXPECT_TRUE(QFileInfo::exists(QDir(workspace.path()).filePath(QStringLiteral("test.txt"))));
}

TEST(ArchiveSessionTest, ExtractionReportsIntermediateByteProgress)
{
    using namespace dcarchive;
    QTemporaryDir work;
    ASSERT_TRUE(work.isValid());
    const QByteArray payload(2 * 1024 * 1024, 'p');
    const QString name = QStringLiteral("payload.bin");
    QFile input(work.filePath(name));
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write(payload), payload.size());
    input.close();

    for (const auto &format : {qMakePair("zip", "application/zip"),
                               qMakePair("tar", "application/x-tar")}) {
        SCOPED_TRACE(format.first);
        const QString mime = QString::fromLatin1(format.second);
        const QString archive = work.filePath(QStringLiteral("archive.") + QLatin1String(format.first));
        const auto preference = mime == QLatin1String("application/zip")
                ? BackendPreference::Libzip : BackendPreference::Libarchive;
        std::unique_ptr<ArchiveSession> writer(ArchiveSession::create(archive, mime, true, preference));
        ASSERT_NE(writer, nullptr);
        FileEntry entry;
        entry.strFullPath = input.fileName();
        CompressOptions compress;
        compress.bCreateNew = true;
        ASSERT_EQ(writer->addFiles({entry}, compress), PFT_Nomral);

        std::unique_ptr<ArchiveSession> reader(ArchiveSession::create(archive, mime, false));
        ASSERT_NE(reader, nullptr);
        ExtractionOptions extract;
        extract.strTargetPath = work.filePath(QString::fromLatin1(format.first));
        extract.bExistList = false;
        QList<double> progress;
        QObject::connect(reader.get(), &ArchiveSession::signalprogress, [&](double value) {
            progress.append(value);
            if (value == 100.0)
                EXPECT_EQ(QFileInfo(QDir(extract.strTargetPath).filePath(name)).size(), payload.size());
        });
        ASSERT_EQ(reader->extractFiles({}, extract), PFT_Nomral);
        EXPECT_TRUE(std::any_of(progress.cbegin(), progress.cend(), [](double value) {
            return value > 0.0 && value < 99.0;
        }));
        ASSERT_FALSE(progress.isEmpty());
        EXPECT_TRUE(std::is_sorted(progress.cbegin(), progress.cend()));
        EXPECT_EQ(progress.last(), 100.0);
    }
}

TEST(ArchiveSessionTest, ExtractWithoutExistingListCreatesTargetAndRefreshesData)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = testSourceDir() + QStringLiteral("/test_sources/tar/extract/test.tar");
    std::unique_ptr<dcarchive::ArchiveSession> bridge(
            dcarchive::ArchiveSession::create(archive, QStringLiteral("application/x-tar"), false));
    ASSERT_NE(bridge, nullptr);

    const QString target = workspace.filePath(QStringLiteral("nested/target"));
    ExtractionOptions options;
    options.strTargetPath = target;
    options.bAllExtract = true;
    options.bExistList = false;
    ASSERT_EQ(bridge->extractFiles({}, options), PFT_Nomral);
    EXPECT_TRUE(QFileInfo(target).isDir());
    EXPECT_TRUE(QFileInfo::exists(QDir(target).filePath(QStringLiteral("test.txt"))));
    EXPECT_GT(ArchiveViewState::instance().archiveData().listRootEntry.size(), 0);
}

TEST(ArchiveSessionTest, UsesAppPasswordForEncryptedHeader)
{
    if (QStandardPaths::findExecutable(QStringLiteral("7z")).isEmpty())
        GTEST_SKIP() << "7z is not available";
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("encrypted.7z"));
    const QString mime = QStringLiteral("application/x-7z-compressed");

    dcarchive::AddRequest add;
    dcarchive::ArchiveEntry entry;
    entry.path = testSourceDir() + QStringLiteral("/test_sources/tar/compress/test.txt");
    add.entries.append(entry);
    add.password = QStringLiteral("bridge-password");
    add.encryptList = true;
    dcarchive::SevenZipBackend writer(archive, mime);
    dcarchive::BackendObserver observer;
    ASSERT_TRUE(writer.addFiles(add, observer).result.ok());

    std::unique_ptr<dcarchive::ArchiveSession> bridge(
            dcarchive::ArchiveSession::create(archive, mime, false));
    ASSERT_NE(bridge, nullptr);
    int prompts = 0;
    QObject::connect(bridge.get(), &dcarchive::ArchiveSession::signalQuery, [&](Query *query) {
        ++prompts;
        query->setResponse(Result_Cancel);
    });
    EXPECT_EQ(bridge->list(), PFT_Cancel);
    EXPECT_EQ(prompts, 1);
    bridge->setPassword(add.password);
    EXPECT_EQ(bridge->list(), PFT_Nomral);
    EXPECT_EQ(prompts, 1);
    const ArchiveData &data = ArchiveViewState::instance().archiveData();
    EXPECT_TRUE(data.mapFileEntry.contains(QStringLiteral("test.txt")));
}

TEST(ArchiveSessionTest, RepeatedTarGzAppendPreservesExistingEntries)
{
    if (QStandardPaths::findExecutable(QStringLiteral("pigz")).isEmpty())
        GTEST_SKIP() << "pigz is not available";
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("entries.tar.gz"));
    std::unique_ptr<dcarchive::ArchiveSession> bridge(dcarchive::ArchiveSession::create(
            archive, QStringLiteral("application/x-compressed-tar"), true));
    ASSERT_NE(bridge, nullptr);

    const QStringList names{QStringLiteral("first"), QStringLiteral("second"), QStringLiteral("third")};
    for (const QString &name : names) {
        QFile source(workspace.filePath(name));
        ASSERT_TRUE(source.open(QIODevice::WriteOnly));
        ASSERT_EQ(source.write("file"), 4);
        source.close();
        FileEntry entry;
        entry.strFullPath = source.fileName();
        entry.strFileName = name;
        ASSERT_EQ(bridge->addFiles({entry}, CompressOptions()), PFT_Nomral);
    }

    ASSERT_EQ(bridge->list(), PFT_Nomral);
    const ArchiveData &data = ArchiveViewState::instance().archiveData();
    for (const QString &name : names)
        EXPECT_TRUE(data.mapFileEntry.contains(name)) << name.toStdString();
}

TEST(ArchiveSessionTest, CreatesAndReadsZipVolumes)
{
    if (QStandardPaths::findExecutable(QStringLiteral("7z")).isEmpty())
        GTEST_SKIP() << "7z is not available";
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("archive.zip"));
    const QString mime = QStringLiteral("application/zip");
    const QString name = QStringLiteral("sample.txt");
    QFile input(workspace.filePath(name));
    const QByteArray payload(32 * 1024, 'x');
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write(payload), payload.size());
    input.close();

    std::unique_ptr<dcarchive::ArchiveSession> writer(
            dcarchive::ArchiveSession::create(archive, mime, true));
    ASSERT_NE(writer, nullptr);
    FileEntry entry;
    entry.strFullPath = input.fileName();
    entry.strFileName = name;
    CompressOptions compress;
    compress.iCompressionLevel = 0;
    compress.bSplit = true;
    compress.iVolumeSize = 4;
    ASSERT_EQ(writer->addFiles({entry}, compress), PFT_Nomral);
    EXPECT_FALSE(QFileInfo::exists(archive));
    const QString firstVolume = archive + QStringLiteral(".001");
    ASSERT_EQ(QFileInfo(firstVolume).size(), 4096);
    EXPECT_TRUE(QFileInfo::exists(archive + QStringLiteral(".002")));

    std::unique_ptr<dcarchive::ArchiveSession> reader(
            dcarchive::ArchiveSession::create(firstVolume, mime, false));
    ASSERT_NE(reader, nullptr);
    ASSERT_EQ(reader->list(), PFT_Nomral);
    EXPECT_TRUE(ArchiveViewState::instance().archiveData().mapFileEntry.contains(name));
    ExtractionOptions extract;
    extract.strTargetPath = workspace.filePath(QStringLiteral("out"));
    ASSERT_TRUE(QDir().mkpath(extract.strTargetPath));
    ASSERT_EQ(reader->extractFiles({}, extract), PFT_Nomral);
    QFile output(QDir(extract.strTargetPath).filePath(name));
    ASSERT_TRUE(output.open(QIODevice::ReadOnly));
    EXPECT_EQ(output.readAll(), payload);
}

TEST(ArchiveSessionTest, MissingZipVolumeRequiresConsentAndPreservesIncompleteFiles)
{
    using namespace dcarchive;
    const QString tool = QStandardPaths::findExecutable(QStringLiteral("7z"));
    if (tool.isEmpty())
        GTEST_SKIP() << "7z is not available";
    QTemporaryDir work;
    ASSERT_TRUE(work.isValid());
    const QString goodName = QStringLiteral("a-good.txt");
    const QString cutName = QStringLiteral("z-incomplete.bin");
    const QByteArray goodBytes("complete file\n");
    const auto put = [](const QString &path, const QByteArray &bytes) {
        QFile file(path);
        return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
    };
    const auto get = [](const QString &path) {
        QFile file(path);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    };
    ASSERT_TRUE(put(work.filePath(goodName), goodBytes));
    ASSERT_TRUE(put(work.filePath(cutName), QByteArray(12 * 1024, 'x')));
    QProcess creator;
    creator.setWorkingDirectory(work.path());
    creator.start(tool, {QStringLiteral("a"), QStringLiteral("-tzip"), QStringLiteral("-mx=0"),
                         QStringLiteral("-v4096b"), QStringLiteral("fixture.zip"), goodName, cutName});
    ASSERT_TRUE(creator.waitForFinished());
    ASSERT_EQ(creator.exitCode(), 0);
    const QString first = work.filePath(QStringLiteral("fixture.zip.001"));
    const QString third = work.filePath(QStringLiteral("fixture.zip.003"));
    const QByteArray firstBytes = get(first);
    const QByteArray thirdBytes = get(third);
    ASSERT_FALSE(firstBytes.isEmpty());
    ASSERT_FALSE(thirdBytes.isEmpty());
    ASSERT_TRUE(QFile::remove(work.filePath(QStringLiteral("fixture.zip.002"))));

    const QString mime = QStringLiteral("application/zip");
    std::unique_ptr<ArchiveSession> session(ArchiveSession::create(first, mime, false));
    ASSERT_NE(session, nullptr);
    int prompts = 0;
    bool consent = false;
    QObject::connect(session.get(), &ArchiveSession::signalQuery, [&](Query *query) {
        if (qobject_cast<LoadCorruptQuery *>(query)) {
            ++prompts;
            query->setResponse(consent ? Result_Readonly : Result_Cancel);
        } else if (qobject_cast<OverwriteQuery *>(query)) {
            query->setResponse(Result_Overwrite);
        } else {
            ADD_FAILURE() << "Unexpected archive query";
            query->setResponse(Result_Cancel);
        }
    });
    EXPECT_EQ(session->list(), PFT_Cancel);
    EXPECT_EQ(prompts, 1);
    consent = true;
    ASSERT_EQ(session->list(), PFT_Nomral);
    EXPECT_EQ(prompts, 2);
    const auto &entries = ArchiveViewState::instance().archiveData().mapFileEntry;
    ASSERT_TRUE(entries.contains(goodName));
    ASSERT_TRUE(entries.contains(cutName));
    EXPECT_EQ(session->deleteFiles({entries.value(goodName)}), PFT_Error);

    ExtractionOptions extract;
    extract.strTargetPath = work.filePath(QStringLiteral("out"));
    ASSERT_TRUE(QDir().mkpath(extract.strTargetPath));
    const QString existing = QDir(extract.strTargetPath).filePath(cutName);
    ASSERT_TRUE(put(existing, "keep this file"));
    EXPECT_EQ(session->extractFiles({}, extract), PFT_Error);
    EXPECT_EQ(session->errorType(), ET_MissingVolume);
    EXPECT_EQ(get(QDir(extract.strTargetPath).filePath(goodName)), goodBytes);
    EXPECT_EQ(get(existing), QByteArray("keep this file"));
    EXPECT_EQ(prompts, 2);
    EXPECT_EQ(get(first), firstBytes);
    EXPECT_EQ(get(third), thirdBytes);
    const QStringList outputNames{goodName, cutName};
    EXPECT_EQ(QDir(extract.strTargetPath).entryList(QDir::Files | QDir::Hidden), outputNames);

    extract.strTargetPath = work.filePath(QStringLiteral("selected"));
    QString output;
    EXPECT_EQ(session->extractFiles({entries.value(goodName)}, extract, &output), PFT_Nomral);
    EXPECT_EQ(get(output), goodBytes);
}

TEST(ArchiveSessionTest, DestinationOptionsReachBackendForAppendAndExtract)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("destination.zip"));
    ASSERT_TRUE(QFile::copy(testSourceDir() + QStringLiteral("/test_sources/zip/extract/test.zip"),
                            archive));
    const QString added = workspace.filePath(QStringLiteral("added.txt"));
    QFile input(added);
    ASSERT_TRUE(input.open(QIODevice::WriteOnly));
    ASSERT_EQ(input.write("added"), 5);
    input.close();

    std::unique_ptr<dcarchive::ArchiveSession> bridge(
            dcarchive::ArchiveSession::create(archive, QStringLiteral("application/zip"), true));
    ASSERT_NE(bridge, nullptr);
    FileEntry file;
    file.strFullPath = added;
    file.strFileName = QStringLiteral("added.txt");
    CompressOptions append;
    append.strDestination = QStringLiteral("sub/child");
    ASSERT_EQ(bridge->addFiles({file}, append), PFT_Nomral);
    ASSERT_EQ(bridge->list(), PFT_Nomral);
    const ArchiveData &data = ArchiveViewState::instance().archiveData();
    const auto parent = data.mapFileEntry.constFind(QStringLiteral("sub/"));
    const auto child = data.mapFileEntry.constFind(QStringLiteral("sub/child/"));
    ASSERT_NE(parent, data.mapFileEntry.cend());
    ASSERT_NE(child, data.mapFileEntry.cend());
    EXPECT_TRUE(parent->isDirectory);
    EXPECT_TRUE(child->isDirectory);
    EXPECT_EQ(data.listRootEntry.count(parent.value()), 1);
    EXPECT_FALSE(data.listRootEntry.contains(child.value()));
    ASSERT_TRUE(data.mapFileEntry.contains(QStringLiteral("sub/child/added.txt")));

    FileEntry selected = data.mapFileEntry.value(QStringLiteral("sub/child/added.txt"));
    ExtractionOptions extract;
    extract.strTargetPath = workspace.filePath(QStringLiteral("target"));
    extract.strDestination = QStringLiteral("sub/child/");
    extract.bAllExtract = false;
    extract.bExistList = true;
    ASSERT_EQ(bridge->extractFiles({selected}, extract), PFT_Nomral);
    EXPECT_TRUE(QFileInfo::exists(workspace.filePath(QStringLiteral("target/added.txt"))));
}

TEST(ArchiveSessionTest, CancelledExtractionCanRetryOnSameInterface)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = testSourceDir() + QStringLiteral("/test_sources/tar/extract/test.tar");
    std::unique_ptr<dcarchive::ArchiveSession> bridge(
            dcarchive::ArchiveSession::create(archive, QStringLiteral("application/x-tar"), false));
    ASSERT_NE(bridge, nullptr);
    ASSERT_EQ(bridge->list(), PFT_Nomral);
    ExtractionOptions extract;
    extract.bAllExtract = true;
    extract.strTargetPath = workspace.filePath(QStringLiteral("out"));
    bridge->prepareOperation();
    bridge->cancel();
    EXPECT_EQ(bridge->extractFiles({}, extract), PFT_Cancel);
    ASSERT_EQ(bridge->extractFiles({}, extract), PFT_Nomral);
    EXPECT_TRUE(QFileInfo::exists(workspace.filePath(QStringLiteral("out/test.txt"))));
}

TEST(ArchiveSessionTest, ReplaceCommitDoesNotAppendOldEntries)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString archive = workspace.filePath(QStringLiteral("existing.tar"));
    ASSERT_TRUE(QFile::copy(testSourceDir() + QStringLiteral("/test_sources/tar/extract/test.tar"), archive));
    QFile source(workspace.filePath(QStringLiteral("replacement.txt")));
    ASSERT_TRUE(source.open(QIODevice::WriteOnly));
    ASSERT_EQ(source.write("new"), 3);
    source.close();
    FileEntry input;
    input.strFullPath = source.fileName();
    std::unique_ptr<dcarchive::ArchiveSession> bridge(
            dcarchive::ArchiveSession::create(archive, QStringLiteral("application/x-tar"), true));
    ASSERT_NE(bridge, nullptr);
    CompressOptions options;
    options.bCreateNew = true;
    ASSERT_EQ(bridge->addFiles({input}, options), PFT_Nomral);
    ASSERT_EQ(bridge->list(), PFT_Nomral);
    const auto &files = ArchiveViewState::instance().archiveData().mapFileEntry;
    EXPECT_TRUE(files.contains(QStringLiteral("replacement.txt")));
    EXPECT_FALSE(files.contains(QStringLiteral("test.txt")));
}

TEST(ArchiveSessionTest, RenameAndDeleteNestedDirectory)
{
    for (const QString &extension : {QStringLiteral("zip"), QStringLiteral("tar")}) {
        SCOPED_TRACE(extension.toStdString());
        QTemporaryDir workspace;
        ASSERT_TRUE(workspace.isValid());
        const QString mime = extension == QLatin1String("zip") ? QStringLiteral("application/zip")
                                                               : QStringLiteral("application/x-tar");
        const QString archive = workspace.filePath(QStringLiteral("archive.") + extension);
        const QString folder = workspace.filePath(QStringLiteral("folder"));
        ASSERT_TRUE(QDir().mkpath(folder));
        QFile child(QDir(folder).filePath(QStringLiteral("child.txt")));
        ASSERT_TRUE(child.open(QIODevice::WriteOnly));
        ASSERT_EQ(child.write("child"), 5);
        child.close();
        std::unique_ptr<dcarchive::ArchiveSession> bridge(dcarchive::ArchiveSession::create(
                archive, mime, true, extension == QLatin1String("zip")
                    ? dcarchive::BackendPreference::Libzip : dcarchive::BackendPreference::Auto));
        ASSERT_NE(bridge, nullptr);
        CompressOptions options;
        options.strDestination = QStringLiteral("parent/");
        FileEntry input;
        input.strFullPath = folder;
        input.strFileName = QStringLiteral("folder");
        input.isDirectory = true;
        ASSERT_EQ(bridge->addFiles({input}, options), PFT_Nomral);
        ASSERT_EQ(bridge->list(), PFT_Nomral);
        const auto &files = ArchiveViewState::instance().archiveData().mapFileEntry;
        ASSERT_TRUE(files.contains(QStringLiteral("parent/folder/")));
        FileEntry directory = files.value(QStringLiteral("parent/folder/"));
        directory.strAlias = QStringLiteral("renamed");
        ASSERT_EQ(bridge->renameFiles({directory}), PFT_Nomral);
        ASSERT_EQ(bridge->list(), PFT_Nomral);
        EXPECT_FALSE(files.contains(QStringLiteral("parent/folder/child.txt")));
        ASSERT_TRUE(files.contains(QStringLiteral("parent/renamed/child.txt")));
        ASSERT_TRUE(files.contains(QStringLiteral("parent/renamed/")));
        const FileEntry renamed = files.value(QStringLiteral("parent/renamed/"));
        ASSERT_EQ(bridge->deleteFiles({renamed}), PFT_Nomral);
        if (QFileInfo::exists(archive)) {
            ASSERT_EQ(bridge->list(), PFT_Nomral);
            EXPECT_FALSE(files.contains(QStringLiteral("parent/renamed/child.txt")));
        }
    }
}

TEST(ArchiveSessionTest, CancelledReplacementKeepsOriginalBytes)
{
    for (const QString &extension : {QStringLiteral("zip"), QStringLiteral("tar")}) {
        SCOPED_TRACE(extension.toStdString());
        QTemporaryDir workspace;
        ASSERT_TRUE(workspace.isValid());
        const QString formatRoot = extension == QLatin1String("zip") ? QStringLiteral("zip") : QStringLiteral("tar");
        const QString archive = workspace.filePath(QStringLiteral("original.") + extension);
        const QString mime = QStringLiteral("application/") + (extension == QLatin1String("zip") ? extension : QStringLiteral("x-tar"));
        ASSERT_TRUE(QFile::copy(testSourceDir() + QStringLiteral("/test_sources/") + formatRoot
                               + QStringLiteral("/extract/test.") + extension, archive));
        QFile original(archive);
        ASSERT_TRUE(original.open(QIODevice::ReadOnly));
        const QByteArray bytes = original.readAll();
        original.close();

        std::unique_ptr<dcarchive::ArchiveSession> bridge(dcarchive::ArchiveSession::create(
                archive, mime, true, extension == QLatin1String("zip")
                    ? dcarchive::BackendPreference::Libzip : dcarchive::BackendPreference::Auto));
        ASSERT_NE(bridge, nullptr);
        QFile source(workspace.filePath(QStringLiteral("new.txt")));
        ASSERT_TRUE(source.open(QIODevice::WriteOnly));
        ASSERT_EQ(source.write("new"), 3);
        source.close();
        FileEntry entry;
        entry.strFullPath = source.fileName();
        bool cancelled = false;
        QList<double> updates;
        QObject::connect(bridge.get(), &dcarchive::ArchiveSession::signalprogress,
                         [&](double percentage) { updates.append(percentage); });
        const auto connection = QObject::connect(bridge.get(), &dcarchive::ArchiveSession::signalCurFileName,
                [&](const QString &) {
                    cancelled = true;
                    bridge->cancel();
                });
        CompressOptions options;
        options.bCreateNew = true;
        EXPECT_EQ(bridge->addFiles({entry}, options), PFT_Cancel);
        EXPECT_TRUE(cancelled);
        EXPECT_FALSE(updates.contains(100.0));
        QObject::disconnect(connection);
        ASSERT_TRUE(original.open(QIODevice::ReadOnly));
        EXPECT_EQ(original.readAll(), bytes);
        original.close();
        EXPECT_EQ(bridge->addFiles({entry}, options), PFT_Nomral);
        ASSERT_FALSE(updates.isEmpty());
        EXPECT_EQ(updates.last(), 100.0);
    }
}

} // namespace
