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
#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include "commonstruct.h"
#include "archivejob.h"
#include "archiveinterface.h"
#include "plugin.h"

#include <QObject>

class ArchiveManager : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveManager(QObject *parent = nullptr);
    ~ArchiveManager() override;

    /**
     * @brief createArchive     创建压缩包
     * @param files             待压缩文件
     * @param strDestination    压缩包名称（含路径）
     * @param options           压缩参数
     * @param bBatch            是否批量压缩（多路径）
     */
    void createArchive(const QVector<FileEntry> &files, const QString &strDestination, const CompressOptions &options, bool useLibArchive = false, bool bBatch = false);

private:
    ReadOnlyArchiveInterface *createInterface(const QString &fileName, bool bWrite, bool bUseLibArchive = false);
    ReadOnlyArchiveInterface *createInterface(const QString &fileName, Plugin *plugin);

Q_SIGNALS:
    /**
     * @brief signalPercent     进度信号
     * @param iPercent          百分比
     */
    void signalPercent(int iPercent);

    /**
     * @brief signalCurrentFileName     发送当前正在操作的文件名
     * @param strFileName               文件名
     */
    void signalCurrentFileName(const QString &strFileName);

    /**
     * @brief signalError       错误信号
     * @param eErrorType        错误类型
     */
    void signalError(ErrorType eErrorType);

    /**
     * @brief signalFinished    操作结束信号
     */
    void signalFinished();

private Q_SLOTS:


private:
    ArchiveJob *m_pArchiveJob = nullptr;     // 当前操作指针操作
};

#endif // ARCHIVEMANAGER_H
