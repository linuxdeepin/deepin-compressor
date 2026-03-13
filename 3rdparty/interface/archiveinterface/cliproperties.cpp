/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2016 Ragnar Thomsen <rthomsen6@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cliproperties.h"
#include "archiveformat.h"
//#include "datamanager.h"
#include <QFileInfo>
#include <QDir>

#include <QRegularExpression>

CliProperties::CliProperties(QObject *parent, const KPluginMetaData &metaData, const CustomMimeType &archiveType)
    : QObject(parent)
    , m_mimeType(archiveType)
    , m_metaData(metaData)
{
}

QStringList CliProperties::addArgs(const QString &archive, const QStringList &files, const QString &password, bool headerEncryption, int compressionLevel, const QString &compressionMethod,
                                   const QString &encryptionMethod,
                                   int volumeSize, bool isTar7z, const QString &globalWorkDir,
                                   const QStringList &renameList)
{
    Q_UNUSED(globalWorkDir);

    if (!encryptionMethod.isEmpty()) {
        Q_ASSERT(!password.isEmpty());
    }

    // tar.7z 已改为 getTar7zTarArgs/getTar7z7zArgs + QProcess 管道，不再用脚本，此处不再拼命令串
    if (isTar7z) {
        return QStringList();
    } else {
        QStringList args;
        for (const QString &s : qAsConst(m_addSwitch)) {
            args << s;
        }

        if (!password.isEmpty()) {
            args << substitutePasswordSwitch(password, headerEncryption);
        }

        if (compressionLevel > -1) {
            args << substituteCompressionLevelSwitch(compressionLevel);
        }

        if (!compressionMethod.isEmpty()) {
            args << substituteCompressionMethodSwitch(compressionMethod);
        }

        if (!encryptionMethod.isEmpty()) {
            args << substituteEncryptionMethodSwitch(encryptionMethod);
        }
        if (volumeSize > 0) {
            args << substituteMultiVolumeSwitch(volumeSize);
        }

        if (!m_progressarg.isEmpty()) {
            args << m_progressarg;
        }

        args << archive;
        args << files;

        args.removeAll(QString());
        return args;
    }
}

QStringList CliProperties::getTar7zTarArgs(const QStringList &files, const QStringList &renameList)
{
    QStringList args;
    if (renameList.isEmpty()) {
        args << QStringLiteral("cf") << QStringLiteral("-");
    } else {
        args << renameList;
        args << QStringLiteral("-cf") << QStringLiteral("-");
    }
    for (QString file : files) {
        if (file.endsWith(QLatin1Char('/'))) {
            file.chop(1);
        }
        int pos = file.lastIndexOf(QLatin1Char('/'));
        if (pos > 0) {
            args << QStringLiteral("-C") << file.mid(0, pos) << file.mid(pos + 1);
        } else if (pos == 0) {
            args << QStringLiteral("-C") << QStringLiteral("/") << file.mid(1);
        } else {
            args << QStringLiteral("-C") << QStringLiteral(".") << file;
        }
    }
    return args;
}

QStringList CliProperties::getTar7z7zArgs(const QString &archive, const QString &password, bool headerEncryption,
                                         int compressionLevel, const QString &compressionMethod,
                                         const QString &encryptionMethod, int volumeSize)
{
    QStringList args;
    args << QStringLiteral("a") << QStringLiteral("-si") << archive;
    if (!password.isEmpty()) {
        // 单参数 -p+密码，避免 7z 再从 stdin 提示输入导致管道卡死
        args << (QStringLiteral("-p") + password);
    }
    if (compressionLevel > -1) {
        const QString s = substituteCompressionLevelSwitch(compressionLevel);
        if (!s.isEmpty()) {
            args << s;
        }
    }
    if (!compressionMethod.isEmpty()) {
        const QString s = substituteCompressionMethodSwitch(compressionMethod);
        if (!s.isEmpty()) {
            args << s;
        }
    }
    if (!encryptionMethod.isEmpty()) {
        const QString s = substituteEncryptionMethodSwitch(encryptionMethod);
        if (!s.isEmpty()) {
            args << s;
        }
    }
    if (volumeSize > 0) {
        const QString s = substituteMultiVolumeSwitch(volumeSize);
        if (!s.isEmpty()) {
            args << s;
        }
    }
    if (!m_progressarg.isEmpty()) {
        args << m_progressarg;
    }
    args.removeAll(QString());
    return args;
}

