// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fakes.h"
#include "libzip_backend.h"
#include "rar_backend.h"
#include "sevenzip_backend.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <zip.h>

namespace {
using namespace dcarchive;

QByteArray contents(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

bool writeFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

class LongNameExtractionTest : public testing::TestWithParam<bool> {};

TEST_P(LongNameExtractionTest, NumberingAndConflicts)
{
    QTemporaryDir work;
    QTemporaryDir target;
    ASSERT_TRUE(work.isValid());
    ASSERT_TRUE(target.isValid());
    const bool sevenZip = GetParam();
    if (sevenZip && QStandardPaths::findExecutable(QStringLiteral("7z")).isEmpty())
        GTEST_SKIP() << "7z is not installed";
    const QChar letter = sevenZip ? QLatin1Char('a') : QChar(0x6587);
    const QString prefix(60, letter);
    const QString stem(sevenZip ? 256 : 84, letter);
    const QString parent = sevenZip ? QString(256, QLatin1Char('d')) : QStringLiteral("input");
    const QString dir = parent + QLatin1Char('/');
    const QString first = dir + stem + QStringLiteral("A.tar.gz");
    const QString second = dir + stem + QStringLiteral("B.tar.gz");
    const QString shortName(255, QLatin1Char('s'));
    const QString archivePath = work.filePath(sevenZip ? QStringLiteral("input.7z") : QStringLiteral("input.zip"));
    std::unique_ptr<ArchiveBackend> backend;
    if (sevenZip) {
        ASSERT_TRUE(QDir(work.path()).mkdir(QStringLiteral("input")));
        for (const auto &item : {qMakePair("a", "first"), qMakePair("b", "second"), qMakePair("c", "short")})
            ASSERT_TRUE(writeFile(work.filePath(QStringLiteral("input/") + QLatin1String(item.first)), item.second));
        QProcess tool;
        tool.setWorkingDirectory(work.path());
        tool.start(QStringLiteral("7z"), {QStringLiteral("a"), archivePath, QStringLiteral("input")});
        ASSERT_TRUE(tool.waitForFinished());
        ASSERT_EQ(tool.exitCode(), 0);
        tool.start(QStringLiteral("7z"), {QStringLiteral("rn"), archivePath, QStringLiteral("input/a"), first,
                   QStringLiteral("input/b"), second, QStringLiteral("input/c"), dir + shortName,
                   QStringLiteral("input"), parent});
        ASSERT_TRUE(tool.waitForFinished());
        ASSERT_EQ(tool.exitCode(), 0) << tool.readAllStandardError().constData();
        backend.reset(new SevenZipBackend(archivePath, QStringLiteral("application/x-7z-compressed")));
    } else {
        zip_t *zip = zip_open(QFile::encodeName(archivePath).constData(), ZIP_CREATE, nullptr);
        ASSERT_NE(zip, nullptr);
        for (const auto &item : {qMakePair(first, "first"), qMakePair(second, "second"),
                                 qMakePair(dir + shortName, "short")}) {
            auto *source = zip_source_buffer(zip, item.second, qstrlen(item.second), 0);
            ASSERT_NE(source, nullptr);
            ASSERT_GE(zip_file_add(zip, item.first.toUtf8().constData(), source, ZIP_FL_ENC_UTF_8), 0);
        }
        ASSERT_EQ(zip_close(zip), 0);
        backend.reset(new LibzipBackend(archivePath, QStringLiteral("application/zip")));
    }
    const QByteArray archiveBytes = contents(archivePath);
    const QString outputDir = sevenZip ? parent.left(60) + QStringLiteral("(001)") : parent;
    ASSERT_TRUE(QDir(target.path()).mkpath(outputDir));
    const QString base = target.filePath(outputDir + '/' + prefix);
    ASSERT_TRUE(writeFile(base + QStringLiteral("(001).tar.gz"), "keep"));
    ExtractRequest request;
    request.targetPath = target.path();
    FakeObserver observer;
    observer.conflictResponse = FileConflictRequest::Response::Skip;
    const auto result = backend->extractFiles(request, observer).result;
    ASSERT_TRUE(result.ok()) << result.message.toStdString();
    EXPECT_EQ(result.warning, ArchiveError::LongName);
    ASSERT_EQ(observer.conflictRequests.size(), 1);
    EXPECT_EQ(observer.conflictRequests.first().path, base + QStringLiteral("(001).tar.gz"));
    EXPECT_EQ(contents(base + QStringLiteral("(001).tar.gz")), QByteArray("keep"));
    EXPECT_EQ(contents(base + QStringLiteral("(002).tar.gz")), QByteArray("second"));
    EXPECT_EQ(contents(target.filePath(outputDir + '/' + shortName)), QByteArray("short"));

    observer.reset();
    observer.conflictResponse = FileConflictRequest::Response::OverwriteAll;
    ASSERT_TRUE(backend->extractFiles(request, observer).result.ok());
    EXPECT_EQ(contents(base + QStringLiteral("(001).tar.gz")), QByteArray("first"));
    EXPECT_EQ(contents(base + QStringLiteral("(002).tar.gz")), QByteArray("second"));

    const auto listed = backend->list();
    ASSERT_TRUE(listed.result.ok());
    request.allEntries = false;
    request.destinationPrefix = dir;
    request.targetPath = target.filePath(QStringLiteral("selected"));
    request.reportOutputPath = true;
    for (const auto &entry : listed.info.entries) {
        if (entry.path == second)
            request.entries.append(entry);
    }
    ASSERT_EQ(request.entries.size(), 1);
    const auto selected = backend->extractFiles(request, observer).result;
    ASSERT_TRUE(selected.ok()) << selected.message.toStdString();
    const QString selectedName = prefix + QStringLiteral("(001).tar.gz");
    EXPECT_EQ(selected.warning, ArchiveError::LongName);
    EXPECT_EQ(selected.outputPath, QDir(request.targetPath).filePath(selectedName));
    EXPECT_EQ(contents(selected.outputPath), QByteArray("second"));
    EXPECT_EQ(QDir(request.targetPath).entryList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot),
              QStringList{selectedName});

    observer.conflictResponse = FileConflictRequest::Response::Cancel;
    ASSERT_TRUE(writeFile(selected.outputPath, "local"));
    const auto cancelled = backend->extractFiles(request, observer).result;
    EXPECT_TRUE(cancelled.cancelled);
    EXPECT_EQ(contents(selected.outputPath), QByteArray("local"));
    EXPECT_EQ(QDir(request.targetPath).entryList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot),
              QStringList{selectedName});
    EXPECT_EQ(contents(archivePath), archiveBytes);

