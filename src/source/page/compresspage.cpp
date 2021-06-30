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
    , m_bRootIndex(false)
{
    initUI();
    initConnections();
}

CompressPage::~CompressPage()
{

}

void CompressPage::addCompressFiles(const QStringList &listFiles)
{
    m_pCompressView->addCompressFiles(listFiles);
}

QStringList CompressPage::compressFiles()
{
    return m_pCompressView->getCompressFiles();
}

void CompressPage::refreshCompressedFiles(bool bChanged, const QString &strFileName)
{
    m_pCompressView->refreshCompressedFiles(bChanged, strFileName);
}

void CompressPage::clear()
{
    m_pCompressView->clear();
}

void CompressPage::initUI()
{
    // 初始化相关变量
    m_pCompressView = new CompressView(this);
    m_pNextBtn = new CustomPushButton(tr("Next"), this);
    m_pNextBtn->setMinimumSize(340, 36);

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pNextBtn, 2);   // 按照比例缩放
    pBtnLayout->addStretch(1);

    // 主界面布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(m_pCompressView);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setStretchFactor(m_pCompressView, 10);
    pMainLayout->setStretchFactor(pBtnLayout, 1);
    pMainLayout->setContentsMargins(20, 1, 20, 20);

    // 设置快捷键
    auto openkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, &CompressPage::slotFileChoose);

    setAcceptDrops(true);
    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void CompressPage::initConnections()
{
    connect(m_pNextBtn, &DPushButton::clicked, this, &CompressPage::slotCompressNextClicked);
    connect(m_pCompressView, &CompressView::signalLevelChanged, this, &CompressPage::slotCompressLevelChanged);
}

void CompressPage::slotCompressNextClicked()
{
    qDebug() << "点击了压缩下一步按钮";

    // 如果没有待压缩文件，弹出提示框
    if (m_pCompressView->getCompressFiles().isEmpty()) {
        TipDialog dialog(this);
        dialog.showDialog(tr("Please add files"), tr("OK", "button"));
    } else {
        emit signalCompressNextClicked();  // 发送下一步信号
    }
}

void CompressPage::slotCompressLevelChanged(bool bRootIndex)
{
    m_bRootIndex = bRootIndex;
    emit signalLevelChanged(bRootIndex);

    setAcceptDrops(bRootIndex);
}

void CompressPage::slotFileChoose()
{
    if (m_bRootIndex)
        emit signalFileChoose();
}

CustomPushButton *CompressPage::getNextBtn() const
{
    return m_pNextBtn;
}

CompressView *CompressPage::getCompressView()
{
    return m_pCompressView;
}
