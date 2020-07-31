/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#include "queries.h"
#include "kprocess.h"
#include "analysepsdtool.h"
#include "filewatcher.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QMimeDatabase>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QUrl>

#include <linux/limits.h>

CliInterface::CliInterface(QObject *parent, const QVariantList &args) : ReadWriteArchiveInterface(parent, args)
{
    mType = ENUM_PLUGINTYPE::PLUGIN_CLIINTERFACE;
    // because this interface uses the event loop
    setWaitForFinishedSignal(true);

    if (QMetaType::type("QProcess::ExitStatus") == 0) {
        qRegisterMetaType< QProcess::ExitStatus >("QProcess::ExitStatus");
    }
    m_cliProps = new CliProperties(this, m_metaData, mimetype());
}

CliInterface::~CliInterface()
{
//    Q_ASSERT(!m_process);

    if (m_process != nullptr) {
        m_process->kill();
        m_process->waitForFinished(1);
    }

    if (pAnalyseHelp != nullptr) {
        delete pAnalyseHelp;
        pAnalyseHelp = nullptr;
    }

    if (pFileWatcherdd != nullptr) {
        this->watchDestFilesEnd();
        delete pFileWatcherdd;
        pFileWatcherdd = nullptr;
    }
}

void CliInterface::init()
{
}

void CliInterface::setListEmptyLines(bool emptyLines)
{
    m_listEmptyLines = emptyLines;
}

int CliInterface::copyRequiredSignals() const
{
    return 2;
}

bool CliInterface::list(bool isbatch)
{
    resetParsing();
    m_operationMode = List;
    m_numberOfEntries = 0;
    m_isbatchlist = isbatch;
    // To compute progress.
    m_archiveSizeOnDisk = static_cast< qulonglong >(QFileInfo(filename()).size());
    connect(this, &ReadOnlyArchiveInterface::entry, this, &CliInterface::onEntry);
    if ((m_cliProps->property("listProgram").toString().contains("unrar")) && ("" == password())) {
        // qprocess can't read all the output string,so set wrong password to detect if need password
        setPassword("temp");
    }

    return runProcess(m_cliProps->property("listProgram").toString(), m_cliProps->listArgs(filename(), password()));
}

bool CliInterface::extractFiles(const QVector< Archive::Entry * > &files, const QString &destinationDirectory,
                                const ExtractionOptions &options)
{
    qDebug() << "destination directory:" << destinationDirectory;
    this->extractPsdStatus = NotChecked;


    return this->extractFF(files, destinationDirectory, options);
}

bool CliInterface::extractFF(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    if (this->extractPsdStatus == ReadOnlyArchiveInterface::WrongPsd) {
        return false;
    }

    if (pAnalyseHelp != nullptr) {
        delete pAnalyseHelp;
        pAnalyseHelp = nullptr;
    }

    m_operationMode = Extract;
    m_extractionOptions = options;
    m_extractedFiles = files;
    QString destPath = "";

    ifReplaceTip = false;
    if (this->extractPsdStatus == NotChecked) {
        pAnalyseHelp = new AnalyseHelp(destinationDirectory, this->extractTopFolderName);
        destPath = pAnalyseHelp->getTempPath();
    } else {
        destPath = destinationDirectory;
        this->extractPsdStatus = Checked;
        emit sigExtractPwdCheckDown();
    }
    if (m_extractionOptions.isBatchExtract()) {
        destPath = destinationDirectory;
    }
    qDebug() << "####destpath：" << destPath;
    m_extractDestDir = destPath;
//    qDebug() << m_extractDestDir;
    if (extractDst7z_.isEmpty() == false) {
        destDirName = extractDst7z_;
        updateDestFileSignal(m_extractDestDir + "/" + extractDst7z_);
//        extractDst7z_.clear();
    } else {
        if (destDirName == "") {
            destDirName = extractTopFolderName;
        }
    }


    bool ifNeedPsd = options.encryptedArchiveHint();
    if (ifNeedPsd == false) {
        //don't need psd
        this->extractPsdStatus = ReadOnlyArchiveInterface::Reextract;
        if (this->pAnalyseHelp != nullptr) {
            return this->extractFF(m_extractedFiles, this->pAnalyseHelp->getDestDir(), m_extractionOptions);
        }
    }


    //get user input password
    QString psdd = password();
    if (!m_cliProps->property("passwordSwitch").toStringList().isEmpty() && ifNeedPsd
            && psdd.isEmpty()) {
        qDebug() << "Password hint enabled, querying user";
        if (m_extractionOptions.isBatchExtract()) {
            if (!passwordQuery()) {
                return false;
            }
        } else {
            emit sigExtractNeedPassword();
            return false;
        }
    }

    QUrl destDir = QUrl(destPath);
    m_oldWorkingDirExtraction = QDir::currentPath();
    QDir::setCurrent(destDir.adjusted(QUrl::RemoveScheme).url());

    const bool useTmpExtractDir = options.isDragAndDropEnabled() || options.alwaysUseTempDir();

    if (useTmpExtractDir) {
        // Create an hidden temp folder in the current directory.
        m_extractTempDir.reset(new QTemporaryDir(QStringLiteral(".%1-").arg(QCoreApplication::applicationName())));

        qDebug() << "Using temporary extraction dir:" << m_extractTempDir->path();
        if (!m_extractTempDir->isValid()) {
            qDebug() << "Creation of temporary directory failed.";
            emit finished(false);
            return false;
        }
        destDir = QUrl(m_extractTempDir->path());
        QDir::setCurrent(destDir.adjusted(QUrl::RemoveScheme).url());
    }



    return runProcess(
               m_cliProps->property("extractProgram").toString(),
               m_cliProps->extractArgs(filename(), extractFilesList(files), options.preservePaths(), password()));
}

