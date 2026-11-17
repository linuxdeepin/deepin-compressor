// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cli_runner.h"

#include "common.h"

#include <limits.h>

#include <QTemporaryDir>

#include "datamanager.h"
#include "queries.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFile>
#include <QFileInfo>
#include <QSocketNotifier>
#include <QStandardPaths>
#include <QTimer>

#include <csignal>
#include <fcntl.h>
#include <pty.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <vector>

namespace {

// 单个命令的输出上限，避免异常输出把内存吃光
constexpr int kMaxOutputBytes = 8 * 1024 * 1024;

} // namespace

CliRunner::CliRunner(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
    // 命令行后端都是异步的：结果通过 signalFinished 回来
    setWaitForFinishedSignal(true);
}

CliRunner::~CliRunner()
{
    stopChild();
    closePty();
}

bool CliRunner::openPty()
{
    closePty();

    int master = -1;
    int slave = -1;
    if (openpty(&master, &slave, nullptr, nullptr, nullptr) != 0)
        return false;

    // 主设备不能被 exec 继承下去
    ::fcntl(master, F_SETFD, FD_CLOEXEC);

    // 关掉回显：写进去的密码不应该再出现在输出里
    termios term {};
    if (tcgetattr(slave, &term) == 0) {
        term.c_lflag &= ~(ECHO | ECHONL);
        tcsetattr(slave, TCSANOW, &term);
    }

    m_ptyMaster = master;
    m_ptySlave = slave;
    return true;
}

void CliRunner::closePty()
{
    if (m_outFd >= 0) {
        ::close(m_outFd);
        m_outFd = -1;
    }
    if (m_ptySlave >= 0) {
        ::close(m_ptySlave);
        m_ptySlave = -1;
    }
    if (m_ptyMaster >= 0) {
        ::close(m_ptyMaster);
        m_ptyMaster = -1;
    }
}

qint64 CliRunner::readPty(char *buffer, qint64 size)
{
    // 输出走管道，只有 stdin 挂在伪终端上（与旧实现的 StdinChannel 一致）
    if (m_outFd < 0)
        return -1;
    return ::read(m_outFd, buffer, size_t(size));
}

bool CliRunner::startChild(const QString &programPath, const QStringList &arguments)
{
    if (m_ptySlave < 0)
        return false;

    // 参数按数组交给 execv，不经过 shell，杜绝命令注入
    QByteArray program = QFile::encodeName(programPath);
    std::vector<QByteArray> storage;
    storage.reserve(size_t(arguments.size()) + 1);
    storage.push_back(program);
    for (const QString &argument : arguments)
        storage.push_back(argument.toLocal8Bit());

    std::vector<char *> argv;
    argv.reserve(storage.size() + 1);
    for (QByteArray &item : storage)
        argv.push_back(item.data());
    argv.push_back(nullptr);

    const int slave = m_ptySlave;
    const int master = m_ptyMaster;

    int outPipe[2] = {-1, -1};
    if (::pipe(outPipe) != 0)
        return false;

    const pid_t pid = ::fork();
    if (pid < 0)
        return false;

    if (pid == 0) {
        // 子进程里只做 async-signal-safe 的事
        ::setsid();

        // 只有解压需要伪终端 stdin（密码提示）；其它操作接 /dev/null，
        // 否则 7z 会把 stdin 当交互输入，删除/压缩时静默等待确认
        if (m_workStatus == WT_Extract) {
            ::ioctl(slave, TIOCSCTTY, 0);
            ::dup2(slave, STDIN_FILENO);
        } else {
            const int nullFd = ::open("/dev/null", O_RDONLY);
            if (nullFd >= 0) {
                ::dup2(nullFd, STDIN_FILENO);
                if (nullFd > STDERR_FILENO)
                    ::close(nullFd);
            }
        }
        ::dup2(outPipe[1], STDOUT_FILENO);
        ::dup2(outPipe[1], STDERR_FILENO);
        if (slave > STDERR_FILENO)
            ::close(slave);
        ::close(master);
        ::close(outPipe[0]);
        ::close(outPipe[1]);
        // 绝对路径执行，不做 PATH 查找
        ::execv(program.constData(), argv.data());
        ::_exit(127);
    }

    m_pid = pid;
    m_childRunning = true;
    m_exitCode = -1;

    // 父进程释放从设备和写端，才能读到 EOF
    ::close(m_ptySlave);
    m_ptySlave = -1;
    ::close(outPipe[1]);

    m_outFd = outPipe[0];
    ::fcntl(m_outFd, F_SETFD, FD_CLOEXEC);

    const int flags = ::fcntl(m_outFd, F_GETFL, 0);
    ::fcntl(m_outFd, F_SETFL, flags | O_NONBLOCK);

    return true;
}

