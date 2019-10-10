/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#include "homepage.h"
#include "utils.h"
#include <QApplication>
#include <QDir>
#include <DFileDialog>
#include <DPalette>
DGUI_USE_NAMESPACE

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      m_layout(new QVBoxLayout(this)),
      m_iconLabel(new DLabel),
      m_tipsLabel(new DLabel(tr("拖拽文件（夹）到此"))),
      m_splitLine(new DLabel),
      m_chooseBtn(new DCommandLinkButton(tr("选择文件"))),
      m_settings(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                               QSettings::IniFormat))
{
    m_unloadPixmap = Utils::renderSVG(":/images/folder-128 .svg", QSize(128, 128));
    m_loadedPixmap = Utils::renderSVG(":/images/folder-128 .svg", QSize(128, 128));

    m_iconLabel->setFixedSize(128, 128);
    m_iconLabel->setPixmap(m_unloadPixmap);
    m_splitLine->setPixmap(QPixmap(":/images/split_line.svg"));

    QFont font;
    font.setPixelSize(12);

    DPalette pa;
    pa.setColor(DPalette::WindowText,QColor(82, 106, 127));
    m_tipsLabel->setPalette(pa);
    m_tipsLabel->setFont(font);

    m_chooseBtn->setFont(font);

    // initalize the configuration file.
    if (m_settings->value("dir").toString().isEmpty()) {
        m_settings->setValue("dir", "");
    }

    DLabel* buttomlabel = new DLabel();
    buttomlabel->setFixedHeight(50);

    m_layout->addStretch();
    m_layout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    m_layout->addSpacing(13);
    m_layout->addWidget(m_tipsLabel, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_splitLine, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_chooseBtn, 0, Qt::AlignHCenter);
    m_layout->addWidget(buttomlabel, 0, Qt::AlignHCenter);
    m_layout->addStretch();

    connect(m_chooseBtn, &DCommandLinkButton::clicked, this, &HomePage::onChooseBtnClicked);

    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(40, 40);

    m_spinner->move(285, 200);
    m_spinner->hide();

    pa.setColor(DPalette::Background,QColor(248, 248, 248));
    setPalette(pa);
}

HomePage::~HomePage()
{
}

void HomePage::setIconPixmap(bool isLoaded)
{
    if (isLoaded) {
        m_iconLabel->setPixmap(m_loadedPixmap);
    } else {
        m_iconLabel->setPixmap(m_unloadPixmap);
    }
}

void HomePage::spinnerStart()
{
    m_spinner->show();
    m_spinner->start();
    m_chooseBtn->setEnabled(false);
}

void HomePage::spinnerStop()
{
    m_spinner->hide();
    m_spinner->stop();
    m_chooseBtn->setEnabled(true);
}

void HomePage::onChooseBtnClicked()
{
    DFileDialog dialog;
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::ExistingFiles);
//    dialog.setNameFilter(Utils::suffixList());

    QString historyDir = m_settings->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    m_settings->setValue("dir", dialog.directoryUrl().toLocalFile());

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }

    emit fileSelected(dialog.selectedFiles());
}
