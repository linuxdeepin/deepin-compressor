// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openFileWatcher.h"
#include <QDebug>

#include <QFileSystemWatcher>


OpenFileWatcher::OpenFileWatcher(QObject *parent)
    : QObject(parent)
{
    qDebug() << "Initializing OpenFileWatcher";
    m_pOpenFileWatcher = new QFileSystemWatcher(this);
    connect(m_pOpenFileWatcher, &QFileSystemWatcher::fileChanged, this, &OpenFileWatcher::fileChanged);
}

OpenFileWatcher::~OpenFileWatcher()
{

}

void OpenFileWatcher::reset()
{
    qDebug() << "Resetting OpenFileWatcher, removing" << m_pOpenFileWatcher->files().size() << "watched files";
    m_pOpenFileWatcher->removePaths(m_pOpenFileWatcher->files());       // 清空所有监控的文件
    m_strOpenFile.clear();
    m_mapFileHasModified.clear();
    m_mapFilePassword.clear();
    qDebug() << "OpenFileWatcher reset completed";
}

void OpenFileWatcher::setCurOpenFile(const QString &strCurOpenFile)
{
    m_strOpenFile = strCurOpenFile;
}

void OpenFileWatcher::addCurOpenWatchFile()
{
    qDebug() << "Adding file to watch:" << m_strOpenFile;
    m_pOpenFileWatcher->addPath(m_strOpenFile); // 添加监控
    m_mapFileHasModified[m_strOpenFile] = false;    // 初始监控状态
}

void OpenFileWatcher::addWatchFile(const QString &strFile)
{
    qDebug() << "Adding watch file:" << strFile;
    m_pOpenFileWatcher->addPath(strFile);

    // 第一次默认文件未修改
    if (m_mapFileHasModified.find(strFile) != m_mapFileHasModified.end()) {
        qDebug() << "File already in watch list, resetting modified flag";
        m_mapFileHasModified[m_strOpenFile] = false;
    }
}

QMap<QString, bool> &OpenFileWatcher::getFileHasModified()
{
    return m_mapFileHasModified;
}

void OpenFileWatcher::setCurFilePassword(const QString &strPassword)
{
    qDebug() << "Setting password for file:" << m_strOpenFile;
    m_mapFilePassword[m_strOpenFile] = strPassword;
}

QMap<QString, QString> OpenFileWatcher::getFilePassword()
{
    return m_mapFilePassword;
}
