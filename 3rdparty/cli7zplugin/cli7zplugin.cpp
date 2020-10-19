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
    setupCliProperties();
}

Cli7zPlugin::~Cli7zPlugin()
{

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

    if (workStatus == WT_Add) {
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
                }
            }
        }
    }

    return true;
}