bool CliInterface::addFiles(const QVector< Archive::Entry * > &files, const Archive::Entry *destination,
                            const CompressionOptions &options, uint numberOfEntriesToAdd)
{
    Q_UNUSED(numberOfEntriesToAdd)

    m_operationMode = Add;
    m_addFiles = files;
    QVector< Archive::Entry * > filesToPass = QVector< Archive::Entry * >();
    // If destination path is specified, we have recreate its structure inside the temp directory
    // and then place symlinks of targeted files there.
    const QString destinationPath = (destination == nullptr) ? QString() : destination->fullPath();

    qDebug() << "Adding" << files.count() << "file(s) to destination:" << destinationPath;
    m_curfilenumber = 0;
    m_allfilenumber = static_cast<int>(numberOfEntriesToAdd);

    if (!destinationPath.isEmpty()) {
        m_extractTempDir.reset(new QTemporaryDir());
        const QString absoluteDestinationPath = m_extractTempDir->path() + QLatin1Char('/') + destinationPath;

        QDir qDir;
        qDir.mkpath(absoluteDestinationPath);

        QObject *preservedParent = nullptr;
        for (Archive::Entry *file : files) {
            // The entries may have parent. We have to save and apply it to our new entry in order to prevent memory
            // leaks.
            if (preservedParent == nullptr) {
                preservedParent = file->getParent();
            }
            const QString filePath = file->fullPath();
            const QString newFilePath = absoluteDestinationPath + file->name();
            if (QFile::link(filePath, newFilePath)) {
                qDebug() << "Symlink's created:" << filePath << newFilePath;
            } else {
                qDebug() << "Can't create symlink" << filePath << newFilePath;
                emit finished(false);
                return false;
            }
        }

        qDebug() << "Changing working dir again to " << m_extractTempDir->path();
        QDir::setCurrent(m_extractTempDir->path());

        filesToPass.push_back(new Archive::Entry(
                                  preservedParent, destinationPath.split(QLatin1Char('/'), QString::SkipEmptyParts).at(0)));
    } else {
        filesToPass = files;
    }

    if (!m_cliProps->property("passwordSwitch").toString().isEmpty() && options.encryptedArchiveHint()
            && password().isEmpty()) {
        qDebug() << "Password hint enabled, querying user";
//        TODO:追加暂时不设置密码
#if 0
        if (!passwordQuery()) {
            return false;
        }
#endif
    }

    QStringList arguments = m_cliProps->addArgs(filename(),
                                                entryFullPaths(filesToPass, NoTrailingSlash),
                                                password(),
                                                isHeaderEncryptionEnabled(),
                                                options.compressionLevel(),
                                                options.compressionMethod(),
                                                options.encryptionMethod(),
                                                options.volumeSize(),
                                                options.isTar7z(),
                                                options.globalWorkDir());

    if (destinationPath.isEmpty()) {//如果不是追加，需要去除-l
        arguments.removeOne("-l");
    }

    bool ret = false;
    //tar.7z： Different compression commands
    if (options.isTar7z()) {
        m_filesSize = options.getfilesSize();
        QString strProgram = QStandardPaths::findExecutable("bash");
        ret = runProcess(strProgram, arguments);
    } else {
        ret = runProcess(m_cliProps->property("addProgram").toString(), arguments);
    }
    if (ret == true) {
        this->watchDestFilesBegin();
    }
    return ret;
}

bool CliInterface::moveFiles(const QVector< Archive::Entry * > &files, Archive::Entry *destination,
                             const CompressionOptions &options)
{
    Q_UNUSED(options);

    m_operationMode = Move;

    m_removedFiles = files;
    QVector< Archive::Entry * > withoutChildren = entriesWithoutChildren(files);
    setNewMovedFiles(files, destination, withoutChildren.count());

    return runProcess(m_cliProps->property("moveProgram").toString(),
                      m_cliProps->moveArgs(filename(), withoutChildren, destination, password()));
}

bool CliInterface::copyFiles(const QVector< Archive::Entry * > &files, Archive::Entry *destination,
                             const CompressionOptions &options)
{
    m_oldWorkingDir = QDir::currentPath();
    m_tempWorkingDir.reset(new QTemporaryDir());
    m_tempAddDir.reset(new QTemporaryDir());
    QDir::setCurrent(m_tempWorkingDir->path());
    m_passedFiles = files;
    m_passedDestination = destination;
    m_passedOptions = options;
    m_numberOfEntries = 0;

    m_subOperation = Extract;
    connect(this, &CliInterface::finished, this, &CliInterface::continueCopying);

    return extractFiles(files, QDir::currentPath(), ExtractionOptions());
}

bool CliInterface::deleteFiles(const QVector< Archive::Entry * > &files)
{
    m_operationMode = Delete;

    m_removedFiles = files;

    return runProcess(m_cliProps->property("deleteProgram").toString(),
                      m_cliProps->deleteArgs(filename(), files, password()));
}

bool CliInterface::testArchive()
{
    resetParsing();
    m_operationMode = Test;

    return runProcess(m_cliProps->property("testProgram").toString(), m_cliProps->testArgs(filename(), password()));
}

bool CliInterface::runProcess(const QString &programName, const QStringList &arguments)
{
    Q_ASSERT(!m_process);

    QString programPath = QStandardPaths::findExecutable(programName);
    if (programPath.isEmpty()) {
        //emit error(tr("@info", "Failed to locate program <filename>%1</filename> on disk."));
        emit error("@info Failed to locate program <filename>%1</filename> on disk.");
        emit finished(false);
        return false;
    }

    m_process = new KProcess;
    //    m_process->setPtyChannels(KPtyProcess::StdinChannel);//TODO_DS

    m_process->setOutputChannelMode(KProcess::MergedChannels);
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    m_process->setProgram(programPath, arguments);
    // qDebug() << programPath << arguments;

    connect(m_process, &QProcess::readyReadStandardOutput, this, [ = ]() {
        readStdout();
    });

    if (m_operationMode == Extract) {
        // Extraction jobs need a dedicated post-processing function.
        connect(m_process,
                QOverload< int, QProcess::ExitStatus >::of(&QProcess::finished),
                this,
                &CliInterface::extractProcessFinished);
    } else {
        connect(m_process,
                QOverload< int, QProcess::ExitStatus >::of(&QProcess::finished),
                this,
                &CliInterface::processFinished);
    }

    m_stdOutData.clear();

    m_process->start();

    return true;
}

void CliInterface::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_exitCode = exitCode;
    qDebug() << "Process finished, exitcode:" << exitCode << "exitstatus:" << exitStatus;

