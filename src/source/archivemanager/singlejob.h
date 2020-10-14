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

#include <QThread>

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

protected:
    ReadOnlyArchiveInterface *m_pInterface;

private:
    SingleJobThread *const d;
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

};

// 压缩操作
class AddJob : public SingleJob
{
    Q_OBJECT
public:
    explicit AddJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~AddJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QVector<FileEntry> m_vecFiles;
};

// 创建压缩包操作
class CreateJob : public SingleJob
{
    Q_OBJECT
public:
    explicit CreateJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~CreateJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QVector<FileEntry> m_vecFiles;

};

// 解压操作
class ExtractJob : public SingleJob
{
    Q_OBJECT
public:
    explicit ExtractJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~ExtractJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QVector<FileEntry> m_vecFiles;

};

// 删除操作
class DeleteJob : public SingleJob
{
    Q_OBJECT
public:
    explicit DeleteJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~DeleteJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QVector<FileEntry> m_vecFiles;

};

// 临时解压操作
class TempExtractJob : public SingleJob
{
    Q_OBJECT
public:
    explicit TempExtractJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~TempExtractJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

protected:
    QVector<FileEntry> m_vecFiles;

};

// 打开文件操作
class OpenJob : public TempExtractJob
{
    Q_OBJECT
public:
    explicit OpenJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~OpenJob() override;
};

// 以...打开文件操作
class OpenWithJob : public TempExtractJob
{
    Q_OBJECT
public:
    explicit OpenWithJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~OpenWithJob() override;
};

#endif // SINGLEJOB_H
