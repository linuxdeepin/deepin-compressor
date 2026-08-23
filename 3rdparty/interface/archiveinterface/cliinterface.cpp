/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2009-2011 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 * Copyright (c) 2016 Vladyslav Batyrenko <mvlabat@gmail.com>
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

#include "cliinterface.h"
#include "queries.h"
#include "datamanager.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QUrl>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QTimer>
#include <QUuid>
#include <algorithm>

#include "common.h"
#include <linux/limits.h>

CliInterface::CliInterface(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
    //    m_bHandleCurEntry = true;
    setWaitForFinishedSignal(true);
    if (QMetaType::type("QProcess::ExitStatus") == 0) {
        qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    }

    m_cliProps = new CliProperties(this, m_metaData, m_mimetype);
}

CliInterface::~CliInterface()
{
    deleteProcess();
}

PluginFinishType CliInterface::list()
{
    setPassword(QString());
    DataManager::get_instance().resetArchiveData();
    m_setHasRootDirs.clear();
    m_setHasHandlesDirs.clear();

    m_workStatus = WT_List;
    //是否支持seek
    if(!m_common->isSupportSeek(m_strArchiveName)) {
        QTimer::singleShot(1000, this, [=]() {
            m_eErrorType = ET_FileSeekError;
            emit signalprogress(100);
            emit signalFinished(PFT_Error);
        });
        return PFT_Error;
    }

    bool ret = false;

    ret = runProcess(m_cliProps->property("listProgram").toString(), m_cliProps->listArgs(m_strArchiveName, DataManager::get_instance().archiveData().strPassword));

    return ret ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliInterface::testArchive()
{
    m_workStatus = WT_Add;

    return PFT_Nomral;
}

PluginFinishType CliInterface::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    //是否支持seek
    if(!m_common->isSupportSeek(m_strArchiveName)) {
        QTimer::singleShot(1000, this, [=]() {
            m_eErrorType = ET_FileSeekError;
            emit signalprogress(100);
            emit signalFinished(PFT_Error);
        });
        return PFT_Nomral;
    }
    bool bLnfs = m_common->isSubpathOfLnfs(options.strTargetPath);
    setProperty("lnfs", bLnfs);
    ArchiveData arcData = DataManager::get_instance().archiveData();
    m_files = files;
    m_extractOptions = options;

    if (!bLnfs) {
        if (arcData.listRootEntry.isEmpty()) {
            emit signalprogress(1);
            setProperty("list", "tmpList");
            list();
            setProperty("list", "");
            return PFT_Nomral;
        }
    }
    return extractFiles(files, options, bLnfs);
}

PluginFinishType CliInterface::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options, bool bLnfs)
{
    ArchiveData arcData = DataManager::get_instance().archiveData();
    setProperty("list", "");
    setPassword(QString());
    m_workStatus = WT_Extract;
    m_files = files;
    m_extractOptions = options;
    QString destPath;

    bool ret = false;
    m_rootNode.clear();
    m_mapLongName.clear();
    m_mapLongDirName.clear();
    m_mapRealDirValue.clear();
    QStringList fileList;
    foreach (FileEntry file, files) {
        QString fileName = file.strFullPath;
        // rar提取文件夹需要去掉尾部斜杠，否则会解出多层文件夹，7z有无斜杠无影响
        if (fileName.endsWith(QLatin1Char('/'))) {
            fileName.chop(1);
        }

        fileList << fileName;
    }

    // 设置解压目标路径
    destPath = options.strTargetPath;
    if (destPath.endsWith("/")) {
        destPath.chop(1);
        qInfo() << "解压目标路径 --- " << destPath;
    }
    bool bHandleLongName = false;
    QDir::setCurrent(destPath);
    if (!m_extractOptions.bAllExtract) {   // 提取部分文件
        m_files.clear();
        foreach (FileEntry entry, files) {
            if (m_rootNode.isEmpty()) {   // 获取待提取文件的节点
                if (entry.isDirectory) {
                    m_rootNode = entry.strFullPath.left(entry.strFullPath.length() - entry.strFileName.length() - 1);
                } else {
                    m_rootNode = entry.strFullPath.left(entry.strFullPath.length() - entry.strFileName.length());
                }
            }

            // 提取文件夹需要在map里面查找文件夹下的文件，将文件从临时文件夹移除需要用到m_files
            if (entry.isDirectory) {
                QList<FileEntry> listEntry;
                ArchiveData stArchiveData = DataManager::get_instance().archiveData();
                auto iter = stArchiveData.mapFileEntry.find(entry.strFullPath);
                for (; iter != stArchiveData.mapFileEntry.end();) {
                    if (!iter.key().startsWith(entry.strFullPath)) {
                        break;
                    } else {
                        listEntry << iter.value();
                        ++iter;
                    }
                }

                m_files << listEntry;
            } else {
                m_files << entry;
            }
        }
        //长文件解压
        if (!bLnfs) {
            for (FileEntry entry : m_files) {
                if (NAME_MAX < entry.strFileName.toLocal8Bit().length() || NAME_MAX < entry.strFullPath.toLocal8Bit().length()) {
                    bHandleLongName = true;
                    break;
                }
            }
        }

        if (bHandleLongName && !checkMoveCapability()) {
            qWarning() << "Long filename detected, but moveProgram (" << m_cliProps->property("moveProgram").toString() << ") is not available.";
            qWarning() << "Archive format:" << m_mimetype.name() << "Skipping long name handling.";
            qWarning() << "The extraction tool will report errors for files with names exceeding system limit (255 bytes).";
            bHandleLongName = false;
        }

        if (destPath.startsWith("/tmp") && destPath.contains("/deepin-compressor-")) {   // 打开解压列表文件
            if (!QDir(destPath).exists()) {
                QDir(destPath).mkpath(destPath);
            }
        } else if (!bHandleLongName) {   // 判断不是打开解压列表文件的临时目录，设置提取的临时目录
            // 设置临时目录
            m_extractTempDir.reset(new QTemporaryDir(QStringLiteral(".%1-").arg(QCoreApplication::applicationName())));
            if (!m_extractTempDir->isValid()) {
                qInfo() << "Creation of temporary directory failed.";
                emit signalFinished(PFT_Error);
                return PFT_Error;
            }

            destPath = m_extractTempDir->path();
            qInfo() << "extract temp path--- " << destPath;
        }
        if (bHandleLongName) {
            // 部分解压时补全长名目录条目（files 中可能不含目录本身）
            QList<FileEntry> filesWithDirs = collectLongDirEntries(m_files);
            if (!handleLongNameExtract(filesWithDirs)) {
                qWarning() << "extractFiles(partial): handleLongNameExtract failed, errorType:" << m_eErrorType;
                if (m_eErrorType == ET_NoError) {
                    m_eErrorType = ET_FileWriteError;
                }
                emit signalFinished(PFT_Error);
                return PFT_Error;
            }
        }
    } else {
        if (!QDir(destPath).exists() && !QDir(destPath).mkpath(destPath)) {
            if (isInsufficientDiskSpace(destPath, FILE_MAX_SIZE)) {   // 暂取小于10M作为磁盘空间不足的判断标准
                m_eErrorType = ET_InsufficientDiskSpace;
            } else {
                emit signalFileWriteErrorName("destPath");
                m_eErrorType = ET_FileWriteError;
            }
            emit signalFinished(PFT_Error);
            return PFT_Error;
        }
        //ADD
        QString password;
        if (options.password.isEmpty()) {
            // 对列表加密文件进行追加解压的时候使用压缩包的密码
            password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : QString();
        } else {
            password = options.password;
        }

        if (!bLnfs) {
            for (QMap<QString, FileEntry>::const_iterator iter = arcData.mapFileEntry.begin(); iter != arcData.mapFileEntry.end(); iter++) {
                if (NAME_MAX < iter.value().strFileName.toLocal8Bit().length()) {
                    bHandleLongName = true;
                    break;
                }
            }
        }

        if (bHandleLongName && !checkMoveCapability()) {
            qWarning() << "Long filename detected, but moveProgram (" << m_cliProps->property("moveProgram").toString() << ") is not available.";
            qWarning() << "Archive format:" << m_mimetype.name() << "Skipping long name handling.";
            qWarning() << "The extraction tool will report errors for files with names exceeding system limit (255 bytes).";
            bHandleLongName = false;
        }

        if (bHandleLongName) {
            if (!handleLongNameExtract(arcData.mapFileEntry.values())) {
                qWarning() << "extractFiles(all): handleLongNameExtract failed, errorType:" << m_eErrorType;
                if (m_eErrorType == ET_NoError) {
                    m_eErrorType = ET_FileWriteError;
                }
                emit signalFinished(PFT_Error);
                return PFT_Error;
            }
        }
    }
    if (bHandleLongName) {
        if (m_longNamePhase != LNE_None) {
            // 异步长文件名解压已启动, 返回 PFT_Nomral 表示"操作进行中"非"已完成";
            // 最终结果由 onLongNameProcessFinished 通过 signalFinished 发出
            return PFT_Nomral;
        }
        m_eErrorType = ET_LongNameError;
        return list();
    }

    QDir::setCurrent(destPath);

    QString password;
    if (options.password.isEmpty()) {
        // 对列表加密文件进行追加解压的时候使用压缩包的密码
        password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : QString();
    } else {
        password = options.password;
    }

    ret = runProcess(m_cliProps->property("extractProgram").toString(),
                     m_cliProps->extractArgs(m_strArchiveName, fileList, true, password));

    return ret ? PFT_Nomral : PFT_Error;
}

void CliInterface::pauseOperation()
{
    if (!m_childProcessId.empty()) {
        for (int i = m_childProcessId.size() - 1; i >= 0; i--) {
            if (m_childProcessId[i] > 0) {
                kill(static_cast<__pid_t>(m_childProcessId[i]), SIGSTOP);
            }
        }
    }

    if (m_processId > 0) {
        kill(static_cast<__pid_t>(m_processId), SIGSTOP);
    }
    m_bPause = true;
}

