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

CliInterface::CliInterface(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
    m_bHandleCurEntry = true;
    setWaitForFinishedSignal(true);
    if (QMetaType::type("QProcess::ExitStatus") == 0) {
        qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    }

    m_cliProps = new CliProperties(this, m_metaData, m_mimetype);
}

CliInterface::~CliInterface()
{

}

PluginFinishType CliInterface::list()
{
    setPassword(QString());
    DataManager::get_instance().resetArchiveData();
    m_setHasRootDirs.clear();
    m_setHasHandlesDirs.clear();

    m_workStatus = WT_List;

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
    setPassword(QString());
    m_workStatus = WT_Extract;
    m_files = files;
    m_extractOptions = options;
    QString destPath;

    bool ret = false;

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
        qDebug() << "解压目标路径 --- " << destPath;
    }

    QDir::setCurrent(destPath);
    if (!m_extractOptions.bAllExtract) {  // 提取部分文件
        m_files.clear();
        foreach (FileEntry entry, files) {
            if (m_rootNode.isEmpty()) {  // 获取待提取文件的节点
                if (entry.isDirectory) {
                    m_rootNode = entry.strFullPath.left(entry.strFullPath.length() - entry.strFileName.length() - 1);
                } else {
                    m_rootNode = entry.strFullPath.right(entry.strFullPath.length() - entry.strFileName.length());
                }
            }

            // 提取文件夹需要在map里面查找文件夹下的文件，将文件从临时文件夹移除需要用到m_files
            if (entry.isDirectory) {
                QList<FileEntry> listEntry;
                ArchiveData stArchiveData =  DataManager::get_instance().archiveData();
                auto iter = stArchiveData.mapFileEntry.find(entry.strFullPath);
                for (; iter != stArchiveData.mapFileEntry.end() ;) {
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

        if (destPath.startsWith("/tmp") && destPath.contains("/deepin-compressor-")) { // 打开解压列表文件
            if (!QDir(destPath).exists()) {
                QDir(destPath).mkpath(destPath);
            }
        } else {  // 判断不是打开解压列表文件的临时目录，设置提取的临时目录
            // 设置临时目录
            m_extractTempDir.reset(new QTemporaryDir(QStringLiteral(".%1-").arg(QCoreApplication::applicationName())));
            if (!m_extractTempDir->isValid()) {
                qDebug() << "Creation of temporary directory failed.";
                emit signalFinished(PFT_Error);
                return PFT_Error;
            }

            destPath = m_extractTempDir->path();
            qDebug() << "提取临时路径 --- " << destPath;
        }
    } else {
        if (!QDir(destPath).exists() && !QDir(destPath).mkpath(destPath)) {
            if (isInsufficientDiskSpace(destPath, 10 * 1024 * 1024)) {  // 暂取小于10M作为磁盘空间不足的判断标准
                m_eErrorType = ET_InsufficientDiskSpace;
            } else {
                emit signalFileWriteErrorName("destPath");
                m_eErrorType = ET_FileWriteError;
            }
            emit signalFinished(PFT_Error);
            return PFT_Error;
        }
    }

    QDir::setCurrent(destPath);

    // 对列表加密文件进行追加解压的时候使用压缩包的密码
    QString password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : QString();
    ret =  runProcess(m_cliProps->property("extractProgram").toString(),
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
    setPassword(QString());
    m_workStatus = WT_Add;
    m_files = files;
    m_compressOptions = options;

    m_isTar7z = false;

    bool ret = false;
    QStringList fileList;

    // 压缩目标路径
    const QString destinationPath = (options.strDestination == QString()) ? QString() : options.strDestination;
    qDebug() << "Adding" << files.count() << "file(s) to destination:" << destinationPath;

    if (!destinationPath.isEmpty()) { // 向压缩包非第一层文件里面追加压缩
        m_extractTempDir.reset(new QTemporaryDir());
        // 临时路径
        const QString absoluteDestinationPath = m_extractTempDir->path() + QLatin1Char('/') + destinationPath;

        QDir qDir;
        qDir.mkpath(absoluteDestinationPath);  // 创建临时路径，存放待压缩文件

        for (FileEntry file : files) {
            // 待压缩文件的实际全路径
            const QString filePath = file.strFullPath;
            // 待压缩文件的文件名(临时路径全路径)
            const QString newFilePath = absoluteDestinationPath + QFileInfo(file.strFullPath).fileName();

            // 在临时路径创建待压缩文件的链接
            if (QFile::link(filePath, newFilePath)) {
                qDebug() << "Symlink's created:" << filePath << newFilePath;
            } else { // 创建链接失败
                qDebug() << "Can't create symlink" << filePath << newFilePath;
                emit signalFinished(PFT_Error);
                return PFT_Error;
            }
        }

        qDebug() << "Changing working dir again to " << m_extractTempDir->path();
        QDir::setCurrent(m_extractTempDir->path());

        // 添加临时路径中的第一层文件（夹）
        fileList.append(destinationPath.split(QLatin1Char('/'), QString::SkipEmptyParts).at(0));
    } else { // 压缩、向压缩包第一层文件追加压缩
        QList<FileEntry> tempfiles = files;
        // 获取待压缩的文件
        for (int i = 0; i < tempfiles.size(); i++) {
            fileList.append(tempfiles.at(i).strFullPath);
        }
    }

    // 对列表加密文件进行追加压缩的时候使用压缩包的密码
    QString password = DataManager::get_instance().archiveData().isListEncrypted ? DataManager::get_instance().archiveData().strPassword : options.strPassword;
    // 压缩命令的参数
    QStringList arguments = m_cliProps->addArgs(m_strArchiveName,
                                                fileList,
                                                password,
                                                options.bHeaderEncryption,
                                                options.iCompressionLevel,
                                                options.strCompressionMethod,
                                                options.strEncryptionMethod,
                                                options.iVolumeSize,
                                                options.bTar_7z,
                                                QFileInfo(m_strArchiveName).path());

    if (options.bTar_7z) { // 压缩tar.7z文件
        m_isTar7z = true;
        m_filesSize = options.qTotalSize; // 待压缩文件总大小
        QString strProgram = QStandardPaths::findExecutable("bash");
        ret = runProcess(strProgram, arguments);
    } else {
        ret = runProcess(m_cliProps->property("addProgram").toString(), arguments);
    }

    return ret ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliInterface::moveFiles(const QList<FileEntry> &/*files*/, const CompressOptions &/*options*/)
{
//    m_workStatus = WT_Add;

    return PFT_Nomral;
}

PluginFinishType CliInterface::copyFiles(const QList<FileEntry> &/*files*/, const CompressOptions &/*options*/)
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

PluginFinishType CliInterface::addComment(const QString &/*comment*/)
{
//    m_workStatus = WT_Add;

    return PFT_Nomral;
}

PluginFinishType CliInterface::updateArchiveData(const UpdateOptions &options)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();

    m_rootEntry = QString();
    foreach (FileEntry entry, options.listEntry) {
        if (options.eType == UpdateOptions::Delete) { // 删除
            if (entry.isDirectory) { // 删除文件夹
                // 在map中查找该文件夹下的文件并删除
                QMap<QString, FileEntry>::iterator itor = stArchiveData.mapFileEntry.begin();
                while (itor != stArchiveData.mapFileEntry.end()) {
                    if (itor->strFullPath.startsWith(entry.strFullPath)) {
                        if (!itor->isDirectory) {
                            stArchiveData.qSize -= itor->qSize; // 更新压缩包内文件原始总大小
                        }
                        itor = stArchiveData.mapFileEntry.erase(itor);
                    } else {
                        ++itor;
                    }
                }

                // 文件夹是第一层的数据
                if (entry.strFullPath.endsWith(QLatin1Char('/')) && entry.strFullPath.count(QLatin1Char('/')) == 1) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            } else { // 删除文件
                stArchiveData.qSize -= entry.qSize; // 更新压缩包内文件原始总大小
                stArchiveData.mapFileEntry.remove(entry.strFullPath); //在map中删除该文件
                // 文件是第一层的数据
                if (!entry.strFullPath.contains(QLatin1Char('/'))) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            }
        } else if (options.eType == UpdateOptions::Add) { // 追加压缩
            QString destinationPath = options.strParentPath; // 追加目标路径
            QFileInfo file(entry.strFullPath);

            if (m_rootEntry == "") { // 获取所有追加文件的父目录
                m_rootEntry = file.filePath().left(file.filePath().size() - file.fileName().size());
            }

            entry.strFullPath = destinationPath + entry.strFullPath.remove(m_rootEntry); // entry在压缩包中的全路径
            if (file.isDir()) { // 文件夹
                entry.strFullPath = entry.strFullPath + QDir::separator(); // 手动添加'/'
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
                    if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一层数据中找到entry，不添加数据
                        stArchiveData.listRootEntry.removeAt(i);
                        break;
                    }
                }

                // 在第一层数据中没有找到entry，在第一层数据中添加entry
                stArchiveData.listRootEntry.push_back(entry);
            }

            stArchiveData.mapFileEntry.insert(entry.strFullPath, entry); // 在map中插入数据
        }
    }

    stArchiveData.qComressSize = QFileInfo(m_strArchiveName).size(); // 更新压缩包大小

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

    connect(m_process, &QProcess::readyReadStandardOutput, this, [ = ] {
        readStdout();
    });

    if (m_workStatus == WT_Extract) {
        // Extraction jobs need a dedicated post-processing function.
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(extractProcessFinished(int, QProcess::ExitStatus)));
    } else {
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    }

    m_stdOutData.clear();
    m_isProcessKilled = false;
    m_process->start();

    if (m_process->waitForStarted()) {
        m_childProcessId.clear();
        m_processId = m_process->processId();

        if (m_isTar7z) {
            getChildProcessIdTar7z(QString::number(m_processId), m_childProcessId);
        }

        return true;
    }

    return true;
}