bool CliRunner::startPipeline(const QString &program1, const QStringList &arguments1,
                              const QString &program2, const QStringList &arguments2)
{
    if (m_ptySlave < 0)
        return false;

    const QString path1 = QStandardPaths::findExecutable(program1);
    const QString path2 = QStandardPaths::findExecutable(program2);
    if (path1.isEmpty() || path2.isEmpty())
        return false;

    const QByteArray prog1 = QFile::encodeName(path1);
    const QByteArray prog2 = QFile::encodeName(path2);

    auto buildArgv = [](const QStringList &list, QByteArray program,
                        std::vector<QByteArray> &storage, std::vector<char *> &argv) {
        storage.clear();
        storage.push_back(program);
        for (const QString &item : list)
            storage.push_back(item.toLocal8Bit());
        argv.clear();
        for (QByteArray &item : storage)
            argv.push_back(item.data());
        argv.push_back(nullptr);
    };

    std::vector<QByteArray> storage1;
    std::vector<char *> argv1;
    buildArgv(arguments1, prog1, storage1, argv1);

    std::vector<QByteArray> storage2;
    std::vector<char *> argv2;
    buildArgv(arguments2, prog2, storage2, argv2);

    int inPipe[2] = {-1, -1};
    int outPipe[2] = {-1, -1};
    if (::pipe(inPipe) != 0)
        return false;
    if (::pipe(outPipe) != 0) {
        ::close(inPipe[0]);
        ::close(inPipe[1]);
        return false;
    }

    // 上游：输出进管道，stdin 用 /dev/null
    const pid_t upstream = ::fork();
    if (upstream == 0) {
        ::setsid();
        const int devNull = ::open("/dev/null", O_RDONLY);
        if (devNull >= 0)
            ::dup2(devNull, STDIN_FILENO);
        ::dup2(inPipe[1], STDOUT_FILENO);
        ::close(inPipe[0]);
        ::close(inPipe[1]);
        ::close(outPipe[0]);
        ::close(outPipe[1]);
        ::execv(prog1.constData(), argv1.data());
        ::_exit(127);
    }

    // 下游：从管道读，输出进 outPipe
    const pid_t downstream = ::fork();
    if (downstream == 0) {
        ::setsid();
        ::dup2(inPipe[0], STDIN_FILENO);
        ::dup2(outPipe[1], STDOUT_FILENO);
        ::dup2(outPipe[1], STDERR_FILENO);
        ::close(inPipe[0]);
        ::close(inPipe[1]);
        ::close(outPipe[0]);
        ::close(outPipe[1]);
        ::execv(prog2.constData(), argv2.data());
        ::_exit(127);
    }

    ::close(inPipe[0]);
    ::close(inPipe[1]);
    ::close(outPipe[1]);

    if (m_ptySlave >= 0) {
        ::close(m_ptySlave);
        m_ptySlave = -1;
    }

    m_outFd = outPipe[0];
    ::fcntl(m_outFd, F_SETFD, FD_CLOEXEC);
    const int flags = ::fcntl(m_outFd, F_GETFL, 0);
    ::fcntl(m_outFd, F_SETFL, flags | O_NONBLOCK);

    m_pid = downstream;
    m_pid2 = upstream;
    m_childRunning = true;
    m_exitCode = -1;
    return true;
}

bool CliRunner::childExited()
{
    if (m_pid <= 0)
        return false;

    int status = 0;
    const pid_t result = ::waitpid(pid_t(m_pid), &status, WNOHANG);
    if (result != m_pid)
        return false;

    m_exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    m_pid = -1;
    m_childRunning = false;
    return true;
}

void CliRunner::reapChild()
{
    if (m_pid <= 0)
        return;

    int status = 0;
    for (int i = 0; i < 150; ++i) {
        if (::waitpid(pid_t(m_pid), &status, WNOHANG) == m_pid) {
            m_exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            break;
        }
        ::usleep(20 * 1000);
    }

    m_pid = -1;
    m_childRunning = false;
}

void CliRunner::stopChild(bool hard)
{
    if (m_pid2 > 0) {
        ::kill(pid_t(-m_pid2), SIGKILL);
        ::kill(pid_t(m_pid2), SIGKILL);
        int status2 = 0;
        ::waitpid(pid_t(m_pid2), &status2, WNOHANG);
        m_pid2 = -1;
    }

    if (m_pid > 0) {
        // 子进程是会话首进程，按进程组清理；解压/等密码必须强杀，其余用 SIGTERM 让它自清理临时文件
        const int sig = hard ? SIGKILL : SIGTERM;
        ::kill(pid_t(-m_pid), SIGCONT);
        ::kill(pid_t(-m_pid), sig);
        ::kill(pid_t(m_pid), sig);

        // SIGTERM 不生效时再强杀
        int status = 0;
        bool finished = false;
        for (int i = 0; i < 50; ++i) {
            if (::waitpid(pid_t(m_pid), &status, WNOHANG) == m_pid) {
                finished = true;
                break;
            }
            ::usleep(20 * 1000);
        }
        if (!finished) {
            ::kill(pid_t(-m_pid), SIGKILL);
            ::kill(pid_t(m_pid), SIGKILL);
            reapChild();
        } else {
            m_exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            m_pid = -1;
        }
    }
    m_childRunning = false;
    m_isProcessKilled = true;

    // 异步执行时进程被我们自己结束，也要补发结束信号
    if (m_exitTimer && !m_finishEmitted)
        QTimer::singleShot(0, this, &CliRunner::finishRun);
}