void CliInterface::continueOperation()
{
    if (!m_childProcessId.empty()) {
        for (int i = m_childProcessId.size() - 1; i >= 0; i--) {
            if (m_childProcessId[i] > 0) {
                kill(static_cast<__pid_t>(m_childProcessId[i]), SIGCONT);
            }
        }
    }

    if (m_processId > 0) {
        kill(static_cast<__pid_t>(m_processId), SIGCONT);
    }

    m_bPause = false;
}

bool CliInterface::doKill()
{
    if (m_process) {
        killProcess(false);
        return true;
    }

    return false;
}

PluginFinishType CliInterface::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    //是否支持seek
    if(!files.isEmpty() && !m_common->isSupportSeek(m_strArchiveName)) {
        QTimer::singleShot(1000, this, [=]() {
            m_eErrorType = ET_FileSeekError;
            emit signalprogress(100);
            emit signalFinished(PFT_Error);
        });
        return PFT_Nomral;
    }
    setPassword(QString());
    m_workStatus = WT_Add;
    m_files = files;
    m_compressOptions = options;

    m_isTar7z = false;

    // 计算并缓存总文件大小，避免在handleProgress中重复计算
    m_totalFileSize = 0;
    for (const auto &file : m_files) {
        m_totalFileSize += file.qSize;
    }

    bool ret = false;
    QStringList fileList;

    // 压缩目标路径
    const QString destinationPath = (options.strDestination == QString()) ? QString() : options.strDestination;
    qInfo() << "Adding" << files.count() << "file(s) to destination:" << destinationPath;
    bool isLink = true;

    if (!destinationPath.isEmpty()) {   // 向压缩包非第一层文件里面追加压缩
        m_extractTempDir.reset(new QTemporaryDir());
        // 临时路径
        const QString absoluteDestinationPath = m_extractTempDir->path() + QLatin1Char('/') + destinationPath;

        QDir qDir;
        qDir.mkpath(absoluteDestinationPath);   // 创建临时路径，存放待压缩文件

        for (FileEntry file : files) {
            // 待压缩文件的实际全路径
            const QString filePath = file.strFullPath;
            // 待压缩文件的文件名(临时路径全路径)
            const QString newFilePath = absoluteDestinationPath + QFileInfo(file.strFullPath).fileName();

            isLink = (isLink && QFileInfo(filePath).isSymLink());
            // 在临时路径创建待压缩文件的链接
            if (QFile::link(filePath, newFilePath)) {
                qInfo() << "Symlink's created:" << filePath << newFilePath;
            } else {   // 创建链接失败
                qInfo() << "Can't create symlink" << filePath << newFilePath;
                emit signalFinished(PFT_Error);
                return PFT_Error;
            }
        }

        qInfo() << "Changing working dir again to " << m_extractTempDir->path();
        QDir::setCurrent(m_extractTempDir->path());

        // 添加临时路径中的第一层文件（夹）
        fileList.append(destinationPath.split(QLatin1Char('/'), QString::SkipEmptyParts).at(0));
    } else {   // 压缩、向压缩包第一层文件追加压缩
        QList<FileEntry> tempfiles = files;
        // 获取待压缩的文件
        for (int i = 0; i < tempfiles.size(); i++) {
            fileList.append(tempfiles.at(i).strFullPath);
        }
    }

    // 对列表加密文件进行追加压缩的时候使用压缩包的密码
    QString password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : options.strPassword;
    //进程方式重命名，建立软连接
    QVariantMap mapdata;
    QList<QTemporaryDir *> lstTmpDir;
    QStringList sRenameList;
    for (FileEntry file : files) {
        bool isAlias = !(file.strAlias.isEmpty() || file.strAlias.isNull());
        if (isAlias) {
            QTemporaryDir *tmpdir = new QTemporaryDir();
            tmpdir->setAutoRemove(true);
            const QString linkpath = tmpdir->path() + QDir::separator() + file.strAlias;

            if (!options.bTar_7z) {
                if (QFile::link(file.strFullPath, linkpath)) {
                    qInfo() << "process Symlink's created:" << file.strFullPath << linkpath;
                    mapdata.insert(file.strFullPath, linkpath);
                } else {   // 创建链接失败
                    qInfo() << "process Can't create symlink" << file.strFullPath << linkpath;
                    emit signalFinished(PFT_Error);
                    return PFT_Error;
                }
            } else {
                QString sRename = "--transform='flags=r;s|%1|%2|'";
                sRename = sRename.arg(file.strFileName).arg(file.strAlias);
                sRenameList.append(sRename);
            }
            lstTmpDir.append(tmpdir);
        }
    }
    //替换重命名文件
    if (!mapdata.isEmpty()) {
        for (QString sfileName : fileList) {
            if (mapdata.contains(sfileName)) {
                fileList.replace(fileList.indexOf(sfileName), mapdata.value(sfileName).toString());
            }
        }
    }

    //判断是否是在以mtp方式挂载的目录中进行压缩.建立临时文件
    QString temp_archiveName("");
    QScopedPointer<QTemporaryDir> temp_dir;
    if (IsMtpFileOrDirectory(m_strArchiveName)) {
        temp_dir.reset(new QTemporaryDir);
        temp_dir->setAutoRemove(true);
        qInfo() << "mtp 挂载压缩，建立临时文件夹：" << temp_dir->path();
        temp_archiveName = temp_dir->path() + QDir::separator() + QFileInfo(m_strArchiveName).fileName();
        //如果文件已经存在了，则为追加操作，move过去
        if (QFileInfo(m_strArchiveName).exists()) {
            QStringList args_list;
            args_list << m_strArchiveName << temp_archiveName;
            QProcess mover;
            ret = 0 == mover.execute("mv", args_list);
            mover.waitForFinished();
            ret = mover.exitCode() == QProcess::NormalExit;
            if (!ret) {
                qInfo() << "建立临时文件失败!";
                return PFT_Error;
            }
        }
    }
    // 压缩命令的参数,在mtp中进行压缩的时候，先放在临时文件 temp_archiveName 中，最后再mv过去
    QStringList arguments = m_cliProps->addArgs(temp_archiveName.isEmpty() ? m_strArchiveName : temp_archiveName,
                                                fileList,
                                                password,
                                                options.bHeaderEncryption,
                                                options.iCompressionLevel,
                                                options.strCompressionMethod,
                                                options.strEncryptionMethod,
                                                options.iVolumeSize,
                                                options.bTar_7z,
                                                QFileInfo(m_strArchiveName).path(),
                                                sRenameList);

    if (options.bTar_7z) {   // 压缩tar.7z文件
        m_isTar7z = true;
        m_filesSize = options.qTotalSize;   // 待压缩文件总大小
        m_scriptPath = QDir::tempPath() + "/tempScript_" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) + ".sh";
        QFile scriptFile(m_scriptPath);
        if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&scriptFile);
            out << "#!/bin/bash\n";
            for (const QString &arg : arguments) {
                out << arg << "\n";
            }
            scriptFile.close();
            QProcess::execute("chmod", { "+x", m_scriptPath });
            ret = runProcess(m_scriptPath, QStringList());
        } else {
            qWarning() << "Failed to create temporary script file.";
            ret = false;
        }
    } else {
        QString processName = m_cliProps->property("addProgram").toString();
        if(processName == "7z" && !destinationPath.isEmpty()) {
            if(!isLink) {
                for(int i = 0; i < arguments.count(); i++) {
                    if(arguments.at(i) == QStringLiteral("-snl")) {
                        arguments.replace(i, QStringLiteral("-l"));
                        break;
                    }
                }
            }
        }
        ret = runProcess(processName, arguments);
    }

    if (ret && !temp_archiveName.isEmpty()) {
        qInfo() << "mtp 压缩完成,现在开始移动";
        QStringList args_list;
        args_list << temp_archiveName << m_strArchiveName;
        m_process->waitForFinished();
        QProcess mover;
        ret = 0 == mover.execute("mv", args_list);
        ret = mover.exitCode() == QProcess::NormalExit;
        qInfo() << "mtp 移动成功? " << ret;
    }
    //删除临时目录
    for (QTemporaryDir *tmdDir : lstTmpDir) {
        delete tmdDir;
    }

    return ret ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliInterface::moveFiles(const QList<FileEntry> & /*files*/, const CompressOptions & /*options*/)
{
    //    m_workStatus = WT_Add;

    return PFT_Nomral;
}

PluginFinishType CliInterface::copyFiles(const QList<FileEntry> & /*files*/, const CompressOptions & /*options*/)
{
    //    m_workStatus = WT_Add;

    return PFT_Nomral;
}