//    if (m_process) {
//        // handle all the remaining data in the process
//        readStdout(true);

//        delete m_process;
//        m_process = nullptr;
//    }

    deleteProcess();

    // #193908 - #222392
    // Don't emit finished() if the job was killed quietly.
    if (m_abortingOperation) {
        return;
    }

    if (m_operationMode == Delete || m_operationMode == Move) {
        const QStringList removedFullPaths = entryFullPaths(m_removedFiles);
        for (const QString &fullPath : removedFullPaths) {
            emit entryRemoved(fullPath);
        }
        for (Archive::Entry *e : qAsConst(m_newMovedFiles)) {
            emit entry(e);
        }
        m_newMovedFiles.clear();
    }

    //    if (m_operationMode == Add && !isMultiVolume()) {
    //        list();
    //    } else
    if (m_operationMode == List && isCorrupt()) {
        LoadCorruptQuery query(filename());
        //query.execute();
        emit userQuery(&query);
        query.waitForResponse();
        if (!query.responseYes()) {
            emit cancelled();
            emit finished(false);
        } else {
            emit progress(1.0);
            emit finished(true);
        }
    } else if (m_operationMode == List && (isWrongPassword() || 9 == exitCode || 2 == exitCode)) {
        if (m_isbatchlist && 2 == exitCode) {

            PasswordNeededQuery query(filename());
            //query.execute();
            emit userQuery(&query);
            query.waitForResponse();
            if (query.responseCancelled()) {
                emit error("Canceal when batchextract.");
                emit cancelled();
                // There is no process running, so finished() must be emitted manually.
                emit finished(false);
                return;
            }
            setPassword(query.password());

            setWrongPassword(false); //初始化错误状态
            m_isPasswordPrompt = false; //初始化错误状态
            emit sigBatchExtractJobWrongPsd(password()); //批量解压时，列表加密密码错误重新走list流程
            return;
        }
        if (m_isPasswordPrompt || password().size() > 0) {
            emit error("wrong password");
            setPassword(QString());
        }
        return;
    } else {
        emit progress(1.0);
        emit finished(true);
    }
}

void CliInterface::cleanIfCanceled()
{
    //qDebug() << "取消操作！";
}

void CliInterface::watchDestFilesBegin()
{
    if (this->pFileWatcherdd == nullptr) {
        this->pFileWatcherdd = new FileWatcher(this);
    }
    connect(this->pFileWatcherdd, &FileWatcher::sigFileChanged, this, &CliInterface::slotFilesWatchedChanged);
    this->pFileWatcherdd->beginWork();
}

void CliInterface::watchDestFilesEnd()
{
    if (this->pFileWatcherdd != nullptr) {
        this->pFileWatcherdd->finishWork();
    }
}

void CliInterface::watchFileList(QStringList *strList)
{
    qDebug() << "%%%%%%%待监控的文件：" << *strList;
    if (this->pFileWatcherdd == nullptr) {
        this->pFileWatcherdd = new FileWatcher(this);
    }
    this->pFileWatcherdd->watch(strList);
}

void CliInterface::slotFilesWatchedChanged(QString /*fileChanged*/)
{
    this->watchDestFilesEnd();
    emit cancelled();
    emit finished(false);

    killProcess();
}

void CliInterface::extractProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_ASSERT(m_operationMode == Extract);

    m_exitCode = exitCode;
    qDebug() << "Extraction process finished, exitcode:" << exitCode << "exitstatus:" << exitStatus;

    if (m_process) {
        // Handle all the remaining data in the process.
        readStdout(true);

        delete m_process;
        m_process = nullptr;
    }

    // Don't emit finished() if the job was killed quietly.
    if (m_abortingOperation) {
        return;
    }

    if (m_extractionOptions.alwaysUseTempDir()) {
        // unar exits with code 1 if extraction fails.
        // This happens at least with wrong passwords or not enough space in the destination folder.
        if (m_exitCode == 1) {
            if (password().isEmpty()) {
                qDebug() << "Extraction aborted, destination folder might not have enough space.";
                //emit error(tr("Extraction failed. Make sure that enough space is available."));
                emit error(("Extraction failed. Make sure that enough space is available."));
            } else {
                qDebug() << "Extraction aborted, either the password is wrong or the destination folder doesn't have "
                         "enough space.";
                //emit error(tr("Extraction failed. Make sure you provided the correct password and that enough space is "
                //              "available."));
                emit error(("Extraction failed. Make sure you provided the correct password and that enough space is "
                            "available."));
                setPassword(QString());
            }
            cleanUpExtracting();
            emit finished(false);
            return;
        }

        if (!m_extractionOptions.isDragAndDropEnabled()) {
            if (!moveToDestination(QDir::current(), QDir(m_extractDestDir), m_extractionOptions.preservePaths())) {
                //emit error(tr("Could not move the extracted file to the destination directory."));
                emit error("Could not move the extracted file to the destination directory.");
                cleanUpExtracting();
                emit finished(false);
                return;
            }

            cleanUpExtracting();
        }
    }

    if (this->extractPsdStatus == Reextract) {
        if (m_extractionOptions.isBatchExtract()) {

        } else {
            qDebug() << this->destDirName;
            if (this->pAnalyseHelp != nullptr) {
                this->extractFF(m_extractedFiles, this->pAnalyseHelp->getDestDir(), m_extractionOptions);
                //qDebug()<<"==========直接解压文件";
                return;
            }
        }
    } else if (this->extractPsdStatus == Checked) {

    } else if (this->extractPsdStatus == Canceled) {
        if (ifReplaceTip == false) {
//            qDebug()<<"==========删除临时文件";
            if (this->m_extractDestDir == "" || this->destDirName == "") {

            } else {
                QString fullPath = m_extractDestDir + QDir::separator() + this->destDirName;
                QFileInfo fileInfo(fullPath);
                if (fileInfo.exists()) {
                    ReadWriteArchiveInterface::clearPath(fullPath);
                }
            }
        }
    }

    if (m_exitCode == 2 || m_exitCode == 3 || m_exitCode == 255) {
        if (m_extractionOptions.isBatchExtract() && 2 == m_exitCode) {
            qDebug() << "wrong password";
            emit sigBatchExtractJobWrongPsd(); //批量解压时，密码错误重新走解压流程
            setPassword(QString());
            return;
        }
        if (password().isEmpty()) {
            //            qDebug() << "Extraction failed, the file is broken";
            //            emit error(tr("Extraction failed. the file is broken"));
            if (destDirName.toUtf8().length() > NAME_MAX) { //Is the file name too long
                emit error("Filename is too long");
            }
        } else {
            qDebug() << "Extraction failed, the file is broken";
            //emit error(tr("Extraction failed. the file is broken"));
            emit error("Extraction failed. the file is broken");
            setPassword(QString());
        }
        cleanUpExtracting();
        emit finished(false);
        return;
    } else if (m_exitCode == 9) {
        if (m_extractionOptions.isBatchExtract()) {
            qDebug() << "wrong password";
            emit sigBatchExtractJobWrongPsd(); //批量解压时，密码错误重新走解压流程
            setPassword(QString());
            return;
        } else {
            qDebug() << "wrong password";
            //emit error(tr("wrong password"));
            emit error("wrong password");
            setPassword(QString());
            // emit finished(false);
            return;
        }
    }

    if (m_extractionOptions.isDragAndDropEnabled()) {
        const bool droppedFilesMoved = moveDroppedFilesToDest(m_extractedFiles, m_extractDestDir);
        if (!droppedFilesMoved) {
            cleanUpExtracting();
            return;
        }

        cleanUpExtracting();
    }

    // #395939: make sure we *always* restore the old working dir.
    restoreWorkingDirExtraction();

    emit progress(1.0);
    emit finished(true);
}