void CliInterface::deleteProcess()
{
    if (m_process) {
        readStdout(true);

        delete m_process;
        m_process = nullptr;
    }
}

void CliInterface::killProcess(bool emitFinished)
{
    Q_UNUSED(emitFinished);
    if (!m_process) {
        return;
    }

    if (!m_childProcessId.empty()) {
        for (int i = m_childProcessId.size() - 1; i >= 0; i--) {
            if (m_childProcessId[i] > 0) {
                kill(static_cast<__pid_t>(m_childProcessId[i]), SIGKILL);
            }
        }
    }

    m_process->kill();
    m_isProcessKilled = true;

    //取消删除操作时，删除掉临时文件
    if (m_workStatus == WT_Delete || m_workStatus == WT_Add) {
        QFile fi(m_strArchiveName + ".tmp");
        if (fi.exists()) {
            fi.remove();
        }
    }
}

void CliInterface::handleProgress(const QString &line)
{
    if (m_process && m_process->program().at(0).contains("7z")) {  // 解析7z相关进度、文件名
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            if (percentage > 0) {
                if (line.contains("\b\b\b\b") == true) {
                    QString strfilename;
                    if (m_workStatus == WT_Extract || m_workStatus == WT_Add) { // 解压、压缩解析文件名
                        int count = line.indexOf("+");  // 获取压缩参数解析

                        // 解压参数解析
                        if (-1 == count) {
                            count = line.indexOf("-");
                        }

                        // 更新参数解析
                        if (-1 == count) {
                            count = line.indexOf("U");
                        }

                        if (count > 0) {
                            strfilename = line.midRef(count + 2).toString();  // 文件名
                            // 右键 解压到当前文件夹
                            if (m_workStatus == WT_Extract && m_extractOptions.bExistList && m_indexOfListRootEntry == 0) {
                                m_indexOfListRootEntry++;
                                FileEntry entry;
                                entry.strFullPath = strfilename;
                                DataManager::get_instance().archiveData().listRootEntry << entry;
                            }
                        }
                    } else { // 删除解析文件名
                        if (line.contains("% = ")) {
                            strfilename = line.right(line.length() - line.indexOf(QLatin1Char('=')) - 2);
                        } else if (line.contains("% R ")) {
                            strfilename = line.right(line.length() - line.indexOf(QLatin1Char('R')) - 2);
                        }
                    }

                    emit signalprogress(percentage);
                    emit signalCurFileName(strfilename);
                }
            } else {
                // 7z解压小文件无法获取文件名添加一个空的entry
                if (m_workStatus == WT_Extract && m_extractOptions.bExistList && m_indexOfListRootEntry == 0 && m_isEmptyArchive == false) {
                    m_indexOfListRootEntry++;
                    FileEntry entry;
                    entry.strFullPath = QString();
                    DataManager::get_instance().archiveData().listRootEntry << entry;
                }
            }
        }
    } else if (m_process && m_process->program().at(0).contains("unrar")) { // 解析rar相关进度、文件名
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            emit signalprogress(percentage);
        }

        if (line.startsWith("Extracting")) {
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

            // 右键 解压到当前文件夹
            if (m_extractOptions.bExistList && m_indexOfListRootEntry == 0 && fileName.count('/') <= 1) {
                m_indexOfListRootEntry++;
                FileEntry entry;
                if (fileName.count('/') == 0) { // 压缩包内第一层的文件
                    entry.strFullPath = fileName;
                } else { // 压缩包内第一层的文件夹
                    QString name = fileName.left(fileName.indexOf(QLatin1Char('/')));
                    entry.strFullPath = name;
                }

                DataManager::get_instance().archiveData().listRootEntry << entry;
            }

            emit signalCurFileName(fileName);
        }
    } else if (m_process && m_process->program().at(0).contains("bash")) {
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
            // 无法获取正在压缩的某个文件名
            //            emit signalCurFileName();
        }
    }
}