PluginFinishType CliInterface::deleteFiles(const QList<FileEntry> &files)
{
    setPassword(QString());
    m_workStatus = WT_Delete;
    m_files = files;

    bool ret = false;

    // 对列表加密文件进行追加删除的时候使用压缩包的密码
    QString password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : QString();
    ret = runProcess(m_cliProps->property("deleteProgram").toString(),
                     m_cliProps->deleteArgs(m_strArchiveName, files, password));

    return ret ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliInterface::renameFiles(const QList<FileEntry> &files)
{
    setPassword(QString());
    m_workStatus = WT_Delete;
    m_files = files;

    bool ret = false;

    // 对列表加密文件进行追加删除的时候使用压缩包的密码
    QString password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : QString();
    ret = runProcess(m_cliProps->property("moveProgram").toString(),
                     m_cliProps->moveArgs(m_strArchiveName, files, DataManager::get_instance().archiveData(), password));

    return ret ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliInterface::addComment(const QString & /*comment*/)
{
    //    m_workStatus = WT_Add;

    return PFT_Nomral;
}

PluginFinishType CliInterface::updateArchiveData(const UpdateOptions &options)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();

    m_rootEntry = QString();
    foreach (FileEntry entry, options.listEntry) {
        if (options.eType == UpdateOptions::Delete) {   // 删除
            if (entry.isDirectory) {   // 删除文件夹
                // 在map中查找该文件夹下的文件并删除
                QMap<QString, FileEntry>::iterator itor = stArchiveData.mapFileEntry.begin();
                while (itor != stArchiveData.mapFileEntry.end()) {
                    if (itor->strFullPath.startsWith(entry.strFullPath)) {
                        if (!itor->isDirectory) {
                            stArchiveData.qSize -= itor->qSize;   // 更新压缩包内文件原始总大小
                        }
                        itor = stArchiveData.mapFileEntry.erase(itor);
                    } else {
                        ++itor;
                    }
                }

                // 文件夹是第一层的数据
                if (entry.strFullPath.endsWith(QLatin1Char('/')) && entry.strFullPath.count(QLatin1Char('/')) == 1) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) {   // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            } else {   // 删除文件
                stArchiveData.qSize -= entry.qSize;   // 更新压缩包内文件原始总大小
                stArchiveData.mapFileEntry.remove(entry.strFullPath);   //在map中删除该文件
                // 文件是第一层的数据
                if (!entry.strFullPath.contains(QLatin1Char('/'))) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) {   // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            }
        } else if (options.eType == UpdateOptions::Rename) {   // 重命名，更新压缩包数据
            QMap<QString, FileEntry> tmpMapFileEntry;
            QString strAlias;
            if (entry.isDirectory) {   // 重命名文件夹
                // 在map中查找该文件夹下的文件并重命名
                QMap<QString, FileEntry>::iterator itor = stArchiveData.mapFileEntry.begin();
                while (itor != stArchiveData.mapFileEntry.end()) {
                    if (itor->strFullPath.startsWith(entry.strFullPath)) {
                        QString strPath = QFileInfo(entry.strFullPath.left(entry.strFullPath.length() - 1)).path();
                        if (strPath == ".") {
                            strAlias = entry.strAlias + QDir::separator();
                        } else {
                            strAlias = strPath + QDir::separator() + entry.strAlias + QDir::separator();
                        }
                        strAlias = strAlias + QString(itor->strFullPath).right(QString(itor->strFullPath).length() - entry.strFullPath.length());
                        FileEntry tmpEntry = itor.value();
                        tmpEntry.strFullPath = strAlias;
                        if (tmpEntry.isDirectory) {
                            tmpEntry.strFileName = QFileInfo(strAlias.left(strAlias.length() - 1)).fileName();
                        } else {
                            tmpEntry.strFileName = QFileInfo(strAlias).fileName();
                        }
                        tmpEntry.strAlias.clear();
                        tmpMapFileEntry.insert(strAlias, tmpEntry);
                        itor = stArchiveData.mapFileEntry.erase(itor);
                    } else {
                        ++itor;
                    }
                }
                if (!tmpMapFileEntry.isEmpty()) {
                    for (QString strFullPath : tmpMapFileEntry.keys()) {
                        stArchiveData.mapFileEntry.insert(strFullPath, tmpMapFileEntry.value(strFullPath));
                    }
                }
                // 更新文件夹第一层的数据
                if (entry.strFullPath.endsWith(QLatin1Char('/')) && entry.strFullPath.count(QLatin1Char('/')) == 1) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) {   // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            strAlias = entry.strAlias + QDir::separator();
                            entry.strFullPath = strAlias;
                            entry.strFileName = strAlias;
                            stArchiveData.listRootEntry.append(entry);
                            break;
                        }
                    }
                }
            } else {   // 重命名文件
                stArchiveData.mapFileEntry.remove(entry.strFullPath);   //在map中重命名该文件
                QString strPath = QFileInfo(entry.strFullPath).path();
                if (strPath == "." || strPath.isEmpty() || strPath.isNull()) {
                    strAlias = entry.strAlias;
                } else {
                    strAlias = strPath + QDir::separator() + entry.strAlias;
                }
                FileEntry tmpEntry = entry;
                tmpEntry.strFullPath = strAlias;
                stArchiveData.mapFileEntry.insert(strAlias, tmpEntry);
                // 更新文件夹第一层的数据
                if (!entry.strFullPath.contains(QLatin1Char('/'))) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) {   // 在第一次层数据中找到entry重命名
                            stArchiveData.listRootEntry.removeAt(i);
                            strAlias = entry.strAlias;
                            entry.strFullPath = strAlias;
                            entry.strFileName = strAlias;
                            stArchiveData.listRootEntry.append(entry);
                            break;
                        }
                    }
                }
            }
        } else if (options.eType == UpdateOptions::Add) {   // 追加压缩
            QString destinationPath = options.strParentPath;   // 追加目标路径
            QFileInfo file(entry.strFullPath);

            if (m_rootEntry == "") {   // 获取所有追加文件的父目录
                m_rootEntry = file.filePath().left(file.filePath().size() - file.fileName().size());
            }

            entry.strFullPath = destinationPath + entry.strFullPath.remove(m_rootEntry);   // entry在压缩包中的全路径
            if (file.isDir()) {   // 文件夹
                entry.strFullPath = entry.strFullPath + QDir::separator();   // 手动添加'/'
                //entry.qSize = QDir(entry.strFullPath).entryInfoList().count(); // 获取文件夹大小为遍历文件夹获取文件夹下子文件的数目
            } else {
                //                entry.qSize = file.size(); // 文件大小
                // 更新压缩包内文件原始总大小
                stArchiveData.qSize -= stArchiveData.mapFileEntry.value(entry.strFullPath).qSize;
                stArchiveData.qSize += entry.qSize;
            }

            // 判断是否追加到第一层数据
            if (destinationPath == "" && ((entry.strFullPath.count('/') == 1 && entry.strFullPath.endsWith('/')) || entry.strFullPath.count('/') == 0)) {
                for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                    if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) {   // 在第一层数据中找到entry，不添加数据
                        stArchiveData.listRootEntry.removeAt(i);
                        break;
                    }
                }

                // 在第一层数据中没有找到entry，在第一层数据中添加entry
                stArchiveData.listRootEntry.push_back(entry);
            }

            stArchiveData.mapFileEntry.insert(entry.strFullPath, entry);   // 在map中插入数据
        }
    }

    stArchiveData.qComressSize = QFileInfo(m_strArchiveName).size();   // 更新压缩包大小

    return PFT_Nomral;
}

void CliInterface::setListEmptyLines(bool emptyLines)
{
    m_listEmptyLines = emptyLines;
}

bool CliInterface::runProcess(const QString &programName, const QStringList &arguments)
{
    Q_ASSERT(!m_process);

    QString programPath = QStandardPaths::findExecutable(programName);
    if (programPath.isEmpty()) {
        return false;
    }

    m_process = new KPtyProcess;
    m_process->setPtyChannels(KPtyProcess::StdinChannel);
    m_process->setOutputChannelMode(KProcess::MergedChannels);
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    m_process->setProgram(programPath, arguments);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=] {
        readStdout();
    });

    if (m_workStatus == WT_Extract) {
        // Extraction jobs need a dedicated post-processing function.
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(extractProcessFinished(int, QProcess::ExitStatus)));
    } else if (property("list").toString() != "tmpList") {
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    } else {
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                [=](int exitCode, QProcess::ExitStatus) {
                    if (m_process) {
                        if (exitCode != 0) {
                            emit signalprogress(100);
                            emit signalFinished(PFT_Error);
                            return;
                        }
                        deleteProcess();
                        PluginFinishType ret = extractFiles(m_files, m_extractOptions, property("lnfs").toBool());
                        if (ret == PFT_Error) {
                            emit signalprogress(100);
                            emit signalFinished(PFT_Error);
                        }
                    }
                });
    }

    m_stdOutData.clear();
    m_isProcessKilled = false;
    m_process->start();

    if (m_process->waitForStarted()) {
        m_childProcessId.clear();
        m_processId = m_process->processId();

        if (m_isTar7z) {
            getChildProcessId(m_processId, QStringList() << "tar"
                                                         << "7z",
                              m_childProcessId);
        } else if (m_process->program().at(0).contains("7z")) {   // 普通7z获取子进程（RPM环境会出现多个7z进
            getChildProcessId(m_processId, QStringList() << "7z", m_childProcessId);
        }

        return true;
    }

    return true;
}

void CliInterface::deleteProcess()
{
    if (m_process) {
        readStdout(true);
        m_process->blockSignals(true);   // delete m_process之前需要断开所有m_process信号，防止重复处理
        delete m_process;
        m_process = nullptr;
        if(!m_scriptPath.isEmpty()) {
            QFile::remove(m_scriptPath);
        }
    }
}