void CliRunner::handleOutput(const QByteArray &chunk)
{
    if (m_stdOutData.size() > kMaxOutputBytes) {
        m_stdOutData.clear();
        return;
    }

    m_stdOutData += QString::fromLocal8Bit(chunk);

    QList<QString> lines = m_stdOutData.split(QLatin1Char('\n'));
    const bool partial = !m_stdOutData.endsWith(QLatin1Char('\n'));
    m_stdOutData = lines.takeLast();

    for (QString line : lines) {
        line.remove(QLatin1Char('\r')); // pty 会把换行变成 CRLF
        if (!handleLine(line, m_workStatus)) {
            killProcess();
            return;
        }
    }

    if (!partial || m_stdOutData.isEmpty())
        return;

    // 密码提示和 7z 的进度行都不带换行
    const bool waiting = isPasswordPrompt(m_stdOutData) || isFileExistsMsg(m_stdOutData);
    if (!waiting && m_workStatus == WT_List)
        return;

    QString line = m_stdOutData;
    m_stdOutData.clear();
    line.remove(QLatin1Char('\r'));

    // 只有列表阶段遇到密码提示才放弃；解压/压缩阶段交给 handleLine 去询问
    if (isPasswordPrompt(line) && m_workStatus == WT_List) {
        DataManager::get_instance().archiveData().isListEncrypted = true;
        m_passwordAborted = true;
        return;
    }

    if (!handleLine(line, m_workStatus))
        killProcess();
}

void CliRunner::finishRun()
{
    if (m_finishEmitted)
        return;
    m_finishEmitted = true;

    if (m_exitTimer) {
        m_exitTimer->stop();
        m_exitTimer->deleteLater();
        m_exitTimer = nullptr;
    }
    if (m_ptyNotifier) {
        m_ptyNotifier->setEnabled(false);
        m_ptyNotifier->deleteLater();
        m_ptyNotifier = nullptr;
    }

    // 收尾：把缓冲区里剩下的数据读完
    char buffer[4096];
    for (;;) {
        const qint64 count = readPty(buffer, sizeof(buffer));
        if (count <= 0)
            break;
        m_stdOutData += QString::fromLocal8Bit(QByteArray(buffer, int(count)));
    }

    closePty();
    m_isProcessKilled = false;

    // 非致命损坏（如分卷缺失）旧实现会询问是否以只读方式打开
    if (m_isCorruptArchive && m_workStatus == WT_List) {
        LoadCorruptQuery query(m_strArchiveName);
        emit signalQuery(&query);
        query.waitForResponse();

        if (query.responseYes()) {
            // 用户选择只读打开：不影响后续操作
            m_finishType = PFT_Nomral;
        } else {
            m_eErrorType = ET_MissingVolume;
            m_finishType = PFT_Cancel;
        }
        m_isCorruptArchive = false;
    }

    if (m_exitCode == 0 && m_finishType != PFT_Error && m_finishType != PFT_Cancel)
        m_finishType = PFT_Nomral;
    else if (m_exitCode != 0 && m_finishType == PFT_Nomral)
        m_finishType = PFT_Error;

    // 整包解压失败（例如密码错误）时清掉残留的空文件
    if (m_workStatus == WT_Extract && m_exitCode != 0 && m_extractOptions.bAllExtract
            && !m_extractOptions.strTargetPath.isEmpty()) {
        removeExtractedFilesOnFailure(m_extractOptions.strTargetPath, {});
    }

    // 部分解压：先解到临时目录，再逐个搬到目标路径
    if (m_workStatus == WT_Extract && !m_extractOptions.bAllExtract && m_exitCode == 0) {
        QList<FileEntry> entries = m_selectedFiles;
        if (entries.isEmpty())
            entries = DataManager::get_instance().archiveData().mapFileEntry.values();
        moveExtractTempFilesToDest(entries, m_extractOptions);
        m_rootNode.clear();
        // 先离开临时目录再删，否则工作目录还在里面，rmdir 会失败留下空壳
        QDir::setCurrent(m_extractOptions.strTargetPath);
        m_extractTempDir.reset();
    }

    emit signalprogress(100);
    emit signalFinished(m_finishType);
}

bool CliRunner::runProcess(const QString &program, const QStringList &arguments)
{
    // 上一个命令还没结束就等它，直接杀会打断调用方紧邻的上一次操作
    for (int i = 0; i < 300 && m_childRunning; ++i) {
        if (childExited())
            break;
        ::usleep(10 * 1000);
    }
    stopChild();
    closePty();

    const QString programPath = QStandardPaths::findExecutable(program);
    const QFileInfo programInfo(programPath);
    if (programPath.isEmpty() || !programInfo.isAbsolute() || !programInfo.isFile()
            || !programInfo.isExecutable()) {
        return false;
    }

    if (!openPty())
        return false;

    m_stdOutData.clear();
    m_isProcessKilled = false;
    m_passwordAborted = false;
    m_finishEmitted = false;
    m_programName = programInfo.fileName();

    if (!startChild(programPath, arguments)) {
        closePty();
        return false;
    }

    attachWatchers();
    return true;
}