PluginFinishType CliInterface::handlePassword()
{
    m_eErrorType = ET_NoError;

    QString name;
    if (m_process && m_process->program().at(0).contains("unrar")) { // rar解压会提示加密的文件名
        name = m_strEncryptedFileName;
    } else { // 7z不会提示加密的文件名
        if (m_files.count() == 1 && m_workStatus != WT_Add) { // 选则压缩包中的一个文件进行提取操作
            name = m_files.at(0).strFileName;
        } else { // 解压或是选择压缩包内多个文件进行提取操作
            name = m_strArchiveName;
        }
    }

    PasswordNeededQuery query(name);
    emit signalQuery(&query);
    query.waitForResponse();

    if (query.responseCancelled()) {
        DataManager::get_instance().archiveData().strPassword = QString();
        setPassword(QString()); // 函数暂时保留
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

    if (isFileExistsMsg(line)) {  // 提示是否替换已存在的文件
        const QStringList choices = m_cliProps->property("fileExistsInput").toStringList();  // 提示选项
        QString response;  // 选择结果

        OverwriteQuery query(m_parseName);
        emit signalQuery(&query);
        query.waitForResponse();

        if (query.responseCancelled()) {  // 取消
            // (Q)uit
            response = choices.at(4);
            emit signalCancel();
            m_eErrorType = ET_UserCancelOpertion;
            emit signalFinished(PFT_Cancel);
        } else if (query.responseSkip()) { // 跳过
            // (N)o
            response = choices.at(1);
            m_eErrorType = ET_NoError;
        } else if (query.responseSkipAll()) { // 全部跳过
            // (S)kip all  |  n[E]ver
            response = choices.at(3);
            m_eErrorType = ET_NoError;
        } else if (query.responseOverwrite()) { // 替换
            // (Y)es
            response = choices.at(0);
        } else if (query.responseOverwriteAll()) { // 全部替换
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

    qDebug() << "Writing data to the process: " << data;
//    m_process->write(data);
    m_process->pty()->write(data);
}

bool CliInterface::moveExtractTempFilesToDest(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    QDir finalDestDir(options.strTargetPath); // 提取目标路径
    bool overwriteAll = false;  // 全部替换
    bool skipAll = false;  // 全部跳过
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
                qDebug() << "Failed to create directory" << extractEntry.filePath() << "in final destination.";
            }
        } else {
            // 目标路径下文件已经存在
            if (extractEntryDest.exists()) {
                qDebug() << "File" << extractEntryDest.absoluteFilePath() << "exists.";
                if (!skipAll && !overwriteAll) {
                    OverwriteQuery query(extractEntryDest.absoluteFilePath());
                    emit signalQuery(&query);
                    query.waitForResponse();

                    if (query.responseOverwrite() || query.responseOverwriteAll()) {
                        if (query.responseOverwriteAll()) { // 全部替换
                            overwriteAll = true;
                        }

                        if (!QFile::remove(extractEntryDest.absoluteFilePath())) {
                            qDebug() << "Failed to remove" << extractEntryDest.absoluteFilePath();
                        }
                    } else if (query.responseSkip() || query.responseSkipAll()) {
                        if (query.responseSkipAll()) { // 全部跳过
                            skipAll = true;
                        }

                        continue;
                    } else if (query.responseCancelled()) { // 取消
                        emit signalCancel();
                        emit signalFinished(PFT_Cancel);
                        return false;
                    }
                } else if (skipAll) { // 全部跳过
                    return true;
                } else if (overwriteAll) { // 全部替换
                    if (!QFile::remove(extractEntryDest.absoluteFilePath())) {
                        qDebug() << "Failed to remove" << extractEntryDest.absoluteFilePath();
                    }
                }
            }

            // 创建文件夹路径
            if (!finalDestDir.mkpath(extractEntry.path())) {
                qDebug() << "Failed to create parent directory for file:" << extractEntryDest.filePath();
            }

            // 对临时文件夹内的文件进行rename操作，移到目标路径下
            if (!QFile(etractEntryTemp.absoluteFilePath()).rename(extractEntryDest.absoluteFilePath())) {
                qDebug() << "Failed to move file" << etractEntryTemp.filePath() << "to final destination.";
//                emit signalFinished(PFT_Error);
                moveSuccess = false;
//                return false;
            }
        }
    }

    return moveSuccess;
}

