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

#include "compresspage.h"
#include "compressview.h"
#include "customwidget.h"
#include "uitools.h"
#include "popupdialog.h"

#include <DDialog>
#include <DPalette>

#include <QHBoxLayout>
#include <QShortcut>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QFileInfo>

DGUI_USE_NAMESPACE

CompressPage::CompressPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

CompressPage::~CompressPage()
{

}

void CompressPage::addCompressFiles(const QStringList &listFiles)
{
    QStringList listExistFiles = m_pCompressView->getCompressFiles();

    int mode = 0;
    bool applyAll = false;

    // 对新添加的文件进行判重
    QStringList listSelFiles = listFiles;

    foreach (QString oldPath, listExistFiles) {
        QFileInfo oldFile(oldPath);  // 已存在的文件
        foreach (QString newPath, listFiles) {
            QFileInfo newFile(newPath);  // 新添加的文件
            if (oldFile.fileName() == newFile.fileName()) {  // 文件名相同的文件需要询问是否替换
                if (!applyAll) { // // 判断不是应用到全部文件，继续弹出询问对话框
                    OverwriteQueryDialog dialog;
                    dialog.showDialog(newFile.fileName());

                    mode = dialog.getDialogResult();
                    applyAll = dialog.getApplyAll();
                }

                if (mode == 0 || mode == -1) {  // -1：取消  0：跳过
                    listSelFiles.removeOne(newPath); // 在新添加的文件中删除该同名文件
                } else { // 替换
                    listExistFiles.removeOne(oldPath); // 在已存在的文件中删除该同名文件
                }
            }
        }
    }


    m_pCompressView->addCompressFiles(listSelFiles);
}

QStringList CompressPage::compressFiles()
{
    return m_pCompressView->getCompressFiles();
}

void CompressPage::initUI()
{
    // 初始化相关变量
    m_pCompressView = new CompressView(this);
    m_pNextBtn = new CustomPushButton(tr("Next"), this);
    m_pNextBtn->setMinimumSize(340, 36);

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch();
    pBtnLayout->addWidget(m_pNextBtn);
    pBtnLayout->addStretch();

    // 主界面布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(m_pCompressView);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setStretchFactor(m_pCompressView, 10);
    pMainLayout->setStretchFactor(pBtnLayout, 1);
    pMainLayout->setContentsMargins(12, 1, 20, 20);

    // 设置快捷键
    auto openkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, &CompressPage::signalFileChoose);

    setAcceptDrops(true);
    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void CompressPage::initConnections()
{
    connect(m_pNextBtn, &DPushButton::clicked, this, &CompressPage::slotCompressNextClicked);
    connect(m_pCompressView, &CompressView::signalLevelChanged, this, &CompressPage::slotCompressLevelChanged);
}

void CompressPage::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // 判断是否有url
    if (!mime->hasUrls()) {
        e->ignore();
    }

    e->accept();
}

void CompressPage::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void CompressPage::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (false == mime->hasUrls()) {
        e->ignore();
    }

    e->accept();

    // 判断本地文件
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        fileList << url.toLocalFile();
    }

    if (fileList.size() == 0) {
        return;
    }

    addCompressFiles(fileList);
}

void CompressPage::slotCompressNextClicked()
{
    qDebug() << "点击了压缩下一步按钮";

    // 如果没有待压缩文件，弹出提示框
    if (m_pCompressView->getCompressFiles().isEmpty()) {

        DDialog *dialog = new DDialog(this);

        QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(30, 30));
        dialog->setIcon(pixmap);
        dialog->setMessage(tr("Please add files"));
        dialog->addButton(tr("OK"));
        dialog->addSpacing(15);

        dialog->exec();
        delete dialog;

    } else {
        emit signalCompressNextClicked();  // 发送下一步信号
    }
}

void CompressPage::slotCompressLevelChanged(bool bRootIndex)
{
    emit signalLevelChanged(bRootIndex);

    setAcceptDrops(bRootIndex);
}
