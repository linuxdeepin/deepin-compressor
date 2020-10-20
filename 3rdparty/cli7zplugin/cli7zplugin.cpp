/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "cli7zplugin.h"

#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QDateTime>

Cli7zPluginFactory::Cli7zPluginFactory()
{
    registerPlugin<Cli7zPlugin>();
}

Cli7zPluginFactory::~Cli7zPluginFactory()
{

}

Cli7zPlugin::Cli7zPlugin(QObject *parent, const QVariantList &args)
    : CliInterface(parent, args)
{
    m_parseState = ParseStateTitle;
    setupCliProperties();
}

Cli7zPlugin::~Cli7zPlugin()
{

}

bool Cli7zPlugin::isPasswordList()
{
    QStringList programLst = m_process->program();

    foreach (auto str, programLst) {
        if (str.startsWith("-p")) {
            return true;
        }
    }

    return false;
}

bool Cli7zPlugin::readListLine(const QString &line)
{
    static const QLatin1String archiveInfoDelimiter1("--"); // 7z 9.13+
    static const QLatin1String archiveInfoDelimiter2("----"); // 7z 9.04
    static const QLatin1String entryInfoDelimiter("----------");

    const QRegularExpression rxVersionLine(QStringLiteral("^p7zip Version ([\\d\\.]+) .*$"));
    QRegularExpressionMatch matchVersion;

    switch (m_parseState) {
    case ParseStateTitle:
        matchVersion = rxVersionLine.match(line);
        if (matchVersion.hasMatch()) {
            m_parseState = ParseStateHeader;
            const QString p7zipVersion = matchVersion.captured(1);
        }
        break;
    case ParseStateHeader:
        if (line.startsWith(QLatin1String("Listing archive:"))) {

        } else if ((line == archiveInfoDelimiter1) || (line == archiveInfoDelimiter2)) {  // 开始读压缩包整体信息
            m_stArchiveData.qComressSize = line.mid(14).trimmed().toInt();
            m_parseState = ParseStateArchiveInformation;
        } else if (line.contains(QLatin1String("Error: "))) {

        }
        break;
    case ParseStateArchiveInformation:
        if (line == entryInfoDelimiter) {
            m_parseState = ParseStateEntryInformation;
        } else if (line.startsWith(QLatin1String("Type = "))) {
            const QString type = line.mid(7).trimmed();
            if (type == QLatin1String("7z")) {
                m_archiveType = ArchiveType7z;
            } else if (type == QLatin1String("bzip2")) {
                m_archiveType = ArchiveTypeBZip2;
            } else if (type == QLatin1String("gzip")) {
                m_archiveType = ArchiveTypeGZip;
            } else if (type == QLatin1String("xz")) {
                m_archiveType = ArchiveTypeXz;
            } else if (type == QLatin1String("tar")) {
                m_archiveType = ArchiveTypeTar;
            } else if (type == QLatin1String("zip")) {
                m_archiveType = ArchiveTypeZip;
            } else if (type == QLatin1String("Rar")) {
                m_archiveType = ArchiveTypeRar;
            } else if (type == QLatin1String("Split")) {
//                setMultiVolume(true);
            } else if (type == QLatin1String("Udf")) {
                m_archiveType = ArchiveTypeUdf;
            } else if (type == QLatin1String("Iso")) {
                m_archiveType = ArchiveTypeIso;
            } else {
                // Should not happen
                return false;
            }
        }
        break;
    case ParseStateComment:
        if (line == entryInfoDelimiter) {
            m_parseState = ParseStateEntryInformation;
        }
        break;
    case ParseStateEntryInformation:  // 读压缩包内具体文件信息
        if (line.startsWith(QLatin1String("Path = "))) {
            const QString entryFilename = QDir::fromNativeSeparators(line.mid(7).trimmed());
            m_fileEntry.strFullPath = entryFilename;   // 文件在压缩包内绝对路径路径
            const QStringList pieces = m_fileEntry.strFullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
            m_fileEntry.strFileName = pieces.isEmpty() ? QString() : pieces.last();   // 文件名称
        } else if (line.startsWith(QLatin1String("Size = "))) {
            m_fileEntry.qSize = line.mid(7).trimmed().toInt();  // 文件实际大小
            m_stArchiveData.qSize += line.mid(7).trimmed().toInt();  // 累加获取压缩包内所有文件总大小
        } else if (line.startsWith(QLatin1String("Modified = "))) {
            // 文件最后修改时间
            m_fileEntry.uLastModifiedTime = QDateTime::fromString(line.mid(11).trimmed(), QStringLiteral("yyyy-MM-dd hh:mm:ss")).toTime_t();
        } else if (line.startsWith(QLatin1String("Attributes = "))) {  // 文件权限
            const QString attributes = line.mid(13).trimmed();
            if (attributes.contains(QLatin1Char('D'))) {  // D开头为文件夹
                m_fileEntry.isDirectory = true;
                m_fileEntry.qSize = 0;
                if (!m_fileEntry.strFullPath.endsWith("/")) {  // 如果是文件夹且路径最后没有分隔符，手动添加
                    m_fileEntry.strFullPath = m_fileEntry.strFullPath + "/";
                }
            } else {
                m_fileEntry.isDirectory = false;
            }
        } else if (line.startsWith(QLatin1String("Block = ")) || line.startsWith(QLatin1String("Version = "))) {  // 单文件最后一行信息
            QString name = m_fileEntry.strFullPath;
            // 获取第一层数据
            if (!name.contains(QDir::separator()) || (name.count(QDir::separator()) == 1 && name.endsWith(QDir::separator()))) {
                m_stArchiveData.listRootEntry.push_back(m_fileEntry);
            }

            // 存储总数据
            m_stArchiveData.mapFileEntry[name] = m_fileEntry;
        }
        break;
    }

    return true;
}

