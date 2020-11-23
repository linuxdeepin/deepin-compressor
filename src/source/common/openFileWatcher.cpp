/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <gaoxiang@uniontech.com>
*
* Maintainer: chendu <gaoxiang@uniontech.com>
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

#include "openFileWatcher.h"

#include <QFileSystemWatcher>


OpenFileWatcher::OpenFileWatcher(QObject *parent)
    : QObject(parent)
{
    m_pOpenFileWatcher = new QFileSystemWatcher(this);
    connect(m_pOpenFileWatcher, &QFileSystemWatcher::fileChanged, this, &OpenFileWatcher::fileChanged);
}

OpenFileWatcher::~OpenFileWatcher()
{

}

void OpenFileWatcher::reset()
{
    m_pOpenFileWatcher->removePaths(m_pOpenFileWatcher->files());       // 清空所有监控的文件
    m_listOpenFiles.clear();
    m_strOpenFile.clear();
    m_mapFileHasModified.clear();
}

void OpenFileWatcher::setCurOpenFile(const QString &strCurOpenFile)
{
    m_strOpenFile = strCurOpenFile;
}

void OpenFileWatcher::addCurOpenWatchFile()
{
    m_pOpenFileWatcher->addPath(m_strOpenFile); // 添加监控
    m_mapFileHasModified[m_strOpenFile] = false;    // 初始监控状态
}

void OpenFileWatcher::addWatchFile(const QString &strFile)
{
    m_pOpenFileWatcher->addPath(strFile);

    // 第一次默认文件未修改
    if (m_mapFileHasModified.find(strFile) != m_mapFileHasModified.end()) {
        m_mapFileHasModified[m_strOpenFile] = false;
    }
}

QMap<QString, bool> &OpenFileWatcher::getFileHasModified()
{
    return m_mapFileHasModified;
}
