// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPZIPPLUGIN_H
#define CLIPZIPPLUGIN_H

#include "archiveinterface.h"
#include "kpluginfactory.h"
#include "kptyprocess.h"

#include <QProcess>
#include <QTimer>

class CliPzipPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_clipzip.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit CliPzipPluginFactory();
    ~CliPzipPluginFactory();
};

/**
 * @brief CliPzipPlugin - 使用 pzip 进行高性能并行 ZIP 压缩的插件
 * 
 * 该插件调用内置的 pzip 命令行工具进行压缩，支持多线程并行压缩，
 * 在 ARM 平台上性能提升显著。
 */
class CliPzipPlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit CliPzipPlugin(QObject *parent, const QVariantList &args);
    ~CliPzipPlugin() override;

    // ReadOnlyArchiveInterface interface
    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    // ReadWriteArchiveInterface interface
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

private:
    /**
     * @brief getPzipPath 获取 pzip 可执行文件路径
     */
    QString getPzipPath() const;

    /**
     * @brief getPunzipPath 获取 punzip 可执行文件路径
     */
    QString getPunzipPath() const;

    /**
     * @brief handleLine 处理命令行输出
     */
    bool handleLine(const QString &line);

    /**
     * @brief killProcess 结束进程
     */
    void killProcess(bool emitFinished = true);

    /**
     * @brief deleteProcess 删除进程
     */
    void deleteProcess();

    /**
     * @brief getChildProcessId 获取子进程 ID
     */
    void getChildProcessId(qint64 processId, const QStringList &listKey, QVector<qint64> &childprocessid);

private slots:
    void readStdout(bool handleAll = false);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    KPtyProcess *m_process = nullptr;
    QByteArray m_stdOutData;
    bool m_isProcessKilled = false;
    qint64 m_processId = 0;
    QVector<qint64> m_childProcessId;
    qint64 m_qTotalSize = 0;
    QTimer *m_timer = nullptr;

    // 解压相关
    QString m_extractDestPath;
    int m_extractedCount = 0;
    int m_totalFilesCount = 0;
};

#endif // CLIPZIPPLUGIN_H

