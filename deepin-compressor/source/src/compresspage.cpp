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

#include "compresspage.h"
#include "utils.h"
#include "queries.h"
#include "fileViewer.h"
#include "customwidget.h"

#include <DFileDialog>
#include <DDialog>
#include <DRecentManager>

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>
#include <QSettings>
#include <DFontSizeManager>


DWIDGET_USE_NAMESPACE

CompressPage::CompressPage(QWidget *parent) : DWidget(parent)
{
    m_fileviewer = new fileViewer(this, PAGE_COMPRESS);
    m_nextbutton = new CustomPushButton(tr("Next"), this);
    m_nextbutton->setMinimumSize(340, 36);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_fileviewer);

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(m_nextbutton, 2);
    buttonlayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addLayout(contentLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonlayout);
    mainLayout->setStretchFactor(contentLayout, 10);
    mainLayout->setStretchFactor(buttonlayout, 1);
    mainLayout->setContentsMargins(12, 1, 20, 20);

    m_settings = new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this);
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

void CompressPage::onNextPress()
{
    if (m_filelist.isEmpty()) {
        DDialog *dialog = new DDialog(this);

        QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(30, 30));
        dialog->setIcon(pixmap);
        dialog->setMessage(tr("Please add files"));
        dialog->addButton(tr("OK"));
        dialog->addSpacing(15);

        dialog->exec();

        delete dialog;
    } else {
        emit sigNextPress();
        m_fileviewer->resetTempFile();
    }
}

void CompressPage::showDialog()
{
    DDialog *dialog = new DDialog(this);
    QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);
    dialog->addSpacing(32);
    dialog->setMinimumSize(380, 140);
    dialog->addButton(tr("OK"), true, DDialog::ButtonNormal);

    DLabel *pContent = new DLabel(tr("Please add files to the top-level directory"), dialog);
    pContent->setAlignment(Qt::AlignmentFlag::AlignHCenter);

    DPalette pa;
    pa = DApplicationHelper::instance()->palette(pContent);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ButtonText));

    DFontSizeManager::instance()->bind(pContent, DFontSizeManager::T6, QFont::Medium);
    pContent->setMinimumSize(293, 20/*this->width()*/);
    pContent->move(dialog->width() / 2 - pContent->width() / 2, /*dialog->height() / 2 - pContent->height() / 2 - 10 */48);

    dialog->exec();
    delete dialog;

    return;
}

int CompressPage::showReplaceDialog(QString name)
{
    OverwriteQuery query(name);
    query.setParent(this);
    query.execute();
    return query.getExecuteReturn();
}

void CompressPage::setRootPathIndex()
{
    m_fileviewer->setRootPathIndex();
}

void CompressPage::onAddfileSlot()
{
    if (0 != m_fileviewer->getPathIndex()) {
        showDialog();
        return;
    }
    DFileDialog dialog(this);
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
    // 判断是否在根目录下添加文件，若不是，给出提示
    if (0 != m_fileviewer->getPathIndex()) {
        showDialog();
        return;
    }

    QStringList inputlist = files;
    int mode = 0;
    bool bAll = false;

    // 对重复文件进行判断处理
    foreach (QString m_path, m_filelist) {
        QFileInfo mfile(m_path);
        foreach (QString path, files) {
            QFileInfo file(path);
            if (file.fileName() == mfile.fileName()) {

                if (!bAll) {    // 判断是否全部应用，若不是，则继续弹出对话框
                    OverwriteQuery query(path);
                    query.setParent(this);
                    query.execute();
                    mode = query.getExecuteReturn();

                    bAll = query.applyAll();
                }

                if (-1 == mode || 0 == mode) {      // -1：取消  0：跳过
                    inputlist.removeOne(path);
                } else {                            // 覆盖
                    m_filelist.removeOne(m_path);
                }
                /*int mode = showReplaceDialog(path);
                if (0 == mode) {
                    inputlist.removeOne(path);
                } else {
                    m_filelist.removeOne(m_path);
                }*/
            }
        }
    }

    m_filelist.append(inputlist);

    m_fileviewer->setFileList(m_filelist);      // 设置所有压缩文件
    m_fileviewer->setSelectFiles(inputlist);    // 设置选中的文件
}

void CompressPage::onRefreshFilelist(const QStringList &filelist)
{
    m_filelist = filelist;
    m_fileviewer->setFileList(m_filelist);

    emit sigRefreshFileList(m_filelist);

    if (m_filelist.size() == 0) {
        emit sigFilelistIsEmpty();
    }
}

void CompressPage::onPathIndexChanged()
{
    if (m_fileviewer->getPathIndex() > 0) {
        emit sigiscanaddfile(false);
    } else {
        emit sigiscanaddfile(true);
    }
}

void CompressPage::clearFiles()
{
    m_filelist.clear();
}

QStringList CompressPage::getCompressFilelist()
{
    return m_filelist;
}
