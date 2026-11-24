// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archive_registry.h"
#include "legacy_cli7z_adapter.h"
#include "legacy_clirar_adapter.h"
#include "legacy_clipzip_adapter.h"
#include "legacy_libarchive_adapter.h"
#include "legacy_libzip_adapter.h"
#include "legacy_minizip_adapter.h"
#include "legacy_pigz_adapter.h"
#include "legacy_singlefile_adapter.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

#include <iostream>
#include <memory>

namespace {

struct Options
{
    QString mode = QStringLiteral("new");
    QString op;
    QString path;
    QString mime;
    QString target;
    QString destination;
    QStringList inputs;
    int level = 6;
    int threads = 1;
    bool overwrite = false;
    bool tar7z = false;
};

QString guessMime(const QString &path)
{
    const QString lower = path.toLower();
    if (lower.endsWith(QLatin1String(".zip")) || lower.endsWith(QLatin1String(".jar")))
        return QStringLiteral("application/zip");
    if (lower.endsWith(QLatin1String(".7z")))
        return QStringLiteral("application/x-7z-compressed");
    if (lower.endsWith(QLatin1String(".rar")))
        return QStringLiteral("application/vnd.rar");
    if (lower.endsWith(QLatin1String(".tar.gz")) || lower.endsWith(QLatin1String(".tgz")))
        return QStringLiteral("application/x-compressed-tar");
    if (lower.endsWith(QLatin1String(".tar.bz2")) || lower.endsWith(QLatin1String(".tbz2")))
        return QStringLiteral("application/x-bzip-compressed-tar");
    if (lower.endsWith(QLatin1String(".tar.xz")) || lower.endsWith(QLatin1String(".txz")))
        return QStringLiteral("application/x-xz-compressed-tar");
    if (lower.endsWith(QLatin1String(".tar")))
        return QStringLiteral("application/x-tar");
    if (lower.endsWith(QLatin1String(".gz")))
        return QStringLiteral("application/gzip");
    if (lower.endsWith(QLatin1String(".bz2")))
        return QStringLiteral("application/x-bzip");
    if (lower.endsWith(QLatin1String(".xz")))
        return QStringLiteral("application/x-xz");
    if (lower.endsWith(QLatin1String(".crx")))
        return QStringLiteral("application/x-chrome-extension");
    return QStringLiteral("application/octet-stream");
}

std::unique_ptr<dcarchive::ArchiveBackend> createLegacyBackend(const Options &options)
{
    const QString mime = options.mime;
    const QString lower = options.path.toLower();
    const bool create = options.op == QLatin1String("create");

    if (mime == QLatin1String("application/gzip")
            || mime == QLatin1String("application/x-gzip")
            || mime == QLatin1String("application/x-bzip")
            || mime == QLatin1String("application/x-bzip2")
            || mime == QLatin1String("application/x-xz")
            || mime == QLatin1String("application/x-lzma")) {
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacySingleFileAdapter(options.path, mime));
    }
    if (mime == QLatin1String("application/x-chrome-extension") || mime == QLatin1String("application/x-crx")) {
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyMinizipAdapter(options.path, mime));
    }
    if (mime == QLatin1String("application/x-7z-compressed")) {
        const QString meta = QStringLiteral(ARCHIVE_SOURCE_DIR)
                + QStringLiteral("/3rdparty/cli7zplugin/kerfuffle_cli7z.json");
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyCli7zAdapter(options.path, mime, meta));
    }
    if (mime == QLatin1String("application/vnd.rar")
            || mime == QLatin1String("application/x-rar-compressed")) {
        const QString meta = QStringLiteral(ARCHIVE_SOURCE_DIR)
                + QStringLiteral("/3rdparty/clirarplugin/kerfuffle_clirar.json");
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyCliRarAdapter(options.path, mime, meta));
    }
    if (mime == QLatin1String("application/zip")
            || mime == QLatin1String("application/x-zip-compressed")
            || mime == QLatin1String("application/java-archive")) {
        if (create) {
            return std::unique_ptr<dcarchive::ArchiveBackend>(
                    new dcarchive::LegacyClipzipAdapter(options.path, mime));
        }
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyLibzipAdapter(options.path, mime));
    }
    if (lower.endsWith(QLatin1String(".tar.gz")) || lower.endsWith(QLatin1String(".tgz"))) {
        if (create) {
            return std::unique_ptr<dcarchive::ArchiveBackend>(
                    new dcarchive::LegacyPigzAdapter(options.path, mime));
        }
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyLibarchiveAdapter(options.path, mime));
    }
    if (mime.startsWith(QLatin1String("application/x-tar"))
            || mime == QLatin1String("application/x-compressed-tar")
            || mime == QLatin1String("application/x-bzip-compressed-tar")
            || mime == QLatin1String("application/x-xz-compressed-tar")) {
        return std::unique_ptr<dcarchive::ArchiveBackend>(
                new dcarchive::LegacyLibarchiveAdapter(options.path, mime));
    }
    return nullptr;
}

std::unique_ptr<dcarchive::ArchiveBackend> createBackend(const Options &options)
{
    if (options.mode == QLatin1String("legacy"))
        return createLegacyBackend(options);
    return dcarchive::ArchiveRegistry::create(options.path, options.mime,
                                              options.op == QLatin1String("create"));
}

dcarchive::ArchiveEntry makeEntry(const QString &path)
{
    dcarchive::ArchiveEntry entry;
    const QFileInfo info(path);
    entry.path = info.absoluteFilePath();
    entry.name = info.fileName();
    entry.size = info.isDir() ? 0 : info.size();
    entry.type = info.isDir() ? dcarchive::EntryType::Directory : dcarchive::EntryType::File;
    return entry;
}