void CliInterface::handleProgress(const QString &line)
{
    // 长文件名 rename 阶段（7z rn）不向 UI 发送真实进度。
    // 原因：ZipCrypto 只加密文件内容不加密文件名，7z rn 无需密码即可运行并输出进度，
    // 会导致进度条在密码框弹出前先跳一大截；而随后的 extract 阶段从 0 重新计数，
    // 进度值被 ProgressPage 的单调递增逻辑丢弃，剩余时间卡死。
    // 解决：rename 阶段屏蔽进度，真实进度仅在 extract 阶段（LNE_Extract）上报。
    if (m_longNamePhase == LNE_Rename || m_longNamePhase == LNE_RenameDirs) {
        return;
    }

    if (m_process && m_process->program().at(0).contains("7z")) {   // 解析7z相关进度、文件名
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            if (percentage > 0) {
                if (line.contains("\b\b\b\b") == true) {
                    QString strfilename;
                    if (m_workStatus == WT_Extract) {   // 解压解析文件名
                        int count = line.indexOf("-");   // 解压参数解析

                        if (count > 0) {
                            strfilename = line.midRef(count + 2).toString();   // 文件名
                            // 右键 解压到当前文件夹
                            if (!m_extractOptions.bExistList && m_indexOfListRootEntry == 0) {
                                m_indexOfListRootEntry++;
                                FileEntry entry;
                                entry.strFullPath = strfilename;
                                DataManager::get_instance().archiveData().listRootEntry << entry;
                            }
                        }
                    } else if (m_workStatus == WT_Add) {   // 压缩：根据进度百分比计算当前文件
                        // p7zip在压缩时只显示最后一个文件名，需要根据进度推断当前文件
                        if (!m_files.isEmpty()) {
                            // 如果总大小为0，使用文件数量平均分配
                            if (m_totalFileSize == 0) {
                                int fileIndex = (percentage * m_files.size()) / 100;
                                if (fileIndex >= m_files.size()) {
                                    fileIndex = m_files.size() - 1;
                                }
                                strfilename = m_files[fileIndex].strFileName;
                                if (strfilename.isEmpty()) {
                                    strfilename = QFileInfo(m_files[fileIndex].strFullPath).fileName();
                                }
                            } else {
                                // 根据进度计算已压缩的大小
                                qint64 compressedSize = (percentage * m_totalFileSize) / 100;

                                // 找到当前正在压缩的文件
                                qint64 accumulatedSize = 0;
                                int fileIndex = 0;
                                for (int i = 0; i < m_files.size(); ++i) {
                                    accumulatedSize += m_files[i].qSize;
                                    if (accumulatedSize > compressedSize) {
                                        fileIndex = i;
                                        break;
                                    }
                                    fileIndex = i; // 防止最后一个文件时索引越界
                                }

                                // 确保索引在有效范围内
                                if (fileIndex >= m_files.size()) {
                                    fileIndex = m_files.size() - 1;
                                }

                                strfilename = m_files[fileIndex].strFileName;
                                if (strfilename.isEmpty()) {
                                    strfilename = QFileInfo(m_files[fileIndex].strFullPath).fileName();
                                }
                            }
                        }
                    } else {   // 删除、重命名等操作解析文件名
                        if (line.contains("% = ")) {
                            strfilename = line.right(line.length() - line.indexOf(QLatin1Char('=')) - 2);
                        } else if (line.contains("% R ")) {
                            strfilename = line.right(line.length() - line.indexOf(QLatin1Char('R')) - 2);
                        } else if (line.contains("U ")) {   // 重命名操作
                            int count = line.indexOf("U ");
                            if (count > 0) {
                                strfilename = line.midRef(count + 2).toString();
                            }
                        }
                    }

                    emit signalprogress(percentage);
                    if (!strfilename.isEmpty()) {
                        emit signalCurFileName(strfilename);
                    }
                }
            } else {
                // percentage == 0：固实压缩（solid）的小包解压时，7z 的进度百分比始终为 0%，
                // 因为固实块（solid block）数据量小，解压瞬间完成，进度停留在 0% 直到文件全部写出。
                // 此时根据 7z 输出的文件索引估算进度，避免进度条一直停留在"计算中"。
                // 7z 输出格式：  0% 1233 - filename
                if (line.contains("\b\b\b\b") && m_workStatus == WT_Extract) {
                    int sepPos = line.indexOf("-");   // 查找文件名分隔符
                    if (sepPos > 0) {
                        QString strfilename = line.mid(sepPos + 2);   // 文件名
                        // 在 '-' 之前反向查找最近的 '%'，用于定位文件索引
                        // （一行可能包含多个进度更新，第一个 % 是初始 0%，需要找到与当前文件名关联的 %）
                        int percentPos = line.lastIndexOf(QLatin1Char('%'), sepPos);
                        if (percentPos > 0) {
                            QString numPart = line.mid(percentPos + 1, sepPos - percentPos - 1).trimmed();
                            bool ok = false;
                            int fileIndex = numPart.toInt(&ok);
                            if (ok && fileIndex > 0) {
                                int totalFiles = DataManager::get_instance().archiveData().mapFileEntry.count();
                                if (totalFiles > 0) {
                                    int estimatedPercent = (fileIndex * 100) / totalFiles;
                                    if (estimatedPercent > 0 && estimatedPercent < 100) {
                                        emit signalprogress(estimatedPercent);
                                    }
                                }
                            }
                        }
                        if (!strfilename.isEmpty()) {
                            emit signalCurFileName(strfilename);
                        }
                        // 右键解压到当前文件夹
                        if (!m_extractOptions.bExistList && m_indexOfListRootEntry == 0) {
                            m_indexOfListRootEntry++;
                            FileEntry entry;
                            entry.strFullPath = strfilename;
                            DataManager::get_instance().archiveData().listRootEntry << entry;
                        }
                    }
                }

                // 7z解压小文件无法获取文件名添加一个空的entry
                if (m_workStatus == WT_Extract && !m_extractOptions.bExistList && m_indexOfListRootEntry == 0 && m_isEmptyArchive == false) {
                    m_indexOfListRootEntry++;
                    FileEntry entry;
                    entry.strFullPath = QString();
                    DataManager::get_instance().archiveData().listRootEntry << entry;
                }
            }
        }
    } else if (m_process && m_process->program().at(0).contains("unrar")) {   // 解析rar相关进度、文件名
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            emit signalprogress(percentage);
        }

        QStringRef strfilename;
        QString fileName;
        if (line.startsWith("Extracting")) {   // 普通文件
            strfilename = line.midRef(12, pos - 24);
            fileName = strfilename.toString();
        } else if (line.startsWith("Creating")) {   // 文件夹
            strfilename = line.midRef(10, pos - 22);
            fileName = strfilename.toString();
        }

        if (!fileName.isEmpty()) {
            for (int i = fileName.length() - 1; i > 0; i--) {
                if (fileName.at(i) == " ") {
                    continue;
                } else {
                    fileName = fileName.left(i + 1);
                    break;
                }
            }

            // 右键 解压到当前文件夹（因为快捷解压少了list步骤，因此需要在解压过程中存储首层文件数据，防止误报压缩包无数据）
            if (!m_extractOptions.bExistList && m_indexOfListRootEntry == 0) {
                m_indexOfListRootEntry++;
                FileEntry entry;
                if (fileName.count('/') == 0) {   // 压缩包内第一层的文件
                    entry.strFullPath = fileName;
                } else {   // 压缩包内第一层的文件夹
                    QString name = fileName.left(fileName.indexOf(QLatin1Char('/')));
                    entry.strFullPath = name;
                }

                DataManager::get_instance().archiveData().listRootEntry << entry;
            }

            emit signalCurFileName(fileName);
        }
    } else if (m_process && m_process->program().at(0).contains("tempScript")) {
        // 处理tar.7z进度
        // "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b  7M + [Content]"
        // "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b274M 1 + [Content]"
        int pos = line.lastIndexOf(" + [Content]");
        if (pos > 1) {
            int mPos = line.lastIndexOf("M ");
            int bPos = line.lastIndexOf("\b", mPos);
            QString tempLine = line.left(mPos);
            // 已经压缩的文件大小
            qint64 compressedSize = tempLine.right(tempLine.size() - bPos - 1).toLongLong();
            // 计算文件大小计算百分比
            qint64 percentage = compressedSize * 1024 * 1024 * 100 / m_filesSize;

            emit signalprogress(percentage);
            // tar.7z 无法获取正在压缩的单个文件名，但可以提示正在打包成 tar 文件
            // 发送 tar 文件名提示（去掉 .7z 后缀，显示正在打包的 tar 文件名）
            QString tarFileName = QFileInfo(m_strArchiveName).fileName();
            if (tarFileName.endsWith(".7z", Qt::CaseInsensitive)) {
                tarFileName = tarFileName.left(tarFileName.length() - 3);
            }
            emit signalCurFileName(tarFileName);
        }
    }
}

PluginFinishType CliInterface::handlePassword()
{
    // 加密分卷缺失时 7z 已置 ET_MissingVolume，handlePassword 入口不得
    // 无条件清零，否则后续 CRC 失败行会误判为「密码错误」（PMS BUG-373669）
    if (m_eErrorType != ET_MissingVolume) {
        m_eErrorType = ET_NoError;
    }

    QString name;
    if (m_process && m_process->program().at(0).contains("unrar")) {   // rar解压会提示加密的文件名
        name = m_strEncryptedFileName;
    } else {   // 7z不会提示加密的文件名
        if (m_files.count() == 1 && m_workStatus != WT_Add) {   // 选则压缩包中的一个文件进行提取操作
            name = m_files.at(0).strFileName;
        } else {   // 解压或是选择压缩包内多个文件进行提取操作
            name = m_strArchiveName;
        }
    }

    PasswordNeededQuery query(name, m_bWrongPasswordRetry);
    m_bWrongPasswordRetry = false;
    emit signalQuery(&query);
    query.waitForResponse();

    if (query.responseCancelled()) {
        DataManager::get_instance().archiveData().strPassword = QString();
        setPassword(QString());   // 函数暂时保留
        return PFT_Cancel;
    }

    DataManager::get_instance().archiveData().strPassword = query.password();
    setPassword(query.password());
    writeToProcess((query.password() + QLatin1Char('\n')).toLocal8Bit());

    return PFT_Nomral;
}