void CliInterface::continueCopying(bool result)
{
    if (!result) {
        finishCopying(false);
        return;
    }

    switch (m_subOperation) {
    case Extract:
        m_subOperation = Add;
        m_passedFiles = entriesWithoutChildren(m_passedFiles);
        if (!setAddedFiles() || !addFiles(m_tempAddedFiles, m_passedDestination, m_passedOptions)) {
            finishCopying(false);
        }
        break;
    case Add:
        finishCopying(true);
        break;
    default:
        Q_ASSERT(false);
    }
}

bool CliInterface::moveDroppedFilesToDest(const QVector< Archive::Entry * > &files, const QString &finalDest)
{
    // Move extracted files from a QTemporaryDir to the final destination.

    QDir finalDestDir(finalDest);
    qDebug() << "Setting final dir to" << finalDest;

    bool overwriteAll = false;
    bool skipAll = false;

    for (const Archive::Entry *file : files) {

        QFileInfo relEntry(file->fullPath().remove(file->rootNode));
        QFileInfo absSourceEntry(QDir::current().absolutePath() + QLatin1Char('/') + file->fullPath());
        QFileInfo absDestEntry(finalDestDir.path() + QLatin1Char('/') + relEntry.filePath());

        if (absSourceEntry.isDir()) {

            // For directories, just create the path.
            if (!finalDestDir.mkpath(relEntry.filePath())) {
                qDebug() << "Failed to create directory" << relEntry.filePath() << "in final destination.";
            }
        } else {

            // If destination file exists, prompt the user.
            if (absDestEntry.exists()) {
                qDebug() << "File" << absDestEntry.absoluteFilePath() << "exists.";

                if (!skipAll && !overwriteAll) {

                    OverwriteQuery query(absDestEntry.absoluteFilePath());
                    query.setNoRenameMode(true);
                    //query.execute();
                    emit userQuery(&query);
                    query.waitForResponse();

                    if (query.responseOverwrite() || query.responseOverwriteAll()) {
                        if (query.responseOverwriteAll()) {
                            overwriteAll = true;
                        }
                        if (!QFile::remove(absDestEntry.absoluteFilePath())) {
                            qDebug() << "Failed to remove" << absDestEntry.absoluteFilePath();
                        }
                    } else if (query.responseSkip() || query.responseAutoSkip()) {
                        if (query.responseAutoSkip()) {
                            skipAll = true;
                        }
                        continue;
                    } else if (query.responseCancelled()) {
                        emit cancelled();
                        emit finished(true);
                        return false;
                    }
                } else if (skipAll) {
                    continue;
                } else if (overwriteAll) {
                    if (!QFile::remove(absDestEntry.absoluteFilePath())) {
                        qDebug() << "Failed to remove" << absDestEntry.absoluteFilePath();
                    }
                }
            }

            // Create any parent directories.
            if (!finalDestDir.mkpath(relEntry.path())) {
                qDebug() << "Failed to create parent directory for file:" << absDestEntry.filePath();
            }

            // Move files to the final destination.
            if (!QFile(absSourceEntry.absoluteFilePath()).rename(absDestEntry.absoluteFilePath())) {
                qDebug() << "Failed to move file" << absSourceEntry.filePath() << "to final destination.";
                //emit error(tr("Could not move the extracted file to the destination directory."));
                emit error("Could not move the extracted file to the destination directory.");
                emit finished(false);
                return false;
            }
        }
    }
    return true;
}

bool CliInterface::isEmptyDir(const QDir &dir)
{
    QDir d = dir;
    d.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    return d.count() == 0;
}

void CliInterface::cleanUpExtracting()
{
    restoreWorkingDirExtraction();
    m_extractTempDir.reset();
}

void CliInterface::restoreWorkingDirExtraction()
{
    if (m_oldWorkingDirExtraction.isEmpty()) {
        return;
    }

    if (!QDir::setCurrent(m_oldWorkingDirExtraction)) {
        qDebug() << "Failed to restore old working directory:" << m_oldWorkingDirExtraction;
    } else {
        m_oldWorkingDirExtraction.clear();
    }
}

void CliInterface::finishCopying(bool result)
{
    disconnect(this, &CliInterface::finished, this, &CliInterface::continueCopying);
    emit progress(1.0);
    emit finished(result);
    cleanUp();
}