QStringList CliProperties::commentArgs(const QString &archive, const QString &commentfile)
{
    QStringList args;
    const QStringList commentSwitches = substituteCommentSwitch(commentfile);
    for (const QString &s : commentSwitches) {
        args << s;
    }

    args << archive;

    args.removeAll(QString());
    return args;
}

QStringList CliProperties::deleteArgs(const QString &archive, const QList<FileEntry> &files, const QString &password)
{
    QStringList args;
    args << m_deleteSwitch;

    if (!m_progressarg.isEmpty()) {
        args << m_progressarg;
    }

    if (!password.isEmpty()) {
        args << substitutePasswordSwitch(password);
    }

    args << archive;
    for (const FileEntry &e : files) {
        QString path = e.strFullPath;
        if (path.endsWith(QLatin1Char('/'))) {
            path.chop(1);
        }

        args << path;
    }

    args.removeAll(QString());
    return args;

}

QStringList CliProperties::extractArgs(const QString &archive, const QStringList &files, bool preservePaths, const QString &password)
{
    QStringList args;

    if (preservePaths && !m_extractSwitch.isEmpty()) {
        args << m_extractSwitch;
    } else if (!preservePaths && !m_extractSwitchNoPreserve.isEmpty()) {
        args << m_extractSwitchNoPreserve;
        args << "-y";
    }

    if (!password.isEmpty()) {
        args << substitutePasswordSwitch(password);
    }

    if (!m_progressarg.isEmpty()) {
        args << m_progressarg;
    }

    args << archive;
    args << files;

    args.removeAll(QString());
    return args;
}

QStringList CliProperties::listArgs(const QString &archive, const QString &password)
{
    QStringList args;
    for (const QString &s : qAsConst(m_listSwitch)) {
        args << s;
    }

    const auto encryptionType = ArchiveFormat::fromMetadata(m_mimeType, m_metaData).encryptionType();
    if (!password.isEmpty() && encryptionType == EncryptionType::HeaderEncrypted) {
        args << substitutePasswordSwitch(password);
    }
    args << archive;

    args.removeAll(QString());
    return args;
}

QStringList CliProperties::moveArgs(const QString &archive, const QList<FileEntry> &entries, const ArchiveData &stArchiveData, const QString &password)
{
    QStringList args;
    args << m_moveSwitch;

    if (!m_progressarg.isEmpty()) {
        args << m_progressarg;
    }

    if (!password.isEmpty()) {
        args << substitutePasswordSwitch(password);
    }

    args << archive;
    for (const FileEntry &entry : entries) {
        QString path = entry.strFullPath;
        QString strAlias;
        QMap<QString, FileEntry>::const_iterator iter = stArchiveData.mapFileEntry.find(entry.strFullPath);
        for (; iter != stArchiveData.mapFileEntry.end() ;) {
            if (!iter.key().startsWith(path)) {
                break;
            } else {
                if(entry.isDirectory) {
                    QString strPath = QFileInfo(entry.strFullPath.left(entry.strFullPath.length() - 1)).path();
                    if(strPath == "."){
                        strAlias = entry.strAlias + QDir::separator();
                    } else {
                        strAlias = strPath + QDir::separator() + entry.strAlias + QDir::separator();
                    }
                    strAlias = strAlias + QString(iter->strFullPath).right(QString(iter->strFullPath).length()-entry.strFullPath.length());
                } else {
                    QString strPath = QFileInfo(entry.strFullPath).path();
                    if(strPath == "." || strPath.isEmpty() || strPath.isNull()) {
                        strAlias = entry.strAlias;
                    } else {
                        strAlias = strPath + QDir::separator() + entry.strAlias;
                    }
                }
                args << path;
                args << strAlias;
                ++iter;
            }
        }
    }


    args.removeAll(QString());
    return args;
}

QStringList CliProperties::testArgs(const QString &archive, const QString &password)
{
    QStringList args;
    for (const QString &s : qAsConst(m_testSwitch)) {
        args << s;
    }

    if (!password.isEmpty()) {
        args << substitutePasswordSwitch(password);
    }

    args << archive;

    args.removeAll(QString());
    return args;
}