QJsonObject resultJson(const dcarchive::ArchiveResult &result)
{
    QJsonObject object;
    object.insert(QStringLiteral("ok"), result.ok());
    object.insert(QStringLiteral("cancelled"), result.cancelled);
    object.insert(QStringLiteral("error"), int(result.error));
    object.insert(QStringLiteral("message"), result.message);
    return object;
}

class CliObserver : public dcarchive::BackendObserver
{
public:
    explicit CliObserver(bool overwrite)
        : m_overwrite(overwrite)
    {
    }

    void onFileConflict(dcarchive::FileConflictRequest &request) override
    {
        request.response = m_overwrite
                ? dcarchive::FileConflictRequest::Response::Overwrite
                : dcarchive::FileConflictRequest::Response::Skip;
    }

    void onPasswordRequired(dcarchive::PasswordRequest &request) override
    {
        request.response = dcarchive::PasswordRequest::Response::Cancel;
    }

private:
    bool m_overwrite;
};

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Options options;
    const QStringList args = app.arguments().mid(1);
    for (int i = 0; i < args.size(); ++i) {
        const QString &arg = args.at(i);
        auto next = [&]() -> QString { return i + 1 < args.size() ? args.at(++i) : QString(); };
        if (arg == QLatin1String("--mode")) options.mode = next();
        else if (arg == QLatin1String("--op")) options.op = next();
        else if (arg == QLatin1String("--path")) options.path = next();
        else if (arg == QLatin1String("--mime")) options.mime = next();
        else if (arg == QLatin1String("--target")) options.target = next();
        else if (arg == QLatin1String("--input")) options.inputs << next();
        else if (arg == QLatin1String("--level")) options.level = next().toInt();
        else if (arg == QLatin1String("--threads")) options.threads = next().toInt();
        else if (arg == QLatin1String("--destination")) options.destination = next();
        else if (arg == QLatin1String("--tar7z")) options.tar7z = true;
        else if (arg == QLatin1String("--overwrite")) options.overwrite = true;
    }

    if (options.path.isEmpty() || options.op.isEmpty()) {
        std::cerr << "usage: archive-cli --op list|extract|create --path <archive> "
                     "[--mode new|legacy] [--mime <mime>] [--target <dir>] "
                     "[--destination <path>] [--input <file>]... [--level N] "
                     "[--threads N] [--overwrite] [--tar7z]\n";
        return 2;
    }
    if (options.mime.isEmpty())
        options.mime = guessMime(options.path);

    QElapsedTimer timer;
    timer.start();

    std::unique_ptr<dcarchive::ArchiveBackend> backend = createBackend(options);
    if (!backend) {
        std::cout << QJsonDocument(QJsonObject{
                {QStringLiteral("ok"), false},
                {QStringLiteral("message"), QStringLiteral("no backend for format")}})
                             .toJson(QJsonDocument::Compact).toStdString()
                  << std::endl;
        return 3;
    }

    CliObserver observer(options.overwrite);
    dcarchive::ArchiveResult result;
    QJsonArray entries;

    dcarchive::OpenRequest openRequest;
    openRequest.path = options.path;
    openRequest.mimeType = options.mime;
    openRequest.mode = options.op == QLatin1String("create")
            ? dcarchive::OpenMode::ReadWrite : dcarchive::OpenMode::ReadOnly;
    result = backend->open(openRequest).result;

    if (result.ok()) {
        if (options.op == QLatin1String("list")) {
            const dcarchive::EntryResult listed = backend->list();
            result = listed.result;
            for (const dcarchive::ArchiveEntry &entry : listed.info.entries) {
                QJsonObject item;
                item.insert(QStringLiteral("path"), entry.path);
                item.insert(QStringLiteral("name"), entry.name);
                item.insert(QStringLiteral("size"), double(entry.size));
                item.insert(QStringLiteral("index"), entry.index);
                item.insert(QStringLiteral("type"), int(entry.type));
                entries.append(item);
            }
        } else if (options.op == QLatin1String("extract")) {
            dcarchive::ExtractRequest request;
            request.targetPath = options.target;
            request.destinationPrefix = options.destination;
            request.allEntries = true;
            request.overwrite = options.overwrite
                    ? dcarchive::OverwritePolicy::Overwrite : dcarchive::OverwritePolicy::Skip;
            result = backend->extractFiles(request, observer).result;
        } else if (options.op == QLatin1String("create")) {
            dcarchive::AddRequest request;
            request.archivePath = options.path;
            request.format = options.mime;
            request.compressionLevel = options.level;
            request.cpuThreads = options.threads;
            request.destinationPath = options.destination;
            request.tar7z = options.tar7z;
            for (const QString &input : options.inputs)
                request.entries.append(makeEntry(input));
            result = backend->addFiles(request, observer).result;
        } else {
            std::cerr << "unsupported op: " << options.op.toStdString() << std::endl;
            return 2;
        }
    }

    QJsonObject output;
    output.insert(QStringLiteral("mode"), options.mode);
    output.insert(QStringLiteral("op"), options.op);
    output.insert(QStringLiteral("path"), options.path);
    output.insert(QStringLiteral("mime"), options.mime);
    output.insert(QStringLiteral("elapsed_ms"), double(timer.elapsed()));
    output.insert(QStringLiteral("result"), resultJson(result));
    output.insert(QStringLiteral("entries"), entries);
    std::cout << QJsonDocument(output).toJson(QJsonDocument::Compact).toStdString() << std::endl;
    return result.ok() ? 0 : 1;
}