bool CliInterface::moveToDestination(const QDir &tempDir, const QDir &destDir, bool preservePaths)
{
    qDebug() << "Moving extracted files from temp dir" << tempDir.path() << "to final destination" << destDir.path();

    bool overwriteAll = false;
    bool skipAll = false;

    QDirIterator dirIt(
        tempDir.path(), QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        dirIt.next();

        // We skip directories if:
        // 1. We are not preserving paths
        // 2. The dir is not empty. Only empty directories need to be explicitly moved.
        // The non-empty ones are created by QDir::mkpath() below.
        if (dirIt.fileInfo().isDir()) {
            if (!preservePaths || !isEmptyDir(QDir(dirIt.filePath()))) {
                continue;
            }
        }

        QFileInfo relEntry;
        if (preservePaths) {
            relEntry = QFileInfo(dirIt.filePath().remove(tempDir.path() + QLatin1Char('/')));
        } else {
            relEntry = QFileInfo(dirIt.fileName());
        }

        QFileInfo absDestEntry(destDir.path() + QLatin1Char('/') + relEntry.filePath());

        if (absDestEntry.exists()) {
            qDebug() << "File" << absDestEntry.absoluteFilePath() << "exists.";

            OverwriteQuery query(absDestEntry.absoluteFilePath());
            query.setNoRenameMode(true);
            //query.execute();
            emit userQuery(&query);
            query.waitForResponse();

            if (query.responseOverwrite() || query.responseOverwriteAll()) {
                if (query.responseOverwriteAll()) {
                    overwriteAll = true;
                }
                if (!QFile::remove(absDestEntry.absoluteFilePath())) {
                    qDebug() << "Failed to remove" << absDestEntry.absoluteFilePath();
                }
            } else if (query.responseSkip() || query.responseAutoSkip()) {
                if (query.responseAutoSkip()) {
                    skipAll = true;
                }
                continue;
            } else if (query.responseCancelled()) {
                qDebug() << "Copy action cancelled.";
                return false;
            }
        } else if (skipAll) {
            continue;
        } else if (overwriteAll) {
            if (!QFile::remove(absDestEntry.absoluteFilePath())) {
                qDebug() << "Failed to remove" << absDestEntry.absoluteFilePath();
            }
        }

        if (preservePaths) {
            // Create any parent directories.
            if (!destDir.mkpath(relEntry.path())) {
                qDebug() << "Failed to create parent directory for file:" << absDestEntry.filePath();
            }
        }

        // Move file to the final destination.
        if (!QFile(dirIt.filePath()).rename(absDestEntry.absoluteFilePath())) {
            qDebug() << "Failed to move file" << dirIt.filePath() << "to final destination.";
            return false;
        }
    }

    return true;
}

void CliInterface::setNewMovedFiles(const QVector< Archive::Entry * > &entries, const Archive::Entry *destination,
                                    int entriesWithoutChildren)
{
    m_newMovedFiles.clear();
    QMap< QString, const Archive::Entry * > entryMap;
    for (const Archive::Entry *entry : entries) {
        entryMap.insert(entry->fullPath(), entry);
    }

    QString lastFolder;

    QString newPath;
    int nameLength = 0;
    for (const Archive::Entry *entry : qAsConst(entryMap)) {
        if (lastFolder.count() > 0 && entry->fullPath().startsWith(lastFolder)) {
            // Replace last moved or copied folder path with destination path.
            int charsCount = entry->fullPath().count() - lastFolder.count();
            if (entriesWithoutChildren > 1) {
                charsCount += nameLength;
            }
            newPath = destination->fullPath() + entry->fullPath().right(charsCount);
        } else {
            if (entriesWithoutChildren > 1) {
                newPath = destination->fullPath() + entry->name();
            } else {
                // If there is only one passed file in the list,
                // we have to use destination as newPath.
                newPath = destination->fullPath(NoTrailingSlash);
            }
            if (entry->isDir()) {
                newPath += QLatin1Char('/');
                nameLength = entry->name().count() + 1;  // plus slash
                lastFolder = entry->fullPath();
            } else {
                nameLength = 0;
                lastFolder = QString();
            }
        }
        Archive::Entry *newEntry = new Archive::Entry(nullptr);
        newEntry->copyMetaData(entry);
        newEntry->setFullPath(newPath);
        m_newMovedFiles << newEntry;
    }
}

QStringList CliInterface::extractFilesList(const QVector< Archive::Entry * > &entries) const
{
    QStringList filesList;
    for (const Archive::Entry *e : entries) {
        filesList << escapeFileName(e->fullPath(NoTrailingSlash));
    }

    return filesList;
}

void CliInterface::killProcess(bool emitFinished)
{
    // TODO: Would be good to unit test #304764/#304178.

    if (!m_process) {
        return;
    }

    m_abortingOperation = !emitFinished;

    m_process->kill();
    //    // Give some time for the application to finish gracefully
    //    if (!m_process->waitForFinished(5)) {
    //        m_process->kill();

    //        // It takes a few hundred ms for the process to be killed.
    //        m_process->waitForFinished(1000);
    //    }

    m_abortingOperation = false;
}

bool CliInterface::passwordQuery()
{
    PasswordNeededQuery query(filename());
    //query.execute();
    emit userQuery(&query);
    query.waitForResponse();

    if (query.responseCancelled()) {
        emit cancelled();
        // There is no process running, so finished() must be emitted manually.
        emit finished(false);
        return false;
    }

    setPassword(query.password());
    return true;
}

void CliInterface::cleanUp()
{
    qDeleteAll(m_tempAddedFiles);
    m_tempAddedFiles.clear();
    QDir::setCurrent(m_oldWorkingDir);
    m_tempWorkingDir.reset();
    m_tempAddDir.reset();
}

void CliInterface::deleteProcess()
{
    if (m_process) {
        //handle all the remaining data in the process
        readStdout(true);

        delete m_process;
        m_process = nullptr;
    }

}

