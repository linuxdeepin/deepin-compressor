// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compresspage.h"
#include "compressview.h"
#include "customwidget.h"
#include "uitools.h"
#include "popupdialog.h"
#include "common/ddlog.h"

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
    qCDebug(appLog) << "CompressPage constructor called";
    initUI();
    initConnections();
    qCDebug(appLog) << "CompressPage initialization completed";
}

CompressPage::~CompressPage()
{
    qCDebug(appLog) << "CompressPage destructor called";
}

void CompressPage::addCompressFiles(const QStringList &listFiles)
{
    qCDebug(appLog) << "Adding compress files, count:" << listFiles.size();
    m_pCompressView->addCompressFiles(listFiles);
    qCDebug(appLog) << "Compress files added successfully";
}

QStringList CompressPage::compressFiles()
{
    qCDebug(appLog) << "Getting compress files list";
    QStringList files = m_pCompressView->getCompressFiles();
    qCDebug(appLog) << "Current compress files count:" << files.size();
    return files;
}

QList<FileEntry> CompressPage::getEntrys()
{
    return m_pCompressView->getEntrys();
}

void CompressPage::refreshCompressedFiles(bool bChanged, const QString &strFileName)
{
    qCInfo(appLog) << "Refreshing compressed files, changed:" << bChanged << "file:" << strFileName;
    m_pCompressView->refreshCompressedFiles(bChanged, strFileName);
    qCInfo(appLog) << "Compressed files refreshed";
}

void CompressPage::clear()
{
    qCDebug(appLog) << "Clearing compress page";
    m_pCompressView->clear();
    qCDebug(appLog) << "Compress page cleared";
}

void CompressPage::initUI()
{
    qCInfo(appLog) << "Initializing compress page UI";
    // 初始化相关变量
    m_pCompressView = new CompressView(this);
    m_pNextBtn = new CustomPushButton(tr("Next"), this);
    m_pNextBtn->setMinimumWidth(340);

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
    pMainLayout->setContentsMargins(10, 1, 10, 20);

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
    qCInfo(appLog) << "Initializing compress page connections";
    connect(m_pNextBtn, &DPushButton::clicked, this, &CompressPage::slotCompressNextClicked);
    connect(m_pCompressView, &CompressView::signalLevelChanged, this, &CompressPage::slotCompressLevelChanged);
}

void CompressPage::slotCompressNextClicked()
{
    qCInfo(appLog) << "点击了压缩下一步按钮";

    // 如果没有待压缩文件，弹出提示框
    if (m_pCompressView->getCompressFiles().isEmpty()) {
        qCInfo(appLog) << "No files to compress, showing tip dialog";
        TipDialog dialog(this);
        dialog.showDialog(tr("Please add files"), tr("OK", "button"));
    } else {
        qCInfo(appLog) << "Files to compress, emit signalCompressNextClicked";
        emit signalCompressNextClicked();  // 发送下一步信号
    }
}

void CompressPage::slotCompressLevelChanged(bool bRootIndex)
{
    qCInfo(appLog) << "Compress level changed, root index:" << bRootIndex;
    m_bRootIndex = bRootIndex;
    emit signalLevelChanged(bRootIndex);
    setAcceptDrops(bRootIndex);
    qCInfo(appLog) << "Compress level change processed";
}

void CompressPage::slotFileChoose()
{
    qCDebug(appLog) << "File choose requested, root index:" << m_bRootIndex;
    if (m_bRootIndex) {
        qCDebug(appLog) << "Emitting file choose signal";
        emit signalFileChoose();
    }
}

CustomPushButton *CompressPage::getNextBtn() const
{
    // qCInfo(appLog) << "Getting next button pointer";
    return m_pNextBtn;
}

CompressView *CompressPage::getCompressView()
{
    // qCInfo(appLog) << "Getting compress view pointer";
    return m_pCompressView;
}
