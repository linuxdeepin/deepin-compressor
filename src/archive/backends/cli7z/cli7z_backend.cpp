// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cli7z_backend.h"

#include "datamanager.h"

#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>

#include <csignal>
#include <linux/limits.h>

namespace {

const QLatin1String kArchiveInfoDelimiter1("--");
const QLatin1String kArchiveInfoDelimiter2("----");
const QLatin1String kEntryInfoDelimiter("----------");

} // namespace

Cli7zBackend::Cli7zBackend(QObject *parent, const QVariantList &args)
    : CliRunner(parent, args)
{
    m_archiveType = ArchiveType7z;
}

Cli7zBackend::~Cli7zBackend()
{
}

bool Cli7zBackend::supportsLongNameRename() const
{
    return true;
}

QString Cli7zBackend::programFor(WorkType work) const
{
    return work == WT_List || work == WT_Add || work == WT_Extract || work == WT_Delete
                    || work == WT_Move
            ? QStringLiteral("7z")
            : QString();
}

QStringList Cli7zBackend::argumentsFor(WorkType work, const QString &archive,
                                       const QStringList &files, const CompressOptions *options) const
{
    QStringList args;

    switch (work) {
    case WT_List:
        args << QStringLiteral("l") << QStringLiteral("-slt");
        if (!m_strPassword.isEmpty())
            args << QStringLiteral("-p%1").arg(m_strPassword);
        break;
    case WT_Extract:
        args << QStringLiteral("x") << QStringLiteral("-bsp1");
        if (!m_strPassword.isEmpty())
            args << QStringLiteral("-p%1").arg(m_strPassword);
        break;
    case WT_Add:
        args << QStringLiteral("a") << QStringLiteral("-snl");
        if (options) {
            if (options->iCompressionLevel > -1)
                args << QStringLiteral("-mx=%1").arg(options->iCompressionLevel);

            // 7z 用 -m0=，zip 用 -mm=，与旧实现一致
            if (!options->strCompressionMethod.isEmpty()) {
                const bool isZip = m_mimetype.name() == QLatin1String("application/zip");
                args << (isZip ? QStringLiteral("-mm=%1") : QStringLiteral("-m0=%1"))
                                .arg(options->strCompressionMethod);
            }

            if (!options->strPassword.isEmpty()) {
                args << QStringLiteral("-p%1").arg(options->strPassword);
                if (options->bHeaderEncryption)
                    args << QStringLiteral("-mhe=on");
                if (!options->strEncryptionMethod.isEmpty()
                        && m_mimetype.name() == QLatin1String("application/zip")) {
                    args << QStringLiteral("-mem=%1").arg(options->strEncryptionMethod);
                }
            }

            if (options->bSplit && options->iVolumeSize > 0)
                args << QStringLiteral("-v%1k").arg(options->iVolumeSize);
        }
        break;
    case WT_Delete:
        args << QStringLiteral("d") << QStringLiteral("-bsp1");
        break;
    case WT_Move:
        args << QStringLiteral("rn");
        break;
    default:
        return args;
    }

    args << archive;
    args << files;
    args.removeAll(QString());
    return args;
}


bool Cli7zBackend::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed):"));
}

bool Cli7zBackend::isWrongPasswordMsg(const QString &line)
{
    return line.contains(QLatin1String("Wrong password"));
}

bool Cli7zBackend::isCorruptArchiveMsg(const QString &line)
{
    return line == QLatin1String("Unexpected end of archive")
            || line.startsWith(QLatin1String("ERROR = Missing volume :"))
            || line == QLatin1String("Headers Error");
}

bool Cli7zBackend::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device"));
}

bool Cli7zBackend::isFileExistsMsg(const QString &line)
{
    return line == QLatin1String("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ")
            || line == QLatin1String("? (Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ");
}

bool Cli7zBackend::isFileExistsFileName(const QString &line)
{
    return line.startsWith(QLatin1String("file ./")) || line.startsWith(QLatin1String("  Path:     ./"));
}

bool Cli7zBackend::isMultiPasswordPrompt(const QString &line)
{
    Q_UNUSED(line)
    return false;
}

bool Cli7zBackend::isOpenFileFailed(const QString &line)
{
    return line.contains(QLatin1String("ERROR: Can not open output file :"));
}

