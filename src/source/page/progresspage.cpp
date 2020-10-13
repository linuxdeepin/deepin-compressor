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

#include "progresspage.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QFileIconProvider>


ProgressPage::ProgressPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

ProgressPage::~ProgressPage()
{

}

void ProgressPage::setProgressType(Progress_Type eType)
{
    m_eType = eType;
}

void ProgressPage::setArchiveName(const QString &strArchiveName, qint64 qTotalSize)
{
    m_pArchiveNameLbl->setText(strArchiveName);     // 设置压缩包名称
    m_qTotalSize = qTotalSize;

    // 设置类型图片
    QFileInfo fileinfo(strArchiveName);
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo("temp." + fileinfo.completeSuffix()));
    m_pPixmapLbl->setPixmap(icon.pixmap(128, 128));
}

void ProgressPage::setProgress(int iPercent)
{
    m_iPerent = iPercent;
    m_pProgressBar->setValue(iPercent);
}

void ProgressPage::setCurrentFileName(const QString &strFileName)
{
    QFontMetrics elideFont(m_pFileNameLbl->font());

    if (m_eType == PT_Compress || m_eType == PT_CompressAdd) {   // 压缩和追加压缩
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Compressing") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (m_eType == PT_Delete) {   // 删除
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Deleting") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (m_eType == PT_Convert) {     // 转换
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Converting") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else {    // 解压
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Extracting") + ": " + strFileName, Qt::ElideMiddle, 520));
    }
}

void ProgressPage::initUI()
{
    // 初始化控件
    m_pPixmapLbl = new DLabel(this);
    m_pArchiveNameLbl = new DLabel(this);
    m_pProgressBar = new DProgressBar(this);
    m_pFileNameLbl = new DLabel(this);
    m_pSpeedLbl = new DLabel(this);
    m_pRemainingTimeLbl = new DLabel(this);
    m_pCancelBtn = new DPushButton(tr("Cancel"), this);
    m_pPauseContinueButton = new DSuggestButton(tr("Pause"), this);

    // 初始化压缩包名称样式
    DFontSizeManager::instance()->bind(m_pArchiveNameLbl, DFontSizeManager::T5, QFont::DemiBold);
    m_pArchiveNameLbl->setForegroundRole(DPalette::ToolTipText);

    // 配置进度条
    m_pProgressBar->setRange(0, 100);
    m_pProgressBar->setFixedSize(240, 8);
    m_pProgressBar->setValue(0);
    m_pProgressBar->setOrientation(Qt::Horizontal);  //水平方向
    m_pProgressBar->setAlignment(Qt::AlignVCenter);
    m_pProgressBar->setTextVisible(false);

    // 设置文件名样式
    m_pFileNameLbl->setMaximumWidth(520);
    m_pFileNameLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pFileNameLbl, DFontSizeManager::T8);

    // 设置速度和剩余时间样式
    DFontSizeManager::instance()->bind(m_pSpeedLbl, DFontSizeManager::T8);
    DFontSizeManager::instance()->bind(m_pRemainingTimeLbl, DFontSizeManager::T8);

    // 设置取消按钮样式
    m_pCancelBtn->setMinimumSize(200, 36);

    // 设置暂停继续按钮样式
    m_pPauseContinueButton->setMinimumSize(200, 36);
    m_pPauseContinueButton->setCheckable(true);

    // 速度和剩余时间布局
    QHBoxLayout *pSpeedLayout = new QHBoxLayout;
    pSpeedLayout->addStretch();
    pSpeedLayout->addWidget(m_pSpeedLbl);
    pSpeedLayout->addSpacing(15);
    pSpeedLayout->addWidget(m_pRemainingTimeLbl);
    pSpeedLayout->addStretch();

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pCancelBtn, 2);
    pBtnLayout->addSpacing(10);
    pBtnLayout->addWidget(m_pPauseContinueButton, 2);
    pBtnLayout->addStretch(1);

    // 主布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pPixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pArchiveNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(25);
    pMainLayout->addWidget(m_pProgressBar, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pFileNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pSpeedLayout);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);

    // 临时测试界面
    setArchiveName("新建归档文件.zip", 102400);
    setProgress(50);
    setCurrentFileName("55555.txt");
    refreshSpeedAndRemainingTime();
}

void ProgressPage::initConnections()
{

}

void ProgressPage::refreshSpeedAndRemainingTime()
{
    qint64 hour = 7200 / 3600;
    qint64 minute = (7200 - hour * 3600) / 60;
    qint64 seconds = 7200 - hour * 3600 - minute * 60;

    QString hh = QString("%1").arg(hour, 2, 10, QLatin1Char('0'));
    QString mm = QString("%1").arg(minute, 2, 10, QLatin1Char('0'));
    QString ss = QString("%1").arg(seconds, 2, 10, QLatin1Char('0'));

    //add update speed and time label
    if (m_eType == PT_Compress) {

    } else if (m_eType == PT_Delete) {

    } else if (m_eType == PT_CompressAdd) {

    } else if (m_eType == PT_UnCompress) {

    } else if (m_eType == PT_Convert) {

    } else {
        m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + ">300MB/S");
    }

    m_pRemainingTimeLbl->setText(tr("Time left") + ": " + hh + ":" + mm + ":" + ss);
}