void CliInterface::readStdout(bool handleAll)
{
    // when hacking this function, please remember the following:
    //- standard output comes in unpredictable chunks, this is why
    // you can never know if the last part of the output is a complete line or not
    //- console applications are not really consistent about what
    // characters they send out (newline, backspace, carriage return,
    // etc), so keep in mind that this function is supposed to handle
    // all those special cases and be the lowest common denominator

    if (m_abortingOperation)
        return;

    Q_ASSERT(m_process);

    if (!m_process->bytesAvailable()) {
        // if process has no more data, we can just bail out
        return;
    }

    QByteArray dd = m_process->readAllStandardOutput();
    m_stdOutData += dd;

    QList< QByteArray > lines = m_stdOutData.split('\n');
    // for (const QByteArray &line : qAsConst(lines)) {
    //     if (List == m_operationMode) {
    //         qDebug() << line;
    //     }
    // }

    // The reason for this check is that archivers often do not end
    // queries (such as file exists, wrong password) on a new line, but
    // freeze waiting for input. So we check for errors on the last line in
    // all cases.
    // TODO: QLatin1String() might not be the best choice here.
    //       The call to handleLine() at the end of the method uses
    //       QString::fromLocal8Bit(), for example.
    // TODO: The same check methods are called in handleLine(), this
    //       is suboptimal.

    bool wrongPasswordMessage = isWrongPasswordMsg(QLatin1String(lines.last()));

    if (m_process->program().length() > 2) {
        if ((m_process->program().at(0).contains("7z") && m_process->program().at(1) != "l") && !wrongPasswordMessage) {
            handleAll = true;  // 7z output has no \n
        }
        if ((m_process->program().at(0).contains("bash") && m_process->program().at(2).contains("7z")) && !wrongPasswordMessage) {
            handleAll = true;  // compress .tar.7z output has no \n
        }
    }

    bool foundErrorMessage = (wrongPasswordMessage || isDiskFullMsg(QLatin1String(lines.last()))
                              || isFileExistsMsg(QLatin1String(lines.last())))
                             || isPasswordPrompt(QLatin1String(lines.last()));

    if (foundErrorMessage) {
        handleAll = true;
    }

    if (wrongPasswordMessage) {
        setPassword(QString());
        if (m_extractionOptions.isBatchExtract() && !m_isBatchExtractWrongPsd) {
            m_isBatchExtractWrongPsd = true;
            //批量解压密码错误时，不直接结束取消，后面要继续提示输入密码
//            emit cancelled();
//            // There is no process running, so finished() must be emitted manually.
//            emit finished(false);
            return;
        }
    }

    // this is complex, here's an explanation:
    // if there is no newline, then there is no guaranteed full line to
    // handle in the output. The exception is that it is supposed to handle
    // all the data, OR if there's been an error message found in the
    // partial data.
    if (lines.size() == 1 && !handleAll) {
        return;
    }

    if (handleAll) {
        m_stdOutData.clear();
    } else {
        // because the last line might be incomplete we leave it for now
        // note, this last line may be an empty string if the stdoutdata ends
        // with a newline
        m_stdOutData = lines.takeLast();
    }

    for (const QByteArray &line : qAsConst(lines)) {
        if (!line.isEmpty() || (m_listEmptyLines && m_operationMode == List)) {
            if (!handleLine(QString::fromLocal8Bit(line))) {
                killProcess();
                return;
            }
        }
    }
}

bool CliInterface::setAddedFiles()
{
    QDir::setCurrent(m_tempAddDir->path());
    for (const Archive::Entry *file : qAsConst(m_passedFiles)) {
        const QString oldPath = m_tempWorkingDir->path() + QLatin1Char('/') + file->fullPath(NoTrailingSlash);
        const QString newPath = m_tempAddDir->path() + QLatin1Char('/') + file->name();
        if (!QFile::rename(oldPath, newPath)) {
            return false;
        }
        m_tempAddedFiles << new Archive::Entry(nullptr, file->name());
    }
    return true;
}

void CliInterface::emitProgress(float value)
{
    if (this->pAnalyseHelp == nullptr) {
        emit progress(static_cast<double>(value));
    }
}

void CliInterface::emitFileName(QString name)
{
    if (this->pAnalyseHelp == nullptr) {
        emit progress_filename(name);
    }
}

QString CliInterface::getFileName(int percent)
{
    if (percent > 100) {
        percent = 100;
    }
    if (percent <= 0) {
        percent = 0;
    }

    if (m_operationMode == ReadWriteArchiveInterface::Delete) {
        int lenV = m_removedFiles.length();
        double cell = 100 * 1.0 / lenV;
        int index = static_cast<int>(percent / cell);
        index = index > (lenV - 1)  ? (lenV - 1) : index;
        return m_removedFiles[index]->name();
    } else if (m_operationMode == ReadWriteArchiveInterface::Add) {
        int lenV = m_addFiles.length();
        double cell = 100 * 1.0 / lenV;
        int index = static_cast<int>(percent / cell);
        index = index > (lenV - 1) ? (lenV - 1) : index;
        return m_addFiles[index]->name();
    }
    return "";
}

