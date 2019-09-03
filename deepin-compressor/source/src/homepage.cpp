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

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      m_layout(new QVBoxLayout(this)),
      m_iconLabel(new QLabel),
      m_tipsLabel(new QLabel(tr("Drag file(folder) here"))),
      m_splitLine(new QLabel),
      m_chooseBtn(new DLinkButton(tr("Select file"))),
      m_settings(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                               QSettings::IniFormat))
{
    m_unloadPixmap = Utils::renderSVG(":/images/folder-128 .svg", QSize(128, 128));
    m_loadedPixmap = Utils::renderSVG(":/images/folder-128 .svg", QSize(128, 128));

    m_iconLabel->setFixedSize(128, 128);
    m_iconLabel->setPixmap(m_unloadPixmap);
    m_splitLine->setPixmap(QPixmap(":/images/split_line.svg"));
    m_tipsLabel->setStyleSheet("QLabel { color: #6a6a6a; }");

    // initalize the configuration file.
    if (m_settings->value("dir").toString().isEmpty()) {
        m_settings->setValue("dir", "");
    }

    m_layout->addSpacing(40);
    m_layout->addWidget(m_iconLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
    m_layout->addSpacing(20);
    m_layout->addWidget(m_tipsLabel, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_splitLine, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_chooseBtn, 0, Qt::AlignHCenter);
    m_layout->addStretch();
    m_layout->setSpacing(0);

    connect(m_chooseBtn, &DLinkButton::clicked, this, &HomePage::onChooseBtnClicked);
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