bool Cli7zBackend::isNoFilesArchive(const QString &line)
{
    return line.startsWith(QLatin1String("No files to process"));
}

void Cli7zBackend::killProcess(bool emitFinished)
{
    Q_UNUSED(emitFinished)
    // 解压和等密码时必须强杀，其余场景让工具自己清理临时文件
    stopChild(m_workStatus == WT_Extract || m_bWaitingPassword);
}

bool Cli7zBackend::readListLine(const QString &line)
{
    ArchiveData &data = DataManager::get_instance().archiveData();

    if (line.startsWith(QLatin1String("Open ERROR: Can not open the file as [7z] archive"))) {
        m_eErrorType = ET_ArchiveDamaged;
        m_finishType = PFT_Error;
        return false;
    }

    const QRegularExpression rxVersionLine(QStringLiteral("^(?:p7zip Version|7-Zip) ([\\d\\.]+).*$"));

    switch (m_parseState) {
    case ParseStateTitle:
        if (rxVersionLine.match(line).hasMatch())
            m_parseState = ParseStateHeader;
        break;

    case ParseStateHeader:
        if (line == kArchiveInfoDelimiter1 || line == kArchiveInfoDelimiter2) {
            data.qComressSize = line.mid(14).trimmed().toLongLong();
            m_parseState = ParseStateArchiveInformation;
        }
        break;

    case ParseStateArchiveInformation:
        if (line == kEntryInfoDelimiter) {
            m_parseState = ParseStateEntryInformation;
        } else if (line.startsWith(QLatin1String("Type = "))) {
            const QString type = line.mid(7).trimmed();
            if (type == QLatin1String("7z"))
                m_archiveType = ArchiveType7z;
            else if (type == QLatin1String("bzip2"))
                m_archiveType = ArchiveTypeBZip2;
            else if (type == QLatin1String("gzip"))
                m_archiveType = ArchiveTypeGZip;
            else if (type == QLatin1String("xz"))
                m_archiveType = ArchiveTypeXz;
            else if (type == QLatin1String("tar"))
                m_archiveType = ArchiveTypeTar;
            else if (type == QLatin1String("zip"))
                m_archiveType = ArchiveTypeZip;
            else if (type == QLatin1String("Rar"))
                m_archiveType = ArchiveTypeRar;
            else if (type == QLatin1String("Udf"))
                m_archiveType = ArchiveTypeUdf;
            else if (type == QLatin1String("Iso"))
                m_archiveType = ArchiveTypeIso;
            else if (type != QLatin1String("Split"))
                return false;
        }
        break;

    case ParseStateEntryInformation:
        if (line.startsWith(QLatin1String("Path = "))) {
            const QString path = QDir::fromNativeSeparators(line.mid(7).trimmed());
            m_fileEntry.strFullPath = path;
            const QStringList pieces = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
            m_fileEntry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
        } else if (line.startsWith(QLatin1String("Size = "))) {
            m_fileEntry.qSize = line.mid(7).trimmed().toLongLong();
            data.qSize += m_fileEntry.qSize;
        } else if (line.startsWith(QLatin1String("Modified = "))) {
            QString modified = line.mid(11).trimmed();
            // 新版 7-Zip 的 -slt 会带小数秒
            const int dot = modified.indexOf(QLatin1Char('.'));
            if (dot != -1)
                modified.truncate(dot);
            m_fileEntry.uLastModifiedTime =
                    uint(QDateTime::fromString(modified, QStringLiteral("yyyy-MM-dd hh:mm:ss")).toSecsSinceEpoch());

            if (m_archiveType == ArchiveTypeIso || m_archiveType == ArchiveTypeUdf) {
                // iso / udf 没有 Block/Version 字段，Modified 就是条目最后一行
                handleEntry(m_fileEntry);
                data.mapFileEntry.insert(m_fileEntry.strFullPath, m_fileEntry);
                m_fileEntry.reset();
            }
        } else if (line.startsWith(QLatin1String("Attributes = "))) {
            const QString attributes = line.mid(13).trimmed();
            m_fileEntry.isDirectory = attributes.startsWith(QLatin1Char('D'));
            if (m_fileEntry.isDirectory && !m_fileEntry.strFullPath.endsWith(QLatin1Char('/')))
                m_fileEntry.strFullPath += QLatin1Char('/');
        } else if (line.startsWith(QLatin1String("Folder = "))) {
            const QString folder = line.mid(9, 1);
            m_fileEntry.isDirectory = (folder == QLatin1String("+"));
            if (m_fileEntry.isDirectory && !m_fileEntry.strFullPath.endsWith(QLatin1Char('/')))
                m_fileEntry.strFullPath += QLatin1Char('/');
        } else if (line.startsWith(QLatin1String("Block = ")) || line.startsWith(QLatin1String("Version = "))) {
            handleEntry(m_fileEntry);
            data.mapFileEntry.insert(m_fileEntry.strFullPath, m_fileEntry);
            m_fileEntry.reset();
        }
        break;
    }

    return true;
}

