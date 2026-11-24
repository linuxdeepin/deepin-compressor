// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archive_registry.h"

#include "clipzip_backend.h"
#include "libarchive_backend.h"
#include "libzip_backend.h"
#include "minizip_backend.h"
#include "pigz_backend.h"
#include "rar_backend.h"
#include "sevenzip_backend.h"
#include "singlefile_backend.h"

#include <QFileInfo>

namespace dcarchive {
namespace {

QString normalizedMime(const QString &mimeType)
{
    return mimeType.toLower();
}

bool matchesMime(const QString &mime, const QStringList &candidates)
{
    for (const QString &candidate : candidates) {
        if (mime == candidate)
            return true;
    }
    return false;
}

std::unique_ptr<ArchiveBackend> makeSingleFile(const QString &path, const QString &mime)
{
    return std::unique_ptr<ArchiveBackend>(new SingleFileBackend(path, mime));
}

} // namespace

namespace {

bool isZip(const QString &mime, const QString &lower)
{
    return matchesMime(mime, {QStringLiteral("application/zip"),
                              QStringLiteral("application/x-zip-compressed"),
                              QStringLiteral("application/java-archive")})
            || lower.endsWith(QLatin1String(".zip"))
            || lower.endsWith(QLatin1String(".jar"));
}

bool isTar(const QString &mime, const QString &lower)
{
    return matchesMime(mime, {QStringLiteral("application/x-tar"),
                              QStringLiteral("application/x-gtar"),
                              QStringLiteral("application/x-compressed-tar"),
                              QStringLiteral("application/x-bzip-compressed-tar"),
                              QStringLiteral("application/x-tarz"),
                              QStringLiteral("application/x-xz-compressed-tar"),
                              QStringLiteral("application/x-lzma-compressed-tar"),
                              QStringLiteral("application/x-lzip-compressed-tar"),
                              QStringLiteral("application/x-lrzip-compressed-tar"),
                              QStringLiteral("application/x-lz4-compressed-tar"),
                              QStringLiteral("application/x-zstd-compressed-tar"),
                              QStringLiteral("application/x-tzo")})
            || lower.endsWith(QLatin1String(".tar"))
            || lower.endsWith(QLatin1String(".tar.gz"))
            || lower.endsWith(QLatin1String(".tgz"))
            || lower.endsWith(QLatin1String(".tar.bz2"))
            || lower.endsWith(QLatin1String(".tbz2"))
            || lower.endsWith(QLatin1String(".tar.xz"))
            || lower.endsWith(QLatin1String(".txz"))
            || lower.endsWith(QLatin1String(".tar.lzma"))
            || lower.endsWith(QLatin1String(".tlz"))
            || lower.endsWith(QLatin1String(".tar.lz"))
            || lower.endsWith(QLatin1String(".tar.lzo"))
            || lower.endsWith(QLatin1String(".tzo"))
            || lower.endsWith(QLatin1String(".tar.lrz"))
            || lower.endsWith(QLatin1String(".tar.lrzip"))
            || lower.endsWith(QLatin1String(".tar.lz4"))
            || lower.endsWith(QLatin1String(".tar.zst"))
            || lower.endsWith(QLatin1String(".tar.zstd"))
            || lower.endsWith(QLatin1String(".tzst"))
            || lower.endsWith(QLatin1String(".tar.z"))
            || lower.endsWith(QLatin1String(".taz"));
}

bool isSevenZip(const QString &mime, const QString &lower)
{
    return mime == QLatin1String("application/x-7z-compressed")
            || lower.endsWith(QLatin1String(".7z"));
}

bool isRar(const QString &mime, const QString &lower)
{
    return matchesMime(mime, {QStringLiteral("application/vnd.rar"),
                              QStringLiteral("application/x-rar-compressed"),
                              QStringLiteral("application/x-rar")})
            || lower.endsWith(QLatin1String(".rar"));
}

bool isSingleFile(const QString &mime)
{
    return matchesMime(mime, {QStringLiteral("application/gzip"),
                              QStringLiteral("application/x-gzip"),
                              QStringLiteral("application/x-bzip"),
                              QStringLiteral("application/x-bzip2"),
                              QStringLiteral("application/x-xz"),
                              QStringLiteral("application/x-lzma")});
}

bool isIso(const QString &mime, const QString &lower)
{
    return mime == QLatin1String("application/x-iso9660-image")
            || lower.endsWith(QLatin1String(".iso"));
}

bool isLibarchiveFormat(const QString &mime, const QString &lower)
{
    if (isTar(mime, lower))
        return true;
    return matchesMime(mime, {QStringLiteral("application/x-deb"),
                              QStringLiteral("application/vnd.debian.binary-package"),
                              QStringLiteral("application/x-bcpio"),
                              QStringLiteral("application/x-cpio"),
                              QStringLiteral("application/x-cpio-compressed"),
                              QStringLiteral("application/x-sv4cpio"),
                              QStringLiteral("application/x-sv4crc"),
                              QStringLiteral("application/x-rpm"),
                              QStringLiteral("application/x-source-rpm"),
                              QStringLiteral("application/vnd.ms-cab-compressed"),
                              QStringLiteral("application/x-xar"),
                              QStringLiteral("application/x-iso9660-appimage"),
                              QStringLiteral("application/x-archive")})
            || lower.endsWith(QLatin1String(".deb"))
            || lower.endsWith(QLatin1String(".cpio"))
            || lower.endsWith(QLatin1String(".bcpio"))
            || lower.endsWith(QLatin1String(".rpm"))
            || lower.endsWith(QLatin1String(".cab"))
            || lower.endsWith(QLatin1String(".xar"))
            || lower.endsWith(QLatin1String(".appimage"));
}

std::unique_ptr<ArchiveBackend> createForRead(const QString &path,
                                              const QString &mimeType,
                                              const QString &mime,
                                              const QString &lower)
{
    if (lower.endsWith(QLatin1String(".7z.001")) || lower.endsWith(QLatin1String(".zip.001"))
            || (lower.endsWith(QLatin1String(".zip"))
                && QFileInfo::exists(path.chopped(2) + QStringLiteral("01")))) {
        return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
    }
    if (isSingleFile(mime))
        return std::unique_ptr<ArchiveBackend>(new SingleFileBackend(path, mimeType));
    if (mime == QLatin1String("application/x-chrome-extension")
            || mime == QLatin1String("application/x-crx")
            || lower.endsWith(QLatin1String(".crx"))) {
        return std::unique_ptr<ArchiveBackend>(new MinizipBackend(path, mimeType));
    }
    if (isRar(mime, lower))
        return std::unique_ptr<ArchiveBackend>(new RarBackend(path, mimeType));
    if (isIso(mime, lower))
        return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
    if (isSevenZip(mime, lower))
        return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
    if (isZip(mime, lower))
        return std::unique_ptr<ArchiveBackend>(new LibzipBackend(path, mimeType));
    if (isLibarchiveFormat(mime, lower))
        return std::unique_ptr<ArchiveBackend>(new LibarchiveBackend(path, mimeType));
    return nullptr;
}

std::unique_ptr<ArchiveBackend> createPreferred(const QString &path,
                                                const QString &mimeType,
                                                ArchiveOperation operation,
                                                BackendPreference preference)
{
    switch (preference) {
    case BackendPreference::SevenZip:
        return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
    case BackendPreference::Libarchive:
        return std::unique_ptr<ArchiveBackend>(new LibarchiveBackend(path, mimeType));
    case BackendPreference::Libzip:
        if (isZip(normalizedMime(mimeType), path.toLower()))
            return std::unique_ptr<ArchiveBackend>(new LibzipBackend(path, mimeType));
        return nullptr;
    case BackendPreference::Pigz:
        if (operation == ArchiveOperation::Create || operation == ArchiveOperation::CreateSplit)
            return std::unique_ptr<ArchiveBackend>(new PigzBackend(path, mimeType));
        return nullptr;
    case BackendPreference::Auto:
        break;
    }
    return nullptr;
}

} // namespace

std::unique_ptr<ArchiveBackend> ArchiveRegistry::create(const QString &path,
                                                        const QString &mimeType,
                                                        ArchiveOperation operation,
                                                        BackendPreference preference)
{
    const QString mime = normalizedMime(mimeType);
    const QString lower = path.toLower();

    if (preference != BackendPreference::Auto)
        return createPreferred(path, mimeType, operation, preference);

    switch (operation) {
    case ArchiveOperation::Read:
        return createForRead(path, mimeType, mime, lower);

    case ArchiveOperation::CreateSplit:
    case ArchiveOperation::Create:
        if (isZip(mime, lower)) {
            if (operation == ArchiveOperation::CreateSplit)
                return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
            return std::unique_ptr<ArchiveBackend>(new ClipzipBackend(path, mimeType));
        }
        if (lower.endsWith(QLatin1String(".tar.gz")) || lower.endsWith(QLatin1String(".tgz")))
            return std::unique_ptr<ArchiveBackend>(new PigzBackend(path, mimeType));
        if (isSevenZip(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
        if (isTar(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new LibarchiveBackend(path, mimeType));
        if (isRar(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new RarBackend(path, mimeType));
        return nullptr;

    case ArchiveOperation::Append:
        if (isZip(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new LibzipBackend(path, mimeType));
        if (isSevenZip(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
        if (isTar(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new LibarchiveBackend(path, mimeType));
        if (isRar(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new RarBackend(path, mimeType));
        return nullptr;

    case ArchiveOperation::Modify:
        if (isZip(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new LibzipBackend(path, mimeType));
        if (isSevenZip(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new SevenZipBackend(path, mimeType));
        if (isTar(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new LibarchiveBackend(path, mimeType));
        if (isRar(mime, lower))
            return std::unique_ptr<ArchiveBackend>(new RarBackend(path, mimeType));
        return nullptr;
    }
    return nullptr;
}

std::unique_ptr<ArchiveBackend> ArchiveRegistry::create(const QString &path,
                                                        const QString &mimeType,
                                                        ArchiveOperation operation)
{
    return create(path, mimeType, operation, BackendPreference::Auto);
}

std::unique_ptr<ArchiveBackend> ArchiveRegistry::create(const QString &path,
                                                        const QString &mimeType,
                                                        bool forWrite)
{
    return create(path, mimeType,
                  forWrite ? ArchiveOperation::Create : ArchiveOperation::Read);
}

} // namespace dcarchive