bool CliRunner::hasLongName(const QList<FileEntry> &files)
{
    for (const FileEntry &entry : files) {
        if (entry.isDirectory)
            continue;
        if (QFileInfo(entry.strFullPath).fileName().toLocal8Bit().length() > NAME_MAX)
            return true;
    }
    return false;
}

bool CliRunner::handleLongNameExtract(const QList<FileEntry> &files)
{
    // 旧实现的做法：先把归档复制到临时目录，把超长条目在归档内改成截断名，
    // 再一次性解压到目标目录，并标记 ET_LongNameError 让界面知道名字被截断。
    QString password = m_extractOptions.password;
    if (password.isEmpty())
        password = DataManager::get_instance().archiveData().strPassword;

    QTemporaryDir tempDir;
    if (!tempDir.isValid())
        return false;

    const QString archiveCopy = tempDir.path() + QLatin1Char('/') + QFileInfo(m_strArchiveName).fileName();
    if (!QFile::copy(m_strArchiveName, archiveCopy))
        return false;

    QList<FileEntry> renameEntries;
    QStringList normalFiles;

    for (const FileEntry &entry : files) {
        QFileInfo info(entry.strFullPath);
        QString strFilePath = info.path();
        QString strFileName = entry.strFullPath;

        Common common;
        const QString sDir = common.handleLongNameforPath(strFilePath, strFileName, m_mapLongDirName, m_mapRealDirValue);
        if (!sDir.isEmpty()) {
            strFilePath = sDir.endsWith(QLatin1Char('/')) ? sDir.left(sDir.length() - 1) : sDir;
            if (entry.strFullPath.endsWith(QLatin1Char('/')))
                strFileName = sDir;
            else if (NAME_MAX >= info.fileName().toLocal8Bit().length())
                strFileName = sDir + info.fileName();
        }

        const bool needRename = NAME_MAX < info.fileName().toLocal8Bit().length()
                && !entry.strFullPath.endsWith(QLatin1Char('/'));

        if (needRename) {
            const QString strTemp = info.fileName().left(TRUNCATION_FILE_LONG);
            const QString tempFilePathName = strFilePath + QLatin1Char('/') + strTemp;
            if (m_mapLongName[tempFilePathName]++ >= LONGFILE_SAME_FILES) {
                emit signalCurFileName(entry.strFullPath);
                m_eErrorType = ET_LongNameError;
                return false;
            }
            m_eErrorType = ET_LongNameError;
            QString strTempFileName = strTemp
                    + QStringLiteral("(%1)").arg(m_mapLongName[tempFilePathName], LONGFILE_SUFFIX_FieldWidth, 10, QLatin1Char('0'))
                    + QLatin1Char('.') + info.completeSuffix();
            strFileName = strTempFileName;
            if (strFilePath != QLatin1String("."))
                strFileName = strFilePath + QLatin1Char('/') + strTempFileName;
        }

        QString strDestFileName = m_extractOptions.strTargetPath + QLatin1Char('/') + strFileName;
        if (!m_extractOptions.bAllExtract) {
            QString destFileName = strFileName;
            const int count = m_rootEntryCount > 0 ? m_rootEntryCount : 0;
            for (int i = 0; i < count; i++) {
                const int index = destFileName.indexOf(QLatin1Char('/'));
                if (index > 0)
                    destFileName.remove(0, index + 1);
            }
            strDestFileName = m_extractOptions.strTargetPath + QLatin1Char('/') + destFileName;
        }

        if (entry.strFullPath.endsWith(QLatin1Char('/'))) {
            if (!QDir(strDestFileName).exists() && !QDir().mkpath(strDestFileName))
                return false;
        } else {
            FileEntry newEntry = entry;
            newEntry.strAlias = QFileInfo(strFileName).fileName();
            if (needRename)
                renameEntries.append(newEntry);
            else
                normalFiles.append(info.path() != QLatin1String(".")
                                   ? info.path() + QLatin1Char('/') + newEntry.strAlias
                                   : newEntry.strAlias);
        }
    }

    // 归档内改名
    for (const FileEntry &entry : renameEntries) {
        const QStringList moveArgs = argumentsFor(WT_Move, archiveCopy,
                                                 {entry.strFullPath, entry.strAlias}, nullptr);
        if (!runProcessSync(programFor(WT_Move), moveArgs))
            return false;
    }

    QStringList allFiles;
    for (const FileEntry &entry : renameEntries) {
        QFileInfo info(entry.strFullPath);
        allFiles.append(info.path() != QLatin1String(".")
                        ? info.path() + QLatin1Char('/') + entry.strAlias
                        : entry.strAlias);
    }
    allFiles.append(normalFiles);

    if (allFiles.isEmpty())
        return true;

    QStringList extractArgs = argumentsFor(WT_Extract, archiveCopy, allFiles, nullptr);
    extractArgs << QStringLiteral("-o%1").arg(m_extractOptions.strTargetPath);
    if (!runProcessSync(programFor(WT_Extract), extractArgs))
        return false;

    return true;
}

