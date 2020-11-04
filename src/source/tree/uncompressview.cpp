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

#include "uncompressview.h"
#include "datamodel.h"
#include "treeheaderview.h"
#include "openwithdialog.h"
#include "DebugTimeManager.h";

#include <DMenu>
#include <DFileDialog>
#include <DFileDrag>

#include <QHeaderView>
#include <QMouseEvent>
#include <QDebug>
#include <QDateTime>
#include <QApplication>
#include <QMimeData>

UnCompressView::UnCompressView(QWidget *parent)
    : DataTreeView(parent)
{
    initUI();
    initConnections();
}

UnCompressView::~UnCompressView()
{

}

void UnCompressView::setArchiveData(const ArchiveData &stArchiveData)
{
    m_stArchiveData = stArchiveData;

    // 刷新第一层级文件夹子项的数目
    for (int i = 0; i < m_stArchiveData.listRootEntry.count(); ++i) {
        if (m_stArchiveData.listRootEntry[i].isDirectory) {
            m_stArchiveData.listRootEntry[i].qSize = calDirItemCount(m_stArchiveData.listRootEntry[i].strFullPath);
        }
    }

    // 刷新数据（显示第一层数据）
    m_pModel->refreshFileEntry(m_stArchiveData.listRootEntry);
    m_mapShowEntry["/"] = m_stArchiveData.listRootEntry;

    // 重置目录层级
    resetLevel();
}

void UnCompressView::setDefaultUncompressPath(const QString &strPath)
{
    m_strUnCompressPath = strPath;
}

void UnCompressView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_dragPos = event->pos();
    }

    viewport()->update();

    DataTreeView::mousePressEvent(event);
}

void UnCompressView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndexList listSel = selectedIndexes();

    if (listSel.size() < 1) {
        return;
    }

    if (!(event->buttons() & Qt::MouseButton::LeftButton) || m_pFileDragServer) {
        return;
    }

    // 曼哈顿距离处理，避免误操作
    if ((event->pos() - m_dragPos).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    // 创建文件拖拽服务，处理拖拽到文管操作
    m_pFileDragServer = new DFileDragServer(this);
    DFileDrag *drag = new DFileDrag(this, m_pFileDragServer);
    QMimeData *m = new QMimeData();

    QVariant value = listSel[0].data(Qt::DecorationRole);

    if (value.isValid()) {
        if (value.type() == QVariant::Pixmap) {
            drag->setPixmap(qvariant_cast<QPixmap>(value));
        } else if (value.type() == QVariant::Icon) {
            drag->setPixmap((qvariant_cast<QIcon>(value)).pixmap(24, 24));
        }
    }

    drag->setMimeData(m);

    // 拖拽操作连接槽函数，返回目标路径
    connect(drag, &DFileDrag::targetUrlChanged, this, &UnCompressView::slotDragPath);
    Qt::DropAction result = drag->exec(Qt::CopyAction);

    m_pFileDragServer->setProgress(100);
    m_pFileDragServer->deleteLater();
    m_pFileDragServer = nullptr;
    qDebug() << "sigdragLeave";

    if (result == Qt::DropAction::CopyAction) {
        extract2Path(m_strSelUnCompressPath);
    }

    m_strSelUnCompressPath.clear();
    DataTreeView::mouseMoveEvent(event);
}

void UnCompressView::mouseReleaseEvent(QMouseEvent *event)
{
    DataTreeView::mouseReleaseEvent(event);
}

void UnCompressView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        handleDoubleClick(indexAt(event->pos()));   // 双击处理
    }

    DataTreeView::mouseDoubleClickEvent(event);
}

void UnCompressView::initUI()
{

}

void UnCompressView::initConnections()
{
    connect(this, &UnCompressView::signalDragFiles, this, &UnCompressView::slotDragFiles);
    connect(this, &UnCompressView::customContextMenuRequested, this, &UnCompressView::slotShowRightMenu);
}

