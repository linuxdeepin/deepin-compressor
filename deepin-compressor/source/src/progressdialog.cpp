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
#include "progressdialog.h"
#include "kprocess.h"

#include <DFontSizeManager>

#include <QBoxLayout>
#include <QDebug>
#include <QFileInfo>

ProgressDialog::ProgressDialog(QWidget *parent):
    DAbstractDialog(parent)
{
    initUI();
    initConnect();
}

void ProgressDialog::initUI()
{
    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint /*& ~Qt::Dialog*/) | Qt::Window);
    setFixedWidth(m_defaultWidth);
    setMinimumHeight(m_defaultHeight);

    m_titlebar = new DTitlebar(this);
    m_titlebar->setFixedHeight(50);
    m_titlebar->layout()->setContentsMargins(0, 0, 0, 0);
    m_titlebar->setMenuVisible(false);
    m_titlebar->setIcon(QIcon::fromTheme("deepin-compressor"));
    m_titlebar->setFixedWidth(m_defaultWidth);
    m_titlebar->setTitle(tr("%1 task(s) in progress").arg(1));
    m_titlebar->setBackgroundTransparent(true);

    QVBoxLayout *contentlayout = new QVBoxLayout;
    DPalette pa;

    m_tasklable = new DLabel(this);
    DFontSizeManager::instance()->bind(m_tasklable, DFontSizeManager::T6, QFont::Medium);
    m_tasklable->setForegroundRole(DPalette::WindowText);
    m_filelable = new DLabel(this);
    DFontSizeManager::instance()->bind(m_filelable, DFontSizeManager::T8, QFont::Normal);
    m_filelable->setForegroundRole(DPalette::TextTips);
    m_tasklable->setText(tr("Task") + ": ");
    m_filelable->setText(tr("Extracting") + ":");

    m_circleprogress = new  DProgressBar(this);
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
    m_extractdialog = new ExtractPauseDialog(this);
}

void ProgressDialog::initConnect()
{
    connect(m_extractdialog, &ExtractPauseDialog::sigbuttonpress, this, &ProgressDialog::slotextractpress);
}

void ProgressDialog::slotextractpress(int index)
{
    qDebug() << index;
    if (1 == index) {
        emit stopExtract();
        emit sigResetPercentAndTime();
    } else {
        exec();
    }
}

void ProgressDialog::closeEvent(QCloseEvent *)
{
    if (m_circleprogress->value() < 100 && m_circleprogress->value() > 0) {
        accept();
        m_extractdialog->move(this->geometry().topLeft()); //解决提取时取消提示框不居中显示
        m_extractdialog->exec();
    }

}

void ProgressDialog::setCurrentTask(const QString &file)
{
    QFileInfo fileinfo(file);

    m_tasklable->setText(tr("Task") + ":" + fileinfo.fileName());
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
        m_filelable->setText(tr("Extraction successful") + ":" + tr("Extract to") + path);
        m_extractdialog->reject();
        //reject();
        hide();
        m_filelable->setText(tr("Extracting") + ":");
        emit extractSuccess(tr("Extraction successful", "progressdialog"));
        emit sigResetPercentAndTime();
    }
}

void ProgressDialog::setMsg(const QString &msg)
{
    if (100 != m_circleprogress->value()) {
        setWindowTitle(tr(""));
        m_circleprogress->setValue(100);
        m_filelable->setText(msg);
        m_extractdialog->reject();
        //reject();
        hide();
        emit extractSuccess(msg);
    }
}

void ProgressDialog::showdialog()
{
    //show();
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