bool CliInterface::handleFileExists(const QString &line)
{
    if (isFileExistsFileName(line)) {   // 提示已存在的文件名，开始解析
        const QStringList fileExistsFileNameRegExp = m_cliProps->property("fileExistsFileNameRegExp").toStringList();
        for (const QString &pattern : fileExistsFileNameRegExp) {
            const QRegularExpression rxFileNamePattern(pattern);
            const QRegularExpressionMatch rxMatch = rxFileNamePattern.match(line);

            if (rxMatch.hasMatch()) {
                m_parseName = rxMatch.captured(1);
            }
        }
    }

    if (isFileExistsMsg(line)) {   // 提示是否替换已存在的文件
        const QStringList choices = m_cliProps->property("fileExistsInput").toStringList();   // 提示选项
        QString response;   // 选择结果

        OverwriteQuery query(m_parseName);
        emit signalQuery(&query);
        query.waitForResponse();

        if (query.responseCancelled()) {   // 取消
            // (Q)uit
            response = choices.at(4);
            emit signalCancel();
            m_eErrorType = ET_UserCancelOpertion;
            emit signalFinished(PFT_Cancel);
        } else if (query.responseSkip()) {   // 跳过
            // (N)o
            response = choices.at(1);
            m_eErrorType = ET_NoError;
        } else if (query.responseSkipAll()) {   // 全部跳过
            // (S)kip all  |  n[E]ver
            response = choices.at(3);
            m_eErrorType = ET_NoError;
        } else if (query.responseOverwrite()) {   // 替换
            // (Y)es
            response = choices.at(0);
        } else if (query.responseOverwriteAll()) {   // 全部替换
            // (A)lways  |  [A]ll
            response = choices.at(2);
        }

        Q_ASSERT(!response.isEmpty());

        response += QLatin1Char('\n');
        writeToProcess(response.toLocal8Bit());
    } else {
        return false;
    }

    return true;
}

PluginFinishType CliInterface::handleCorrupt()
{
    LoadCorruptQuery query(m_strArchiveName);
    emit signalQuery(&query);
    query.waitForResponse();

    /**
      * 对于一些损坏的包，没有致命错误的，可以选择以只读方式打开或提示打开失败
      * 例如：zip分卷缺失也是可以打开的
      */
    if (!query.responseYes()) {
        return PFT_Error;
    } else {
        return PFT_Nomral;
    }
}

void CliInterface::writeToProcess(const QByteArray &data)
{
    Q_ASSERT(m_process);
    Q_ASSERT(!data.isNull());

    //    m_process->write(data);
    m_process->pty()->write(data);
}

bool CliInterface::moveExtractTempFilesToDest(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    QDir finalDestDir(options.strTargetPath);   // 提取目标路径
    bool overwriteAll = false;   // 全部替换
    bool skipAll = false;   // 全部跳过
    bool moveSuccess = true;

    // 循环待提取文件
    foreach (const FileEntry entry, files) {
        QString path = entry.strFullPath;
        // 已经解压出的文件
        QFileInfo extractEntry(path.remove(0, m_rootNode.length()));
        // 已经解压到临时文件夹的文件(全路径)
        QFileInfo etractEntryTemp(QDir::current().absolutePath() + QLatin1Char('/') + entry.strFullPath);
        // 最终会提取出的文件
        QFileInfo extractEntryDest(finalDestDir.path() + QLatin1Char('/') + extractEntry.filePath());

        if (etractEntryTemp.isDir()) {
            // 提取文件夹，创建目标文件夹路径
            if (!finalDestDir.mkpath(extractEntry.filePath())) {
                qInfo() << "Failed to create directory" << extractEntry.filePath() << "in final destination.";
            }
        } else {
            // 目标路径下文件已经存在
            if (extractEntryDest.exists()) {
                qInfo() << "File" << extractEntryDest.absoluteFilePath() << "exists.";
                if (!skipAll && !overwriteAll) {
                    OverwriteQuery query(extractEntryDest.absoluteFilePath());
                    emit signalQuery(&query);
                    query.waitForResponse();

                    if (query.responseOverwrite() || query.responseOverwriteAll()) {
                        if (query.responseOverwriteAll()) {   // 全部替换
                            overwriteAll = true;
                        }

                        if (!QFile::remove(extractEntryDest.absoluteFilePath())) {
                            qInfo() << "Failed to remove" << extractEntryDest.absoluteFilePath();
                        }
                    } else if (query.responseSkip() || query.responseSkipAll()) {
                        if (query.responseSkipAll()) {   // 全部跳过
                            skipAll = true;
                        }

                        continue;
                    } else if (query.responseCancelled()) {   // 取消
                        emit signalCancel();
                        emit signalFinished(PFT_Cancel);
                        return false;
                    }
                } else if (skipAll) {   // 全部跳过
                    return true;
                } else if (overwriteAll) {   // 全部替换
                    if (!QFile::remove(extractEntryDest.absoluteFilePath())) {
                        qInfo() << "Failed to remove" << extractEntryDest.absoluteFilePath();
                    }
                }
            }

            // 创建文件夹路径
            if (!finalDestDir.mkpath(extractEntry.path())) {
                qInfo() << "Failed to create parent directory for file:" << extractEntryDest.filePath();
            }

            // 对临时文件夹内的文件进行rename操作，移到目标路径下
            if (!QFile(etractEntryTemp.absoluteFilePath()).rename(extractEntryDest.absoluteFilePath())) {
                qInfo() << "Failed to move file" << etractEntryTemp.filePath() << "to final destination.";
                //                emit signalFinished(PFT_Error);
                moveSuccess = false;
                //                return false;
            }
        }
    }

    return moveSuccess;
}

bool CliInterface::checkMoveCapability()
{
    bool hasMoveCapability = false;
    QString moveProgram = m_cliProps->property("moveProgram").toString();
    if (!moveProgram.isEmpty()) {
        QString moveProgramPath = QStandardPaths::findExecutable(moveProgram);
        hasMoveCapability = !moveProgramPath.isEmpty();
    }
    return hasMoveCapability;
}

void CliInterface::removeExtractedFilesOnFailure(const QString &strTargetPath, const QList<FileEntry> &entries)
{
    QList<FileEntry> listToRemove = entries;
    if (listToRemove.isEmpty()) {
        listToRemove = DataManager::get_instance().archiveData().mapFileEntry.values();
    }
    if (listToRemove.isEmpty()) {
        return;
    }

    QDir targetDir(strTargetPath);
    if (!targetDir.exists()) {
        return;
    }

    const QString canonicalTarget = QDir::cleanPath(targetDir.absolutePath());

    QList<QPair<QString, bool> > paths;   // path, isDirectory
    for (const FileEntry &entry : listToRemove) {
        QString relPath = entry.strFullPath;
        if (relPath.endsWith(QLatin1Char('/'))) {
            relPath.chop(1);
        }
        if (relPath.isEmpty()) {
            continue;
        }
        // 路径规范化后校验仍位于目标目录内，防止 "../" 路径遍历越界删除（PMS BUG-371831）
        const QString absPath = QDir::cleanPath(targetDir.absoluteFilePath(relPath));
        if (!absPath.startsWith(canonicalTarget + QLatin1Char('/')) && absPath != canonicalTarget) {
            continue;
        }
        paths.append(qMakePair(absPath, entry.isDirectory));
    }

    for (const auto &p : paths) {
        const QString &path = p.first;
        if (!p.second) {   // 文件
            QFileInfo fi(path);
            if (fi.exists() && fi.isFile() && fi.size() == 0) {
                QFile::remove(path);
            }
        }
    }
    // 空目录可能有多层，循环直到本轮没有可删的空目录
    bool removed;
    do {
        removed = false;
        for (const auto &p : paths) {
            if (!p.second) {
                continue;
            }
            QDir d(p.first);
            if (d.exists() && d.isEmpty()) {
                d.removeRecursively();
                removed = true;
            }
        }
    } while (removed);
}

bool CliInterface::copyArchiveVolumesToDir(const QString &srcArchive, const QString &destDir, QString &outFirstVolumePath)
{
    QFileInfo srcInfo(srcArchive);
    QString srcDir = srcInfo.absolutePath();
    QString srcName = srcInfo.fileName();

    // 7z/zip 分卷: xxx.7z.001, xxx.zip.001 ...
    // 合并为单个文件, 使 7z rn (不支持分卷) 能正常工作
    // (.+\.(?:7z|zip)) 捕获含扩展名的完整基本名, 确保 7z 能识别合并后的文件格式
    static const QRegularExpression reSplitVol(QStringLiteral("^(.+\\.(?:7z|zip))\\.[0-9]{3}$"));
    QRegularExpressionMatch m = reSplitVol.match(srcName);
    if (m.hasMatch()) {
        QString base = m.captured(1);   // 例如 "archive.7z" / "archive.zip"
        QStringList volumePaths;
        for (int i = 1;; ++i) {
            QString volName = QString("%1.%2").arg(base).arg(i, 3, 10, QChar('0'));
            QString volPath = srcDir + QDir::separator() + volName;
            if (!QFile::exists(volPath)) {
                break;
            }
            volumePaths << volPath;
        }

        if (volumePaths.count() >= 2) {
            // 安全: 使用 O_CREAT|O_EXCL (QIODevice::NewOnly) 原子创建文件,
            // 不预先 isSymLink 检查或 QFile::remove, 避免 TOCTOU 竞态条件
            QString mergedPath = destDir + QDir::separator() + base;

            QFile mergedFile(mergedPath);
            if (!mergedFile.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
                // 文件已存在可能是上次失败残留, 使用唯一名称重试
                mergedPath = destDir + QDir::separator() + base + QStringLiteral(".merged");
                mergedFile.setFileName(mergedPath);
                if (!mergedFile.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
                    qWarning() << "copyArchiveVolumesToDir: FAILED to create merged file" << mergedPath;
                    return false;
                }
            }

            for (const QString &volPath : volumePaths) {
                QFile volFile(volPath);
                if (!volFile.open(QIODevice::ReadOnly)) {
                    qWarning() << "copyArchiveVolumesToDir: FAILED to open volume" << volPath;
                    mergedFile.close();
                    return false;
                }
                const qint64 chunkSize = 4 * 1024 * 1024;  // 4MB
                while (!volFile.atEnd()) {
                    QByteArray chunk = volFile.read(chunkSize);
                    if (chunk.isEmpty()) {
                        // 修复: 检查读取错误
                        if (volFile.error() != QFile::NoError) {
                            qWarning() << "copyArchiveVolumesToDir: Error reading volume" << volPath;
                            mergedFile.close();
                            return false;
                        }
                        break;
                    }
                    mergedFile.write(chunk);
                }
                volFile.close();
            }
            // 检查 flush 结果, 避免 write 缓冲区写入失败时静默丢数据
            mergedFile.close();
            if (mergedFile.error() != QFile::NoError) {
                qWarning() << "copyArchiveVolumesToDir: write error on merged file" << mergedPath;
                return false;
            }

            outFirstVolumePath = mergedPath;
            qInfo() << "copyArchiveVolumesToDir: merged" << volumePaths.count()
                     << "volumes into" << mergedPath;
            return true;
        }
    }

    // rar 分卷 / 非分卷: 复制单个文件 (rar 由 unrar 原生处理分卷)
    // 安全: 使用含 UUID 的唯一临时文件名, 避免 exists+remove 竞态条件;
    // 复制成功后用原子 rename 放入目标路径
    QString destPath = destDir + QDir::separator() + srcName;
    QString tempPath = destDir + QDir::separator() + srcName + QStringLiteral(".tmpcopy.") + QUuid::createUuid().toString(QUuid::WithoutBraces);

    if (!QFile::copy(srcArchive, tempPath)) {
        qWarning() << "copyArchiveVolumesToDir: FAILED to copy" << srcArchive << "->" << tempPath;
        return false;
    }
    QFile::remove(destPath);
    if (!QFile::rename(tempPath, destPath)) {
        qWarning() << "copyArchiveVolumesToDir: FAILED to rename" << tempPath << "->" << destPath;
        QFile::remove(tempPath);
        return false;
    }
    outFirstVolumePath = destPath;
    return true;
}

