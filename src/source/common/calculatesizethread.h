/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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
#ifndef CALCULATESIZETHREAD_H
#define CALCULATESIZETHREAD_H

#include "commonstruct.h"
#include <QThread>

/**
 * @brief The CalculateSizeThread class
 * 多线程递归计算文件(夹)大小
 */
class CalculateSizeThread: public QThread
{
    Q_OBJECT
public:
    explicit CalculateSizeThread(QStringList sfiles, QString strArchiveFullPath, QList<FileEntry> listAddEntry, CompressOptions stOptions, QObject *parent = nullptr);
    void set_thread_stop(bool thread_stop);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    void ConstructAddOptionsByThread(const QString &path);

Q_SIGNALS:
    void signalFinishCalculateSize(qint64 size, QString strArchiveFullPath, QList<FileEntry> listAddEntry, CompressOptions stOptions, QList<FileEntry> listAllEntry);
    void signalError(const QString &strError, const QString &strToolTip);

private:
    QStringList m_files;
    QString m_strArchiveFullPath;
    QList<FileEntry> m_listAddEntry;
    CompressOptions m_stOptions;
    qint64 m_qTotalSize = 0;
    QList<FileEntry> m_listAllEntry;
    bool m_thread_stop = false; // 结束线程
};

#endif // CALCULATESIZETHREAD_H