bool CliInterface::handleLine(const QString &line)
{
    // TODO: This should be implemented by each plugin; the way progress is
    //       shown by each CLI application is subject to a lot of variation.

    //qDebug() << "#####" << line;

    if (pAnalyseHelp != nullptr) {
        pAnalyseHelp->analyseLine(line);
        if (pAnalyseHelp->isNotKnown() == true) {
            this->extractPsdStatus = Reextract;
            return false;
        }
    }

    if (pAnalyseHelp != nullptr) {
        //        添加!m_isbatchlist条件可解决，批量解压时强制kill掉该解压进程会多次提示输入正确密码，
        if (pAnalyseHelp->isRightPsd() == 1 && !m_isbatchlist) {
            //            qDebug() << "%%%%%%RightPassword";
            this->extractPsdStatus = Reextract;
            return false;
        }
    }

    if ((m_operationMode == Extract || m_operationMode == Add) && m_cliProps->property("captureProgress").toBool()) {
        // read the percentage
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            emitProgress(float(percentage) / 100);
            if (line.contains("Extracting")) {
                QStringRef strfilename = line.midRef(12, pos - 24);
                emitFileName(strfilename.toString());
            }

            return true;
        }
    }

    if ((m_operationMode == Extract || m_operationMode == Add) && m_process
            && (m_process->program().at(0).contains("zip"))) {
        // read the percentage
        int pos = line.indexOf(QLatin1Char(':'));
        if (pos > 1 && line.length() > 17) {
            m_curfilenumber++;
            emitProgress(float(m_curfilenumber) / m_allfilenumber);
            QStringRef strfilename = line.midRef(pos + 2, line.length() - 24);
            emitFileName(strfilename.toString());
            return true;
        }
    } else if (m_process && m_process->program().at(0).contains("7z") && !isWrongPasswordMsg(line)) {
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            if (percentage > 0 && percentage != 46) {
                if (line.contains(OneBBBB) == true) {
                    QStringRef strfilename;
                    if (m_operationMode == ReadWriteArchiveInterface::Delete) {//如果是删除

                        QString filename = getFileName(percentage);
                        if (!strfilename.toString().contains("Wrong password")) {
                            if (percentage > 0) {
                                emitProgress(float(percentage) / 100);
                                //qDebug() << "delete..." << filename;
                                emitFileName(filename);
                            }
                        }
                    } else if (m_operationMode == ReadWriteArchiveInterface::Add) {
                        QString filename = getFileName(percentage);
                        if (!strfilename.toString().contains("Wrong password")) {
                            if (percentage > 0) {
                                emitProgress(float(percentage) / 100);
                                //qDebug() << "add..." << filename;
                                emitFileName(filename);
                            }
                        }
                    } else {
                        int count = line.indexOf("+");
                        if (-1 == count) {
                            count = line.indexOf("-");
                        }
                        if (count > 0) {
                            strfilename = line.midRef(count + 2);
                        }
                        if (!strfilename.toString().contains("Wrong password")) {
                            if (percentage > 0) {
                                emitProgress(float(percentage) / 100);
                                emitFileName(strfilename.toString());
                            }
                        }
                    }
                }
            }
        }
    } else if (m_process && m_process->program().at(0).contains("bash") && !isWrongPasswordMsg(line)) {
//      压缩tar.7z输出:  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b306M 1 + [Content]
        int pos = line.lastIndexOf(" + [Content]");
        if (pos > 1) {
            const QString bstr = "\b\b\b\b\b\b";
            const int bstrLength = bstr.length();

            int pos2 = line.lastIndexOf("M ");
            int pos1 = line.lastIndexOf(bstr, pos2);
            qint64 compressedSize = line.midRef(pos1 + bstrLength, pos2 - (pos1 + bstrLength)).toLongLong();
            float percentage = compressedSize / float(m_filesSize);
            if (percentage > 0) {
//                qDebug() << compressedSize << m_filesSize << percentage;
                if (percentage > 1) {
                    emitProgress(0.999);
                } else {
                    emitProgress(percentage);
                }
//                emitFileName(strfilename.toString());//TODO
            }
        }
    }

    if (m_operationMode == Extract) {

        //        if (isPasswordPrompt(line)) {
        //            qDebug() << "Found a password prompt";

        //            PasswordNeededQuery query(filename());
        //            query.execute();

        //            if (query.responseCancelled()) {
        //                emit cancelled();
        //                return false;
        //            }

        //            setPassword(query.password());

        //            const QString response(password() + QLatin1Char('\n'));
        //            writeToProcess(response.toLocal8Bit());

        //            return true;
        //        }

        if (isDiskFullMsg(line)) {
            qDebug() << "Found disk full message:" << line;
            QString message = "@info";
            size_t length = strlen(message.toUtf8().data());
            char *cMsg = static_cast<char *>(malloc((length + 1) * sizeof(char)));
            strcpy(cMsg, message.toUtf8().data());
            emit error(cMsg, "Extraction failed because the disk is full.");
            free(cMsg);
            return false;
        }

        if (handleFileExistsMessage(line)) {
            ifReplaceTip = true;
            return true;
        }

        if (isWrongPasswordMsg(line)) {
            setPassword(QString());
            if (m_extractionOptions.isBatchExtract()) {
            } else {
                if (this->extractPsdStatus != ReadOnlyArchiveInterface::WrongPsd) {
                    if (pAnalyseHelp != nullptr) {
                        pAnalyseHelp->mark(ENUMLINEINFO::WRONGPSD, line, true);
                    }
                    this->extractPsdStatus = ReadOnlyArchiveInterface::WrongPsd;
//                    qDebug() << "$$$$$WrongPassword";
                    emit sigExtractNeedPassword();
                }

                return false;
            }
        }

        return readExtractLine(line);
    } else if (m_operationMode == List) {
        if (isPasswordPrompt(line)) {
            m_isPasswordPrompt = true;
            qDebug() << "Found a password prompt" << m_isbatchlist;

            if (m_isbatchlist) {
                PasswordNeededQuery query(filename());
                //query.execute();
                emit userQuery(&query);
                query.waitForResponse();

                if (query.responseCancelled()) {
                    emit error("Canceal when batchextract.");
                    emit cancelled();
                    // There is no process running, so finished() must be emitted manually.
                    emit finished(false);
                    return false;
                }

                setPassword(query.password());

                const QString response(password() + QLatin1Char('\n'));
                writeToProcess(response.toLocal8Bit());
            } else {
                emit sigExtractNeedPassword();
                emit error("nopassword");
                return false;
            }
        }

        if (isWrongPasswordMsg(line)) {
            qDebug() << "Wrong password";
            if (m_isbatchlist) {
            } else {
                setPassword(QString());
            }
            setWrongPassword(true);
            emit error("wrong password");
            return false;
        }

        if (isCorruptArchiveMsg(line)) {
            if (isWrongPassword()) {
                return true;
            }

            qDebug() << "Archive corrupt";
            setCorrupt(true);
            // Special case: corrupt is not a "fatal" error so we return true here.
            return true;
        }

        static bool fisrtPath = false;
        if (line.contains("----------")) {
            fisrtPath = true;
        }

        if (line.startsWith("Path =")) {
            if (fisrtPath) {
                fisrtPath = false;
                QString folder = line;
                extractDst7z_ = folder.remove("Path = ");
            } else if (extractDst7z_.isEmpty() == false) {
                QString folder = line;
                folder.remove("Path = ");

                if (folder.startsWith(extractDst7z_ + (extractDst7z_.endsWith("/") ? "" : "/")) == false) {
                    extractDst7z_.clear();
                }
            }
        }

        if (line.startsWith("        Name: ")) {
            QString folder = line;
            extractDst7z_ = folder.remove("        Name: ");
        }


        return readListLine(line);

    }


    if (m_operationMode == Delete) {
        return readDeleteLine(line);
    }

    if (m_operationMode == Test) {

        if (isPasswordPrompt(line)) {
            qDebug() << "Found a password prompt";
            return false;
        }

        if (m_cliProps->isTestPassedMsg(line)) {
            qDebug() << "Test successful";
            emit testSuccess();
            return true;
        }
    }

    return true;
}

