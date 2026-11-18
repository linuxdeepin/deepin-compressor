// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLI_RUNNER_H
#define CLI_RUNNER_H

#include "archiveinterface.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QScopedPointer>
#include <QTemporaryDir>

class QSocketNotifier;
class QTimer;

// 命令行后端的公共执行器：起进程、逐行分发、密码/同名询问、进度、暂停与结束。
// 子进程直接 fork/exec，不经 shell；stdin/stdout/stderr 挂在伪终端上，
// 7z / unrar 才会输出密码提示。
class CliRunner : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit CliRunner(QObject *parent, const QVariantList &args);
    ~CliRunner() override;

    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    PluginFinishType renameFiles(const QList<FileEntry> &files) override;
    PluginFinishType addComment(const QString &comment) override;
    PluginFinishType updateArchiveData(const UpdateOptions &options) override;

    void pauseOperation() override;
    void continueOperation() override;
    bool doKill() override;

    // 各工具的报文识别
    virtual bool isPasswordPrompt(const QString &line) = 0;
    virtual bool isWrongPasswordMsg(const QString &line) = 0;
    virtual bool isCorruptArchiveMsg(const QString &line) = 0;
    virtual bool isDiskFullMsg(const QString &line) = 0;
    virtual bool isFileExistsMsg(const QString &line) = 0;
    virtual bool isFileExistsFileName(const QString &line) = 0;
    virtual bool isMultiPasswordPrompt(const QString &line) = 0;
    virtual bool isOpenFileFailed(const QString &line) = 0;

    virtual void killProcess(bool emitFinished = true) = 0;

protected:
    // 命令：程序名 + 参数；files 为包内路径，options 仅压缩时非空
    virtual QString programFor(WorkType work) const = 0;
    virtual QStringList argumentsFor(WorkType work, const QString &archive,
                                     const QStringList &files, const CompressOptions *options) const = 0;

    virtual bool handleLine(const QString &line, WorkType workStatus) = 0;

    // 是否支持"归档内改名"（7z rn）；不支持的格式（如 rar）不做长名处理
    virtual bool supportsLongNameRename() const { return false; }
    bool handleLongNameExtract(const QList<FileEntry> &files);
    static bool hasLongName(const QList<FileEntry> &files);

    bool runProcess(const QString &program, const QStringList &arguments);
    bool runProcessSync(const QString &program, const QStringList &arguments);
    void handleProgress(const QString &line);
    PluginFinishType handlePassword();
    bool handleFileExists(const QString &line);
    void writeToProcess(const QByteArray &data);
    QString getTargetPath() const;

protected:
    void stopChild(bool hard = false);
    // 两条命令用管道串起来跑（tar cf - | 7z a -si），不经 shell
    bool startPipeline(const QString &program1, const QStringList &arguments1,
                       const QString &program2, const QStringList &arguments2);

private:
    bool openPty();
    void closePty();
    bool startChild(const QString &programPath, const QStringList &arguments);
    bool childExited();
    void reapChild();
    void handleOutput(const QByteArray &chunk);
    void finishRun();
    void attachWatchers();
    void removeExtractedFilesOnFailure(const QString &targetPath, const QList<FileEntry> &entries);
    // 本次要搬移的条目（部分解压时=选中项按目录展开后的结果）
    QList<FileEntry> m_selectedFiles;

    // 部分解压的搬移逻辑（已由 semantic.py 的 7z.extract.partial 覆盖）
    bool moveExtractTempFilesToDest(const QList<FileEntry> &files, const ExtractionOptions &options);
    bool checkMoveCapability() const;
    qint64 readPty(char *buffer, qint64 size);

    QSocketNotifier *m_ptyNotifier = nullptr;
    QTimer *m_exitTimer = nullptr;
    int m_ptyMaster = -1;
    int m_ptySlave = -1;
    int m_outFd = -1;
    qint64 m_pid = -1;
    qint64 m_pid2 = -1;   // 管道里的上游进程（tar）
    int m_exitCode = -1;
    bool m_childRunning = false;
    bool m_finishEmitted = false;
    quint64 m_opSerial = 0;

public:
    QMap<QString, int> m_mapLongName;
    QMap<QString, int> m_mapLongDirName;
    QMap<QString, int> m_mapRealDirValue;
    QVector<qint64> m_childProcessId;
    QString m_strEncryptedFileName;
    QString m_stdOutData;
    QString m_programName;
    bool m_isProcessKilled = false;
    bool m_isEmptyArchive = false;
    bool m_isCorruptArchive = false;
    bool m_bWaitingPassword = false;
    bool m_passwordAborted = false;
    PluginFinishType m_finishType = PFT_Nomral;
    ExtractionOptions m_extractOptions;
    int m_rootEntryCount = 0;
    QString m_rootNode;   // 部分解压时被剥掉的根目录前缀
    QScopedPointer<QTemporaryDir> m_extractTempDir;
};

#endif // CLI_RUNNER_H
