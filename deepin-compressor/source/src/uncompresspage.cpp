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

#include "uncompresspage.h"
#include "utils.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <DRecentManager>



DWIDGET_USE_NAMESPACE

UnCompressPage::UnCompressPage(QWidget *parent)
    : QWidget(parent)
{

    m_fileviewer = new fileViewer();
    m_nextbutton = new DPushButton(tr("decompress"));
    m_nextbutton->setFixedWidth(260);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_fileviewer);

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch();
    buttonlayout->addWidget(m_nextbutton);
    buttonlayout->addStretch();


    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonlayout);
    mainLayout->setStretchFactor(contentLayout, 10);
    mainLayout->setStretchFactor(buttonlayout, 1);
    mainLayout->setContentsMargins(10, 10, 10, 20);


    connect(m_nextbutton, &DPushButton::clicked, this, &UnCompressPage::onNextPress);
}

UnCompressPage::~UnCompressPage()
{

}



void UnCompressPage::onNextPress()
{
    emit sigNextPress();
}

void UnCompressPage::onAddfileSlot()
{
    if(0 != m_fileviewer->getPathIndex())
    {
        QMessageBox msgBox;
        msgBox.setText("Please add files in the root path!");
        msgBox.exec();
        return;
    }

    DFileDialog dialog;
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::ExistingFiles);

    QString historyDir = m_settings->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    m_settings->setValue("dir", dialog.directoryUrl().toLocalFile());
    qDebug()<<dialog.directoryUrl().toLocalFile();

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }
    onSelectedFilesSlot(dialog.selectedFiles());
}

void UnCompressPage::onSelectedFilesSlot(const QStringList &files)
{
    m_filelist.append(files);
    m_fileviewer->setFileList(m_filelist);
}

void UnCompressPage::setModel(QAbstractItemModel* model)
{
    m_model = model;
    m_fileviewer->setDecompressModel(m_model);
}
