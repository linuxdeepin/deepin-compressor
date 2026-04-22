// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipzipplugin.h"
#include "datamanager.h"
#include "common.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDirIterator>
#include <cmath>

#include <linux/limits.h>
#include <signal.h>

#include <QTemporaryFile>
#include <QTextCodec>


namespace {

constexpr auto kUiEncAes128 = "AES128";
constexpr auto kUiEncAes192 = "AES192";
constexpr auto kUiEncAes256 = "AES256";
    
constexpr auto kCliEncAes128 = "aes128";
constexpr auto kCliEncAes192 = "aes192";
constexpr auto kCliEncAes256 = "aes256";

// 与 LibzipPlugin::passwordUnicode(str, 0) 对 .zip 的逻辑一致，保证与 libzip 产包互解
static QByteArray passwordBytesLikeLibzipForZip(const QString &strPassword, const QString &archiveName)
{
    if (!archiveName.endsWith(QLatin1String(".zip"), Qt::CaseInsensitive)) {
        return strPassword.toUtf8();
    }
    const int nCount = strPassword.count();
    bool hasHan = false;
    for (int i = 0; i < nCount; ++i) {
        const ushort uni = strPassword.at(i).unicode();
        if (uni >= 0x4E00 && uni <= 0x9FA5) {
            hasHan = true;
            break;
        }
    }
    if (hasHan) {
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
        QTextCodec *dst = QTextCodec::codecForName("UTF-8");
        if (!utf8 || !dst) {
            return strPassword.toUtf8();
        }
        const QString strUnicode = utf8->toUnicode(strPassword.toUtf8().constData());
        return dst->fromUnicode(strUnicode);
    }
    return strPassword.toUtf8();
}

static QString encryptionCliArgFromUi(const QString &ui)
{
    if (ui == QLatin1String(kUiEncAes128)) return QString::fromLatin1(kCliEncAes128);
    if (ui == QLatin1String(kUiEncAes192)) return QString::fromLatin1(kCliEncAes192);
    if (ui == QLatin1String(kUiEncAes256)) return QString::fromLatin1(kCliEncAes256);
    return QString::fromLatin1(kCliEncAes256);
}

} // namespace

// pzip
// pzip 安装路径
static const QString PZIP_INSTALL_PATH = QStringLiteral("/usr/lib/deepin-compressor/pzip");
static const QString PUNZIP_INSTALL_PATH = QStringLiteral("/usr/lib/deepin-compressor/punzip");

CliPzipPluginFactory::CliPzipPluginFactory()
{
    registerPlugin<CliPzipPlugin>();
}

CliPzipPluginFactory::~CliPzipPluginFactory()
{
}

CliPzipPlugin::CliPzipPlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
    setWaitForFinishedSignal(true);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (QMetaType::type("QProcess::ExitStatus") == 0) {
#else
    if (!QMetaType::fromName("QProcess::ExitStatus").isValid()) {
#endif
        qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    }

    m_ePlugintype = PT_Libzip; // 复用 Libzip 类型，因为都是 ZIP 格式
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [=]() {
        emitProgressIfArchiveGrew();
    });
}

void CliPzipPlugin::emitProgressIfArchiveGrew()
{
    if (m_qTotalSize <= 0) {
        return;
    }

    QFileInfo info(m_progressArchiveName.isEmpty() ? m_strArchiveName : m_progressArchiveName);
    const qint64 curSize = info.size();
    if (curSize < 0) {
        return;
    }

    // 避免重复发送相同进度：会导致 UI 速度=0，从而“剩余时间”消失
    if (curSize == m_lastProgressArchiveSize) {
        return;
    }
    m_lastProgressArchiveSize = curSize;

    emit signalprogress(static_cast<double>(curSize) / m_qTotalSize * 100);
}

CliPzipPlugin::~CliPzipPlugin()
{
    deleteProcess();
    if (m_timer) {
        m_timer->stop();
    }
}

