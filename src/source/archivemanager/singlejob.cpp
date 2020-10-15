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
#include "singlejob.h"

#include <QThread>
#include <QDebug>

// 工作线程
void SingleJobThread::run()
{
    q->doWork();    // 在线程中执行操作
}

// 单个操作
SingleJob::SingleJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : ArchiveJob(parent)
    , m_pInterface(pInterface)
    , d(new SingleJobThread(this))
{

}

SingleJob::~SingleJob()
{

}

void SingleJob::start()
{
    jobTimer.start();

    // 若插件指针为空，立即异常退出
    if (m_pInterface == nullptr) {
        slotFinished(false);
        return;
    }

    // 判断是否通过线程的方式调用
    if (m_pInterface->waitForFinished()) {
        doWork();   // 直接执行操作
    } else {
        d->start(); // 开启线程，执行操作
    }
}

void SingleJob::initConnections()
{
    connect(m_pInterface, &ReadOnlyArchiveInterface::signalFinished, this, &SingleJob::slotFinished);
}

void SingleJob::slotFinished(bool bRight)
{
    qDebug() << "Job finished, result:" << bRight << ", time:" << jobTimer.elapsed() << "ms";

    if (bRight) {
        emit signalJobFinshed();
    }
}

// 加载操作
LoadJob::LoadJob(ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
{

}

LoadJob::~LoadJob()
{

}

void LoadJob::doWork()
{

}

// 压缩操作
AddJob::AddJob(const QVector<FileEntry> &files, const QString &strDestination, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
    , m_strDestination(strDestination)
    , m_stCompressOptions(options)
{

}

AddJob::~AddJob()
{

}

void AddJob::doWork()
{
    ReadWriteArchiveInterface *pWriteInterface = dynamic_cast<ReadWriteArchiveInterface *>(m_pInterface);

    if (pWriteInterface) {
        pWriteInterface->addFiles(m_vecFiles, m_strDestination, m_stCompressOptions);
    }
}

// 创建压缩包操作
CreateJob::CreateJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
    , m_stCompressOptions(options)
{

}

CreateJob::~CreateJob()
{

}

void CreateJob::doWork()
{
    m_pAddJob = new AddJob(m_vecFiles, "", m_pInterface, m_stCompressOptions, this);
}

// 解压操作
ExtractJob::ExtractJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
{

}

ExtractJob::~ExtractJob()
{

}

void ExtractJob::doWork()
{

}

// 删除操作
DeleteJob::DeleteJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
{

}

DeleteJob::~DeleteJob()
{

}

void DeleteJob::doWork()
{

}

// 临时解压操作
TempExtractJob::TempExtractJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
{

}

TempExtractJob::~TempExtractJob()
{

}

void TempExtractJob::doWork()
{

}

// 临时解压操作
OpenJob::OpenJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : TempExtractJob(files, pInterface, parent)
{

}

OpenJob::~OpenJob()
{

}

// 以...打开文件操作
OpenWithJob::OpenWithJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : TempExtractJob(files, pInterface, parent)
{

}

OpenWithJob::~OpenWithJob()
{

}
