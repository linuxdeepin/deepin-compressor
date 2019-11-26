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

#include "compresspage.h"
#include "utils.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <DRecentManager>
#include <QShortcut>
#include <DDialog>



DWIDGET_USE_NAMESPACE

CompressPage::CompressPage(QWidget *parent)
    : QWidget(parent)
{

    m_fileviewer = new fileViewer(this, PAGE_COMPRESS);
    m_nextbutton = new DPushButton(tr("Next"));
    m_nextbutton->setFixedSize(340, 36);

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
    mainLayout->setContentsMargins(12, 1, 20, 20);

    m_settings = new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                               QSettings::IniFormat);
    // initalize the configuration file.
    if (m_settings->value("dir").toString().isEmpty()) {
        m_settings->setValue("dir", "");
    }

    connect(m_nextbutton, &DPushButton::clicked, this, &CompressPage::onNextPress);
    connect(m_fileviewer, &fileViewer::sigFileRemoved, this, &CompressPage::onRefreshFilelist);
    connect(m_fileviewer, &fileViewer::sigpathindexChanged, this, &CompressPage::onPathIndexChanged);
    auto openkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, &CompressPage::onAddfileSlot);

    setBackgroundRole(DPalette::Base);
    m_fileviewer->setAutoFillBackground(true);

}

CompressPage::~CompressPage()
{

}



void CompressPage::onNextPress()
{
    if (m_filelist.isEmpty()) {
        DDialog *dialog = new DDialog;

        QPixmap pixmap = Utils::renderSVG(":/images/warning.svg", QSize(30, 30));
        dialog->setIconPixmap(pixmap);
        dialog->setMessage(tr("Please add files!"));
        dialog->addButton(tr("OK"));
        dialog->addSpacing(15);

        dialog->exec();
    } else {
        emit sigNextPress();
    }

}

void CompressPage::showDialog()
{
    DDialog *dialog = new DDialog(this);

    QPixmap pixmap = Utils::renderSVG(":/images/warning.svg", QSize(48, 48));
    dialog->setIconPixmap(pixmap);

    DPalette pa;

    DLabel *strlabel = new DLabel;
    strlabel->setForegroundRole(DPalette::WindowText);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);

    strlabel->setText(QObject::tr("Please add files in the root directory!"));
    dialog->addButton(QObject::tr("OK"));
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addWidget(strlabel, 0, Qt::AlignLeft | Qt::AlignVCenter);

    DWidget *widget = new DWidget;

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    dialog->moveToCenter();
    dialog->exec();
    return;
}

int CompressPage::showReplaceDialog(QString name)
{
    DDialog *dialog = new DDialog(this);

    QPixmap pixmap = Utils::renderSVG(":/images/warning.svg", QSize(30, 30));
    dialog->setIconPixmap(pixmap);

    dialog->setMessage("“" + name + "”" + QObject::tr("Already exist, replaced?"));
    dialog->addSpacing(15);
    dialog->addButton(QObject::tr("Cancel"));
    dialog->addButton(QObject::tr("Confirm"));

    dialog->moveToCenter();

    return dialog->exec();
}

void CompressPage::onAddfileSlot()
{

    if (0 != m_fileviewer->getPathIndex()) {
        showDialog();
        return;
    }
    DFileDialog dialog;
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setAllowMixedSelection(true);

    QString historyDir = m_settings->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    m_settings->setValue("dir", dialog.directoryUrl().toLocalFile());
    qDebug() << dialog.directoryUrl().toLocalFile();

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }

    emit sigselectedFiles(dialog.selectedFiles());
}

void CompressPage::onSelectedFilesSlot(const QStringList &files)
{
    if (0 != m_fileviewer->getPathIndex()) {
        showDialog();
        return;
    }

    QStringList inputlist = files;
    foreach (QString m_path, m_filelist) {
        foreach (QString path, files) {
            QFileInfo mfile(m_path);
            QFileInfo file(path);
            if (file.fileName() == mfile.fileName()) {
                int mode = showReplaceDialog(file.fileName());
                if (0 == mode) {
                    inputlist.removeOne(path);
                } else {
                    m_filelist.removeOne(m_path);
                }
            }
        }
    }

    m_filelist.append(inputlist);
    m_fileviewer->setFileList(m_filelist);
}

void CompressPage::onRefreshFilelist(const QStringList &filelist)
{
    m_filelist = filelist;
    m_fileviewer->setFileList(m_filelist);
}

void CompressPage::onPathIndexChanged()
{
    if (m_fileviewer->getPathIndex() > 0) {
        emit sigiscanaddfile(false);
    } else {
        emit sigiscanaddfile(true);
    }
}


QStringList CompressPage::getCompressFilelist()
{
    return m_filelist;
}





