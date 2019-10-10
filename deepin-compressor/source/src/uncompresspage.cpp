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
#include <QStandardPaths>



DWIDGET_USE_NAMESPACE

UnCompressPage::UnCompressPage(QWidget *parent)
    : QWidget(parent)
{
    QFont ft;
    ft.setPixelSize(12);
    m_pathstr = "~/Desktop";
    m_fileviewer = new fileViewer(this, PAGE_UNCOMPRESS);
    m_nextbutton = new DPushButton(tr("解压"));
    m_nextbutton->setFixedSize(340, 36);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_fileviewer);

    m_extractpath = new DCommandLinkButton(tr("解压到") + ": ~/Desktop");
    m_extractpath->setFont(ft);

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch();
    buttonlayout->addWidget(m_nextbutton);
    buttonlayout->addStretch();

    QHBoxLayout *pathlayout = new QHBoxLayout;
    pathlayout->addStretch();
    pathlayout->addWidget(m_extractpath);
    pathlayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(pathlayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setStretchFactor(contentLayout, 9);
    mainLayout->setStretchFactor(pathlayout, 1);
    mainLayout->setStretchFactor(buttonlayout, 1);
    mainLayout->setContentsMargins(20, 1, 20, 20);

    DPalette pa;
    pa.setColor(DPalette::Background,QColor(255, 255, 255));
    setPalette(pa);


    connect(m_nextbutton, &DPushButton::clicked, this, &UnCompressPage::oneCompressPress);
    connect(m_extractpath, &DPushButton::clicked, this, &UnCompressPage::onPathButoonClicked);
    connect(m_fileviewer, &fileViewer::sigextractfiles, this, &UnCompressPage::onextractfilesSlot);
}

UnCompressPage::~UnCompressPage()
{

}

void UnCompressPage::oneCompressPress()
{
    emit sigDecompressPress(m_pathstr);
}


void UnCompressPage::setModel(ArchiveModel* model)
{
    m_model = model;
    m_fileviewer->setDecompressModel(m_model);
}

void UnCompressPage::onPathButoonClicked()
{
    DFileDialog dialog;
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::Directory);
    dialog.setDirectory(m_pathstr);

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    QList<QUrl> pathlist = dialog.selectedUrls();

    QString curpath = pathlist.at(0).toLocalFile();
    m_extractpath->setText(tr("解压到") + ": " + curpath);
    m_pathstr = curpath;
}

void UnCompressPage::setdefaultpath(QString path){
    m_pathstr = path;
    m_extractpath->setText(tr("解压到") + ": " + m_pathstr);
}

QString UnCompressPage::getDecompressPath()
{
    return m_pathstr;
}

void UnCompressPage::onextractfilesSlot(QVector<Archive::Entry*> fileList, EXTRACT_TYPE type, QString path)
{
    if(fileList.count() == 0)
    {
        return;
    }

    if(EXTRACT_TO == type)
    {
        DFileDialog dialog;
        dialog.setAcceptMode(DFileDialog::AcceptOpen);
        dialog.setFileMode(DFileDialog::Directory);
        dialog.setDirectory(m_pathstr);

        const int mode = dialog.exec();

        if (mode != QDialog::Accepted) {
            return;
        }

        QList<QUrl> pathlist = dialog.selectedUrls();
        QString curpath = pathlist.at(0).toLocalFile();

        emit sigextractfiles(fileList, curpath);
    }
    else if (EXTRACT_DRAG == type) {
        emit sigextractfiles(fileList, path);
    }
    else {
        emit sigextractfiles(fileList, m_pathstr);
    }
}
