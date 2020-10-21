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
#include "clirarplugin.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

CliRarPluginFactory::CliRarPluginFactory()
{
    registerPlugin<CliRarPlugin>();
}

CliRarPluginFactory::~CliRarPluginFactory()
{

}

CliRarPlugin::CliRarPlugin(QObject *parent, const QVariantList &args)
    : CliInterface(parent, args)
{
    // 如果rar压缩包含有注释信息，没有标志出comment，直接显示注释内容，需要根据空的line进行解析
    setListEmptyLines(true);
    setupCliProperties();
}

CliRarPlugin::~CliRarPlugin()
{

}

void CliRarPlugin::setupCliProperties()
{
    m_cliProps->setProperty("captureProgress", true);

    m_cliProps->setProperty("addProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("addSwitch", QStringList({QStringLiteral("a")}));

    m_cliProps->setProperty("deleteProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("deleteSwitch", QStringLiteral("d"));

    m_cliProps->setProperty("extractProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("extractSwitch", QStringList{QStringLiteral("x"),
                                                         QStringLiteral("-kb"),
                                                         QStringLiteral("-p-")});
    m_cliProps->setProperty("extractSwitchNoPreserve", QStringList{QStringLiteral("e"),
                                                                   QStringLiteral("-kb"),
                                                                   QStringLiteral("-p-")});

    m_cliProps->setProperty("listProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("listSwitch", QStringList{QStringLiteral("vt"),
                                                      QStringLiteral("-v")});

    m_cliProps->setProperty("moveProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("moveSwitch", QStringLiteral("rn"));

    m_cliProps->setProperty("testProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("testSwitch", QStringLiteral("t"));

    m_cliProps->setProperty("commentSwitch", QStringList{QStringLiteral("c"),
                                                         QStringLiteral("-z$CommentFile")});

    m_cliProps->setProperty("passwordSwitch", QStringList{QStringLiteral("-p$Password")});
    m_cliProps->setProperty("passwordSwitchHeaderEnc", QStringList{QStringLiteral("-hp$Password")});

    m_cliProps->setProperty("compressionLevelSwitch", QStringLiteral("-m$CompressionLevel"));
    m_cliProps->setProperty("compressionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/vnd.rar"), QStringLiteral("-ma$CompressionMethod")},
        {QStringLiteral("application/x-rar"), QStringLiteral("-ma$CompressionMethod")}
    });

    m_cliProps->setProperty("multiVolumeSwitch", QStringLiteral("-v$VolumeSizek"));

    m_cliProps->setProperty("testPassedPatterns", QStringList{QStringLiteral("^All OK$")});
    m_cliProps->setProperty("fileExistsFileNameRegExp", QStringList{QStringLiteral("^(.+) already exists. Overwrite it"),  // unrar 3 & 4
                                                                    QStringLiteral("^Would you like to replace the existing file (.+)$")}); // unrar 5
    m_cliProps->setProperty("fileExistsInput", QStringList{QStringLiteral("Y"),   //Overwrite
                                                           QStringLiteral("N"),   //Skip
                                                           QStringLiteral("A"),   //Overwrite all
                                                           QStringLiteral("E"),   //Autoskip
                                                           QStringLiteral("Q")}); //Cancel

    // rar will sometimes create multi-volume archives where first volume is
    // called name.part1.rar and other times name.part01.rar.
    m_cliProps->setProperty("multiVolumeSuffix", QStringList{QStringLiteral("part01.$Suffix"),
                                                             QStringLiteral("part1.$Suffix")});
}

bool CliRarPlugin::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed) for"));
}

bool CliRarPlugin::isWrongPasswordMsg(const QString &line)
{
    return (line.contains(QLatin1String("password incorrect")) || line.contains(QLatin1String("wrong password"))
            || line.contains(QLatin1String("The specified password is incorrect")));
}

bool CliRarPlugin::isCorruptArchiveMsg(const QString &line)
{
    return (line == QLatin1String("Unexpected end of archive") || line.contains(QLatin1String("the file header is corrupt"))
            || line.endsWith(QLatin1String("checksum error")));
}

bool CliRarPlugin::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device"));
}

bool CliRarPlugin::isFileExistsMsg(const QString &line)
{
    return (line == QLatin1String("[Y]es, [N]o, [A]ll, n[E]ver, [R]ename, [Q]uit "));
}

bool CliRarPlugin::isFileExistsFileName(const QString &line)
{
    return (line.startsWith(QLatin1String("Would you like to replace the existing file ")));
}

