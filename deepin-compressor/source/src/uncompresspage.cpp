/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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

#include <QVBoxLayout>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <DStandardPaths>
#include <DMessageManager>
#include <DDialog>
#include <QFontMetrics>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

UnCompressPage::UnCompressPage(QWidget *parent)
    : DWidget(parent)
{
    m_pathstr = "~/Desktop";
    m_fileviewer = new fileViewer(this, PAGE_UNCOMPRESS);
    m_nextbutton = new DPushButton(tr("Extract"), this);
    m_nextbutton->setMinimumSize(340, 36);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_fileviewer);

    m_extractpath = new DCommandLinkButton(tr("Extract to:") + " ~/Desktop", this);
//    m_extractpath->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
//    m_extractpath->setMinimumSize(129, 18);
    DFontSizeManager::instance()->bind(m_extractpath, DFontSizeManager::T8);

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(m_nextbutton, 2);
    buttonlayout->addStretch(1);

    QHBoxLayout *pathlayout = new QHBoxLayout;
    pathlayout->addStretch(1);
    pathlayout->addWidget(m_extractpath, 2, Qt::AlignCenter);
    pathlayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(pathlayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setStretchFactor(contentLayout, 9);
    mainLayout->setStretchFactor(pathlayout, 1);
    mainLayout->setStretchFactor(buttonlayout, 1);
    mainLayout->setContentsMargins(12, 1, 20, 20);

    setBackgroundRole(DPalette::Base);

    connect(m_nextbutton, &DPushButton::clicked, this, &UnCompressPage::oneCompressPress);
    connect(m_extractpath, &DPushButton::clicked, this, &UnCompressPage::onPathButoonClicked);
    connect(m_fileviewer, &fileViewer::sigextractfiles, this, &UnCompressPage::onextractfilesSlot);
    connect(m_fileviewer, &fileViewer::sigOpenWith,     this, &UnCompressPage::onextractfilesOpenSlot);
}

void UnCompressPage::oneCompressPress()
{
    QFileInfo m_fileDestinationPath(m_pathstr);
    bool m_permission = (m_fileDestinationPath.isWritable() && m_fileDestinationPath.isExecutable());

    if (!m_permission) {
        showWarningDialog(tr("You do not have permission to save files here, please change and retry"));
        return;
    } else {
        emit sigDecompressPress(m_pathstr);
    }
}

void UnCompressPage::setModel(ArchiveSortFilterModel *model)
{
    m_model = model;
    m_fileviewer->setDecompressModel(m_model);
}

void UnCompressPage::onPathButoonClicked()
{
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::Directory);
    dialog.setWindowTitle(tr("Find directory"));
    dialog.setDirectory(m_pathstr);

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    QList<QUrl> pathlist = dialog.selectedUrls();

    const QString curpath = pathlist.at(0).toLocalFile();
//    m_extractpath->setText(tr("Extract to:") + curpath);
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(curpath);//获取之前设置的字符串的像素大小
    QString str = curpath;
    if (fontSize > this->width()) {
        str = fontMetrics.elidedText(curpath, Qt::ElideMiddle, this->width());//返回一个带有省略号的字符串
    }
    m_extractpath->setText(tr("Extract to:") + str);
    m_pathstr = curpath;
}

void UnCompressPage::setdefaultpath(const QString path)
{
    m_pathstr = path;
    m_extractpath->setText(tr("Extract to:") + m_pathstr);
}

int UnCompressPage::getFileCount()
{
    return m_fileviewer->getFileCount();
}

//获取被解压文件里一级文件(夹)个数
int UnCompressPage::getDeFileCount()
{
    return m_fileviewer->getDeFileCount();
}

int UnCompressPage::showWarningDialog(const QString &msg)
{
    DDialog *dialog = new DDialog(this);
    QPixmap pixmap = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);
//    dialog->setMessage(msg);
    dialog->addSpacing(32);
    dialog->addButton(tr("OK"));
    dialog->setMinimumSize(380, 140);
    DLabel *pContent = new DLabel(msg, dialog);
    pContent->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(pContent);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ButtonText));
    DFontSizeManager::instance()->bind(pContent, DFontSizeManager::T6, QFont::Medium);
    pContent->setMinimumWidth(this->width());
    pContent->move(dialog->width() / 2 - pContent->width() / 2, dialog->height() / 2 - pContent->height() / 2 - 10);
    int res = dialog->exec();
    delete dialog;

    return res;
}

EXTRACT_TYPE UnCompressPage::getExtractType()
{
    return extractType;
}

QString UnCompressPage::getDecompressPath()
{
    return m_pathstr;
}

void UnCompressPage::onextractfilesSlot(QVector<Archive::Entry *> fileList, EXTRACT_TYPE type, QString path)
{
    if (fileList.count() == 0) {
        return;
    }
    // get extract type
    extractType = type;

    if (EXTRACT_TO == type) {
        DFileDialog dialog(this);
        dialog.setAcceptMode(DFileDialog::AcceptOpen);
        dialog.setFileMode(DFileDialog::Directory);
        dialog.setDirectory(m_pathstr);

        const int mode = dialog.exec();

        if (mode != QDialog::Accepted) {
            return;
        }

        QList<QUrl> pathlist = dialog.selectedUrls();
        QString curpath = pathlist.at(0).toLocalFile();

        emit sigextractfiles(fileList, curpath, type);
    } else if (EXTRACT_DRAG == type) {
        emit sigextractfiles(fileList, path, type);
    } else if (EXTRACT_TEMP == type) {
        QString tmppath = DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles";
        QDir dir(tmppath);
        if (!dir.exists()) {
            dir.mkdir(tmppath);
        }
        emit sigextractfiles(fileList, tmppath, type);
    } else if (EXTRACT_TEMP_CHOOSE_OPEN == type) {
        QString tmppath = DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles";
        QDir dir(tmppath);
        if (!dir.exists()) {
            dir.mkdir(tmppath);
        }
        emit sigextractfiles(fileList, tmppath, type);
    } else {
        emit sigextractfiles(fileList, m_pathstr, type);
    }

}

void UnCompressPage::onextractfilesOpenSlot(const QVector<Archive::Entry *> &fileList, const QString &programma)
{
    emit sigOpenExtractFile(fileList, programma);
}