QString CliPzipPlugin::getPzipPath() const
{
    // 优先使用安装路径
    if (QFileInfo::exists(PZIP_INSTALL_PATH)) {
        return PZIP_INSTALL_PATH;
    }

    // 开发环境：尝试从应用程序目录查找
    QString appDir = QCoreApplication::applicationDirPath();
    QString devPath = appDir + "/../src/pzip/pzip";
    if (QFileInfo::exists(devPath)) {
        return devPath;
    }

    // 尝试构建目录
    devPath = appDir + "/pzip";
    if (QFileInfo::exists(devPath)) {
        return devPath;
    }

    // 最后尝试 PATH 中的 pzip
    return QStandardPaths::findExecutable("pzip");
}

QString CliPzipPlugin::getPunzipPath() const
{
    // 优先使用安装路径
    if (QFileInfo::exists(PUNZIP_INSTALL_PATH)) {
        return PUNZIP_INSTALL_PATH;
    }

    // 开发环境：尝试从应用程序目录查找
    QString appDir = QCoreApplication::applicationDirPath();
    QString devPath = appDir + "/../src/pzip/punzip";
    if (QFileInfo::exists(devPath)) {
        return devPath;
    }

    // 尝试构建目录
    devPath = appDir + "/punzip";
    if (QFileInfo::exists(devPath)) {
        return devPath;
    }

    // 最后尝试 PATH 中的 punzip
    return QStandardPaths::findExecutable("punzip");
}

PluginFinishType CliPzipPlugin::list()
{
    // pzip 目前不支持列表功能，返回错误让其他插件处理
    // 或者可以用 unzip -l 来实现
    return PFT_Error;
}

PluginFinishType CliPzipPlugin::testArchive()
{
    return PFT_Nomral;
}

PluginFinishType CliPzipPlugin::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    Q_UNUSED(files);

    m_stdOutData.clear();
    m_isProcessKilled = false;
    m_extractDestPath = options.strTargetPath;
    m_extractedCount = 0;

    QString punzipPath = getPunzipPath();
    if (punzipPath.isEmpty()) {
        qWarning() << "punzip not found!";
        m_eErrorType = ET_PluginError;
        return PFT_Error;
    }

    m_process = new KPtyProcess;
    m_process->setPtyChannels(KPtyProcess::StdinChannel);
    m_process->setOutputChannelMode(KProcess::MergedChannels);
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);

    QStringList arguments;
    arguments << m_strArchiveName;     // 压缩包路径
    arguments << m_extractDestPath;    // 解压目标路径

    m_process->setProgram(punzipPath, arguments);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=] {
        readStdout();
    });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CliPzipPlugin::processFinished);
#else
    connect(m_process, &QProcess::finished, this, &CliPzipPlugin::processFinished);
#endif

    m_process->start();

    if (m_process->waitForStarted()) {
        m_childProcessId.clear();
        m_processId = m_process->processId();
        getChildProcessId(m_processId, QStringList() << "punzip", m_childProcessId);
    }

    return PFT_Nomral;
}