QList<FileEntry> CliInterface::collectLongDirEntries(const QList<FileEntry> &files) const
{
    // 部分解压时，若只选中了长名目录下的文件（未选中目录本身），
    // files 列表中没有目录条目，导致 handleLongNameExtract 无法检测到长名目录。
    // 此函数遍历每个文件路径的各级父目录，从 mapFileEntry 中查找并补全超长目录条目。
    QSet<QString> existingPaths;  // files 中已有的路径（用于去重）
    for (const FileEntry &entry : files) {
        existingPaths.insert(entry.strFullPath);
    }

    QList<FileEntry> result = files;
    ArchiveData stArchiveData = DataManager::get_instance().archiveData();
    int supplementCount = 0;

    for (const FileEntry &entry : files) {
        // 从文件路径逐级向上查找父目录条目
        QString path = entry.strFullPath;
        // 去掉尾部斜杠（文件不会有，目录会有）
        if (path.endsWith(QDir::separator())) {
            path.chop(1);
        }
        while (path.contains(QDir::separator())) {
            int idx = path.lastIndexOf(QDir::separator());
            path = path.left(idx);  // 父目录路径（不含分隔符）
            QString dirKey = path + QDir::separator();  // mapFileEntry 中目录的 key 带尾部分隔符
            if (existingPaths.contains(dirKey)) {
                continue;  // 已在列表中
            }
            auto it = stArchiveData.mapFileEntry.find(dirKey);
            if (it != stArchiveData.mapFileEntry.end() && it->isDirectory) {
                // 检查目录名是否超长，只补全超长的目录
                QString dirName = QFileInfo(path).fileName();
                if (NAME_MAX < dirName.toLocal8Bit().length()) {
                    result.prepend(it.value());  // 插入到前面，保证目录在文件之前被处理
                    existingPaths.insert(dirKey);
                    supplementCount++;
                }
            }
        }
    }
    qInfo() << "collectLongDirEntries: supplemented" << supplementCount << "long-named dir entries, total files:" << files.count();
    return result;
}

bool CliInterface::handleLongNameExtract(const QList<FileEntry> &files)
{
    ExtractionOptions &options = m_extractOptions;
    QString password;
    if (options.password.isEmpty()) {
        // 对列表加密文件进行追加解压的时候使用压缩包的密码
        password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : QString();
    } else {
        password = options.password;
    }
    m_longNamePassword = password;

    m_longNameTempDir.reset(new QTemporaryDir());
    QString absoluteDestinationPath;
    if (!copyArchiveVolumesToDir(m_strArchiveName, m_longNameTempDir->path(), absoluteDestinationPath)) {
        qWarning() << "handleLongNameExtract: Failed to copy archive volumes to temp dir" << m_longNameTempDir->path();
        m_eErrorType = ET_FileWriteError;
        m_longNameTempDir.reset();
        return false;
    }
    m_longNameTempArchivePath = absoluteDestinationPath;

    // 遍历所有文件，分离需要重命名的文件和普通文件，同时创建目录结构
    m_renameEntries.clear();
    m_renameDirEntries.clear();
    m_allFileList.clear();
    qInfo() << "handleLongNameExtract: total files:" << files.count();

    for (const FileEntry &entry : files) {
        QFileInfo info(entry.strFullPath);
        QString strFilePath = info.path();
        QString strFileName = entry.strFullPath;
        Common com;
        QString sDir = com.handleLongNameforPath(strFilePath, strFileName, m_mapLongDirName, m_mapRealDirValue);
        if (sDir.length() > 0) {
            strFilePath = sDir.endsWith(QDir::separator()) ? sDir.left(sDir.length() - 1) : sDir;
            if (strFileName.endsWith(QDir::separator())) {
                strFileName = sDir;
            } else if (NAME_MAX >= QString(info.fileName()).toLocal8Bit().length()) {
                strFileName = sDir + info.fileName();
            }
        }

        bool needRename = (NAME_MAX < QString(info.fileName()).toLocal8Bit().length() && !entry.strFullPath.endsWith(QDir::separator()));
        if (needRename) {
            QString strTemp = info.fileName().left(TRUNCATION_FILE_LONG);
            QString tempFilePathName = strFilePath + QDir::separator() + strTemp;
            if (m_mapLongName[tempFilePathName]++ >= LONGFILE_SAME_FILES) {
                qWarning() << "handleLongNameExtract: too many files share same truncated name, aborting long-name extraction, count:" << m_mapLongName[tempFilePathName];
                emit signalCurFileName(entry.strFullPath);
                m_eErrorType = ET_LongNameError;
                m_longNameTempDir.reset();
                return false;
            }
            m_eErrorType = ET_LongNameError;
            QString sSuffix = QFileInfo(entry.strFullPath).completeSuffix();
            if (10 < sSuffix.length()) {
                sSuffix = QFileInfo(entry.strFullPath).suffix();
                if (10 < sSuffix.length()) {
                    sSuffix = sSuffix.right(10);
                }
            }
            QString strTempFileName = strTemp + QString("(%1)").arg(m_mapLongName[tempFilePathName], LONGFILE_SUFFIX_FieldWidth, BINARY_NUM, QChar('0')) + "." + sSuffix;
            strFileName = strTempFileName;
            if (strFilePath != ".") {
                strFileName = strFilePath + QDir::separator() + strTempFileName;
            }
        }

        // 检测目录名是否超过 NAME_MAX，需要将目录在归档内重命名为缩短名
        // （7z rn 重命名目录时会自动移动其下所有子条目）
        if (entry.strFullPath.endsWith(QDir::separator())) {
            QString dirPath = entry.strFullPath;
            dirPath.chop(1);  // 去掉尾部分隔符
            QString ownName = QFileInfo(dirPath).fileName();
            if (NAME_MAX < ownName.toLocal8Bit().length()) {
                m_eErrorType = ET_LongNameError;
                QString strTemp = ownName.left(TRUNCATION_FILE_LONG);
                // 使用归档内原始父路径构造计数 key，与 handleLongNameforPath 的 key 一致
                // （handleLongNameforPath 使用 dirInfo.path() 即原始父路径，不能用可能已被 sDir 缩短过的 strFilePath）
                QString originalParentPath = QFileInfo(dirPath).path();
                QString tempFilePathName;
                if (originalParentPath == ".") {
                    tempFilePathName = strTemp;
                } else {
                    tempFilePathName = originalParentPath + QDir::separator() + strTemp;
                }
                // handleLongNameforPath 已在上方调用时为长名目录递增了 m_mapLongDirName
                int count = m_mapLongDirName.value(tempFilePathName, 0);
                if (count == 0) {
                    count = 1;  // handleLongNameforPath 未填充时使用默认值
                }
                if (count > LONGFILE_SAME_FILES) {
                    qWarning() << "handleLongNameExtract: too many long-named dirs share same truncated name, aborting long-name extraction, count:" << count;
                    emit signalCurFileName(entry.strFullPath);
                    m_longNameTempDir.reset();
                    return false;
                }
                QString shortDirName = strTemp + QString("(%1)").arg(count, LONGFILE_SUFFIX_FieldWidth, BINARY_NUM, QChar('0'));
                FileEntry newEntry = entry;
                newEntry.strAlias = shortDirName;
                m_renameDirEntries.append(newEntry);
                // 更新 strFileName 为缩短后的目录路径。
                // sDir 是父路径的缩短结果（只含父目录前缀，不含本层短名）。
                // 注意：不能补上本层 shortDirName（如 sDir + shortDirName + "/"），
                // 否则会导致磁盘上预建目录与 7z x 解压目录出现重复。
                // sDir 为空时用本层短名；sDir 非空时用父缩短路径（mkpath 会在正确位置创建，
                // 7z x 从重命名后的归档解压时也会创建相同路径，不产生重复）。
                // 已知限制：嵌套长名空目录在部分解压时本层空目录可能落到父缩短路径
                // （触发条件极窄：父层与本层均超 255 字节 + 部分解压 + 空目录），
                // 不影响主修复场景与全量解压。
                if (!sDir.isEmpty()) {
                    strFileName = sDir;
                } else if (originalParentPath == "." || originalParentPath.isEmpty()) {
                    strFileName = shortDirName + QDir::separator();
                } else {
                    strFileName = originalParentPath + QDir::separator() + shortDirName + QDir::separator();
                }
            }
        }

        QString strDestFileName = options.strTargetPath + QDir::separator() + strFileName;
        if (!m_extractOptions.bAllExtract) {
            int nCnt = m_rootNode.count(QDir::separator());
            QString destFileName = strFileName;
            for (int i = 0; i < nCnt; i++) {
                int nIndex = destFileName.indexOf(QDir::separator());
                if (nIndex > 0) {
                    destFileName.remove(0, nIndex + 1);
                }
            }
            strDestFileName = options.strTargetPath + QDir::separator() + destFileName;
        }

        if (entry.strFullPath.endsWith(QDir::separator())) {
            if (!QDir(strDestFileName).exists()) {
                if (!QDir(strDestFileName).mkpath(strDestFileName)) {
                    qWarning() << "handleLongNameExtract: failed to create target dir:" << strDestFileName;
                    m_longNameTempDir.reset();
                    return false;
                }
            }
        } else {
            FileEntry newEntry = entry;
            newEntry.strAlias = QFileInfo(strFileName).fileName();
            if (needRename) {
                m_renameEntries.append(newEntry);
            }
            // 使用缩短后的路径构建解压文件列表
            m_allFileList.append(strFileName);
        }
    }

    // 嵌套长目录重命名必须从深到浅排序：
    // 7z rn 重命名父目录时自动移动子条目，但如果子目录尚未重命名，
    // 父目录重命名后子目录的旧路径已失效。
    // 因此先重命名最深的目录，再逐层向上重命名父目录。
    std::sort(m_renameDirEntries.begin(), m_renameDirEntries.end(),
              [](const FileEntry &a, const FileEntry &b) {
                  return a.strFullPath.count(QDir::separator()) > b.strFullPath.count(QDir::separator());
              });

    qInfo() << "handleLongNameExtract: rename entries:" << m_renameEntries.count()
            << ", rename dir entries:" << m_renameDirEntries.count()
            << ", all files:" << m_allFileList.count();

    // 异步执行 7z rn
    if (!m_renameEntries.isEmpty()) {
        m_longNamePhase = LNE_Rename;
        QString program = m_cliProps->property("moveProgram").toString();
        QStringList args = m_cliProps->moveArgs(m_longNameTempArchivePath, m_renameEntries,
                                                 DataManager::get_instance().archiveData(), m_longNamePassword);
        // -w 插在命令之后、归档之前，符合 7z 惯例
        QString wdir = options.strTargetPath;
        if (wdir.isEmpty()) {
            wdir = QDir::tempPath();
        }
        args.insert(1, QStringLiteral("-w%1").arg(wdir));
        qInfo() << "handleLongNameExtract: starting async rename" << m_renameEntries.count() << "files";
        if (!startLongNameProcess(program, args, options.strTargetPath)) {
            qWarning() << "handleLongNameExtract: FAILED to start async rename (file rename phase), entries:" << m_renameEntries.count();
            m_longNamePhase = LNE_None;
            m_longNameTempDir.reset();
            m_renameEntries.clear();
            m_renameDirEntries.clear();
            m_allFileList.clear();
            return false;
        }
        return true;
    }

    // 异步执行 7z rn（目录重命名，从深到浅，每个目录单独一条命令）
    // p7zip 16.02 的 7z rn 在单条命令中处理多对父子路径重命名时不会级联：
    // 子目录改名后，父目录改名不会把已改名的子目录一起搬走，子目录会孤立在
    // 原始（长名）父路径下，解压时重建长名父目录失败、该路径下文件消失。
    // 因此每个目录单独一条 7z rn，从深到浅经异步回调链式推进。
    if (!m_renameDirEntries.isEmpty()) {
        m_renameDirIndex = 0;
        if (!startNextLongNameDirRename()) {
            return false;
        }
        return true;
    }

    // 异步执行 7z x
    if (!m_allFileList.isEmpty()) {
        m_longNamePhase = LNE_Extract;
        QString program = m_cliProps->property("extractProgram").toString();
        // 全量解压时不传文件列表，避免文件数过多导致命令行超过 ARG_MAX；
        // 7z x 不带文件参数时默认解压全部文件。
        const QStringList &extractFileList = m_extractOptions.bAllExtract ? QStringList() : m_allFileList;
        QStringList args = m_cliProps->extractArgs(m_longNameTempArchivePath, extractFileList, true, m_longNamePassword);
        qInfo() << "handleLongNameExtract: starting async extract" << m_allFileList.count() << "files"
                << (m_extractOptions.bAllExtract ? "(all extract, no file list)" : "(partial extract)");
        if (!startLongNameProcess(program, args, options.strTargetPath)) {
            qWarning() << "handleLongNameExtract: FAILED to start async extract (extract phase), files:" << m_allFileList.count();
            m_longNamePhase = LNE_None;
            m_longNameTempDir.reset();
            m_renameEntries.clear();
            m_renameDirEntries.clear();
            m_allFileList.clear();
            return false;
        }
        return true;
    }

    return true;
}