bool CliRunner::checkMoveCapability() const
{
    // 旧实现按 CLI 属性里的 moveProgram 判断，7z 对应 `rn`，可用即可做长名/搬移处理
    const QString program = programFor(WT_Move);
    return !program.isEmpty() && !QStandardPaths::findExecutable(program).isEmpty();
}

bool CliRunner::moveExtractTempFilesToDest(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    // 部分解压结束时把文件从临时目录搬到目标路径。已由 semantic.py 的
    // 7z.extract.partial 覆盖（落盘文件名/内容/无残留临时目录）。
    QDir finalDestDir(options.strTargetPath);
    bool overwriteAll = false;
    bool skipAll = false;
    bool moveSuccess = true;

    for (const FileEntry &entry : files) {
        QString path = entry.strFullPath;
        QFileInfo extractEntry(path.remove(0, m_rootNode.length()));
        QFileInfo extractEntryTemp(QDir::current().absolutePath() + QLatin1Char('/') + entry.strFullPath);
        QFileInfo extractEntryDest(finalDestDir.path() + QLatin1Char('/') + extractEntry.filePath());

        if (extractEntryTemp.isDir()) {
            if (!finalDestDir.mkpath(extractEntry.filePath()))
                qInfo() << "Failed to create directory" << extractEntry.filePath();
            continue;
        }

        if (extractEntryDest.exists()) {
            if (!skipAll && !overwriteAll) {
                OverwriteQuery query(extractEntryDest.absoluteFilePath());
                emit signalQuery(&query);
                query.waitForResponse();

                if (query.responseOverwrite() || query.responseOverwriteAll()) {
                    if (query.responseOverwriteAll())
                        overwriteAll = true;
                    QFile::remove(extractEntryDest.absoluteFilePath());
                } else if (query.responseSkip() || query.responseSkipAll()) {
                    if (query.responseSkipAll())
                        skipAll = true;
                    continue;
                } else if (query.responseCancelled()) {
                    emit signalCancel();
                    emit signalFinished(PFT_Cancel);
                    return false;
                }
            } else if (skipAll) {
                return true;
            } else if (overwriteAll) {
                QFile::remove(extractEntryDest.absoluteFilePath());
            }
        }

        finalDestDir.mkpath(extractEntry.path());

        if (!QFile(extractEntryTemp.absoluteFilePath()).rename(extractEntryDest.absoluteFilePath())) {
            qInfo() << "Failed to move file" << extractEntryTemp.filePath();
            moveSuccess = false;
        }
    }

    return moveSuccess;
}

void CliRunner::removeExtractedFilesOnFailure(const QString &targetPath, const QList<FileEntry> &entries)
{
    QList<FileEntry> toRemove = entries;
    if (toRemove.isEmpty())
        toRemove = DataManager::get_instance().archiveData().mapFileEntry.values();
    if (toRemove.isEmpty())
        return;

    QDir targetDir(targetPath);
    if (!targetDir.exists())
        return;

    QList<QPair<QString, bool>> paths;
    for (const FileEntry &entry : toRemove) {
        QString relPath = entry.strFullPath;
        if (relPath.endsWith(QLatin1Char('/')))
            relPath.chop(1);
        if (relPath.isEmpty())
            continue;
        paths.append(qMakePair(targetDir.absoluteFilePath(relPath), entry.isDirectory));
    }

    // 失败时只清掉 0 字节的空壳文件
    for (const auto &item : paths) {
        if (item.second)
            continue;
        const QFileInfo info(item.first);
        if (info.exists() && info.isFile() && info.size() == 0)
            QFile::remove(item.first);
    }

    // 空目录可能有多层，反复扫到本轮没有可删的
    bool removed = false;
    do {
        removed = false;
        for (const auto &item : paths) {
            if (!item.second)
                continue;
            QDir dir(item.first);
            if (dir.exists() && dir.isEmpty()) {
                dir.removeRecursively();
                removed = true;
            }
        }
    } while (removed);
}

void CliRunner::attachWatchers()
{
    if (m_outFd >= 0) {
        m_ptyNotifier = new QSocketNotifier(m_outFd, QSocketNotifier::Read, this);
        connect(m_ptyNotifier, &QSocketNotifier::activated, this, [this]() {
            char buffer[4096];
            const qint64 count = readPty(buffer, sizeof(buffer));
            if (count > 0)
                handleOutput(QByteArray(buffer, int(count)));
        });
    }

    m_exitTimer = new QTimer(this);
    m_exitTimer->setInterval(50);
    connect(m_exitTimer, &QTimer::timeout, this, [this]() {
        if (!m_childRunning) {
            m_exitTimer->stop();
            return;
        }
        if (childExited())
            finishRun();
    });
    m_exitTimer->start();
}