PluginFinishType CliPzipPlugin::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    m_qTotalSize = options.qTotalSize;
    m_stdOutData.clear();
    m_isProcessKilled = false;
    m_tempArchiveDir.reset();
    m_tempArchiveName.clear();
    m_progressArchiveName.clear();
    m_lastProgressArchiveSize = -1;
    m_lastUiBytesProgress = -1.0;

    QString pzipPath = getPzipPath();
    if (pzipPath.isEmpty()) {
        qWarning() << "pzip not found!";
        m_eErrorType = ET_PluginError;
        return PFT_Error;
    }

    m_process = new KPtyProcess;
    m_process->setPtyChannels(KPtyProcess::StdinChannel);
    m_process->setOutputChannelMode(KProcess::MergedChannels);
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);

    QStringList arguments;

    m_passwordFile.reset();

    // 静默模式
    arguments << "-q";

    arguments << "--ui-events";

    // MTP 挂载目录不一定支持 seek/rename 等操作，先在临时目录生成，再 mv 回目标路径
    QString outArchiveName = m_strArchiveName;
    if (IsMtpFileOrDirectory(m_strArchiveName)) {
        m_tempArchiveDir = std::make_unique<QTemporaryDir>();
        m_tempArchiveDir->setAutoRemove(true);
        m_tempArchiveName = m_tempArchiveDir->path() + QDir::separator() + QFileInfo(m_strArchiveName).fileName();
        outArchiveName = m_tempArchiveName;
        qInfo() << "[PZIP_ROUTE]" << "clipzip: mtp target detected, staging archive at:" << outArchiveName
                << "final:" << m_strArchiveName;
    }
    m_progressArchiveName = outArchiveName;

    // pzip 静默模式不会输出“当前文件名”，为了避免进度页一直显示“计算中”，先上报一个可展示的文件名
    if (!files.isEmpty()) {
        const FileEntry &f0 = files.first();
        const QString display = !f0.strAlias.isEmpty() ? f0.strAlias
                                                       : (!f0.strFileName.isEmpty() ? f0.strFileName : f0.strFullPath);
        emit signalCurFileName(display);
    }

    // 兜底：部分场景上层未计算总大小（qTotalSize=0），会导致进度条永远不刷新
    if (m_qTotalSize <= 0) {
        qint64 sum = 0;
        for (const FileEntry &f : files) {
            QFileInfo fi(f.strFullPath);
            if (!fi.exists()) continue;
            if (fi.isFile()) {
                sum += fi.size();
                continue;
            }
            if (fi.isDir()) {
                QDirIterator it(fi.absoluteFilePath(), QDir::Files, QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    it.next();
                    sum += it.fileInfo().size();
                }
            }
        }
        m_qTotalSize = sum;
        qInfo() << "[PZIP_ROUTE]" << "clipzip: fallback total size computed:" << m_qTotalSize;
    }

    if (options.bEncryption && !options.strPassword.isEmpty()) {
        m_passwordFile = std::make_unique<QTemporaryFile>();
        if (!m_passwordFile->open()) {
            qWarning() << "Failed to create temporary file for password";
            m_eErrorType = ET_PluginError;
            return PFT_Error;
        }
        const QByteArray pwBytes = passwordBytesLikeLibzipForZip(options.strPassword, m_strArchiveName);
        if (m_passwordFile->write(pwBytes) != pwBytes.size()) {
            qWarning() << "Failed to write password bytes";
            m_eErrorType = ET_PluginError;
            return PFT_Error;
        }
        m_passwordFile->flush();
        arguments << "--password-file" << m_passwordFile->fileName();
        arguments << "-e" << encryptionCliArgFromUi(options.strEncryptionMethod);
    }

    // 压缩等级：0=Store(不压缩)，1-9=deflate 压缩级别
    int level = options.iCompressionLevel;
    if (level < 0 || level > 9) {
        level = 1;
    }
    arguments << "-l" << QString::number(level);

    // 线程数：只有大于1时才指定，否则让 pzip 自动使用全部 CPU 核心
    if (options.iCPUTheadNum > 1) {
        arguments << "-c" << QString::number(options.iCPUTheadNum);
    }

    // 输出文件
    arguments << outArchiveName;

    // 添加所有源文件/目录
    for (const FileEntry &file : files) {
        QString filePath = file.strFullPath;
        // 移除末尾的 '/'
        if (filePath.endsWith('/')) {
            filePath.chop(1);
        }
        arguments << filePath;
    }

    qInfo() << "Running pzip:" << pzipPath << arguments;

    m_process->setProgram(pzipPath, arguments);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=] {
        readStdout();
    });

    connect(m_process, &QProcess::readyReadStandardError, this, [=] {
        QByteArray errorOutput = m_process->readAllStandardError();
        qDebug() << "pzip stderr:" << QString::fromLocal8Bit(errorOutput);
    });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CliPzipPlugin::processFinished);