void Cli7zPlugin::setupCliProperties()
{
    m_cliProps->setProperty("captureProgress", false);

    m_cliProps->setProperty("addProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("addSwitch", QStringList{QStringLiteral("a"),
                                                     QStringLiteral("-l")});

    m_cliProps->setProperty("deleteProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("deleteSwitch", QStringLiteral("d"));

    m_cliProps->setProperty("extractProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("extractSwitch", QStringList{QStringLiteral("x")});
    m_cliProps->setProperty("progressarg", QStringList{QStringLiteral("-bsp1")});
    m_cliProps->setProperty("extractSwitchNoPreserve", QStringList{QStringLiteral("e")});

    m_cliProps->setProperty("listProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("listSwitch", QStringList{QStringLiteral("l"),
                                                      QStringLiteral("-slt")});

    m_cliProps->setProperty("moveProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("moveSwitch", QStringLiteral("rn"));

    m_cliProps->setProperty("testProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("testSwitch", QStringLiteral("t"));

    m_cliProps->setProperty("passwordSwitch", QStringList{QStringLiteral("-p$Password")});
    m_cliProps->setProperty("passwordSwitchHeaderEnc", QStringList{QStringLiteral("-p$Password"),
                                                                   QStringLiteral("-mhe=on")});
    m_cliProps->setProperty("compressionLevelSwitch", QStringLiteral("-mx=$CompressionLevel"));
    m_cliProps->setProperty("compressionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QStringLiteral("-m0=$CompressionMethod")},
        {QStringLiteral("application/zip"), QStringLiteral("-mm=$CompressionMethod")}
    });
    m_cliProps->setProperty("encryptionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QString()},
        {QStringLiteral("application/zip"), QStringLiteral("-mem=$EncryptionMethod")}
    });
    m_cliProps->setProperty("multiVolumeSwitch", QStringLiteral("-v$VolumeSizek"));
    m_cliProps->setProperty("testPassedPatterns", QStringList{QStringLiteral("^Everything is Ok$")});
    m_cliProps->setProperty("fileExistsFileNameRegExp", QStringList{QStringLiteral("^file \\./(.*)$"),
                                                                    QStringLiteral("^  Path:     \\./(.*)$")});
    m_cliProps->setProperty("fileExistsInput", QStringList{QStringLiteral("Y"),   //Overwrite
                                                           QStringLiteral("N"),   //Skip
                                                           QStringLiteral("A"),   //Overwrite all
                                                           QStringLiteral("S"),   //Autoskip
                                                           QStringLiteral("Q")}); //Cancel
    m_cliProps->setProperty("multiVolumeSuffix", QStringList{QStringLiteral("$Suffix.001")});
}

bool Cli7zPlugin::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed):"));
}

bool Cli7zPlugin::isWrongPasswordMsg(const QString &line)
{
    return line.contains(QLatin1String("Wrong password"));
}

bool Cli7zPlugin::isCorruptArchiveMsg(const QString &line)
{
    return (line == QLatin1String("Unexpected end of archive") ||
            line == QLatin1String("Headers Error"));
}

bool Cli7zPlugin::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device"));
}

bool Cli7zPlugin::isFileExistsMsg(const QString &line)
{
    return (line == QLatin1String("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ") ||
            line == QLatin1String("? (Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "));
}

bool Cli7zPlugin::isFileExistsFileName(const QString &line)
{
    return (line.startsWith(QLatin1String("file ./")) ||
            line.startsWith(QLatin1String("  Path:     ./")));
}

bool Cli7zPlugin::handleLine(const QString &line, WorkType workStatus)
{
    qDebug() << line;

    if (workStatus == WT_Add || workStatus == WT_Extract) {  // 压缩、解压进度
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            if (percentage > 0) {
                if (line.contains("\b\b\b\b") == true) {
                    QStringRef strfilename;
                    int count = line.indexOf("+");
                    if (-1 == count) {
                        count = line.indexOf("-");
                    }

                    if (count > 0) {
                        strfilename = line.midRef(count + 2);
                    }

                    emit signalprogress(percentage);
                    emit signalCurFileName(strfilename.toString());
                }
            }
        }
    } else if (workStatus == WT_List) {
        return readListLine(line);   // 加载压缩文件，处理命令行内容
    }

    return true;
}
