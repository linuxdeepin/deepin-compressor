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
   DDialog(parent)
{
    initUI();
    initConnect();
}

void ProgressDialog::initUI()
{
//    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setFixedWidth(m_defaultWidth);
    setTitle(QObject::tr("有1个任务正在进行"));
    QIcon icon = QIcon::fromTheme("deepin-compressor");
    setIcon(icon, QSize(32, 32));

    DWidget* widget = new DWidget;
    DPalette pa;

    m_tasklable = new DLabel();
    QFont ft = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    ft.setWeight(QFont::Medium);
    m_tasklable->setFont(ft);
    pa = DApplicationHelper::instance()->palette(m_tasklable);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::WindowText));
    m_tasklable->setPalette(pa);
    m_filelable = new DLabel();
    ft = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    ft.setWeight(QFont::Normal);
    m_filelable->setFont(ft);
    pa = DApplicationHelper::instance()->palette(m_filelable);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    m_filelable->setPalette(pa);
    m_tasklable->setText(tr("当前任务") + ":");
    m_filelable->setText(tr("正在提取") + ":");

    m_circleprogress = new  DProgressBar();
    m_circleprogress->setFixedSize(336, 6);
    m_circleprogress->setValue(0);


    QVBoxLayout* mainlayout = new QVBoxLayout;
    mainlayout->addWidget(m_tasklable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainlayout->addWidget(m_filelable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainlayout->addWidget(m_circleprogress, 0, Qt::AlignLeft | Qt::AlignVCenter);

    widget->setLayout(mainlayout);
    addContent(widget);
    m_extractdialog = new ExtractPauseDialog();
}

void ProgressDialog::initConnect()
{
    connect(m_extractdialog, &ExtractPauseDialog::sigbuttonpress, this, &ProgressDialog::slotextractpress);
}

void ProgressDialog::slotextractpress(int index)
{
    qDebug()<<index;
    if(0 == index)
    {
        emit stopExtract();
    }
    else {
        exec();
    }
}

void ProgressDialog::closeEvent(QCloseEvent *)
{
    if(m_circleprogress->value() < 100 && m_circleprogress->value() > 0)
    {
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

    m_tasklable->setText(tr("当前任务") + ":" + fileinfo.fileName());
}

void ProgressDialog::setCurrentFile(const QString &file)
{
    QFileInfo fileinfo(file);
    m_filelable->setText(tr("正在提取") + ":" + fileinfo.fileName());
}

void ProgressDialog::setProcess(unsigned long  value)
{
    m_circleprogress->setValue(value);
}

void ProgressDialog::setFinished(const QString &path)
{
    setWindowTitle(tr(""));
    m_circleprogress->setValue(100);
    m_filelable->setText(tr("提取完成") + ":" + tr("已提取到") + path);
    m_extractdialog->reject();
    reject();
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