void CliInterface::readStdout(bool handleAll)
{
    //进程结束，不再对后面命令行缓存数据处理
    if (m_isProcessKilled) {
        return;
    }

    Q_ASSERT(m_process);

    if (!m_process->bytesAvailable()) { // 无数据
        return;
    }

    // 获取命令行输出
    QByteArray dd = m_process->readAllStandardOutput();
    m_stdOutData += dd;

    // 换行分割
    QList<QByteArray> lines = m_stdOutData.split('\n');
//    if (m_workStatus == WT_Add || m_workStatus == WT_Extract) {
//        foreach (auto line, lines) {
//            qDebug() << line;
//        }
//    }
    bool isWrongPwd = isWrongPasswordMsg(lines.last());

    if ((m_process->program().at(0).contains("7z") && m_process->program().at(1) != "l") && !isWrongPwd) {
        handleAll = true; // 7z进度行结束无\n
    }

    if ((m_process->program().at(0).contains("bash") && m_process->program().at(2).contains("7z")) && !isWrongPwd) {
        handleAll = true; // compress .tar.7z progressline has no \n
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
        if (m_process->program().at(0).contains("unrar")) { // 针对unrar的命令行截取
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
                killProcess();
                return;
            }
        }
    }
}

void CliInterface::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Process finished, exitcode:" << exitCode << "exitstatus:" << exitStatus;

    deleteProcess();

    if (exitCode == 0) { // job正常结束
        m_finishType = PFT_Nomral;
    }

