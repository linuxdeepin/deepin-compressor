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
#include "popupdialog.h"
#include "mimetypes.h"
#include "mimetypedisplaymanager.h"
#include "treeheaderview.h"
#include "openwithdialog.h"
#include "uitools.h"

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
    initUI();           // 初始化参数等
    initConnections();  // 初始化信号槽
}

CompressView::~CompressView()
{

}

void CompressView::addCompressFiles(const QStringList &listFiles)
{
    int mode = 0;
    bool applyAll = false;

    // 对新添加的文件进行判重
    QStringList listSelFiles = listFiles;

    foreach (QString oldPath, m_listCompressFiles) {
        QFileInfo oldFile(oldPath);  // 已存在的文件
        foreach (QString newPath, listFiles) {
            QFileInfo newFile(newPath);  // 新添加的文件
            if (oldFile.fileName() == newFile.fileName()) {  // 文件名相同的文件需要询问是否替换
                if (!applyAll) { // // 判断不是应用到全部文件，继续弹出询问对话框
                    OverwriteQueryDialog dialog(this);
                    dialog.showDialog(newFile.fileName());

                    mode = dialog.getDialogResult();
                    applyAll = dialog.getApplyAll();
                }

                if (mode == 0 || mode == -1) {  // -1：取消  0：跳过
                    listSelFiles.removeOne(newPath); // 在新添加的文件中删除该同名文件
                } else { // 替换
                    m_listCompressFiles.removeOne(oldPath); // 在已存在的文件中删除该同名文件
                }
            }
        }
    }


    m_listCompressFiles << listSelFiles;

    // 刷新待压缩数据
    refreshCompressedFiles();
}

QStringList CompressView::getCompressFiles()
{
    return m_listCompressFiles;
}

void CompressView::refreshCompressedFiles(bool bChanged, const QString &strFileName)
{
    m_listEntry.clear();

    // 对变化的文件进行监控
    if (bChanged && !QFile::exists(strFileName)) {
        m_listCompressFiles.removeOne(strFileName);
    }

    // 刷新待压缩数据
    foreach (QString strFile, m_listCompressFiles) {
        m_listEntry << fileInfo2Entry(QFileInfo((strFile)));
    }

    // 刷新数据显示
    m_pModel->refreshFileEntry(m_listEntry);

    resizeColumnWidth();

    sortByColumn(0);
    resetLevel();   // 重置目录层级
}

void CompressView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        handleDoubleClick(indexAt(event->pos()));   // 双击处理
    }

    DataTreeView::mouseDoubleClickEvent(event);
}

void CompressView::initUI()
{
    m_pFileWatcher = new QFileSystemWatcher(this);
}

void CompressView::initConnections()
{
    connect(this, &CompressView::signalDragFiles, this, &CompressView::slotDragFiles);
    connect(this, &CompressView::customContextMenuRequested, this, &CompressView::slotShowRightMenu);
    connect(m_pFileWatcher, &QFileSystemWatcher::directoryChanged, this, &CompressView::slotDirChanged); // 文件目录变化
}

FileEntry CompressView::fileInfo2Entry(const QFileInfo &fileInfo)
{
    FileEntry entry;

    entry.strFullPath = fileInfo.filePath();    // 文件全路径
    entry.strFileName = fileInfo.fileName();    // 文件名
    entry.isDirectory = fileInfo.isDir();   // 是否是文件夹

    if (entry.isDirectory) {
        // 文件夹显示子文件数目
        entry.qSize = QDir(fileInfo.filePath()).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden).count(); //目录下文件数
    } else {
        // 文件直接显示大小
        entry.qSize = fileInfo.size();
    }

    entry.uLastModifiedTime = fileInfo.lastModified().toTime_t();   // 最后一次修改时间

    return entry;
}

void CompressView::handleDoubleClick(const QModelIndex &index)
{
    m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听

    qDebug() << index.data(Qt::DisplayRole);

    if (index.isValid()) {
        FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();

        if (entry.isDirectory) {     // 如果是文件夹，进入下一层

            if (m_iLevel == 0) {
                setPreLblVisible(true, entry.strFullPath);
            }

            m_iLevel++;
            m_strCurrentPath = entry.strFullPath;
            refreshDataByCurrentPath(); // 刷新数据
            handleLevelChanged();       // 处理层级变化
            resizeColumnWidth();        // 重置列宽
            // 重置排序
            m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
            sortByColumn(0);

        } else {    // 如果是文件，选择默认方式打开
            OpenWithDialog::openWithProgram(entry.strFullPath);
        }
    }
}