bool CliInterface::readDeleteLine(const QString &line)
{
    Q_UNUSED(line);
    return true;
}

bool CliInterface::handleFileExistsMessage(const QString &line)
{
    // Check for a filename and store it.
    if (isFileExistsFileName(line)) {
        const QStringList fileExistsFileNameRegExp = m_cliProps->property("fileExistsFileNameRegExp").toStringList();
        for (const QString &pattern : fileExistsFileNameRegExp) {
            const QRegularExpression rxFileNamePattern(pattern);
            const QRegularExpressionMatch rxMatch = rxFileNamePattern.match(line);

            if (rxMatch.hasMatch()) {
                m_storedFileName = rxMatch.captured(1);
                this->extractTopFolderName = m_storedFileName;
                qDebug() << "Detected existing file:" << m_storedFileName;
            }
        }
    }

    if (!isFileExistsMsg(line)) {
        return false;
    }

    OverwriteQuery query(QDir::current().path() + QLatin1Char('/') + m_storedFileName);
    query.setNoRenameMode(true);
    //query.execute();
    emit userQuery(&query);
    query.waitForResponse();

    QString responseToProcess;
    const QStringList choices = m_cliProps->property("fileExistsInput").toStringList();

    if (query.responseOverwrite()) {
        responseToProcess = choices.at(0);
    } else if (query.responseSkip()) {
        responseToProcess = choices.at(1);
    } else if (query.responseOverwriteAll()) {
        responseToProcess = choices.at(2);
    } else if (query.responseAutoSkip()) {
        responseToProcess = choices.at(3);
    } else if (query.responseCancelled()) {
        userCancel = true;
        emit cancelled();
        if (choices.count() < 5) {
            // If the program has no way to cancel the extraction, we resort to killing it
            return doKill();
        }
        responseToProcess = choices.at(4);
    }

    Q_ASSERT(!responseToProcess.isEmpty());

    responseToProcess += QLatin1Char('\n');

    writeToProcess(responseToProcess.toLocal8Bit());

    return true;
}

bool CliInterface::doKill()
{
    if (m_extractTempDir && m_extractTempDir->isValid()) {
        m_extractTempDir->remove();
    }

    if (m_process) {
        killProcess(false);
        return true;
    }

    return false;
}

QString CliInterface::escapeFileName(const QString &fileName) const
{
    return fileName;
}

QStringList CliInterface::entryPathDestinationPairs(const QVector< Archive::Entry * > &entriesWithoutChildren,
                                                    const Archive::Entry *destination)
{
    QStringList pairList;
    if (entriesWithoutChildren.count() > 1) {
        for (const Archive::Entry *file : entriesWithoutChildren) {
            pairList << file->fullPath(NoTrailingSlash) << destination->fullPath() + file->name();
        }
    } else {
        pairList << entriesWithoutChildren.at(0)->fullPath(NoTrailingSlash) << destination->fullPath(NoTrailingSlash);
    }
    return pairList;
}

void CliInterface::writeToProcess(const QByteArray &data)
{
    Q_ASSERT(m_process);
    Q_ASSERT(!data.isNull());

    qDebug() << "Writing" << data << "to the process";

    m_process->write(data);
}

bool CliInterface::addComment(const QString &comment)
{
    m_operationMode = Comment;

    m_commentTempFile.reset(new QTemporaryFile());
    if (!m_commentTempFile->open()) {
        qDebug() << "Failed to create temporary file for comment";
        emit finished(false);
        return false;
    }

    QTextStream stream(m_commentTempFile.data());
    stream << comment << endl;
    m_commentTempFile->close();

    if (!runProcess(m_cliProps->property("addProgram").toString(),
                    m_cliProps->commentArgs(filename(), m_commentTempFile->fileName()))) {
        return false;
    }
    m_comment = comment;
    return true;
}

QString CliInterface::multiVolumeName() const
{
    QString oldSuffix = QMimeDatabase().suffixForFileName(filename());
    QString name;

    const QStringList multiVolumeSuffix = m_cliProps->property("multiVolumeSuffix").toStringList();
    for (const QString &multiSuffix : multiVolumeSuffix) {
        QString newSuffix = multiSuffix;
        newSuffix.replace(QStringLiteral("$Suffix"), oldSuffix);
        name = filename().remove(oldSuffix).append(newSuffix);
        if (QFileInfo::exists(name)) {
            break;
        }
    }
    return name;
}

CliProperties *CliInterface::cliProperties() const
{
    return m_cliProps;
}

void CliInterface::onEntry(Archive::Entry *archiveEntry)
{
    if (archiveEntry->compressedSizeIsSet) {
        m_listedSize += archiveEntry->property("compressedSize").toULongLong();
        if (m_listedSize <= m_archiveSizeOnDisk) {
            emit progress(static_cast<double>(m_listedSize * 1.0 / m_archiveSizeOnDisk));
        } else {
            // In case summed compressed size exceeds archive size on disk.
            emit progress(1.0);
        }
    }
}

bool CliInterface::isPasswordPrompt(const QString &line)
{
    Q_UNUSED(line)
    return false;
}

bool CliInterface::isWrongPasswordMsg(const QString &line)
{
    Q_UNUSED(line)
    return false;
}

bool CliInterface::isCorruptArchiveMsg(const QString &line)
{
    Q_UNUSED(line);
    return false;
}

bool CliInterface::isDiskFullMsg(const QString &line)
{
    Q_UNUSED(line);
    return false;
}

bool CliInterface::isFileExistsMsg(const QString &line)
{
    Q_UNUSED(line);
    return false;
}

bool CliInterface::isFileExistsFileName(const QString &line)
{
    Q_UNUSED(line);
    return false;
}
