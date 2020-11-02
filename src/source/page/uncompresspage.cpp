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

#include "uncompresspage.h"
#include "uncompressview.h"
#include "customwidget.h"
#include "popupdialog.h"
#include "DebugTimeManager.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QDebug>
#include <QFileInfo>

UnCompressPage::UnCompressPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

UnCompressPage::~UnCompressPage()
{

}

void UnCompressPage::setArchiveName(const QString &strArchiveName)
{
    qDebug() << "加载压缩包：" << strArchiveName;
    m_strArchiveName = strArchiveName;
}

QString UnCompressPage::archiveName()
{
    return m_strArchiveName;
}

void UnCompressPage::setDefaultUncompressPath(const QString &strPath)
{
    m_strUnCompressPath = strPath;
    m_pUncompressPathBtn->setToolTip(strPath);
    m_pUncompressPathBtn->setText(tr("Extract to:") + strPath);
    m_pUnCompressView->setDefaultUncompressPath(m_strUnCompressPath);
}

void UnCompressPage::setArchiveData(const ArchiveData &stArchiveData)
{
    m_pUnCompressView->setArchiveData(stArchiveData);
}

void UnCompressPage::initUI()
{
    m_strUnCompressPath = "~/Desktop";

    // 初始化相关变量
    m_pUnCompressView = new UnCompressView(this);
    m_pUncompressPathBtn = new CustomCommandLinkButton(tr("Extract to:") + " ~/Desktop", this);
    m_pUnCompressBtn = new CustomPushButton(tr("Extract"), this);

    m_pUncompressPathBtn->setToolTip(m_strUnCompressPath);

    DFontSizeManager::instance()->bind(m_pUncompressPathBtn, DFontSizeManager::T8);

    // 解压路径布局
    QHBoxLayout *pPathLayout = new QHBoxLayout;
    pPathLayout->addStretch(1);
    pPathLayout->addWidget(m_pUncompressPathBtn, 2, Qt::AlignCenter);
    pPathLayout->addStretch(1);

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pUnCompressBtn, 2);
    pBtnLayout->addStretch(1);

    // 主界面布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(m_pUnCompressView);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pPathLayout);
    pMainLayout->addSpacing(10);
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setStretchFactor(m_pUnCompressView, 9);
    pMainLayout->setStretchFactor(pPathLayout, 1);
    pMainLayout->setStretchFactor(pBtnLayout, 1);
    pMainLayout->setContentsMargins(12, 1, 20, 20);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void UnCompressPage::initConnections()
{
    connect(m_pUnCompressBtn, &DPushButton::clicked, this, &UnCompressPage::slotUncompressClicked);
    connect(m_pUnCompressView, &UnCompressView::signalExtract2Path, this, &UnCompressPage::signalExtract2Path);
    connect(m_pUnCompressView, &UnCompressView::signalDelFiels, this, &UnCompressPage::signalDelFiels);
}

void UnCompressPage::slotUncompressClicked()
{
    QFileInfo file(m_strArchiveName);
    PERF_PRINT_BEGIN("POINT-04", "压缩包名：" + file.fileName() + " 大小：" + QString::number(file.size()));

    // 判断解压路径是否有可执行权限或者路径是否存在进行解压创建文件
    QFileInfo m_fileDestinationPath(m_strUnCompressPath);
    bool m_permission = (m_fileDestinationPath.isWritable() && m_fileDestinationPath.isExecutable());

    if (!m_permission) { // 无法解压到已选中路径
        QString strDes;
        if (!m_fileDestinationPath.exists()) { // 路径不存在
            strDes = tr("The default extraction path does not exist, please retry");
        } else { // 路径无权限
            strDes = tr("You do not have permission to save files here, please change and retry");
        }

        TipDialog dialog(this);
        dialog.showDialog(strDes, tr("OK"), DDialog::ButtonNormal);

        return;
    } else { // 发送解压信号
        emit signalUncompress(m_strUnCompressPath);
    }
}