bool CliRunner::runProcessSync(const QString &program, const QStringList &arguments)
{
    stopChild();
    closePty();

    const QString programPath = QStandardPaths::findExecutable(program);
    const QFileInfo programInfo(programPath);
    if (programPath.isEmpty() || !programInfo.isAbsolute() || !programInfo.isFile()
            || !programInfo.isExecutable()) {
        return false;
    }

    if (!openPty())
        return false;

    m_stdOutData.clear();
    m_isProcessKilled = false;
    m_passwordAborted = false;
    m_programName = programInfo.fileName();

    if (!startChild(programPath, arguments)) {
        closePty();
        return false;
    }

    // 没有 tty 时 7z 会静默等待密码，这里超时兜底
    QElapsedTimer guard;
    guard.start();

    while (m_childRunning && !m_passwordAborted && !m_isProcessKilled) {
        char buffer[4096];
        const qint64 count = readPty(buffer, sizeof(buffer));
        if (count > 0)
            handleOutput(QByteArray(buffer, int(count)));

        if (childExited())
            break;

        if (guard.elapsed() > 3000) {
            stopChild();
            break;
        }

        ::usleep(10 * 1000);
    }

    char buffer[4096];
    for (;;) {
        const qint64 count = readPty(buffer, sizeof(buffer));
        if (count <= 0)
            break;
        m_stdOutData += QString::fromLocal8Bit(QByteArray(buffer, int(count)));
    }

    if (m_childRunning)
        stopChild();
    closePty();

    return m_exitCode == 0;
}

PluginFinishType CliRunner::list()
{
    ++m_opSerial;
    setPassword(QString());
    DataManager::get_instance().resetArchiveData();
    m_setHasRootDirs.clear();
    m_setHasHandlesDirs.clear();

    m_workStatus = WT_List;

    if (!m_common->isSupportSeek(m_strArchiveName)) {
        QTimer::singleShot(1000, this, [this]() {
            m_eErrorType = ET_FileSeekError;
            emit signalprogress(100);
            emit signalFinished(PFT_Error);
        });
        return PFT_Error;
    }

    const bool started = runProcessSync(programFor(WT_List),
                                        argumentsFor(WT_List, m_strArchiveName, {}, nullptr));

    // 列表结果要给后续操作直接用，这里同步完成后立即发出结束信号
    m_finishType = started ? PFT_Nomral : PFT_Error;

    if (m_isCorruptArchive) {
        // 分卷缺失等非致命损坏：由用户决定是否以只读方式打开
        LoadCorruptQuery query(m_strArchiveName);
        emit signalQuery(&query);
        query.waitForResponse();

        if (query.responseYes()) {
            // 用户选择只读打开：不影响后续操作
            m_finishType = PFT_Nomral;
        } else {
            m_eErrorType = ET_MissingVolume;
            m_finishType = PFT_Cancel;
        }
        m_isCorruptArchive = false;
    } else if (!started) {
        m_eErrorType = ET_ArchiveDamaged;
    }

    // 延迟一拍再发：调用方是在 list() 返回之后才进事件循环等这个信号。
    // 若期间又发起了别的操作，这条过期信号要丢掉，否则会被当成后一次操作的结果。
    const quint64 serial = m_opSerial;
    QTimer::singleShot(0, this, [this, serial]() {
        if (m_opSerial != serial)
            return;
        emit signalprogress(100);
        emit signalFinished(m_finishType);
    });

    return m_finishType;
}

PluginFinishType CliRunner::testArchive()
{
    m_workStatus = WT_Add;
    return PFT_Nomral;
}