#else
    connect(m_process, &QProcess::finished, this, &CliPzipPlugin::processFinished);
#endif

    m_process->start();

    if (m_process->waitForStarted()) {
        m_childProcessId.clear();
        m_processId = m_process->processId();
        getChildProcessId(m_processId, QStringList() << "pzip", m_childProcessId);
        m_timer->start(500);  // 每500ms更新一次进度

        // 让 UI 立即脱离“计算中”（速度/剩余时间会在首次 setProgress 后更新）
        if (m_qTotalSize > 0) {
            emit signalprogress(1);
        }
    }

    return PFT_Nomral;
}

PluginFinishType CliPzipPlugin::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files);
    Q_UNUSED(options);
    return PFT_Error; // 不支持
}

PluginFinishType CliPzipPlugin::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files);
    Q_UNUSED(options);
    return PFT_Error; // 不支持
}

PluginFinishType CliPzipPlugin::deleteFiles(const QList<FileEntry> &files)
{
    Q_UNUSED(files);
    return PFT_Error; // 不支持
}

PluginFinishType CliPzipPlugin::renameFiles(const QList<FileEntry> &files)
{
    Q_UNUSED(files);
    return PFT_Error; // 不支持
}

PluginFinishType CliPzipPlugin::addComment(const QString &comment)
{
    Q_UNUSED(comment);
    return PFT_Error; // 不支持
}

PluginFinishType CliPzipPlugin::updateArchiveData(const UpdateOptions &options)
{
    Q_UNUSED(options);
    return PFT_Error; // 不支持
}

void CliPzipPlugin::pauseOperation()
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

void CliPzipPlugin::continueOperation()
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

bool CliPzipPlugin::doKill()
{
    if (m_process) {
        killProcess(false);
        m_timer->stop();
        return true;
    }
    return false;
}

bool CliPzipPlugin::handleLine(const QString &line)
{
    static const QLatin1String kUiPrefix("[PZIP_UI] entry ");
    if (line.startsWith(kUiPrefix)) {
        const QString nameInArchive = line.mid(kUiPrefix.size());
        if (!nameInArchive.isEmpty()) {
            emit signalCurFileName(nameInArchive);
        }
        emitProgressIfArchiveGrew();
        return true;
    }

    static const QLatin1String kUiBytesPrefix("[PZIP_UI] inbytes ");
    if (line.startsWith(kUiBytesPrefix)) {
        if (m_qTotalSize > 0) {
            bool ok = false;
            const qint64 inBytes = line.mid(kUiBytesPrefix.size()).trimmed().toLongLong(&ok);
            if (ok && inBytes >= 0) {
                double p = static_cast<double>(inBytes) / static_cast<double>(m_qTotalSize) * 100.0;
                if (p > 99.9) p = 99.9; // 结束由 processFinished 统一补 100
                if (p < 0.0) p = 0.0;

                // 只要有实际变化就推一把，让 UI 能稳定算速度/剩余时间
                if (m_lastUiBytesProgress < 0.0 || std::abs(p - m_lastUiBytesProgress) >= 0.05) {
                    m_lastUiBytesProgress = p;
                    emit signalprogress(p);
                }
            }
        }
        return true;
    }

    if (line.contains(QLatin1String("No space left on device"))) {
        m_eErrorType = ET_InsufficientDiskSpace;
        return false;
    }

    if (line.contains(QLatin1String("error")) || line.contains(QLatin1String("Error"))) {
        qWarning() << "pzip error:" << line;
        // 不一定是致命错误，继续处理
    }

    emitProgressIfArchiveGrew();
    return true;
}

void CliPzipPlugin::killProcess(bool emitFinished)
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

    qint64 processID = m_process->processId();
    if (processID > 0) {
        kill(static_cast<__pid_t>(processID), SIGCONT);
        kill(static_cast<__pid_t>(processID), SIGTERM);
    }

    m_isProcessKilled = true;
}

