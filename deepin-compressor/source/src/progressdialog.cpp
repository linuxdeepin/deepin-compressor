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
#include "progressdialog.h"

#include <QBoxLayout>
#include <QFileIconProvider>
#include <QDebug>

ProgressDialog::ProgressDialog(QWidget *parent):
    DAbstractDialog(parent)
{
    initUI();
    initConnect();
}

void ProgressDialog::initUI()
{

    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window);
    setFixedWidth(m_defaultWidth);

    m_titlebar = new DTitlebar(this);
    m_titlebar->setFixedHeight(50);
    m_titlebar->layout()->setContentsMargins(0, 0, 0, 0);
    m_titlebar->setMenuVisible(false);
    m_titlebar->setIcon(QIcon::fromTheme("deepin-compressor"));
    m_titlebar->setFixedWidth(m_defaultWidth);
    m_titlebar->setTitle(QObject::tr("There is 1 task in progress"));
    m_titlebar->setBackgroundTransparent(true);

    QVBoxLayout *contentlayout = new QVBoxLayout;
    DPalette pa;

    m_tasklable = new DLabel();
    DFontSizeManager::instance()->bind(m_tasklable, DFontSizeManager::T6, QFont::Medium);
    m_tasklable->setForegroundRole(DPalette::WindowText);
    m_filelable = new DLabel();
    DFontSizeManager::instance()->bind(m_filelable, DFontSizeManager::T8, QFont::Normal);
    m_filelable->setForegroundRole(DPalette::WindowText);
    m_tasklable->setText(tr("Current task") + ":");
    m_filelable->setText(tr("Extracting") + ":");

    m_circleprogress = new  DProgressBar();
    m_circleprogress->setFixedSize(336, 6);
    m_circleprogress->setValue(0);

    contentlayout->setContentsMargins(20, 0, 10, 0);
    contentlayout->addWidget(m_tasklable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    contentlayout->addSpacing(7);
    contentlayout->addWidget(m_filelable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    contentlayout->addSpacing(7);
    contentlayout->addWidget(m_circleprogress, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 10, 20);
    mainlayout->setSpacing(0);
    mainlayout->addWidget(m_titlebar);
    mainlayout->addLayout(contentlayout);

    setLayout(mainlayout);
    m_extractdialog = new ExtractPauseDialog();
}

void ProgressDialog::initConnect()
{
    connect(m_extractdialog, &ExtractPauseDialog::sigbuttonpress, this, &ProgressDialog::slotextractpress);
}

void ProgressDialog::slotextractpress(int index)
{
    qDebug() << index;
    if (0 == index) {
        emit stopExtract();
    } else {
        exec();
    }
}

void ProgressDialog::closeEvent(QCloseEvent *)
{
    if (m_circleprogress->value() < 100 && m_circleprogress->value() > 0) {
        accept();
        m_extractdialog->exec();
    }

}

void ProgressDialog::setCurrentTask(const QString &file)
{
//    QFileIconProvider icon_provider;
    QFileInfo fileinfo(file);
//    QIcon icon = icon_provider.icon(fileinfo);
//    setIcon(icon, QSize(16, 16));
//    setWindowIcon(icon);

    m_tasklable->setText(tr("Current task") + ":" + fileinfo.fileName());
}

void ProgressDialog::setCurrentFile(const QString &file)
{
    QFileInfo fileinfo(file);
    m_filelable->setText(tr("Extracting") + ":" + fileinfo.fileName());
}

void ProgressDialog::setProcess(unsigned long  value)
{
    if (100 != m_circleprogress->value()) {
        m_circleprogress->setValue(value);
    }

}

void ProgressDialog::setFinished(const QString &path)
{
    if (100 != m_circleprogress->value()) {
        setWindowTitle(tr(""));
        m_circleprogress->setValue(100);
        m_filelable->setText(tr("Extraction completed") + ":" + tr("Extracted to") + path);
        m_extractdialog->reject();
        reject();
        emit extractSuccess();
    }
}

void ProgressDialog::showdialog()
{
    exec();
}

void ProgressDialog::clearprocess()
{
    m_circleprogress->setValue(0);
}

bool ProgressDialog::isshown()
{
    return this->isVisible() || m_extractdialog->isVisible();
}

