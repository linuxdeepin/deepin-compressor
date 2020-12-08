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
#ifndef SINGLEJOB_H
#define SINGLEJOB_H

#include "archivejob.h"
#include "archiveinterface.h"
#include "commonstruct.h"
#include "queries.h"

#include <QThread>
#include <QElapsedTimer>

class SingleJob;

// 工作线程
class SingleJobThread : public QThread
{
    Q_OBJECT
public:
    SingleJobThread(SingleJob *job, QObject *parent = nullptr)
        : QThread(parent)
        , q(job)
    {
    }

    /**
     * @brief run
     */
    void run() override;

private:
    SingleJob *q;
};

// 单个操作
class SingleJob : public ArchiveJob
{
    Q_OBJECT
public:
    explicit SingleJob(ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~SingleJob() override;

    /**
     * @brief doWork    执行操作
     */
    virtual void doWork() = 0;

    /**
     * @brief start     开始
     */
    void start() override;

    /**
     * @brief doPause   暂停
     */
    void doPause() override;

    /**
     * @brief doContinue    继续
     */
    void doContinue() override;

protected:
    /**
     * @brief initConnections   初始化插件和job的信号槽连接
     */
    void initConnections();

    /**
     * @brief doKill  强行结束job
     */
    bool doKill() override;

protected Q_SLOTS:

    /**
     * @brief slotFinished  操作结束处理
     * @param eType 结束类型
     */
    void slotFinished(PluginFinishType eType);

//Q_SIGNALS:

protected:
    ReadOnlyArchiveInterface *m_pInterface;
    SingleJobThread *const d;   // 线程
    QElapsedTimer jobTimer;     // 操作计时
};

// 加载操作
class LoadJob : public SingleJob
{
    Q_OBJECT
public:
    explicit LoadJob(ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~LoadJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:

};

// 压缩操作
class AddJob : public SingleJob
{
    Q_OBJECT
public:
    explicit AddJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent = nullptr);
    ~AddJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QList<FileEntry> m_vecFiles; //待压缩文件(夹)
    QString m_strDestination; //追加时当前压缩包内路径
    CompressOptions m_stCompressOptions; //压缩配置
};

// 创建压缩包操作
class CreateJob : public SingleJob
{
    Q_OBJECT
public:
    explicit CreateJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent = nullptr);
    ~CreateJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

protected:
    bool doKill() override;

private:
    /**
     * @brief cleanCompressFileCancel 压缩取消时删除临时压缩包
     */
    void cleanCompressFileCancel();

private:
    QList<FileEntry> m_vecFiles;
    CompressOptions m_stCompressOptions;
};

// 解压操作
class ExtractJob : public SingleJob
{
    Q_OBJECT
public:
    explicit ExtractJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const ExtractionOptions &options, QObject *parent = nullptr);
    ~ExtractJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QList<FileEntry> m_vecFiles;
    ExtractionOptions m_stExtractionOptions;
};

// 删除操作
class DeleteJob : public SingleJob
{
    Q_OBJECT
public:
    explicit DeleteJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~DeleteJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QList<FileEntry> m_vecFiles;

};

// 打开操作
class OpenJob: public SingleJob
{
    Q_OBJECT
public:
    explicit OpenJob(const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~OpenJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

protected Q_SLOTS:
    /**
     * @brief slotFinished  操作结束处理
     * @param eType 结束类型
     */
    void slotFinished(PluginFinishType eType);

private:
    FileEntry m_stEntry;            // 待打开文件
    QString m_strTempExtractPath;   // 临时解压路径
    QString m_strProgram;           // 应用程序
};

// 更新操作（压缩包修改完之后更新缓存数据）
class UpdateJob: public SingleJob
{
    Q_OBJECT
public:
    explicit UpdateJob(const UpdateOptions &options, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~UpdateJob() override;

    /**
     * @brief start     开始
     */
    void start() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    UpdateOptions m_stOptions;      // 更新选项
};

// 更新操作（压缩包修改完之后更新缓存数据）
class CommentJob: public SingleJob
{
    Q_OBJECT
public:
    explicit CommentJob(const QString &strComment, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~CommentJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QString m_strComment;      // 注释
};

// 转换操作
class ConvertJob: public ArchiveJob
{
    Q_OBJECT
public:
    explicit ConvertJob(const QString strOriginalArchiveFullPath, const QString strTargetFullPath, const QString strNewArchiveFullPath, QObject *parent = nullptr);
    ~ConvertJob() override;

    /**
     * @brief start     开始
     */
    void start() override;

    /**
     * @brief doPause   暂停
     */
    void doPause() override;

    /**
     * @brief doContinue    继续
     */
    void doContinue() override;

private Q_SLOTS:
    /**
     * @brief slotHandleSingleJobProgress       处理单个压缩包解压进度
     * @param dPercentage
     */
    void slotHandleSingleJobProgress(double dPercentage);

    /**
     * @brief slotHandleSingleJobCurFileName       处理单个压缩包当前解压文件名
     * @param strName
     */
    void slotHandleSingleJobCurFileName(const QString &strName);

    /**
     * @brief slotHandleExtractFinished       处理解压结束
     */
    void slotHandleExtractFinished();

private:
    ReadOnlyArchiveInterface *m_pIface = nullptr;
    ExtractJob *m_pExtractJob;  // 先解压
    CreateJob *m_pCreateJob;    // 再压缩成想要的格式
    QString m_strOriginalArchiveFullPath;   // 原始压缩包全路径
    QString m_strTargetFullPath;                // 转换目标全路径
    QString m_strNewArchiveFullPath;   // 格式转换后压缩包全路径
    WorkType m_workType = WT_Convert;
};

#endif
