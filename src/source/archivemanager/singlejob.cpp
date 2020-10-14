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
AddJob::AddJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
{

}

AddJob::~AddJob()
{

}

void AddJob::doWork()
{

}

// 创建压缩包操作
CreateJob::CreateJob(const QVector<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent)
    : SingleJob(pInterface, parent)
    , m_vecFiles(files)
{

}

CreateJob::~CreateJob()
{

}

void CreateJob::doWork()
{

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