bool CliInterface::startLongNameProcess(const QString &program, const QStringList &args, const QString &workDir)
{
    Q_ASSERT(!m_process);

    QString programPath = QStandardPaths::findExecutable(program);
    if (programPath.isEmpty()) {
        qWarning() << "startLongNameProcess: program not found:" << program;
        return false;
    }

    m_process = new KPtyProcess;
    m_process->setPtyChannels(KPtyProcess::StdinChannel);
    m_process->setOutputChannelMode(KProcess::MergedChannels);
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    if (!workDir.isEmpty()) {
        m_process->setWorkingDirectory(workDir);
    }
    m_process->setProgram(programPath, args);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        readStdout();
    });
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CliInterface::onLongNameProcessFinished);

    m_stdOutData.clear();
    m_isProcessKilled = false;
    m_finishType = PFT_Nomral;

    m_process->start();
    if (!m_process->waitForStarted()) {
        qWarning() << "startLongNameProcess: failed to start process:" << program;
        deleteProcess();
        return false;
    }
    m_childProcessId.clear();
    m_processId = m_process->processId();

    return true;
}

void CliInterface::onLongNameProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "LongName process finished, exitcode:" << exitCode
             << "exitstatus:" << exitStatus << "phase:" << m_longNamePhase;

    deleteProcess();

    if (m_isProcessKilled || exitCode != 0) {
        qWarning() << "LongName extraction failed in phase" << m_longNamePhase
                   << "killed:" << m_isProcessKilled << "exitCode:" << exitCode
                   << "exitStatus:" << exitStatus
                   << "archive:" << m_longNameTempArchivePath;
        if (m_finishType == PFT_Nomral) {
            m_finishType = PFT_Error;
        }
        m_longNamePhase = LNE_None;
        m_longNameTempDir.reset();
        m_renameEntries.clear();
        m_renameDirEntries.clear();
        m_allFileList.clear();
        emit signalprogress(100);
        emit signalFinished(m_finishType);
        return;
    }

    m_finishType = PFT_Nomral;

    if (m_longNamePhase == LNE_Rename) {
        // 文件重命名完成，接下来重命名目录或直接解压
        if (!m_renameDirEntries.isEmpty()) {
            // 从深到浅逐个目录重命名（每个目录单独一条 7z rn，见 startNextLongNameDirRename）
            m_renameDirIndex = 0;
            emit signalprogress(-1.0);
            if (!startNextLongNameDirRename()) {
                emit signalprogress(100);
                emit signalFinished(PFT_Error);
            }
        } else {
            // 没有目录需要重命名，直接进入解压阶段
            startLongNameExtractAfterRename();
        }
    } else if (m_longNamePhase == LNE_RenameDirs) {
        // 当前目录重命名完成，推进下一个或进入解压阶段
        m_renameDirIndex++;
        if (m_renameDirIndex < m_renameDirEntries.count()) {
            if (!startNextLongNameDirRename()) {
                emit signalprogress(100);
                emit signalFinished(PFT_Error);
            }
        } else {
            startLongNameExtractAfterRename();
        }
    } else if (m_longNamePhase == LNE_Extract) {
        m_longNamePhase = LNE_None;
        m_longNameTempDir.reset();
        m_renameEntries.clear();
        m_renameDirEntries.clear();
        m_allFileList.clear();
        m_eErrorType = ET_LongNameError;
        qInfo() << "LongName: extract done, refreshing file list";
        list();
    }
}

bool CliInterface::startNextLongNameDirRename()
{
    // 每次只重命名一个长名目录（从深到浅），发起单条 7z rn。
    // p7zip 16.02 的 7z rn 在单条命令中处理多对父子路径重命名时不会级联：
    // 子目录改名后，父目录改名不会把已改名的子目录一起搬走，子目录会孤立在
    // 原始（长名）父路径下，解压时重建长名父目录失败、该路径下文件消失。
    // 因此每个目录单独一条 7z rn，从深到浅经异步回调链式推进，
    // 全部完成后再由 onLongNameProcessFinished 进入解压阶段。
    m_longNamePhase = LNE_RenameDirs;
    QString program = m_cliProps->property("moveProgram").toString();
    QList<FileEntry> oneEntry;
    oneEntry << m_renameDirEntries.at(m_renameDirIndex);
    QStringList args = m_cliProps->longNameDirRenameArgs(m_longNameTempArchivePath, oneEntry, m_longNamePassword);
    QString wdir = m_extractOptions.strTargetPath;
    if (wdir.isEmpty()) {
        wdir = QDir::tempPath();
    }
    args.insert(1, QStringLiteral("-w%1").arg(wdir));
    qInfo() << "LongName: starting async dir rename" << (m_renameDirIndex + 1) << "/" << m_renameDirEntries.count();
    if (!startLongNameProcess(program, args, m_extractOptions.strTargetPath)) {
        qWarning() << "LongName: FAILED to start dir rename, index:" << m_renameDirIndex
                   << "total:" << m_renameDirEntries.count();
        m_longNamePhase = LNE_None;
        m_longNameTempDir.reset();
        m_renameEntries.clear();
        m_renameDirEntries.clear();
        m_allFileList.clear();
        return false;
    }
    return true;
}

