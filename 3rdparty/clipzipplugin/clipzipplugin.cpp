// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipzipplugin.h"
#include "datamanager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>

#include <linux/limits.h>
#include <signal.h>

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
        QFileInfo info(m_strArchiveName);
        if (m_qTotalSize > 0) {
            emit signalprogress(static_cast<double>(info.size()) / m_qTotalSize * 100);
        }
    });
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
    QString devPath = appDir + "/../3rdparty/pzip/pzip";
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
    QString devPath = appDir + "/../3rdparty/pzip/punzip";
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
    
    // 静默模式
    arguments << "-q";

    arguments << "-l" << "1";
    Q_UNUSED(options.iCompressionLevel);

    // 线程数：只有大于1时才指定，否则让 pzip 自动使用全部 CPU 核心
    if (options.iCPUTheadNum > 1) {
        arguments << "-c" << QString::number(options.iCPUTheadNum);
    }

    // 输出文件
    arguments << m_strArchiveName;

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
    if (line.contains(QLatin1String("No space left on device"))) {
        m_eErrorType = ET_InsufficientDiskSpace;
        return false;
    }

    if (line.contains(QLatin1String("error")) || line.contains(QLatin1String("Error"))) {
        qWarning() << "pzip error:" << line;
        // 不一定是致命错误，继续处理
    }

    // 更新进度
    if (m_qTotalSize > 0) {
        QFileInfo info(m_strArchiveName);
        emit signalprogress(static_cast<double>(info.size()) / m_qTotalSize * 100);
    }

    emit signalCurFileName(line);
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
        eFinishType = PFT_Nomral;
    } else {
        eFinishType = PFT_Error;
    }

    emit signalprogress(100);
    emit signalFinished(eFinishType);
}