qlonglong UnCompressView::calDirItemCount(const QString &strFilePath)
{
    qlonglong qItemCount = 0;

    auto iter = m_stArchiveData.mapFileEntry.find(strFilePath);
    for (; iter != m_stArchiveData.mapFileEntry.end();) {
        // 找到以当前文件夹开头的数据，进行计算处理
        if (!iter.key().startsWith(strFilePath)) {
            break;
        } else {
            if (iter.key().size() > strFilePath.size()) {
                QString chopStr = iter.key().right(iter.key().size() - strFilePath.size());
                // 只计算当前目录中的第一层数据（包含文件和文件夹）
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    ++qItemCount;
                }
            }

            ++iter;
        }
    }

    return qItemCount;
}

void UnCompressView::handleDoubleClick(const QModelIndex &index)
{
    qDebug() << index.data(Qt::DisplayRole);

    if (index.isValid()) {
        FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();

        if (entry.isDirectory) {     // 如果是文件夹，进入下一层
            m_iLevel++;
            m_strCurrentPath = entry.strFullPath;
            refreshDataByCurrentPath(); // 刷新数据
            resizeColumnWidth();        // 重置列宽
            // 重置排序
            m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
            sortByColumn(0);
        } else {    // 如果是文件，选择默认方式打开

        }
    }
}

void UnCompressView::refreshDataByCurrentPath()
{
    if (m_iLevel == 0) {
        setPreLblVisible(false);
    } else {
        QString strTempPath = m_strCurrentPath;
        int iIndex = strTempPath.lastIndexOf(QDir::separator());
        if (iIndex > 1)
            strTempPath = strTempPath.left(iIndex);
        setPreLblVisible(true, strTempPath);
    }

    // 若缓存中找不到下一层数据，从总数据中查找并同步更新到缓存数据中
    if (m_mapShowEntry.find(m_strCurrentPath) == m_mapShowEntry.end()) {
        m_mapShowEntry[m_strCurrentPath] = getCurPathFiles();
    }

    m_pModel->refreshFileEntry(m_mapShowEntry[m_strCurrentPath]);
}

QList<FileEntry> UnCompressView::getCurPathFiles()
{
    QList<FileEntry> listEntry;

    auto iter = m_stArchiveData.mapFileEntry.find(m_strCurrentPath);
    for (; iter != m_stArchiveData.mapFileEntry.end() ;) {
        if (iter.key().left(m_strCurrentPath.size()) != m_strCurrentPath) {
            break;
        } else {
            QString chopStr = iter.key().right(iter.key().size() - m_strCurrentPath.size());
            if (!chopStr.isEmpty()) {
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    FileEntry &entry = iter.value();

                    // 如果是文件夹，刷新大小（同步更新map压缩包中缓存数据）
                    if (entry.isDirectory) {
                        entry.qSize = calDirItemCount(entry.strFullPath);
                    }

                    listEntry << iter.value();
                }
            }
            ++iter;
        }
    }

    return listEntry;
}

void UnCompressView::getAllFilesByParentPath(const QString &strFullPath, QList<FileEntry> &listEntry, qint64 &qSize)
{
    listEntry.clear();
    qSize = 0;

    auto iter = m_stArchiveData.mapFileEntry.find(strFullPath);
    for (; iter != m_stArchiveData.mapFileEntry.end() ;) {

        if (!iter.key().startsWith(strFullPath)) {
            break;
        } else {
            if (!iter.key().endsWith("/")) {
                qSize += iter.value().qSize; //文件大小
            }

            listEntry << iter.value();

            ++iter;
        }
    }
}

QList<FileEntry> UnCompressView::getSelEntry()
{
    QList<FileEntry> listSelEntry;

    QModelIndexList listModelIndex = selectionModel()->selectedRows();

    foreach (QModelIndex index, listModelIndex) {
        if (index.isValid()) {
            FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();  // 获取文件数据
            listSelEntry << entry;
        }
    }

    return listSelEntry;
}

