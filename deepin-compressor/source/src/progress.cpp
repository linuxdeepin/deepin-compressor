/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
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
#include "progress.h"

#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QVBoxLayout>
#include <QDebug>

Progress::Progress(QWidget *parent)
    : QWidget(parent)
{
    m_filename = "新建归档文件.rar";
    m_progressfile = "设计图111.jpg";
    InitUI();
    InitConnection();
}

void Progress::InitUI()
{
    m_compressicon = Utils::renderSVG(":/images/Compression Packet.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel();
//    m_pixmaplabel->setPixmap(m_compressicon);

    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T5);
    font.setBold(true);

    DPalette pa;


    m_filenamelabel = new DLabel();
    m_filenamelabel->setText(m_filename);
    m_filenamelabel->setFont(font);
    pa = DApplicationHelper::instance()->palette(m_filenamelabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::TextTitle));
    m_filenamelabel->setPalette(pa);

    m_progressbar = new DProgressBar();
    m_progressbar->setRange(0,100);
    m_progressbar->setFixedSize(240, 8);
    m_progressbar->setValue(0);
    m_progressbar->setOrientation(Qt::Horizontal);  //水平方向
    m_progressbar->setAlignment(Qt::AlignVCenter);
    m_progressbar->setTextVisible(false);

    m_progressfilelabel = new DLabel();
    m_progressfilelabel->setMaximumWidth(520);
    pa = DApplicationHelper::instance()->palette(m_progressfilelabel);
    pa.setBrush(DPalette::Text, pa.color(DPalette::TextTips));
    m_progressfilelabel->setPalette(pa);
    m_progressfilelabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    m_progressfilelabel->setText(tr("正在计算中..."));

    m_cancelbutton = new DPushButton();
    m_cancelbutton->setFixedSize(340, 36);
    m_cancelbutton->setText(tr("取消"));
    m_cancelbutton->setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout* mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(m_filenamelabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(25);
    mainlayout->addWidget(m_progressbar, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(m_progressfilelabel, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(m_cancelbutton, 0 , Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(10);

    setBackgroundRole(DPalette::Base);
}

void Progress::InitConnection()
{
    connect(m_cancelbutton, &DPushButton::clicked, this, &Progress::cancelbuttonPressedSlot);
}

void Progress::cancelbuttonPressedSlot()
{
    emit sigCancelPressed();
}

void Progress::setprogress(uint percent)
{
    m_progressbar->setValue(percent);
}

void Progress::setFilename(QString filename)
{
    QFileInfo fileinfo(filename);
    setTypeImage(fileinfo.completeSuffix());
    m_filenamelabel->setText(filename);
}

void Progress::setTypeImage(QString type)
{
    QFileIconProvider provider;
    QIcon icon;
    QString strTemplateName = QDir::tempPath() + QDir::separator()  + "tempfile." + type;

    QTemporaryFile tmpFile(strTemplateName);
    tmpFile.setAutoRemove(false);

    if (tmpFile.open())
    {
        tmpFile.close();
        icon = provider.icon(QFileInfo(strTemplateName));
    }

    m_pixmaplabel->setPixmap(icon.pixmap(128, 128));
}

void Progress::setProgressFilename(QString filename)
{
    QFontMetrics elideFont(m_progressfilelabel->font());
    if(m_type == COMPRESSING)
    {   
        m_progressfilelabel->setText(elideFont.elidedText(tr("正在压缩") + ": " + filename, Qt::ElideMiddle, 520));
    }
    else
    {
        m_progressfilelabel->setText(elideFont.elidedText(tr("正在解压") + ": " + filename, Qt::ElideMiddle, 520));
    }
}

void Progress::settype(COMPRESS_TYPE type)
{
    m_type = type;
}