void CliInterface::startLongNameExtractAfterRename()
{
    // 重命名阶段（文件/目录）完成后，重置进度条并启动解压阶段。
    // 用负值作为重置哨兵，由 MainWindow::slotReceiveProgress 捕获并调用 resetProgress()，
    // 确保 extract 阶段的进度从 0 开始单调递增。
    emit signalprogress(-1.0);

    if (m_allFileList.isEmpty()) {
        // 没有文件需要解压（如仅含空目录），直接完成
        m_longNamePhase = LNE_None;
        m_longNameTempDir.reset();
        m_renameEntries.clear();
        m_renameDirEntries.clear();
        m_allFileList.clear();
        m_eErrorType = ET_LongNameError;
        qInfo() << "LongName: rename done, no files to extract, refreshing file list";
        list();
        return;
    }

    m_longNamePhase = LNE_Extract;
    QString program = m_cliProps->property("extractProgram").toString();
    QStringList args = m_cliProps->extractArgs(m_longNameTempArchivePath,
                                                m_extractOptions.bAllExtract ? QStringList() : m_allFileList,
                                                true, m_longNamePassword);
    qInfo() << "LongName: rename done, starting async extract" << m_allFileList.count() << "files"
            << (m_extractOptions.bAllExtract ? "(all extract, no file list)" : "(partial extract)");
    if (!startLongNameProcess(program, args, m_extractOptions.strTargetPath)) {
        qWarning() << "LongName: FAILED to start extract process, archive:" << m_longNameTempArchivePath;
        m_longNamePhase = LNE_None;
        m_longNameTempDir.reset();
        m_renameEntries.clear();
        m_renameDirEntries.clear();
        m_allFileList.clear();
        emit signalprogress(100);
        emit signalFinished(PFT_Error);
    }
}

void CliInterface::readStdout(bool handleAll)
{
    //进程结束，不再对后面命令行缓存数据处理
    if (m_isProcessKilled) {
        return;
    }

    Q_ASSERT(m_process);

    if (!m_process->bytesAvailable()) {   // 无数据
        return;
    }

    // 获取命令行输出
    QByteArray dd = m_process->readAllStandardOutput();
    m_stdOutData += dd;

    // 换行分割
    QList<QByteArray> lines = m_stdOutData.split('\n');
    //    if (m_workStatus == WT_Add || m_workStatus == WT_Extract) {
    //        foreach (auto line, lines) {
    //            qInfo() << line;
    //        }
    //    }
    bool isWrongPwd = isWrongPasswordMsg(lines.last());

    if ((m_process->program().at(0).contains("7z") && m_process->program().at(1) != "l") && !isWrongPwd) {
        handleAll = true;   // 7z进度行结束无\n
    }

    if ((m_process->program().at(0).contains("tempScript")) && !isWrongPwd) {
        handleAll = true;   // compress .tar.7z progressline has no \n
    }

    bool foundErrorMessage = (isWrongPwd || isDiskFullMsg(QLatin1String(lines.last()))
                              || isFileExistsMsg(QLatin1String(lines.last())))
            || isPasswordPrompt(QLatin1String(lines.last()));

    if (foundErrorMessage) {
        handleAll = true;
    }

    if (handleAll) {
        m_stdOutData.clear();
    } else {
        // because the last line might be incomplete we leave it for now
        // note, this last line may be an empty string if the stdoutdata ends
        // with a newline
        if (m_process->program().at(0).contains("unrar")) {   // 针对unrar的命令行截取
            m_stdOutData.clear();
            if (lines.count() > 0) {
                if (!(lines[lines.count() - 1].endsWith("%") || lines[lines.count() - 1].endsWith("OK "))) {
                    if (isMultiPasswordPrompt(lines[lines.count() - 1]) || isFileExistsMsg(lines[lines.count() - 1]) || isPasswordPrompt(lines[lines.count() - 1])) {
                    } else {
                        m_stdOutData = lines.takeLast();
                    }
                }
            }
        } else {
            if (lines.size() == 1)
                return;
            m_stdOutData = lines.takeLast();
        }
    }

    // 处理命令行输出
    for (const QByteArray &line : qAsConst(lines)) {
        // 第二个判断条件是处理rar的list，当rar文件含有comment信息的时候需要根据空行解析
        if (!line.isEmpty() || (m_listEmptyLines && m_workStatus == WT_List)) {
            if (!handleLine(QString::fromLocal8Bit(line), m_workStatus)) {
                if (m_longNamePhase != LNE_None) {
                    qWarning() << "readStdout: handleLine returned false, killing long-name process. phase:" << m_longNamePhase
                               << "workStatus:" << m_workStatus;
                }
                emit signalprogress(100);
                killProcess();
                return;
            }
        }
    }
}

void CliInterface::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "Process finished, exitcode:" << exitCode << "exitstatus:" << exitStatus;

    deleteProcess();

    if (m_isCorruptArchive && m_workStatus == WT_List) {
        if (handleCorrupt() == PFT_Error) {
            m_eErrorType = ET_MissingVolume;
            m_finishType = PFT_Cancel;   // 取消打开或加载文件，界面停留在初始界面
        }
        m_isCorruptArchive = false;
    }

    if (exitCode == 0) {   // job正常结束
        m_finishType = PFT_Nomral;
    }

    //    setPassword(QString());

    emit signalprogress(100);
    emit signalFinished(m_finishType);
}

void CliInterface::extractProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "Extraction process finished, exitcode:" << exitCode << "   exitstatus:" << exitStatus;

    deleteProcess();

    if (0 == exitCode) {   // job正常结束
        m_finishType = PFT_Nomral;
    }

    m_indexOfListRootEntry = 0;
    m_isEmptyArchive = false;

    // 解压失败（如分卷加密包输错密码）且为全部解压到目标路径时，清理已生成的 size 为 0 等残留文件
    if (0 != exitCode && m_extractOptions.bAllExtract && !m_extractOptions.strTargetPath.isEmpty()) {
        removeExtractedFilesOnFailure(m_extractOptions.strTargetPath, m_files);
    }

    if (!m_extractOptions.bAllExtract && (!(m_extractOptions.strTargetPath.startsWith("/tmp") && m_extractOptions.strTargetPath.contains("/deepin-compressor-") && m_extractOptions.strDestination.isEmpty()))) {
        // job正常结束，或提取（非"打开"操作）失败时也搬运：使提取在分卷缺失等失败
        // 场景下保留已成功解出的文件，与"解压"（bAllExtract）行为保持一致（PMS BUG-371831）。
        // 以 !m_extractOptions.bOpen 收窄，"打开解压列表文件"（bOpen=true）失败仍维持原行为。
        if (0 == exitCode || !m_extractOptions.bOpen) {   // 正常结束，或提取（非打开）失败也搬运
            // 提取操作和打开解压列表文件非第一层的文件
            // 将文件从临时文件夹内移出
            bool droppedFilesMoved = moveExtractTempFilesToDest(m_files, m_extractOptions);
            if (!droppedFilesMoved) {
                m_rootNode.clear();   // 清空缓存数据
                m_extractTempDir.reset();
                emit signalFinished(m_finishType);
                return;
            }
        }

        // 提取（非"打开"操作）失败时，清理目标路径下 size 为 0 的残留文件，与解压失败处理对齐（PMS BUG-371831）
        if (0 != exitCode && !m_extractOptions.bOpen && !m_extractOptions.strTargetPath.isEmpty()) {
            removeExtractedFilesOnFailure(m_extractOptions.strTargetPath, m_files);
        }

        m_rootNode.clear();   // 清空缓存数据
        m_extractTempDir.reset();
    }

    emit signalprogress(100);
    emit signalFinished(m_finishType);
}

void CliInterface::getChildProcessId(qint64 processId, const QStringList &listKey, QVector<qint64> &childprocessid)
{
    /* 7z(23347)---7z(23348)-+-{7z}(23353)
     *                       |-{7z}(23354)
     *                       |-{7z}(23355)
     *                       |-{7z}(23356)
     *                       |-{7z}(23357)
     *                       |-{7z}(23358)
     *                       |-{7z}(23359)
     */

    /* bash(3967)-+-tar(3968)
     *           `-7z(3969)---7z(3971)-+-{7z}(3972)
     *                                 |-{7z}(3973)
     */
    childprocessid.clear();

    // 避免处理system process且有需要获取的关键字
    if (processId <= 0 || listKey.count() == 0) {
        return;
    }

    //使用pstree命令获取子进程号，如pstree -np 23347，子进程号为23348
    QString strProcessId = QString::number(processId);
    QProcess p;
    p.setProgram("pstree");
    p.setArguments(QStringList() << "-np" << strProcessId);
    p.start();

    if (p.waitForReadyRead()) {
        QByteArray dd = p.readAllStandardOutput();
        QList<QByteArray> lines = dd.split('\n');

        if (lines[0].contains(strProcessId.toUtf8())) {   // 从包含有processId这一行开始处理
            for (const QByteArray &line : qAsConst(lines)) {

                for (const QString &strKey : qAsConst(listKey)) {
                    QString str = QString("-%1(").arg(strKey);
                    int iCount = line.count(str.toStdString().c_str());   // 多个子进程都需要获取到
                    int iIndex = 0;
                    for (int i = 0; i < iCount; ++i) {
                        int iStartIndex = line.indexOf(str, iIndex);
                        int iEndIndex = line.indexOf(")", iStartIndex);
                        if (0 < iStartIndex && 0 < iEndIndex) {
                            childprocessid.append(line.mid(iStartIndex + str.length(), iEndIndex - iStartIndex - str.length()).toInt());   // 取-7z(3971)中间的进程号
                        }
                        iIndex = iStartIndex + 1;
                    }
                }
            }
        }
    }

    p.close();
}

QString CliInterface::getTargetPath()
{
    return m_extractOptions.strTargetPath;
}
