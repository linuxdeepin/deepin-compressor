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

    m_pModel->refreshFileEntry(m_stArchiveData.listRootEntry);
}

void UnCompressView::initUI()
{

}

void UnCompressView::initConnections()
{
    connect(this, &UnCompressView::signalDragFiles, this, &UnCompressView::slotDragFiles);
}

qlonglong UnCompressView::calDirItemCount(const QString &strFilePath)
{
    qlonglong qItemCount = 0;

    auto iter = m_stArchiveData.mapFileEntry.find(strFilePath);
    for (; iter != m_stArchiveData.mapFileEntry.end();) {
        if (!iter.key().startsWith(strFilePath)) {
            break;
        } else {
            if (iter.key().size() > strFilePath.size()) {
                QString chopStr = iter.key().right(iter.key().size() - strFilePath.size());
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    ++qItemCount;
                }
            }

            ++iter;
        }
    }

    return qItemCount;
}

void UnCompressView::slotDragFiles(const QStringList &listFiles)
{

}

void UnCompressView::slotPreClicked()
{

}