QList<FileEntry> CompressView::getCurrentDirFiles()
{
    // 获取当前目录下所有文件信息
    QFileInfoList lisfInfo = QDir(m_strCurrentPath).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);     // 获取当前目录下所有子文件
    QList<FileEntry> listEntry;

    // 文件数据转换
    foreach (QFileInfo info, lisfInfo) {
        listEntry << fileInfo2Entry(info);
    }

    return listEntry;
}

void CompressView::handleLevelChanged()
{
    // 发送层级变化信号，通知mainwindow是否是根目录
    bool bRoot = (m_iLevel == 0 ? true : false);
    setAcceptDrops(bRoot);
    emit signalLevelChanged(bRoot);
}

QString CompressView::getPrePathByLevel(const QString &strPath)
{
    // 若层级为0,即根目录，返回空
    if (m_iLevel == 0) {
        return "";
    }

    QString strResult = strPath;
    QString strTempPath = strPath;
    int iIndex = 0;

    // 根据层级截取全路径字符串，避免目录从文件系统根目录开始
    for (int i = 0; i < m_iLevel; ++i) {
        iIndex = strTempPath.lastIndexOf(QDir::separator());
        strTempPath = strTempPath.left(iIndex);
    }
    strTempPath += QDir::separator();

    return strResult.remove(strTempPath);
}

void CompressView::refreshDataByCurrentPath()
{
    m_pHeaderView->preLbl()->setPrePath(getPrePathByLevel(m_strCurrentPath));   // 返回上一级按钮文字刷新
    m_pFileWatcher->addPath(m_strCurrentPath);      // 添加目录监听
    m_pModel->refreshFileEntry(getCurrentDirFiles());  // 刷新数据显示
}

void CompressView::slotShowRightMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);

    if (index.isValid()) {

        m_stRightEntry = index.data(Qt::UserRole).value<FileEntry>();  // 获取文件数据

        DMenu menu(this);

        menu.setMinimumWidth(202);

        // 右键-打开
        menu.addAction(tr("Open"), this, [ = ] {
            OpenWithDialog::openWithProgram(m_stRightEntry.strFullPath);
        });

        // 右键-删除
        menu.addAction(tr("Delete"), this, &CompressView::slotDeleteFile);

        // 右键-打开方式
        DMenu openMenu(tr("Open with"), this);
        menu.addMenu(&openMenu);

        // 右键-打开方式选项
        QAction *pAction = nullptr;
        // 获取支持的打开方式列表
        QList<DesktopFile> listOpenType = OpenWithDialog::getOpenStyle(m_stRightEntry.strFullPath);
        // 添加菜单选项
        for (int i = 0; i < listOpenType.count(); ++i) {
            pAction = openMenu.addAction(QIcon::fromTheme(listOpenType[i].getIcon()), listOpenType[i].getDisplayName(), this, SLOT(slotOpenStyleClicked()));
            pAction->setData(listOpenType[i].getExec());
        }

        // 右键-选择默认应用程序
        openMenu.addAction(tr("Select default program"), this, SLOT(slotOpenStyleClicked()));

        menu.exec(QCursor::pos());
    }
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
    // 刷新显示数据
    m_pModel->refreshFileEntry(getCurrentDirFiles());
}

void CompressView::slotDragFiles(const QStringList &listFiles)
{
    addCompressFiles(listFiles);
}

void CompressView::slotOpenStyleClicked()
{

    QAction *pAction = qobject_cast<QAction *>(sender());
    if (pAction == nullptr) {
        return;
    }

    QString strText = pAction->text();

    if (strText == tr("Select default program")) {
        // 选择默认应用程序（弹出窗口）
        qDebug() << "选择默认应用程序打开";
        OpenWithDialog dialog(m_stRightEntry.strFullPath);
        dialog.showOpenWithDialog(OpenWithDialog::OpenType);
    } else {
        // 用选择的应用程序打开
        qDebug() << "选择打开方式：" << strText;
        OpenWithDialog::openWithProgram(m_stRightEntry.strFullPath, pAction->data().toString());
    }
}

void CompressView::slotPreClicked()
{
    m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听

    m_iLevel--;     // 目录层级减1
    if (m_iLevel == 0) {    // 如果返回到根目录，显示待压缩文件
        setPreLblVisible(false);    // 设置上一级按钮隐藏
        m_strCurrentPath = "";      // 当前路径置空
        m_pModel->refreshFileEntry(m_listEntry);    // 数据模型刷新
    } else {        // 否则传递上级目录
        int iIndex = m_strCurrentPath.lastIndexOf(QDir::separator());
        m_strCurrentPath = m_strCurrentPath.left(iIndex);   // 当前路径截掉最后一级目录
        refreshDataByCurrentPath();     // 刷新数据
    }

    // 重置宽度
    resizeColumnWidth();
    // 目录层级变化处理
    handleLevelChanged();
    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(0);
}
