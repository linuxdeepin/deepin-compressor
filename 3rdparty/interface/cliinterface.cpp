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
#include "cliinterface.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QProcess>

CliInterface::CliInterface(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
    m_cliProps = new CliProperties(this, m_metaData, m_mimetype);
}

CliInterface::~CliInterface()
{

}

bool CliInterface::list()
{
    return true;
}

bool CliInterface::testArchive()
{
    return true;
}

bool CliInterface::extractFiles(const QVector<FileEntry> &files, const ExtractionOptions &options)
{
    return true;
}

bool CliInterface::addFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    bool ret = false;
    QFileInfo fi(m_strArchiveName);  // 压缩文件（全路径）
    QVector<FileEntry> tempfiles = files;
    QStringList fileList;
    // 获取待压缩的文件
    for (int i = 0; i < tempfiles.size(); i++) {
        fileList.append(tempfiles.at(i).strFullPath);
    }

    // 压缩命令的参数
    QStringList arguments = m_cliProps->addArgs(m_strArchiveName,
                                                fileList,
                                                options.strPassword,
                                                options.bHeaderEncryption,
                                                options.iCompressionLevel,
                                                options.strCompressionMethod,
                                                options.strEncryptionMethod,
                                                options.iVolumeSize,
                                                options.bTar_7z,
                                                fi.path());

    ret = runProcess(m_cliProps->property("addProgram").toString(), arguments);
    return ret;
}

bool CliInterface::moveFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    return true;
}

bool CliInterface::copyFiles(const QVector<FileEntry> &files, const CompressOptions &options)
{
    return true;
}

bool CliInterface::deleteFiles(const QVector<FileEntry> &files)
{
    return true;
}

bool CliInterface::addComment(const QString &comment)
{
    return true;
}

bool CliInterface::runProcess(const QString &programName, const QStringList &arguments)
{
    QString programPath = QStandardPaths::findExecutable(programName);
    if (programPath.isEmpty()) {
        return false;
    } else {
        QProcess p;
        p.execute(programName, arguments);
        p.waitForFinished();
        return true;
    }
}

void CliInterface::readStdout(bool handleAll)
{

}
