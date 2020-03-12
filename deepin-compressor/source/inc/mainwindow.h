/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
 *             chenglu <chenglu@uniontech.com>
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

#pragma once

#include <DMainWindow>
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
#include "batchcompress.h"
#include <DFileWatcher>
#include <QElapsedTimer>


#define TITLE_FIXED_HEIGHT 50

DWIDGET_USE_NAMESPACE

enum Page_ID {
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

enum EncryptionType {
    Encryption_NULL,
    Encryption_Load,
    Encryption_Extract,
    Encryption_SingleExtract,
    Encryption_ExtractHere,
    Encryption_TempExtract
};

enum WorkState {
    WorkNone,
    WorkProcess,
};

class QStackedLayout;

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void InitUI();
    void InitConnection();
    void initTitleBar();
    QMenu *createSettingsMenu();
    void loadArchive(const QString &files);
    void creatArchive(QMap<QString, QString> &Args);
    void creatBatchArchive(QMap<QString, QString> &Args, QMap<QString, QStringList> &filetoadd);

    void transSplitFileName(QString& fileName); // *.7z.003 -> *.7z.001

    void ExtractPassword(QString password);
    void ExtractSinglePassword(QString password);
    void LoadPassword(QString password);
    void WatcherFile(const QString &files);
    void renameCompress(QString &filename, QString fixedMimeType);
    static QString getLoadFile();
    qint64 getDiskFreeSpace();
    qint64 getMediaFreeSpace();

    bool applicationQuit();

private:
    void saveWindowState();
    void loadWindowState();

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
    void onCancelCompressPressed(int compressType);
    void onTitleButtonPressed();
    void onCompressAddfileSlot(bool status);

    void slotLoadingFinished(KJob *job);
    void slotExtractionDone(KJob *job);
    void slotextractSelectedFilesTo(const QString &localPath);
    void SlotProgress(KJob *job, unsigned long percent);
    void SlotProgressFile(KJob *job, const QString &filename);
    void SlotNeedPassword();
    void SlotExtractPassword(QString password);
    void slotCompressFinished(KJob *job);
    void slotExtractSimpleFiles(QVector<Archive::Entry *> fileList, QString path);
    void slotKillExtractJob();
    void slotFailRetry();
    void slotBatchExtractFileChanged(const QString& name);
    void slotBatchExtractError(const QString& name);
    void slotClearTempfile();
    void slotquitApp();
    void onUpdateDestFile(QString destFile);
    void onCompressPageFilelistIsEmpty();

    void slotCalDeleteRefreshTotalFileSize(const QStringList &files);
    void resetMainwindow();
    void slotBackButtonClicked();
    void slotResetPercentAndTime();

signals:
    void sigquitApp();
    void sigZipAddFile();
    void sigZipReturn();
    void sigZipSelectedFiles(const QStringList &files);
    void loadingStarted();

private:
    Archive *m_archive_manager;
    ArchiveModel         *m_model = nullptr;
    ArchiveSortFilterModel *m_filterModel;
    QString m_decompressfilename;
    QString m_decompressfilepath;
    static QString m_loadfile;

    void setCompressDefaultPath();
    void setQLabelText(QLabel *label, const QString &text);
    QJsonObject creatShorcutJson();

    QStringList CheckAllFiles(QString path);
    void deleteCompressFile(/*QStringList oldfiles, QStringList newfiles*/);

private:
    DLabel *m_logo;
    QPixmap m_logoicon;
    QFrame *m_titleFrame;
    DLabel *m_titlelabel;
    DWidget *m_mainWidget;
    QStackedLayout *m_mainLayout;
    HomePage *m_homePage;
    UnCompressPage *m_UnCompressPage;
    CompressPage *m_CompressPage;
    CompressSetting *m_CompressSetting;
    Progress *m_Progess;
    Compressor_Success *m_CompressSuccess;
    Compressor_Fail *m_CompressFail;
    EncryptionPage *m_encryptionpage;
    ProgressDialog *m_progressdialog;
    SettingDialog *m_settingsDialog = nullptr;
    EncodingPage *m_encodingpage;
    QSettings *m_settings;
    Page_ID m_pageid;

    QVector<Archive::Entry *> m_extractSimpleFiles;

    DIconButton *m_titlebutton = nullptr;

    ExtractJob *m_encryptionjob = nullptr;
    LoadJob *m_loadjob = nullptr;
    CreateJob *m_createJob = nullptr;
    EncryptionType m_encryptiontype = Encryption_NULL;
    bool m_isrightmenu = false;
    WorkState m_workstatus = WorkNone;

    int m_timerId = 0;
    //bool m_progressTransFlag = false;
    QAction *m_openAction;

    //QStringList m_compressDirFiles;
    QString createCompressFile_;

    QString m_pathstore;
    bool m_initflag = false;
    int m_startTimer = 0;
    int m_watchTimer = 0;

    DFileWatcher* m_fileManager = nullptr;

private:
    void calSelectedTotalFileSize(const QStringList &files);
    qint64 calFileSize(const QString &path);
    void calSpeedAndTime(unsigned long compressPercent);

    QElapsedTimer m_timer;
    unsigned long lastPercent = 0;
    qint64 selectedTotalFileSize = 0;
    qint64 compressTime = 0;

#ifdef __aarch64__
    qint64 maxFileSize_ = 0;
#endif
};