void UnCompressView::extract2Path(const QString &strPath)
{
    QList<FileEntry> listSelCurEntry = getSelEntry();    // 待提取的文件数据
    QList<FileEntry> listSelAllEntry;    // 待提取的文件数据
    ExtractionOptions stOptions;    // 提取参数
    stOptions.strTargetPath = strPath;
    stOptions.strDestination = m_strCurrentPath;

    // 获取所有文件数据
    foreach (FileEntry entry, listSelCurEntry) {
        if (entry.isDirectory) {
            QList<FileEntry> listEntry;
            getAllFilesByParentPath(entry.strFullPath, listEntry, stOptions.qSize);
            listSelAllEntry << listEntry;
        } else {
            listSelAllEntry << entry;
            stOptions.qSize += entry.qSize;
        }
    }

    // 发送提取信号
    emit signalExtract2Path(listSelCurEntry, listSelAllEntry, stOptions);
}

void UnCompressView::slotDragFiles(const QStringList &listFiles)
{

}

void UnCompressView::slotShowRightMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);

    if (index.isValid()) {
        m_stRightEntry = index.data(Qt::UserRole).value<FileEntry>();  // 获取文件数据

        DMenu menu(this);
        menu.setMinimumWidth(202);

        // 右键-提取
        menu.addAction(tr("Extract"), this, &UnCompressView::slotExtract);
        // 右键-提取到当前文件夹
        menu.addAction(tr("Extract to current directory"), this, &UnCompressView::slotExtract2Here);
        // 右键-打开
        menu.addAction(tr("Open"), this, &UnCompressView::slotOpen);
        // 右键-删除
        menu.addAction(tr("Delete"), this, &UnCompressView::slotDeleteFile);

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

void UnCompressView::slotExtract()
{
    // 创建文件选择对话框
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::Directory);
    dialog.setDirectory(m_strUnCompressPath);

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    // 获取选择的路径
    QList<QUrl> listUrl = dialog.selectedUrls();
    if (listUrl.count() > 0) {
        m_strSelUnCompressPath = listUrl.at(0).toLocalFile();
        extract2Path(m_strSelUnCompressPath);
    }

}

void UnCompressView::slotExtract2Here()
{
    extract2Path(m_strUnCompressPath);
}

void UnCompressView::slotDeleteFile()
{
    QList<FileEntry> listSelCurEntry = getSelEntry();    // 待提取的文件数据
    QList<FileEntry> listSelAllEntry;    // 待提取的文件数据
    qint64 qSize = 0;

    // 获取所有文件数据
    foreach (FileEntry entry, listSelCurEntry) {
        if (entry.isDirectory) {
            QList<FileEntry> listEntry;
            getAllFilesByParentPath(entry.strFullPath, listEntry, qSize);
            listSelAllEntry << listEntry;
        } else {
            listSelAllEntry << entry;
            qSize += entry.qSize;
        }
    }

    emit signalDelFiels(listSelCurEntry, listSelAllEntry, qSize);
}

void UnCompressView::slotOpen()
{

}

void UnCompressView::slotOpenStyleClicked()
{

}

void UnCompressView::slotDragPath(QUrl url)
{
    m_strSelUnCompressPath = url.toLocalFile(); // 获取拖拽提取目标路径
}

void UnCompressView::slotPreClicked()
{
    m_iLevel--;     // 目录层级减1

    if (m_iLevel == 0) {    // 如果返回到根目录，显示待压缩文件
        resetLevel();
    } else {        // 否则传递上级目录
        // 如果以'/'结尾,先移除最后一个'/',方便接下来截取倒数第二个'/'
        if (m_strCurrentPath.endsWith(QDir::separator())) {
            m_strCurrentPath.chop(1);
        }
        int iIndex = m_strCurrentPath.lastIndexOf(QDir::separator());
        m_strCurrentPath = m_strCurrentPath.left(iIndex + 1); // 当前路径截掉最后一级目录(保留'/')

    }

    refreshDataByCurrentPath();     // 刷新数据

    // 重置宽度
    resizeColumnWidth();
    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(0);
}
