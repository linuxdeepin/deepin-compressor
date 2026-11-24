// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_CLI_LONG_EXTRACTION_H
#define ARCHIVE_CLI_LONG_EXTRACTION_H

#include "archive_types.h"
#include "extraction_names.h"
#include "extraction_output.h"
#include "path_guard.h"

#include <QSet>
#include <QTemporaryDir>

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

namespace dcarchive {

// CLI extraction cannot write an oversized component before renaming it.
// Give only those entries private identifiers in a working copy; publish
// them under the user-visible names after the extractor has succeeded.
class CliLongExtraction
{
public:
    CliLongExtraction(const QString &target, const QVector<ArchiveEntry> &archiveEntries)
        : m_target(target)
        , m_staging(QDir(target).absoluteFilePath(QStringLiteral(".dc-extract-XXXXXX")))
    {
        QSet<QString> roots;
        for (const auto &entry : archiveEntries)
            roots.insert(QDir::cleanPath(entry.path).section(QLatin1Char('/'), 0, 0));
        while (roots.contains(m_aliasRoot))
            m_aliasRoot += QLatin1Char('_');
    }

    QString diskName(const QString &path, bool directory) { return m_names.outputName(path, directory); }
    void addDirectory(const QString &path) { m_directories.append(path); }
    bool hasOutput() const { return !m_files.isEmpty() || !m_directories.isEmpty(); }
    bool shortened() const { return m_names.shortened(); }
    bool hasFileDestination(const QString &target) const { return m_fileDestinations.contains(target); }

    QString addFile(const ArchiveEntry &entry, const QString &target)
    {
        QString source = entry.path;
        if (hasOversizedComponent(source)) {
            source = m_aliasRoot + QLatin1Char('/') + QString::number(entry.index);
            m_renames << entry.path << source;
        }
        m_files.append({source, target, entry.linkTarget});
        m_fileDestinations.insert(target);
        return source;
    }

    template <typename Rename>
    ArchiveResult prepare(const QString &archive, const std::atomic_bool &cancelled, Rename rename)
    {
        if (cancelled)
            return {ArchiveError::Cancelled, {}, true};
        if (!m_staging.isValid())
            return {ArchiveError::PermissionDenied, m_target};
        m_payload = m_staging.filePath(QStringLiteral(".payload"));
        if (!QDir().mkpath(m_payload))
            return {ArchiveError::PermissionDenied, m_payload};
        for (const auto &file : m_files) {
            if (!file.linkTarget.isEmpty() && !symlinkTargetWithinRoot(m_payload,
                    safeOutputPath(m_payload, file.source), file.linkTarget))
                return {ArchiveError::PermissionDenied, file.source};
        }
        if (m_renames.isEmpty())
            return {};

        const QString copyRoot = m_staging.filePath(QStringLiteral(".archive"));
        if (!QDir().mkpath(copyRoot))
            return {ArchiveError::PermissionDenied, copyRoot};
        m_archive = QDir(copyRoot).filePath(QFileInfo(archive).fileName());
        if (!QFile::copy(archive, m_archive)
                || !QFile::setPermissions(m_archive, QFileDevice::ReadOwner | QFileDevice::WriteOwner))
            return {ArchiveError::PermissionDenied, archive};

        for (qsizetype i = 0; i < m_renames.size();) {
            if (cancelled)
                return {ArchiveError::Cancelled, {}, true};
            QStringList pairs;
            qsizetype bytes = 0;
            do {
                const QString &oldName = m_renames.at(i);
                const QString &newName = m_renames.at(i + 1);
                bytes += QFile::encodeName(oldName).size() + QFile::encodeName(newName).size() + 2;
                pairs << oldName << newName;
                i += 2;
            } while (i < m_renames.size() && pairs.size() < 256 && bytes < 120000);
            auto result = rename(m_archive, pairs);
            if (!result.ok()) {
                if (result.error == ArchiveError::LongName)
                    result.message = pairs.first();
                return result;
            }
        }
        return {};
    }

    QString archivePath() const { return m_archive; }
    QString targetPath() const { return m_payload; }

    ArchiveResult publish(bool reportOutputPath, const std::atomic_bool &cancelled) const
    {
        if (cancelled)
            return {ArchiveError::Cancelled, {}, true};
        for (const auto &directory : m_directories) {
            if (!ensureDirectoryWithinRoot(m_target, directory))
                return {ArchiveError::PermissionDenied, directory};
        }
        const QString stageRoot = QFileInfo(m_payload).canonicalFilePath();
        ArchiveResult result;
        for (const auto &file : m_files) {
            if (cancelled)
                return {ArchiveError::Cancelled, {}, true};
            const QString source = safeOutputPath(m_payload, file.source);
            const QFileInfo info(source);
            if (source.isEmpty() || stageRoot.isEmpty()
                    || !archivePathWithin(QFileInfo(info.absolutePath()).canonicalFilePath(), stageRoot)
                    || !ensureParentWithinRoot(m_target, file.target))
                return {ArchiveError::PermissionDenied, file.target};
            const QByteArray encodedSource = QFile::encodeName(source);
            if (info.isSymLink()) {
                QByteArray buffer(PATH_MAX, '\0');
                const ssize_t length = ::readlink(encodedSource.constData(), buffer.data(), buffer.size());
                if (length <= 0 || length == buffer.size()
                        || !symlinkTargetWithinRoot(m_target, file.target, QFile::decodeName(buffer.left(length))))
                    return {ArchiveError::PermissionDenied, file.target};
            } else if (!info.isFile()) {
                return {ArchiveError::CorruptArchive, file.source};
            }
            if (::rename(encodedSource.constData(), QFile::encodeName(file.target).constData()) != 0) {
                if (errno != EXDEV || info.isSymLink())
                    return {errno == ENOSPC || errno == EDQUOT ? ArchiveError::DiskFull
                                                              : ArchiveError::PermissionDenied, file.target};
                QFile input(source);
                ExtractionOutput output;
                if (!input.open(QIODevice::ReadOnly))
                    return {ArchiveError::PermissionDenied, source};
                if (!output.open(file.target))
                    return {output.error(), file.target};
                while (!input.atEnd() && !cancelled) {
                    const QByteArray data = input.read(256 * 1024);
                    if (data.isEmpty() && input.error() != QFileDevice::NoError)
                        return {ArchiveError::PermissionDenied, source};
                    if (!output.write(data.constData(), data.size()))
                        return {output.error(), file.target};
                }
                if (cancelled)
                    return {ArchiveError::Cancelled, {}, true};
                if (!output.commit())
                    return {output.error(), file.target};
            }
            if (reportOutputPath)
                result.outputPath = file.target;
        }
        return result;
    }

private:
    struct Output {
        QString source;
        QString target;
        QString linkTarget;
    };

    QString m_target;
    QTemporaryDir m_staging;
    ExtractionNames m_names;
    QString m_archive;
    QString m_payload;
    QString m_aliasRoot = QStringLiteral(".dc-long-names");
    QStringList m_renames;
    QStringList m_directories;
    QSet<QString> m_fileDestinations;
    QVector<Output> m_files;
};

} // namespace dcarchive

#endif // ARCHIVE_CLI_LONG_EXTRACTION_H
