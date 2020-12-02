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
#include "DebugTimeManager.h"
#include "datamanager.h"
#include "popupdialog.h"

#include <DMenu>
#include <DFileDialog>
#include <DFileDrag>

#include <QHeaderView>
#include <QMouseEvent>
#include <QDebug>
#include <QDateTime>
#include <QApplication>
#include <QMimeData>
#include <QItemSelectionModel>

UnCompressView::UnCompressView(QWidget *parent)
    : DataTreeView(parent)
{
    initUI();
    initConnections();
}

UnCompressView::~UnCompressView()
{

}

void UnCompressView::refreshArchiveData()
{
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();

    // 刷新第一层级文件夹子项的数目
    for (int i = 0; i < stArchiveData.listRootEntry.count(); ++i) {
        if (stArchiveData.listRootEntry[i].isDirectory) {
            stArchiveData.listRootEntry[i].qSize = calDirItemCount(stArchiveData.listRootEntry[i].strFullPath);
        }
    }

    // 刷新数据（显示第一层数据）
    m_pModel->refreshFileEntry(stArchiveData.listRootEntry);
    m_mapShowEntry["/"] = stArchiveData.listRootEntry;

    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(DC_Name);
    resetLevel();       // 重置目录层级
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

    qDebug() << indexAt(m_dragPos).data(Qt::UserRole).value<FileEntry>().strFileName;
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
    // DataTreeView::mouseMoveEvent(event);
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
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();
    qlonglong qItemCount = 0;

    auto iter = stArchiveData.mapFileEntry.find(strFilePath);
    for (; iter != stArchiveData.mapFileEntry.end();) {
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
            refreshDataByCurrentPath();     // 刷新数据
            resizeColumnWidth();            // 重置列宽
            // 重置排序
            m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
            sortByColumn(DC_Name);
        } else {    // 如果是文件，选择默认方式打开
            slotOpen();
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

void UnCompressView::refreshDataByCurrentPathChanged()
{
    if (0 == m_strCurrentPath.compare("/")) { //当前目录是第一层
        m_mapShowEntry.clear();

        refreshArchiveData();
    } else { //当前目录非第一层
        ArchiveData &stArchiveData = DataManager::get_instance().archiveData();

        // 删除上一级m_mapShowEntry
        if (1 == m_strCurrentPath.count("/")) { //当前目录是第二层
            // 刷新第一层文件夹子项的数目
            for (int i = 0; i < stArchiveData.listRootEntry.count(); ++i) {
                if (stArchiveData.listRootEntry[i].isDirectory) {
                    stArchiveData.listRootEntry[i].qSize = calDirItemCount(stArchiveData.listRootEntry[i].strFullPath);
                }
            }
            m_mapShowEntry["/"] = stArchiveData.listRootEntry;
        } else { //当前目录是第三层及之后
            m_mapShowEntry.remove(m_strCurrentPath.left(m_strCurrentPath.lastIndexOf("/", -2) + 1));
        }

        // 更新当前目录m_mapShowEntry
        foreach (auto &tmp, m_mapShowEntry.keys()) {
            if (tmp.startsWith(m_strCurrentPath)) {
                m_mapShowEntry.remove(tmp);
            }
        }
        m_mapShowEntry[m_strCurrentPath] = getCurPathFiles();

        // 刷新列表数据
        m_pModel->refreshFileEntry(m_mapShowEntry[m_strCurrentPath]);
    }

    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(DC_Name);

    // 追加时需要选中追加的文件
    if (m_eChangeType == CT_Add) {
        // 获取所有的追加文件的文件名
        QStringList listSelName;
        foreach (QString strFile, m_listAddFiles) {
            listSelName.push_back(QFileInfo(strFile).fileName());
        }

        // 设置多选
        QItemSelectionModel *pSelectionModel = selectionModel();
        pSelectionModel->select(m_pModel->getSelectItem(listSelName), QItemSelectionModel::SelectCurrent);

        m_listAddFiles.clear(); // 刷新完之后清空追加的文件数据
    }

    m_eChangeType = CT_None;    // 重置操作类型
}

void UnCompressView::addNewFiles(const QStringList &listFiles)
{
    QString strPassword;
    m_listAddFiles.clear();

    // 追加选项判断


    // 重复提示
    ArchiveData stArchiveData = DataManager::get_instance().archiveData();
    bool bApplyAll = false;
    bool bOverwrite = false;
    for (int i = 0; i < listFiles.count(); ++i) {
        // 初始化数据
        QFileInfo fileInfo(listFiles[i]);
        QString strLocalFile = fileInfo.filePath();
        QString strTempName;

        // 处理文件名（和压缩包中格式一致）
        if (fileInfo.isDir()) {
            strTempName = getCurPath() + strLocalFile.remove(0, fileInfo.path().length() + 1) + QDir::separator();  // 移除路径后添加'/'
        } else {
            strTempName = getCurPath() + strLocalFile.remove(0, fileInfo.path().length() + 1);  // 移除路径
        }

        // 文件判断是否有同名存在
        if (stArchiveData.mapFileEntry.find(strTempName) != stArchiveData.mapFileEntry.end()) {
            // 文件/文件夹已存在时，首先判断是否是全部应用
            if (bApplyAll) {
                // 全部应用处理
                if (bOverwrite) {
                    m_listAddFiles = listFiles;   // 若全部替换，追加所有文件
                    break;
                } else {
                    continue;       // 若全部跳过，重复文件不处理
                }
            } else {
                // 若非全部应用，弹出重复提示对话框
                OverwriteQueryDialog dialog(this);
                dialog.showDialog(strTempName, fileInfo.isDir());

                // 获取重复对话框操作的选项
                int iMode = dialog.getDialogResult();
                bApplyAll = dialog.getApplyAll();

                switch (iMode) {
                // 点击关闭按钮直接跳过追加操作
                case -1:
                    bOverwrite = false;
                    return;
                // 点击跳过，不追加此文件/文件夹
                case 0:
                    bOverwrite = false;
                    break;
                // 追加此文件/文件夹
                default:
                    bOverwrite = true;
                    m_listAddFiles << listFiles[i];
                    break;
                }
            }
        } else {
            // 不存在重复文件
            m_listAddFiles << listFiles[i];
        }
    }

    // 发送追加信号
    m_eChangeType = CT_Add;
    emit signalAddFiles2Archive(m_listAddFiles, strPassword);
}

QString UnCompressView::getCurPath()
{
    return (m_iLevel == 0) ? "" : m_strCurrentPath;     // 根目录提取时上级赋值为空
}

void UnCompressView::setModifiable(bool bModifiable, bool bMultiplePassword)
{
    m_bModifiable = bModifiable;                // 压缩包数据是否可修改
    m_bMultiplePassword = bMultiplePassword;    // 压缩包是否支持多密码

    setAcceptDrops(m_bModifiable);
}

//bool UnCompressView::isModifiable()
//{
//    return m_bModifiable;
//}

void UnCompressView::clear()
{
    // 重置数据
    m_stRightEntry = FileEntry();
    m_mapShowEntry.clear();
    DataManager::get_instance().resetArchiveData();
    resetLevel();
    m_strUnCompressPath = "";
    m_strSelUnCompressPath = "";
    m_bModifiable = false;
    m_bMultiplePassword = false;
}

QList<FileEntry> UnCompressView::getCurPathFiles()
{
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();
    QList<FileEntry> listEntry;

    auto iter = stArchiveData.mapFileEntry.find(m_strCurrentPath);
    for (; iter != stArchiveData.mapFileEntry.end() ;) {
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

//void UnCompressView::getAllFilesByParentPath(const QString &strFullPath, QList<FileEntry> &listEntry, qint64 &qSize)
//{
//    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();
//    listEntry.clear();
//    qSize = 0;

//    auto iter = stArchiveData.mapFileEntry.find(strFullPath);
//    for (; iter != stArchiveData.mapFileEntry.end() ;) {

//        if (!iter.key().startsWith(strFullPath)) {
//            break;
//        } else {
//            if (!iter.key().endsWith("/")) {
//                qSize += iter.value().qSize; //文件大小
//            }

//            listEntry << iter.value();

//            ++iter;
//        }
//    }
//}

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
    QList<FileEntry> listSelEntry = getSelEntry();    // 待提取的文件数据
    ExtractionOptions stOptions;    // 提取参数
    stOptions.strTargetPath = strPath;
    stOptions.strDestination = (m_iLevel == 0) ? "" : m_strCurrentPath;     // 根目录提取时上级赋值为空

    // 获取所有文件数据
    foreach (FileEntry entry, listSelEntry) {
        if (entry.isDirectory) {
            QList<FileEntry> listEntry;
            calEntrySizeByParentPath(entry.strFullPath, stOptions.qSize);
        } else {
            stOptions.qSize += entry.qSize;
        }
    }

    // 发送提取信号
    emit signalExtract2Path(listSelEntry, stOptions);
}

void UnCompressView::calEntrySizeByParentPath(const QString &strFullPath, qint64 &qSize)
{
    ArchiveData stArchiveData =  DataManager::get_instance().archiveData();
    qSize = 0;

    auto iter = stArchiveData.mapFileEntry.find(strFullPath);
    for (; iter != stArchiveData.mapFileEntry.end() ;) {

        if (!iter.key().startsWith(strFullPath)) {
            break;
        } else {
            if (!iter.key().endsWith("/")) {
                qSize += iter.value().qSize; //文件大小
            }
            ++iter;
        }
    }
}

void UnCompressView::slotDragFiles(const QStringList &listFiles)
{
    addNewFiles(listFiles);
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
        QAction *pAction = menu.addAction(tr("Delete"), this, &UnCompressView::slotDeleteFile);

        if (!m_bModifiable) {
            pAction->setEnabled(false); // 若压缩包数据不能更改，深处按钮设置成不可用
        }

        // 右键-打开方式
        DMenu openMenu(tr("Open with"), this);
        menu.addMenu(&openMenu);

        // 右键-选择默认应用程序
        openMenu.addAction(tr("Select default program"), this, SLOT(slotOpenStyleClicked()));

        if (m_stRightEntry.isDirectory) {
            openMenu.setEnabled(false);
        } else {
            // 右键-打开方式选项
            QAction *pAction = nullptr;
            // 获取支持的打开方式列表
            QList<DesktopFile> listOpenType = OpenWithDialog::getOpenStyle(m_stRightEntry.strFullPath);
            // 添加菜单选项
            for (int i = 0; i < listOpenType.count(); ++i) {
                pAction = openMenu.addAction(QIcon::fromTheme(listOpenType[i].getIcon()), listOpenType[i].getDisplayName(), this, SLOT(slotOpenStyleClicked()));
                pAction->setData(listOpenType[i].getExec());
            }
        }

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
    // 询问删除对话框
    SimpleQueryDialog dialog(this);
    int iResult = dialog.showDialog("Do you want to delete the selected file(s)?", tr("Cancel"), DDialog::ButtonNormal, tr("Confirm"), DDialog::ButtonRecommend);
    if (iResult == 1) {
        // 删除压缩包数据
        QList<FileEntry> listSelEntry = getSelEntry();    // 待删除的文件数据
        qint64 qSize = 0;       // 所有需要删除的文件总大小

        // 获取所有文件数据
        foreach (FileEntry entry, listSelEntry) {
            if (entry.isDirectory) {
                QList<FileEntry> listEntry;
                calEntrySizeByParentPath(entry.strFullPath, qSize);
            } else {
                qSize += entry.qSize;
            }
        }

        // 发送删除信号
        m_eChangeType = CT_Delete;
        emit signalDelFiles(listSelEntry, qSize);
    }
}

void UnCompressView::slotOpen()
{
    // 获取当前点击的文件
    QModelIndex curIndex = currentIndex();

    if (curIndex.isValid()) {
        FileEntry entry = curIndex.data(Qt::UserRole).value<FileEntry>();

        if (entry.isDirectory) {
            // 文件夹进入下一层
            handleDoubleClick(curIndex);
        } else {
            // 文件 解压再用默认应用程序打开
            emit signalOpenFile(entry);
        }

    }
}

void UnCompressView::slotOpenStyleClicked()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if (pAction == nullptr) {
        return;
    }

    QString strText = pAction->text();

    if (strText == tr("Select default program")) {
        // 用选择的应用程序打开
        OpenWithDialog dialog(m_stRightEntry.strFullPath);
        QString str = dialog.showOpenWithDialog(OpenWithDialog::SelectType);
        if (!str.isEmpty())
            // 发送打开信号（以xx应用打开）
            emit signalOpenFile(m_stRightEntry, str);
    } else {
        // 发送打开信号（以xx应用打开）
        emit signalOpenFile(m_stRightEntry, pAction->data().toString());
    }
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
    sortByColumn(DC_Name);
}