    const QString literalEntry = dir + selectedName;
    if (sevenZip) {
        ASSERT_TRUE(writeFile(work.filePath(QStringLiteral("literal")), "literal"));
        QProcess tool;
        tool.setWorkingDirectory(work.path());
        tool.start(QStringLiteral("7z"), {QStringLiteral("a"), archivePath, QStringLiteral("literal")});
        ASSERT_TRUE(tool.waitForFinished());
        ASSERT_EQ(tool.exitCode(), 0);
        tool.start(QStringLiteral("7z"), {QStringLiteral("rn"), archivePath, QStringLiteral("literal"), literalEntry});
        ASSERT_TRUE(tool.waitForFinished());
        ASSERT_EQ(tool.exitCode(), 0);
    } else {
        zip_t *zip = zip_open(QFile::encodeName(archivePath).constData(), 0, nullptr);
        ASSERT_NE(zip, nullptr);
        auto *source = zip_source_buffer(zip, "literal", 7, 0);
        ASSERT_NE(source, nullptr);
        ASSERT_GE(zip_file_add(zip, literalEntry.toUtf8().constData(), source, ZIP_FL_ENC_UTF_8), 0);
        ASSERT_EQ(zip_close(zip), 0);
    }
    request = {};
    request.targetPath = target.filePath(QStringLiteral("literal-conflict"));
    observer.reset();
    observer.conflictResponse = FileConflictRequest::Response::Skip;
    const auto literalConflict = backend->extractFiles(request, observer).result;
    ASSERT_TRUE(literalConflict.ok()) << literalConflict.message.toStdString();
    ASSERT_EQ(observer.conflictRequests.size(), 1);
    EXPECT_EQ(contents(QDir(request.targetPath).filePath(outputDir + '/' + selectedName)),
              QByteArray(sevenZip ? "literal" : "first"));
}

TEST(LongNameRarTest, ReadOnlyToolReportsLongName)
{
    const QString unrar = QStandardPaths::findExecutable(QStringLiteral("unrar"));
    if (unrar.isEmpty())
        GTEST_SKIP() << "unrar is not installed";
    QTemporaryDir work;
    QTemporaryDir tools;
    ASSERT_TRUE(work.isValid());
    ASSERT_TRUE(tools.isValid());
    ASSERT_TRUE(QFile::link(unrar, tools.filePath(QStringLiteral("unrar"))));
    struct RestorePath {
        QByteArray path = qgetenv("PATH");
        ~RestorePath() { qputenv("PATH", path); }
    } restorePath;
    qputenv("PATH", QFile::encodeName(tools.path()));
    // RAR5 with two 261-byte names and distinct text payloads.
    const auto repeated = QByteArray(256, 'a');
    const auto fixture = QByteArray::fromHex("526172211a0701003392b5e50a010506000501018080007cc575eaa20202030b180418b48302630cf83f8000018502")
            + repeated + QByteArray::fromHex("412e7478740a0313585b006bd4ec701f6c6f6e672066696c656e616d65207061796c6f616420410a61ca0bb9a20202030b180418b48302a05fd5148000018502")
            + repeated + QByteArray::fromHex("422e7478740a0313585b006bd4ec701f6c6f6e672066696c656e616d65207061796c6f616420420a1d77565103050400");
    const QString archive = work.filePath(QStringLiteral("long.rar"));
    ASSERT_TRUE(writeFile(archive, fixture));
    RarBackend backend(archive, QStringLiteral("application/vnd.rar"));
    ExtractRequest request;
    request.targetPath = work.filePath(QStringLiteral("target"));
    FakeObserver observer;
    const auto result = backend.extractFiles(request, observer).result;
    EXPECT_EQ(result.error, ArchiveError::LongName) << result.message.toStdString();
    EXPECT_FALSE(observer.currentFiles.isEmpty());
    EXPECT_TRUE(QDir(request.targetPath).entryList(QDir::Files | QDir::NoDotAndDotDot).isEmpty());
    EXPECT_EQ(contents(archive), fixture);
}

INSTANTIATE_TEST_SUITE_P(ZipAndSevenZip, LongNameExtractionTest, testing::Bool());

} // namespace
