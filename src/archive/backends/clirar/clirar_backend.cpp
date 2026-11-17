// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clirar_backend.h"

#include "datamanager.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

#include <csignal>
#include <linux/limits.h>

CliRarBackend::CliRarBackend(QObject *parent, const QVariantList &args)
    : CliRunner(parent, args)
{
    // 含注释的 rar 需要解析空行
}

CliRarBackend::~CliRarBackend()
{
}

QString CliRarBackend::programFor(WorkType work) const
{
    switch (work) {
    case WT_List:
    case WT_Extract:
        return QStringLiteral("unrar");
    case WT_Add:
    case WT_Delete:
    case WT_Move:
        return QStringLiteral("rar");
    default:
        return QString();
    }
}

QStringList CliRarBackend::argumentsFor(WorkType work, const QString &archive,
                                        const QStringList &files, const CompressOptions *options) const
{
    QStringList args;

    switch (work) {
    case WT_List:
        args << QStringLiteral("vt") << QStringLiteral("-v");
        break;
    case WT_Extract:
        args << QStringLiteral("x") << QStringLiteral("-kb");
        if (!m_strPassword.isEmpty())
            args << QStringLiteral("-p%1").arg(m_strPassword);
        break;
    case WT_Add:
        args << QStringLiteral("a");
        if (options) {
            if (options->iCompressionLevel > -1)
                args << QStringLiteral("-m%1").arg(options->iCompressionLevel);
            if (!options->strCompressionMethod.isEmpty())
                args << QStringLiteral("-ma%1").arg(options->strCompressionMethod);
            if (!options->strPassword.isEmpty()) {
                args << QStringLiteral("-p%1").arg(options->strPassword);
                if (options->bHeaderEncryption)
                    args << QStringLiteral("-hp%1").arg(options->strPassword);
            }
            if (options->bSplit && options->iVolumeSize > 0)
                args << QStringLiteral("-v%1k").arg(options->iVolumeSize);
        }
        break;
    case WT_Delete:
        args << QStringLiteral("d");
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


bool CliRarBackend::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed) for ")) && line.endsWith(QLatin1String(": "));
}

bool CliRarBackend::isWrongPasswordMsg(const QString &line)
{
    return line.startsWith(QLatin1String("The specified password is incorrect"))
            || line.startsWith(QLatin1String("Checksum error in the encrypted file"))
            || line.contains(QLatin1String("password incorrect"));
}

bool CliRarBackend::isCorruptArchiveMsg(const QString &line)
{
    return line == QLatin1String("Unexpected end of archive")
            || line.contains(QLatin1String("the file header is corrupt"))
            || line.endsWith(QLatin1String("checksum error"));
}

bool CliRarBackend::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device"))
            || (line.startsWith(QLatin1String("Write error in the file"))
                && line.endsWith(QLatin1String("[R]etry, [A]bort ")));
}

bool CliRarBackend::isFileExistsMsg(const QString &line)
{
    return line == QLatin1String("[Y]es, [N]o, [A]ll, n[E]ver, [R]ename, [Q]uit ");
}

bool CliRarBackend::isFileExistsFileName(const QString &line)
{
    return line.startsWith(QLatin1String("Would you like to replace the existing file "));
}

bool CliRarBackend::isMultiPasswordPrompt(const QString &line)
{
    return line.contains(QLatin1String("use current password ? [Y]es, [N]o, [A]ll"));
}

bool CliRarBackend::isOpenFileFailed(const QString &line)
{
    return line.startsWith(QLatin1String("Cannot create "));
}

void CliRarBackend::killProcess(bool emitFinished)
{
    Q_UNUSED(emitFinished)
    // 解压和等密码时必须强杀，其余场景让工具自己清理临时文件
    stopChild(m_workStatus == WT_Extract || m_bWaitingPassword);
}

