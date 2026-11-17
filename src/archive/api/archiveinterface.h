// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVEINTERFACE_H
#define ARCHIVEINTERFACE_H

#include "commonstruct.h"
#include "kpluginmetadata.h"
#include "common.h"

#include <QObject>
#include <QString>
#include <QMimeType>
#include <QFileDevice>
#include <QSet>

class Query;

// 只读后端：加载、校验、解压。一次操作一个实例。
// waitForFinished() 为 true 时由后端自己发 signalFinished。
class ReadOnlyArchiveInterface : public QObject
{
    Q_OBJECT

public:
    explicit ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadOnlyArchiveInterface() override;

    enum Plugintype {
        PT_UnKnown,
        PT_Cliinterface,
        PT_LibArchive,
        PT_Libzip,
        PT_Libminizip,
        PT_Libpigz
    };

    // 加载压缩包，条目写入 DataManager
    virtual PluginFinishType list() = 0;

    virtual PluginFinishType testArchive() = 0;

    // files 为空表示全部解压
    virtual PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) = 0;

    bool waitForFinished();

    void setPassword(const QString &strPassword);

    QString getPassword();

    ErrorType errorType();

    virtual void pauseOperation() = 0;

    virtual void continueOperation() = 0;

    virtual bool doKill();

    // true 表示当前处于暂停状态
    bool status();

    bool isUserCancel() { return m_bCancel; }

protected:
    void setWaitForFinishedSignal(bool value);

    QFileDevice::Permissions getPermissions(const mode_t &perm);

    // 登记条目，必要时补齐缺失的目录项
    void handleEntry(const FileEntry &entry);

    bool isInsufficientDiskSpace(QString diskPath, qint64 standard = 10 * 1024 * 1024);

Q_SIGNALS:
    void signalFinished(PluginFinishType eType);
    void error(const QString &message = "", const QString &details = "");
    void signalprogress(double dPercentage);
    void signalCurFileName(const QString &filename);
    void signalFileWriteErrorName(const QString &filename);
    // 接收方需在返回前调用 Query::setResponse
    void signalQuery(Query *query);
    void signalCancel();

public:
    Plugintype m_ePlugintype = PT_UnKnown;

protected:
    bool m_bWaitForFinished = false;
    uint m_numberOfEntries;
    KPluginMetaData m_metaData;
    QString m_strArchiveName;
    CustomMimeType m_mimetype;
    Common *m_common = nullptr;
    bool m_bOverwriteAll = false;
    bool m_bSkipAll = false;
    WorkType m_workStatus = WT_List;

    ErrorType m_eErrorType = ET_NoError;
    QString m_strPassword;

    bool m_bCancel = false;
    bool m_bPause = false;

    QSet<QString> m_setHasRootDirs;
    QSet<QString> m_setHasHandlesDirs;
};

// 读写后端：压缩、追加、删除、重命名、注释
class ReadWriteArchiveInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    explicit ReadWriteArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadWriteArchiveInterface() override;

    virtual PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) = 0;

    virtual PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) = 0;

    virtual PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) = 0;

    virtual PluginFinishType deleteFiles(const QList<FileEntry> &files) = 0;

    virtual PluginFinishType renameFiles(const QList<FileEntry> &files) = 0;

    virtual PluginFinishType addComment(const QString &comment) = 0;

    QString getArchiveName();

    virtual PluginFinishType updateArchiveData(const UpdateOptions &options) = 0;
};

#endif // ARCHIVEINTERFACE_H