QStringList CliProperties::substituteCommentSwitch(const QString &commentfile) const
{
    Q_ASSERT(!commentfile.isEmpty());

    Q_ASSERT(ArchiveFormat::fromMetadata(m_mimeType, m_metaData).supportsWriteComment());

    QStringList commentSwitches = m_commentSwitch;
    Q_ASSERT(!commentSwitches.isEmpty());

    QMutableListIterator<QString> i(commentSwitches);
    while (i.hasNext()) {
        i.next();
        i.value().replace(QLatin1String("$CommentFile"), commentfile);
    }

    return commentSwitches;
}

QStringList CliProperties::substitutePasswordSwitch(const QString &password, bool headerEnc) const
{
    if (password.isEmpty() || password.isNull()) {
        return QStringList();
    }

    EncryptionType encryptionType = ArchiveFormat::fromMetadata(m_mimeType, m_metaData).encryptionType();
    Q_ASSERT(encryptionType != EncryptionType::Unencrypted);

    QStringList passwordSwitch;
    if (headerEnc) {
        passwordSwitch = m_passwordSwitchHeaderEnc;
    } else {
        passwordSwitch = m_passwordSwitch;
    }

    Q_ASSERT(!passwordSwitch.isEmpty());

    QMutableListIterator<QString> i(passwordSwitch);
    while (i.hasNext()) {
        i.next();
        i.value().replace(QLatin1String("$Password"), password);
    }

    return passwordSwitch;
}

QString CliProperties::substituteCompressionLevelSwitch(int level) const
{
    if (level < 0 || level > 9) {
        return QString();
    }

    Q_ASSERT(ArchiveFormat::fromMetadata(m_mimeType, m_metaData).maxCompressionLevel() != -1);

    QString compLevelSwitch = m_compressionLevelSwitch;
    Q_ASSERT(!compLevelSwitch.isEmpty());

    compLevelSwitch.replace(QLatin1String("$CompressionLevel"), QString::number(level));

    return compLevelSwitch;
}

QString CliProperties::substituteCompressionMethodSwitch(const QString &method) const
{
    if (method.isEmpty()) {
        return QString();
    }

    Q_ASSERT(!ArchiveFormat::fromMetadata(m_mimeType, m_metaData).compressionMethods().isEmpty());

    QString compMethodSwitch = m_compressionMethodSwitch[m_mimeType.name()].toString();
    Q_ASSERT(!compMethodSwitch.isEmpty());

    QString cliMethod = ArchiveFormat::fromMetadata(m_mimeType, m_metaData).compressionMethods().value(method).toString();

    compMethodSwitch.replace(QLatin1String("$CompressionMethod"), cliMethod);

    return compMethodSwitch;
}

QString CliProperties::substituteEncryptionMethodSwitch(const QString &method) const
{
    if (method.isEmpty()) {
        return QString();
    }

    const ArchiveFormat format = ArchiveFormat::fromMetadata(m_mimeType, m_metaData);

    Q_ASSERT(!format.encryptionMethods().isEmpty());

    QString encMethodSwitch = m_encryptionMethodSwitch[m_mimeType.name()].toString();
    if (encMethodSwitch.isEmpty()) {
        return QString();
    }

    Q_ASSERT(format.encryptionMethods().contains(method));

    encMethodSwitch.replace(QLatin1String("$EncryptionMethod"), method);

    return encMethodSwitch;
}

QString CliProperties::substituteMultiVolumeSwitch(int volumeSize) const
{
    // The maximum value we allow in the QDoubleSpinBox is 1,000,000MB. Converted to
    // KB this is 1,024,000,000.
    if (volumeSize <= 0 || volumeSize > 1024000000) {
        return QString();
    }

    Q_ASSERT(ArchiveFormat::fromMetadata(m_mimeType, m_metaData).supportsMultiVolume());

    QString multiVolumeSwitch = m_multiVolumeSwitch;
    Q_ASSERT(!multiVolumeSwitch.isEmpty());

    multiVolumeSwitch.replace(QLatin1String("$VolumeSize"), QString::number(volumeSize));

    return multiVolumeSwitch;
}

bool CliProperties::isTestPassedMsg(const QString &line)
{
    for (const QString &rx : qAsConst(m_testPassedPatterns)) {
        if (QRegularExpression(rx).match(line).hasMatch()) {
            return true;
        }
    }

    return false;
}