//    setPassword(QString());

    emit signalprogress(100);
    emit signalFinished(m_finishType);
}

void CliInterface::extractProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Extraction process finished, exitcode:" << exitCode << "   exitstatus:" << exitStatus;

    if (m_process) {
        // Handle all the remaining data in the process.
        readStdout(true);

        delete m_process;
        m_process = nullptr;
    }

    if (exitCode == 0) { // job正常结束
        m_finishType = PFT_Nomral;
    }

    m_indexOfListRootEntry = 0;
    m_isEmptyArchive = false;

    if (!m_extractOptions.bAllExtract && (!(m_extractOptions.strTargetPath.startsWith("/tmp")
                                            && m_extractOptions.strTargetPath.contains("/deepin-compressor-")
                                            && m_extractOptions.strDestination.isEmpty()))) {
        // 提取操作和打开解压列表文件非第一层的文件
        // 将文件从临时文件夹内移出
        bool droppedFilesMoved = moveExtractTempFilesToDest(m_files, m_extractOptions);
        if (!droppedFilesMoved) {
            m_rootNode.clear(); // 清空缓存数据
            m_extractTempDir.reset();
            emit signalFinished(m_finishType);
            return;
        }

        m_rootNode.clear(); // 清空缓存数据
        m_extractTempDir.reset();
    }

    emit signalprogress(100);
    emit signalFinished(m_finishType);
}

void CliInterface::getChildProcessIdTar7z(const QString &processid, QVector<qint64> &childprocessid)
{
    //使用pstree命令获取子进程号，如pstree -np 17251，子进程号为17252、17253
    /*bash(17251)-+-tar(17252)
     *            `-7z(17253)-+-{7z}(17254)
     *                        |-{7z}(17255)
     *                        |-{7z}(17257)
     *                        |-{7z}(17258)
     *                        |-{7z}(17259)
     *                        |-{7z}(17260)
     */
    QProcess p;
    p.setProgram("pstree");
    p.setArguments(QStringList() << "-np" << processid);
    p.start();
    if (p.waitForReadyRead()) {
        QByteArray dd = p.readAllStandardOutput();
        QList<QByteArray> lines = dd.split('\n');
        if (lines[0].contains(processid.toUtf8())) {
            for (const QByteArray &line : qAsConst(lines)) {
                int a, b;
//                qDebug() << line;
                if (0 < (a = line.indexOf("-tar(")) && 0 < (b = line.indexOf(")", a))) {
//                    qDebug() << a << b << line.mid(a + 5, b - a - 5).toInt();
                    childprocessid.append(line.mid(a + 5, b - a - 5).toInt());
                }
                if (0 < (a = line.indexOf("-7z(")) && 0 < (b = line.indexOf(")", a))) {
//                    qDebug() << a << b << line.mid(a + 4, b - a - 4).toInt();
                    childprocessid.append(line.mid(a + 4, b - a - 4).toInt());
                    break;
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
