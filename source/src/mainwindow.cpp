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

#include "mainwindow.h"
#include "utils.h"
#include "dtitlebar.h"
#include "dhidpihelper.h"
#include "dthememanager.h"

#include <QSvgWidget>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent),
      m_mainWidget(new QWidget),
      m_mainLayout(new QStackedLayout(m_mainWidget)),
      m_homePage(new HomePage),
      m_UnCompressPage(new UnCompressPage),
      m_CompressPage(new CompressPage),
      m_CompressSetting(new CompressSetting),
      m_Progess(new Progress),
      m_CompressSuccess(new Compressor_Success),
      m_CompressFail(new Compressor_Fail),
      m_settings("deepin", "deepin-font-installer"),
      m_themeAction(new QAction(tr("Dark theme"), this))
{
    titlebar()->setIcon(QIcon(":/images/icon.svg"));
    titlebar()->setTitle("");
    titlebar()->setBackgroundTransparent(true);

    // add widget to main layout.
    m_mainLayout->addWidget(m_homePage);
    m_mainLayout->addWidget(m_UnCompressPage);
    m_mainLayout->addWidget(m_CompressPage);
    m_mainLayout->addWidget(m_CompressSetting);
    m_mainLayout->addWidget(m_Progess);
    m_mainLayout->addWidget(m_CompressSuccess);
    m_mainLayout->addWidget(m_CompressFail);

    // init window flags.
    setWindowTitle(tr("Deepin Archive Manager"));
    setWindowRadius(20);
    setCentralWidget(m_mainWidget);
    setAcceptDrops(true);

    // add menu to titlebar.
    QMenu *menu = new QMenu;
    menu->addAction(m_themeAction);
    menu->addSeparator();
    titlebar()->setMenu(menu);
    titlebar()->setFixedHeight(50);

    // init theme action.
    m_themeAction->setCheckable(true);

    // init settings.
    if (!m_settings.contains("darkTheme")) {
        m_settings.setValue("darkTheme", false);
    }

    // init theme.
    initTheme();

    // connect the signals to the slot function.
    connect(m_homePage, &HomePage::fileSelected, this, &MainWindow::onSelected);
    connect(m_themeAction, &QAction::triggered, this, &MainWindow::switchTheme);
    connect(m_CompressPage, &CompressPage::sigNextPress, this, &MainWindow::onCompressNext);
    connect(m_CompressSetting, &CompressSetting::sigCompressPressed, this, &MainWindow::onCompressPressed);
    connect(m_Progess, &Progress::sigCancelPressed, this, &MainWindow::onCancelCompressPressed);
    connect(m_CompressSuccess, &Compressor_Success::sigQuitApp, this, &MainWindow::onCancelCompressPressed);

}

MainWindow::~MainWindow()
{
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // not has urls.
    if (!mime->hasUrls()) {
        return e->ignore();
    }

    // traverse.
    m_homePage->setIconPixmap(true);
    return e->accept();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *e)
{
    m_homePage->setIconPixmap(false);

    DMainWindow::dragLeaveEvent(e);
}

void MainWindow::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (!mime->hasUrls())
        return e->ignore();

    e->accept();

    // find font files.
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        const QString localPath = url.toLocalFile();
        const QFileInfo info(localPath);

        fileList << localPath;
        qDebug()<<fileList;


    }

    m_homePage->setIconPixmap(false);
    onSelected(fileList);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void MainWindow::initTheme()
{
    const bool isDarkTheme = m_settings.value("darkTheme").toBool();

    if (isDarkTheme) {
        DThemeManager::instance()->setTheme("dark");
        m_themeAction->setChecked(true);
    } else {
        DThemeManager::instance()->setTheme("light");
        m_themeAction->setChecked(false);
    }
}

void MainWindow::switchTheme()
{
    const bool isDarkTheme = m_settings.value("darkTheme").toBool();

    if (isDarkTheme) {
        m_settings.setValue("darkTheme", false);
    } else {
        m_settings.setValue("darkTheme", true);
    }

    initTheme();
}

void MainWindow::setEnable()
{
    setAcceptDrops(true);

    // enable titlebar buttons.
    titlebar()->setDisableFlags(Qt::Widget);
}

void MainWindow::setDisable()
{
    setAcceptDrops(false);

    // disable titlebar buttons.
    titlebar()->setDisableFlags(Qt::WindowMinimizeButtonHint
                                | Qt::WindowCloseButtonHint
                                | Qt::WindowMaximizeButtonHint
                                | Qt::WindowSystemMenuHint);
}

void MainWindow::refreshPage()
{
    switch (m_pageid) {
    case PAGE_HOME:
        m_mainLayout->setCurrentIndex(0);
        titlebar()->setTitle("");
        break;
    case PAGE_UNZIP:
        m_mainLayout->setCurrentIndex(1);
        titlebar()->setTitle(tr("UNZIP"));
        break;
    case PAGE_ZIP:
        m_mainLayout->setCurrentIndex(5);
        titlebar()->setTitle(tr("New Archive File"));
        break;
    case PAGE_ZIPSET:
        m_mainLayout->setCurrentIndex(3);
        titlebar()->setTitle(tr("New Archive File"));
        break;
    case PAGE_ZIPPROGRESS:
        m_mainLayout->setCurrentIndex(4);
        titlebar()->setTitle(tr("Compressing"));
        break;
    default:
        break;
    }

}

void MainWindow::onSelected(const QStringList &files)
{
    if(files.count() == 1 && Utils::isCompressed_file(files.at(0)))
    {
        m_pageid = PAGE_UNZIP;
    }
    else {
        m_pageid = PAGE_ZIP;
    }

    refreshPage();
}

void MainWindow::onCompressNext()
{
    m_pageid = PAGE_ZIPSET;
    refreshPage();
}

void MainWindow::onCompressPressed()
{
    m_pageid = PAGE_ZIPPROGRESS;
    refreshPage();
}

void MainWindow::onCancelCompressPressed()
{
    emit sigquitApp();
}

