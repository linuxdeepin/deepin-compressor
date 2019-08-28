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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QStackedLayout>
#include <QSettings>

#include "homepage.h"
#include "uncompresspage.h"
#include "compresspage.h"
#include "compresssetting.h"
#include "progress.h"
#include "compressor_success.h"
#include "compressor_fail.h"

#define TITLE_FIXED_HEIGHT 40
DWIDGET_USE_NAMESPACE

enum Page_ID{
    PAGE_HOME,
    PAGE_UNZIP,
    PAGE_ZIP,
    PAGE_ZIPSET,
    PAGE_ZIPPROGRESS,
    PAGE_UNZIPPROGRESS,
    PAGE_ZIP_SUCCESS,
    PAGE_ZIP_FAIL,
    PAGE_UNZIP_SUCCESS,
    PAGE_UNZIP_FAIL,
    PAGE_MAX
};

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void InitUI();
    void InitConnection();
    void initTitleBar();

protected:
    void dragEnterEvent(QDragEnterEvent *) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;

private slots:

    void setEnable();
    void setDisable();
    void refreshPage();
    void onSelected(const QStringList &);
    void onCompressNext();
    void onCompressPressed();
    void onCancelCompressPressed();
    void onTitleButtonPressed();

signals:
    void sigquitApp();
    void sigZipAddFile();
    void sigZipReturn();
    void sigZipSelectedFiles(const QStringList &files);

private:
    DLabel* m_logo;
    QPixmap m_logoicon;
    QFrame *m_titleFrame;
    DLabel* m_titlelabel;
    QWidget *m_mainWidget;
    QStackedLayout *m_mainLayout;
    HomePage *m_homePage;
    UnCompressPage *m_UnCompressPage;
    CompressPage *m_CompressPage;
    CompressSetting *m_CompressSetting;
    Progress* m_Progess;
    Compressor_Success* m_CompressSuccess;
    Compressor_Fail* m_CompressFail;
    QSettings m_settings;
    QAction *m_themeAction;
    Page_ID m_pageid;

    DPushButton* m_titlebutton;
};

#endif