bool CliRarBackend::readListLine(const QString &line)
{
    ArchiveData &data = DataManager::get_instance().archiveData();

    const QRegularExpression rxVersionLine(QStringLiteral("^UNRAR (\\d+\\.\\d+)( beta \\d)? .*$"));

    switch (m_parseState) {
    case ParseStateHeader:
    case ParseStateTitle:
        if (rxVersionLine.match(line).hasMatch())
            m_parseState = ParseStateArchiveInformation;
        break;

    case ParseStateArchiveInformation:
        if (line.startsWith(QLatin1String("Archive:"))) {
            m_comment = m_comment.trimmed();
            data.strComment = m_comment;
            data.qComressSize = QFileInfo(line.mid(9).trimmed()).size();
        } else if (line.startsWith(QLatin1String("Details:"))) {
            m_parseState = ParseStateEntryInformation;
        } else {
            m_comment.append(line + QLatin1Char('\n'));
        }
        break;

    case ParseStateEntryInformation: {
        const QString left = line.section(QLatin1Char(':'), 0, 0).trimmed();
        const QString right = line.section(QLatin1Char(':'), -1).trimmed();

        if (left == QLatin1String("Name")) {
            m_fileEntry.strFullPath = right;
            const QStringList pieces = right.split(QLatin1Char('/'), Qt::SkipEmptyParts);
            m_fileEntry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
        } else if (left == QLatin1String("Type")) {
            m_fileEntry.isDirectory = (right == QLatin1String("Directory"));
            if (m_fileEntry.isDirectory && !m_fileEntry.strFullPath.endsWith(QLatin1Char('/')))
                m_fileEntry.strFullPath += QLatin1Char('/');
        } else if (left == QLatin1String("Size")) {
            m_fileEntry.qSize = right.toLongLong();
            data.qSize += m_fileEntry.qSize;
        } else if (left == QLatin1String("mtime")) {
            const QString time = line.left(line.length() - 10);
            m_fileEntry.uLastModifiedTime =
                    uint(QDateTime::fromString(time.right(time.length() - 14),
                                               QStringLiteral("yyyy-MM-dd hh:mm:ss")).toSecsSinceEpoch());

            handleEntry(m_fileEntry);
            data.mapFileEntry.insert(m_fileEntry.strFullPath, m_fileEntry);
            m_fileEntry.reset();
        }
        break;
    }
    }

    return true;
}

bool CliRarBackend::handleLine(const QString &line, WorkType workStatus)
{
    if (isPasswordPrompt(line)) {
        if (workStatus == WT_List) {
            m_strEncryptedFileName = m_strArchiveName;
            DataManager::get_instance().archiveData().isListEncrypted = true;
        } else {
            m_strEncryptedFileName = line.right(line.length() - 40).remove(QLatin1String(": "));
        }

        m_eErrorType = ET_NeedPassword;
        if (handlePassword() == PFT_Cancel) {
            m_finishType = PFT_Cancel;
            return false;
        }
        return true;
    }

    if (isWrongPasswordMsg(line)) {
        m_eErrorType = ET_WrongPassword;
        m_finishType = PFT_Error;
        return false;
    }

    if (workStatus == WT_List)
        return readListLine(line);

    if (workStatus == WT_Extract) {
        if (isMultiPasswordPrompt(line))
            writeToProcess(QStringLiteral("Y\n").toLocal8Bit());

        if (handleFileExists(line))
            return true;

        if (isOpenFileFailed(line)) {
            QByteArray diskPath = line.toLocal8Bit();
            diskPath = diskPath.mid(QStringLiteral("Cannot create ").length());
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

        if (isDiskFullMsg(line)) {
            if (line.startsWith(QLatin1String("Write error in the file")))
                writeToProcess(QStringLiteral("A\n").toLocal8Bit());

            if (isInsufficientDiskSpace(getTargetPath(), 10 * 1024 * 1024)) {
                m_eErrorType = ET_InsufficientDiskSpace;
                emit signalFinished(PFT_Error);
                return false;
            }
        }

        handleProgress(line);
    }

    return true;
}
