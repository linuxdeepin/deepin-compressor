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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QStackedLayout>
#include <QSettings>
#include <DTitlebar>
#include "homepage.h"
#include "uncompresspage.h"
#include "compresspage.h"
#include "compresssetting.h"
#include "progress.h"
#include "compressor_success.h"
#include "compressor_fail.h"
#include "archive_manager.h"
#include "archivemodel.h"
#include "encryptionpage.h"
#include "progressdialog.h"
#include "extractpausedialog.h"
#include "settingdialog.h"
#include "encodingpage.h"
#include <DIconButton>
#include "archivesortfiltermodel.h"
#include "batchextract.h"


#define TITLE_FIXED_HEIGHT 50
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
    PAGE_ENCRYPTION,
    PAGE_MAX
};

enum EncryptionType{
    Encryption_NULL,
    Encryption_Load,
    Encryption_Extract,
    Encryption_SingleExtract,
    Encryption_ExtractHere,
};

enum WorkState{
    WorkNone,
    WorkProcess,
};

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent* event) override;

    void InitUI();
    void InitConnection();
    void initTitleBar();
    QMenu* createSettingsMenu();
    void loadArchive(const QString &files);
    void creatArchive(QMap<QString, QString> &Args);

    void ExtractPassword(QString password);
    void ExtractSinglePassword(QString password);
    void LoadPassword(QString password);

    void customMessageHandler(const QString &msg);
    QString renameCompress(QString &filename, QString fixedMimeType);



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
    void onRightMenuSelected(const QStringList &);
    void onCompressNext();
    void onCompressPressed(QMap<QString, QString> &Args);
    void onCancelCompressPressed();
    void onTitleButtonPressed();
    void onCompressAddfileSlot(bool status);

    void slotLoadingFinished(KJob *job);
    void slotExtractionDone(KJob* job);
    void slotextractSelectedFilesTo(const QString& localPath);
    void SlotProgress(KJob *job, unsigned long percent);
    void SlotProgressFile(KJob *job, const QString& filename);
    void SlotNeedPassword();
    void SlotExtractPassword(QString password);
    void slotCompressFinished(KJob *job);
    void slotExtractSimpleFiles(QVector<Archive::Entry*> fileList, QString path);
    void slotKillExtractJob();
    void slotFailRetry();


signals:
    void sigquitApp();
    void sigZipAddFile();
    void sigZipReturn();
    void sigZipSelectedFiles(const QStringList &files);
    void loadingStarted();

private:
    Archive* m_archive_manager;
    ArchiveModel         *m_model;
    ArchiveSortFilterModel *m_filterModel;
    QString m_decompressfilename;
    QString m_decompressfilepath;
    QString m_loadfile;

    void setCompressDefaultPath();
    void setQLabelText(QLabel *label, const QString &text);
    QJsonObject creatShorcutJson();

    QStringList CheckAllFiles(QString path);
    void deleteCompressFile(QStringList oldfiles, QStringList newfiles);

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
    EncryptionPage* m_encryptionpage;
    ProgressDialog* m_progressdialog;
    SettingDialog* m_settingsDialog;
    EncodingPage* m_encodingpage;
    QSettings* m_settings;
    Page_ID m_pageid;

    QVector<Archive::Entry*> m_extractSimpleFiles;

    DIconButton* m_titlebutton;

    ExtractJob* m_encryptionjob;
    LoadJob* m_loadjob;
    CreateJob* m_createJob;
    EncryptionType m_encryptiontype;
    bool m_isrightmenu;
    WorkState m_workstatus;

    int m_timerId = 0;
    bool m_progressTransFlag = false;
    QAction *m_openAction;

    QStringList m_compressDirFiles;
    QString m_pathstore;
};

#endif