void CliPzipPlugin::deleteProcess()
{
    m_passwordFile.reset();
    if (m_process) {
        readStdout(true);
        m_process->blockSignals(true);
        delete m_process;
        m_process = nullptr;
    }
}

void CliPzipPlugin::getChildProcessId(qint64 processId, const QStringList &listKey, QVector<qint64> &childprocessid)
{
    childprocessid.clear();

    if (0 >= processId || 0 == listKey.count()) {
        return;
    }

    QString strProcessId = QString::number(processId);
    QProcess p;
    p.setProgram("pstree");
    p.setArguments(QStringList() << "-np" << strProcessId);
    p.start();

    if (p.waitForReadyRead()) {
        QByteArray dd = p.readAllStandardOutput();
        QList<QByteArray> lines = dd.split('\n');

        if (lines.count() > 0 && lines[0].contains(strProcessId.toUtf8())) {
            for (const QByteArray &line : lines) {
                for (const QString &strKey : listKey) {
                    QString str = QString("-%1(").arg(strKey);
                    QByteArray ba = str.toUtf8();
                    int iCount = line.count(ba);
                    int iIndex = 0;
                    for (int i = 0; i < iCount; ++i) {
                        int iStartIndex = line.indexOf(ba, iIndex);
                        int iEndIndex = line.indexOf(")", iStartIndex);
                        if (0 < iStartIndex && 0 < iEndIndex) {
                            childprocessid.append(line.mid(iStartIndex + str.length(), 
                                iEndIndex - iStartIndex - str.length()).toInt());
                        }
                        iIndex = iStartIndex + 1;
                    }
                }
            }
        }
    }
}

void CliPzipPlugin::readStdout(bool handleAll)
{
    Q_UNUSED(handleAll);

    if (m_isProcessKilled) {
        return;
    }

    Q_ASSERT(m_process);

    if (!m_process->bytesAvailable()) {
        return;
    }

    QByteArray dd = m_process->readAllStandardOutput();
    m_stdOutData += dd;

    QList<QByteArray> lines = m_stdOutData.split('\n');
    m_stdOutData = lines.takeLast();

    for (const QByteArray &line : lines) {
        if (!handleLine(QString::fromLocal8Bit(line))) {
            killProcess();
            return;
        }
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void CliPzipPlugin::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "pzip process finished, exitcode:" << exitCode << "exitstatus:" << exitStatus;
#else
void CliPzipPlugin::processFinished(int exitCode)
{
    QProcess::ExitStatus exitStatus = m_process ? m_process->exitStatus() : QProcess::NormalExit;
    qInfo() << "pzip process finished, exitcode:" << exitCode << "exitstatus:" << exitStatus;
#endif

    deleteProcess();
    m_timer->stop();

    PluginFinishType eFinishType;

    if (0 == exitCode && exitStatus == QProcess::NormalExit) {
        bool ok = true;

        // 若走了 MTP staging，成功后将临时文件移动到最终目标路径
        if (!m_tempArchiveName.isEmpty() && m_tempArchiveDir) {
            QProcess mover;
            const QStringList args = {m_tempArchiveName, m_strArchiveName};
            int ret = mover.execute(QStringLiteral("mv"), args);
            ok = (ret == 0) && (mover.exitStatus() == QProcess::NormalExit) && (mover.exitCode() == 0);
            qInfo() << "[PZIP_ROUTE]" << "clipzip: mv staged archive to final, ok=" << ok
                    << "from" << m_tempArchiveName << "to" << m_strArchiveName;
            if (!ok) {
                m_eErrorType = ET_FileWriteError;
            }
        }

        eFinishType = ok ? PFT_Nomral : PFT_Error;
    } else {
        eFinishType = PFT_Error;
    }

    emit signalprogress(100);
    emit signalFinished(eFinishType);
}

