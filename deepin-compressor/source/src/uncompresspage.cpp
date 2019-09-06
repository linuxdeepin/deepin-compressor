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

    m_pathstr = "~/Desktop";
    m_fileviewer = new fileViewer(this, PAGE_UNCOMPRESS);
    m_nextbutton = new DPushButton(tr("decompress"));
    m_nextbutton->setFixedSize(340, 36);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_fileviewer);


    m_extractpath = new DLineEdit();
    m_extractpath->setText(tr("Extract to") + ": ~/Desktop");
    m_extractpath->setFixedWidth(340);
    m_pathbutton = new Lib_Edit_Button(m_extractpath);

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
    mainLayout->addLayout(buttonlayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(pathlayout);
    mainLayout->setStretchFactor(contentLayout, 9);
    mainLayout->setStretchFactor(buttonlayout, 1);
    mainLayout->setStretchFactor(pathlayout, 1);
    mainLayout->setContentsMargins(20, 1, 20, 20);


    connect(m_nextbutton, &DPushButton::clicked, this, &UnCompressPage::oneCompressPress);
    connect(m_pathbutton, &DPushButton::clicked, this, &UnCompressPage::onPathButoonClicked);
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
    dialog.setDirectory("~/Desktop");

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    QList<QUrl> pathlist = dialog.selectedUrls();

    QString curpath = pathlist.at(0).toLocalFile();
    m_extractpath->setText(tr("Extract to") + ": " + curpath);
    m_pathstr = curpath;
}

void UnCompressPage::setdefaultpath(QString path){
    m_pathstr = path;
    m_extractpath->setText(tr("Extract to") + ": " + m_pathstr);
}

QString UnCompressPage::getDecompressPath()
{
    return m_pathstr;
}