bool CliRarPlugin::readListLine(const QString &line)
{
    QRegularExpression rxVersionLine(QStringLiteral("^UNRAR (\\d+\\.\\d+)( beta \\d)? .*$"));
    QRegularExpressionMatch matchVersion = rxVersionLine.match(line);

    /*
    UNRAR 5.61 beta 1 freeware      Copyright (c) 1993-2018 Alexander Roshal

    Archive: /home/chenglu/Desktop/out.rar
    Details: RAR 5

            Name: out.txt
            Type: File
            Size: 204190
     Packed size: 17557
           Ratio: 8%
           mtime: 2020-08-25 15:37:56,000000000
      Attributes: -rw-r--r--
           CRC32: 530E5FD5
         Host OS: Unix
     Compression: RAR 5.0(v50) -m3 -md=256K
     */

    switch (m_parseState) {
    case ParseStateHeader:
    case ParseStateTitle:
        if (matchVersion.hasMatch()) {  // unrar信息读取完，开始读压缩包相关信息
            m_parseState = ParseStateArchiveInformation;
        }
        break;
    case ParseStateArchiveInformation:  // // 读取压缩包信息
        if (line.startsWith(QLatin1String("Archive:"))) { // 压缩包全路径
            // 压缩包注释信息
            m_comment = m_comment.trimmed();
            m_stArchiveData.strComment = m_comment;

            // 通过压缩包全路径，获取压缩包大小
            QFileInfo file(line.mid(9).trimmed());
            m_stArchiveData.qComressSize = file.size();
        } else if (line.startsWith(QLatin1String("Details:"))) {
            // 读完压缩包信息，开始读内部文件信息
            m_parseState = ParseStateEntryInformation;
        } else {
            // 读取注释信息
            m_comment.append(line + QLatin1Char('\n'));
        }
        break;
    case ParseStateEntryInformation:  // 读压缩包内文件具体信息
        // ':'左侧字符串
        QString parseLineLeft = line.section(QLatin1Char(':'), 0, 0).trimmed();
        // ':'右侧字符串
        QString parseLineRight = line.section(QLatin1Char(':'), -1).trimmed();
        if (parseLineLeft == QLatin1String("Name")) {
            // 文件在压缩包内绝对路径路径
            m_fileEntry.strFullPath = parseLineRight;

            // 文件名称
            const QStringList pieces = m_fileEntry.strFullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
            m_fileEntry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
        } else if (parseLineLeft == QLatin1String("Type")) {
            if (parseLineRight == QLatin1String("Directory")) {
                // 是文件夹
                m_fileEntry.isDirectory = true;

                // 如果是文件夹且路径最后没有分隔符，手动添加
                if (!m_fileEntry.strFullPath.endsWith(QLatin1Char('/'))) {
                    m_fileEntry.strFullPath = m_fileEntry.strFullPath + QLatin1Char('/');
                }
            } else {
                // 不是文件夹
                m_fileEntry.isDirectory = false;
            }
        } else if (parseLineLeft == QLatin1String("Size")) {
            // 单文件实际大小
            m_fileEntry.qSize = line.section(QLatin1Char(':'), -1).trimmed().toInt();

            // 压缩包内所有文件总大小
            m_stArchiveData.qSize += m_fileEntry.qSize;
        } else if (parseLineLeft == QLatin1String("mtime")) {
            // 文件最后修改时间
            m_fileEntry.uLastModifiedTime = QDateTime::fromString(line.mid(11).trimmed(),
                                                                  QStringLiteral("yyyy-MM-dd hh:mm:ss")).toTime_t();

            QString name = m_fileEntry.strFullPath;
            // 获取第一层数据
            if (!name.contains(QDir::separator()) || (name.count(QDir::separator()) == 1 && name.endsWith(QDir::separator()))) {
                m_stArchiveData.listRootEntry.push_back(m_fileEntry);
            }

            // 存储总数据
            m_stArchiveData.mapFileEntry.insert(name, m_fileEntry);

            // clear m_fileEntry
            m_fileEntry.reset();
        }
        break;
    }

    return true;
}

bool CliRarPlugin::handleLine(const QString &line, WorkType workStatus)
{
    if (workStatus == WT_List) {
        return readListLine(line);   // 加载压缩文件，处理命令行内容
    } else if (workStatus == WT_Extract) {
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            if (percentage > 0) {
                if (line.contains("Extracting")) {
                    QStringRef strfilename = line.midRef(12, pos - 24);
                    QString fileName = strfilename.toString();
                    for (int i = fileName.length() - 1; i > 0; i--) {
                        if (fileName.at(i) == " ") {
                            continue;
                        } else {
                            fileName = fileName.left(i + 1);
                            break;
                        }
                    }

                    emit signalprogress(percentage);
                    emit signalCurFileName(fileName);
                }
            }
        }
    }

    return true;
}