PluginFinishType CliRunner::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    ++m_opSerial;
    if (!m_common->isSupportSeek(m_strArchiveName)) {
        QTimer::singleShot(1000, this, [this]() {
            m_eErrorType = ET_FileSeekError;
            emit signalprogress(100);
            emit signalFinished(PFT_Error);
        });
        return PFT_Nomral;
    }

    // 解压前先拿到包内条目：界面要显示进度来源，旧实现也是先 list 再解
    m_passwordAborted = false;
    if (DataManager::get_instance().archiveData().mapFileEntry.isEmpty()) {
        emit signalprogress(1);
        m_workStatus = WT_List;
        DataManager::get_instance().resetArchiveData();
        runProcessSync(programFor(WT_List), argumentsFor(WT_List, m_strArchiveName, {}, nullptr));

        // 列表阶段问了密码也无从回答，交给后面的解压阶段统一询问一次
        m_passwordAborted = false;
    }

    // 部分解压：记下本次选中的条目（目录按前缀展开），结束时据此搬移
    m_selectedFiles.clear();
    if (!options.bAllExtract) {
        const ArchiveData &data = DataManager::get_instance().archiveData();
        for (const FileEntry &entry : files) {
            if (entry.isDirectory) {
                for (auto it = data.mapFileEntry.constBegin(); it != data.mapFileEntry.constEnd(); ++it) {
                    if (it.key().startsWith(entry.strFullPath))
                        m_selectedFiles << it.value();
                }
            } else {
                m_selectedFiles << entry;
            }
        }
    }

    // 超长文件名：先在归档内改名再批量解压（与旧实现同一套流程）
    {
        QList<FileEntry> entries = files;
        if (entries.isEmpty())
            entries = DataManager::get_instance().archiveData().mapFileEntry.values();

        if (options.bAllExtract && supportsLongNameRename() && hasLongName(entries)) {
            m_workStatus = WT_Extract;
            m_extractOptions = options;
            m_finishType = handleLongNameExtract(entries) ? PFT_Nomral : PFT_Error;
            if (m_finishType == PFT_Error && m_eErrorType == ET_NoError)
                m_eErrorType = ET_FileWriteError;

            emit signalprogress(100);
            const quint64 serial = m_opSerial;
            QTimer::singleShot(0, this, [this, serial]() {
                if (m_opSerial != serial)
                    return;
                emit signalFinished(m_finishType);
            });
            return m_finishType;
        }
    }

    m_workStatus = WT_Extract;
    m_extractOptions = options;
    m_finishType = PFT_Nomral;
    m_rootEntryCount = 0;
    m_isEmptyArchive = false;
    m_isCorruptArchive = false;

    QStringList names;
    if (!options.bAllExtract) {
        for (const FileEntry &file : files) {
            QString name = file.strFullPath;
            // rar 解文件夹带尾斜杠会多解一层
            if (name.endsWith(QLatin1Char('/')))
                name.chop(1);
            names << name;
        }
    }

    QString dest = options.strTargetPath;
    if (dest.endsWith(QLatin1Char('/')))
        dest.chop(1);
    if (!QDir().exists(dest))
        QDir().mkpath(dest);

    // 解压要把工作目录切到目标路径，压缩包路径得先转成绝对路径
    const QString archivePath = QFileInfo(m_strArchiveName).absoluteFilePath();
    QDir::setCurrent(dest);

    // 部分解压：先解到目标路径下的隐藏临时目录，结束后再逐条搬到目标路径。
    // 与旧实现一致：临时目录用相对模板，落在 dest 内。
    if (!options.bAllExtract) {
        m_extractTempDir.reset(new QTemporaryDir(QStringLiteral(".deepin-compressor-")));
        if (!m_extractTempDir->isValid()) {
            m_eErrorType = ET_FileWriteError;
            emit signalFinished(PFT_Error);
            return PFT_Error;
        }
        QDir::setCurrent(m_extractTempDir->path());
    }

    const bool started = runProcess(programFor(WT_Extract),
                                    argumentsFor(WT_Extract, archivePath, names, nullptr));
    return started ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliRunner::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    ++m_opSerial;
    m_workStatus = WT_Add;
    m_finishType = PFT_Nomral;

    QStringList paths;
    for (const FileEntry &file : files)
        paths << file.strFullPath;

    // tar.7z：先 tar 打包再用 7z 收进 -si，用管道串起来，不经 shell
    if (options.bTar_7z) {
        QStringList tarArgs{QStringLiteral("cf"), QStringLiteral("-")};
        for (const QString &raw : paths) {
            QString path = raw;
            while (path.endsWith(QLatin1Char('/')))
                path.chop(1);
            const int pos = path.lastIndexOf(QLatin1Char('/'));
            if (pos > 0)
                tarArgs << QStringLiteral("-C") << path.left(pos + 1) << path.mid(pos + 1);
            else
                tarArgs << path;
        }

        QStringList sevenArgs = argumentsFor(WT_Add, m_strArchiveName, {}, &options);
        const int addIndex = sevenArgs.indexOf(QStringLiteral("a"));
        if (addIndex >= 0)
            sevenArgs.insert(addIndex + 1, QStringLiteral("-si"));

        stopChild();
        closePty();
        if (!openPty())
            return PFT_Error;

        m_stdOutData.clear();
        m_isProcessKilled = false;
        m_passwordAborted = false;
        m_finishEmitted = false;
        m_programName = QStringLiteral("7z");

        if (!startPipeline(QStringLiteral("tar"), tarArgs,
                           programFor(WT_Add), sevenArgs)) {
            closePty();
            return PFT_Error;
        }

        attachWatchers();
        return PFT_Nomral;
    }

    const bool started = runProcess(programFor(WT_Add),
                                    argumentsFor(WT_Add, m_strArchiveName, paths, &options));
    return started ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliRunner::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    ++m_opSerial;
    Q_UNUSED(files)
    Q_UNUSED(options)
    // 旧实现的重命名/移动只是置状态后返回成功，实际不改包内容
    m_workStatus = WT_Move;
    return PFT_Nomral;
}

PluginFinishType CliRunner::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    m_workStatus = WT_Copy;
    return PFT_Nomral;
}

PluginFinishType CliRunner::deleteFiles(const QList<FileEntry> &files)
{
    ++m_opSerial;
    m_workStatus = WT_Delete;
    m_finishType = PFT_Nomral;

    QStringList names;
    for (const FileEntry &file : files) {
        QString name = file.strFullPath;
        if (name.endsWith(QLatin1Char('/')))
            name.chop(1);
        names << name;
    }

    const bool started = runProcess(programFor(WT_Delete),
                                    argumentsFor(WT_Delete, m_strArchiveName, names, nullptr));
    return started ? PFT_Nomral : PFT_Error;
}

