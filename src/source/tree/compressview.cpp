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

#include "compressview.h"
#include "datamodel.h"
#include "uistruct.h"
#include "sortfiltermodel.h"
#include "popupdialog.h"
#include "mimetypes.h"
#include "mimetypedisplaymanager.h"

#include <DMenu>

#include <QHeaderView>
#include <QMouseEvent>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileSystemWatcher>

CompressView::CompressView(QWidget *parent)
    : DataTreeView(parent)
{
    initUI();
    initConnections();
}

CompressView::~CompressView()
{

}

void CompressView::addCompressFiles(const QStringList &listFiles)
{
    m_listCompressFiles << listFiles;

    // 刷新待压缩数据
    foreach (QString strFile, listFiles) {

        QFileInfo fileInfo(strFile);
        FileEntry entry;
        QMimeType mimetype;

        entry.strFullPath = fileInfo.filePath();
        entry.strFileName = fileInfo.fileName();
        entry.isDirectory = fileInfo.isDir();
        if (entry.isDirectory) {
            mimetype = determineMimeType(entry.strFullPath);
            entry.qSize = QDir(strFile).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden).count(); //目录下文件数
        } else {
            mimetype = determineMimeType(entry.strFileName);
            entry.qSize = fileInfo.size(); //文件大小
        }
        MimeTypeDisplayManager m_mimetype;
        entry.strType = m_mimetype.displayName(mimetype.name()); // 文件类型
        entry.lastModifiedTime = fileInfo.lastModified();

        m_listEntry << entry;
    }

    m_pModel->refreshFileEntry(m_listEntry);

    //m_pModel->setCurrentLevel(0, m_listCompressFiles);  // 设置第一层显示数据

    resizeColumnWidth();
    //header()->setSortIndicator(0, Qt::AscendingOrder);

    // m_pSortFilterModel->sort(0, Qt::AscendingOrder);

}

QStringList CompressView::getCompressFiles()
{
    return m_listCompressFiles;
}

void CompressView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        handleDoubleClick(indexAt(event->pos()));
    }

    DataTreeView::mouseDoubleClickEvent(event);
}

void CompressView::initUI()
{
    m_pModel = new DataModel(this);
//    m_pSortFilterModel = new SortFilterModel(this);

//    m_pSortFilterModel->setFilterKeyColumn(0);
//    m_pSortFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

//    m_pSortFilterModel->setSourceModel(m_pModel);
    setModel(m_pModel);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);    // 设置自定义右键菜单


    m_pFileWatcher = new QFileSystemWatcher(this);
}

void CompressView::initConnections()
{
    connect(this, &CompressView::customContextMenuRequested, this, &CompressView::slotShowRightMenu);
    connect(m_pFileWatcher, &QFileSystemWatcher::directoryChanged, this, &CompressView::slotDirChanged); // 文件目录变化
}

void CompressView::handleDoubleClick(const QModelIndex &index)
{
    m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听

    qDebug() << index.data(Qt::DisplayRole);

    if (index.isValid()) {
        FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();

        if (entry.isDirectory) {     // 如果是文件夹，进入下一层
            m_iLevel++;
            m_strCurrentPath = entry.strFullPath;
            qDebug() << "当前目录" << m_strCurrentPath;
            m_pFileWatcher->addPath(m_strCurrentPath);      // 添加目录监听
            m_pModel->refreshFileEntry(getDirFiles(m_strCurrentPath));

            // 发送层级变化信号，通知mainwindow是否是根目录
            bool bRoot = (m_iLevel == 0 ? true : false);
            emit signalLevelChanged(bRoot);

            resizeColumnWidth();
            header()->setSortIndicator(0, Qt::AscendingOrder);

        } else {    // 如果是文件，选择默认方式打开

        }
    }
}