bool Cli7zBackend::handleLine(const QString &line, WorkType workStatus)
{
    if (isPasswordPrompt(line)) {
        if (workStatus == WT_List)
            DataManager::get_instance().archiveData().isListEncrypted = true;

        m_eErrorType = ET_NeedPassword;
        if (handlePassword() == PFT_Cancel) {
            m_finishType = PFT_Cancel;
            m_bWaitingPassword = true;
            return false;
        }
        m_bWaitingPassword = false;
        return true;
    }

    if (isWrongPasswordMsg(line)) {
        m_eErrorType = ET_WrongPassword;
        if (workStatus != WT_Delete) {
            m_finishType = PFT_Error;
            return false;
        }
    }

    if (isDiskFullMsg(line))
        return false;

    if (workStatus == WT_List) {
        if (isCorruptArchiveMsg(line))
            m_isCorruptArchive = true;
        return readListLine(line);
    }

    if (workStatus == WT_Add || workStatus == WT_Extract || workStatus == WT_Delete) {
        if (isCorruptArchiveMsg(line) && workStatus == WT_Extract) {
            m_eErrorType = ET_MissingVolume;
            m_finishType = PFT_Error;
        }

        if (isNoFilesArchive(line) && workStatus == WT_Extract && !m_isEmptyArchive)
            m_isEmptyArchive = true;

        if (handleFileExists(line) && workStatus == WT_Extract)
            return true;

        if (workStatus == WT_Extract && isOpenFileFailed(line)) {
            QByteArray diskPath = line.toLocal8Bit();
            diskPath = diskPath.mid(diskPath.indexOf(':', diskPath.indexOf(':', line.indexOf(':') + 1) + 1) + 2);
            const QList<QByteArray> parts = diskPath.split('/');
            for (const QByteArray &part : parts) {
                if (NAME_MAX < part.length()) {
                    m_finishType = PFT_Error;
                    m_eErrorType = ET_LongNameError;
                    emit signalCurFileName(diskPath);
                    return false;
                }
            }
        }

        if (workStatus == WT_Add && line.startsWith(QLatin1String("System ERROR:"))) {
            const int code = line.mid(line.lastIndexOf(':') + 1).toInt();
            if (code == ENOSPC || code == EEXIST) {
                m_finishType = PFT_Error;
                m_eErrorType = (code == EEXIST) ? ET_ExistVolume : ET_InsufficientDiskSpace;
                return false;
            }
        }

        if ((workStatus == WT_Add && line.startsWith(QLatin1String("E_FAIL")))
                || (workStatus == WT_Extract && line.startsWith(QLatin1String("ERROR: E_FAIL")))) {
            m_finishType = PFT_Error;
            if (workStatus == WT_Extract && isInsufficientDiskSpace(getTargetPath(), 10 * 1024 * 1024))
                m_eErrorType = ET_InsufficientDiskSpace;
            return false;
        }

        if (workStatus == WT_Delete && m_eErrorType == ET_WrongPassword && line.startsWith(QLatin1String("E_FAIL"))) {
            m_finishType = PFT_Error;
            return false;
        }

        if (workStatus == WT_Add && line.contains(QLatin1String("MAX_PATHNAME_LEN"))) {
            m_finishType = PFT_Error;
            m_eErrorType = ET_LongNameError;
            return false;
        }

        handleProgress(line);
    }

    return true;
}