PluginFinishType CliRunner::renameFiles(const QList<FileEntry> &files)
{
    return moveFiles(files, CompressOptions());
}

PluginFinishType CliRunner::addComment(const QString &comment)
{
    Q_UNUSED(comment)
    // 命令行后端不支持注释，旧实现也返回成功
    m_workStatus = WT_Comment;
    return PFT_Nomral;
}

PluginFinishType CliRunner::updateArchiveData(const UpdateOptions &options)
{
    Q_UNUSED(options)
    return list();
}

void CliRunner::handleProgress(const QString &line)
{
    const int pos = line.indexOf(QLatin1Char('%'));

    if (m_programName.contains(QLatin1String("unrar"))) {
        if (line.startsWith(QLatin1String("Extracting")))
            emit signalCurFileName(pos > 1 ? line.mid(12, pos - 24) : line.mid(12));
        if (pos > 1)
            emit signalprogress(line.mid(pos - 3, 3).toInt());
        return;
    }

    if (pos < 2)
        return;

    const int percentage = line.mid(pos - 3, 3).toInt();

    if (percentage <= 0) {
        // 小文件没有进度行，界面仍需要一个条目
        if (m_workStatus == WT_Extract && !m_extractOptions.bExistList
                && m_rootEntryCount == 0 && !m_isEmptyArchive) {
            m_rootEntryCount++;
            FileEntry entry;
            DataManager::get_instance().archiveData().listRootEntry << entry;
        }
        return;
    }

    if (!line.contains(QLatin1Char('\b')))
        return;

    QString name;
    if (m_workStatus == WT_Extract || m_workStatus == WT_Add) {
        int flag = line.indexOf(QLatin1Char('+'));
        if (flag == -1)
            flag = line.indexOf(QLatin1Char('-'));
        if (flag == -1)
            flag = line.indexOf(QLatin1Char('U'));
        if (flag > 0) {
            name = line.mid(flag + 2);
            if (m_workStatus == WT_Extract && !m_extractOptions.bExistList && m_rootEntryCount == 0) {
                m_rootEntryCount++;
                FileEntry entry;
                entry.strFullPath = name;
                DataManager::get_instance().archiveData().listRootEntry << entry;
            }
        }
    } else if (line.contains(QLatin1String("% = "))) {
        name = line.mid(line.indexOf(QLatin1Char('=')) + 2);
    } else if (line.contains(QLatin1String("% R "))) {
        name = line.mid(line.indexOf(QLatin1Char('R')) + 2);
    }

    emit signalprogress(percentage);
    emit signalCurFileName(name);
}

PluginFinishType CliRunner::handlePassword()
{
    if (DataManager::get_instance().archiveData().isListEncrypted && m_workStatus == WT_List)
        return PFT_Nomral;

    PasswordNeededQuery query(m_strEncryptedFileName.isEmpty() ? m_strArchiveName : m_strEncryptedFileName);
    emit signalQuery(&query);
    query.waitForResponse();

    if (query.responseCancelled()) {
        // 取消密码：按旧实现的表现结束（错误类型为密码错误，进程终止）
        setPassword(QString());
        m_eErrorType = ET_WrongPassword;
        m_finishType = PFT_Error;
        stopChild(true);
        return PFT_Cancel;
    }

    setPassword(query.password());
    writeToProcess(m_strPassword.toUtf8() + '\n');
    return PFT_Nomral;
}

bool CliRunner::handleFileExists(const QString &line)
{
    if (!isFileExistsFileName(line) && !isFileExistsMsg(line))
        return false;

    if (isFileExistsFileName(line) && !isFileExistsMsg(line))
        return false;

    OverwriteQuery query(line);
    emit signalQuery(&query);
    query.waitForResponse();

    if (query.responseOverwrite())
        writeToProcess("Y\n");
    else if (query.responseSkip())
        writeToProcess("N\n");
    else if (query.responseOverwriteAll()) {
        m_bOverwriteAll = true;
        writeToProcess("A\n");
    } else if (query.responseSkipAll()) {
        m_bSkipAll = true;
        writeToProcess("S\n");
    } else if (query.responseCancelled()) {
        m_bCancel = true;
        writeToProcess("Q\n");
        killProcess();
        return true;
    }

    return true;
}

void CliRunner::writeToProcess(const QByteArray &data)
{
    if (m_ptyMaster >= 0)
        ::write(m_ptyMaster, data.constData(), size_t(data.size()));
}

QString CliRunner::getTargetPath() const
{
    return m_extractOptions.strTargetPath;
}

void CliRunner::pauseOperation()
{
    m_bPause = true;
    if (m_pid > 0)
        ::kill(pid_t(-m_pid), SIGSTOP);
}

void CliRunner::continueOperation()
{
    m_bPause = false;
    if (m_pid > 0)
        ::kill(pid_t(-m_pid), SIGCONT);
}

bool CliRunner::doKill()
{
    m_bPause = false;
    stopChild(true);
    return true;
}