QList<FileEntry> CompressView::getDirFiles(const QString &strPath)
{
    QFileInfoList lisfInfo = QDir(strPath).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);     // 获取当前目录下所有子文件
    QList<FileEntry> listEntry;

    foreach (QFileInfo info, lisfInfo) {
        FileEntry entry;
        QMimeType mimetype;

        entry.strFullPath = info.filePath();
        entry.strFileName = info.fileName();
        entry.isDirectory = info.isDir();
        if (entry.isDirectory) {
            mimetype = determineMimeType(entry.strFullPath);
            entry.qSize = QDir(info.filePath()).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden).count(); //目录下文件数
        } else {
            mimetype = determineMimeType(entry.strFileName);
            entry.qSize = info.size(); //文件大小
        }
        MimeTypeDisplayManager m_mimetype;
        entry.strType = m_mimetype.displayName(mimetype.name()); // 文件类型
        entry.lastModifiedTime = info.lastModified();

        listEntry << entry;
    }

    return listEntry;
}

void CompressView::slotShowRightMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);

    if (index.isValid()) {
        DMenu menu(this);

        menu.setMinimumWidth(202);
        menu.addAction(tr("Open"));
        menu.addAction(tr("Delete"), this, &CompressView::slotDeleteFile);

        if (m_iLevel > 0) {
            QString strTemp;
            if (m_iLevel == 1) {
                strTemp = tr("Return") + "根目录";
            } else {
                strTemp = tr("Return") + "..";
            }

            menu.addAction(strTemp, this, &CompressView::slotBackToPre);
        }

        DMenu openMenu(tr("Open with"), this);
        menu.addMenu(&openMenu);

        menu.exec(QCursor::pos());
    }
}

void CompressView::slotBackToPre()
{
    m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听

    m_iLevel--;
    if (m_iLevel == 0) {    // 如果返回到根目录，显示待压缩文件
        m_strCurrentPath = "";
        m_pModel->refreshFileEntry(m_listEntry);
    } else {        // 否则传递上级目录
        m_strCurrentPath += "/..";
        m_pFileWatcher->addPath(m_strCurrentPath);      // 添加目录监听
        m_pModel->refreshFileEntry(getDirFiles(m_strCurrentPath));
    }
    qDebug() << "返回到上一级：" << m_iLevel << " 目录层级：" << m_strCurrentPath;

    resizeColumnWidth();
    header()->setSortIndicator(0, Qt::AscendingOrder);

    // 发送层级变化信号，通知mainwindow是否是根目录
    bool bRoot = (m_iLevel == 0 ? true : false);
    emit signalLevelChanged(bRoot);
}

void CompressView::slotDeleteFile()
{
    QModelIndexList selectedIndex = selectionModel()->selectedRows(0);    // 获取所有待删除的第一行index

    if (m_iLevel == 0) { // 如果是根目录，删除缓存文件名

        // 从内存中删除选中的文件
        foreach (QModelIndex index, selectedIndex) {
            if (index.isValid()) {
                FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();
                int iRemoveIndex = m_listCompressFiles.indexOf(entry.strFullPath);

                // 从缓存中同步移除数据
                m_listCompressFiles.removeAt(iRemoveIndex);
                m_listEntry.removeAt(iRemoveIndex);
            }
        }

        m_pModel->refreshFileEntry(m_listEntry);   // 刷新列表数据

    } else { // 提示是否删除本地文件

        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(tr("It will permanently delete the file(s). Are you sure you want to continue?"), tr("Cancel"), DDialog::ButtonNormal, tr("Confirm"), DDialog::ButtonWarning);
        if (iResult == 1) {     // 如果点击确定，移动本地文件至回收站中
            // 从本地文件中将需要删除的文件移动到回收站中
            foreach (QModelIndex index, selectedIndex) {
                if (index.isValid()) {
                    FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();
                    if (QFileInfo(entry.strFullPath).exists()) {
                        QString newname = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/Trash/files/") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss-") + entry.strFileName;
                        qDebug() << "移除本地文件：" << entry.strFullPath << " 到回收站：" << newname << " 结果：" << QFile::rename(entry.strFullPath, newname);
                    }
                }
            }
        } else {    // 点击关闭或者取消，不操作
            return;
        }
    }
}

void CompressView::slotDirChanged()
{
    // 本地文件有变化
    m_pModel->refreshFileEntry(getDirFiles(m_strCurrentPath));
}
