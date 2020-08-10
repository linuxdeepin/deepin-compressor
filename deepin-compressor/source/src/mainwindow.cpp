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

#include "mainwindow.h"
#include "homepage.h"
#include "uncompresspage.h"
#include "compresspage.h"
#include "compresssetting.h"
#include "compressor_success.h"
#include "compressor_fail.h"
#include "archive_manager.h"
#include "archivemodel.h"
#include "encryptionpage.h"
#include "progressdialog.h"
#include "extractpausedialog.h"
#include "settingdialog.h"
#include "encodingpage.h"
#include "archivesortfiltermodel.h"
#include "batchextract.h"
#include "batchcompress.h"
#include "openloadingpage.h"
#include "pluginmanager.h"
#include "utils.h"
#include "compressorapplication.h"
#include "structs.h"
#include "openwithdialog/openwithdialog.h"
#include "jobs.h"
#include "kprocess.h"
#include "monitorInterface.h"
//#include "filewatcher.h"
#include "monitorAdaptor.h"

#include <DApplication>
#include <DFileWatcher>
#include <DDesktopServices>
#include <DMessageManager>
#include <DStandardPaths>
#include <DFontSizeManager>
#include <DWidgetUtil>

#include <QDebug>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMimeData>
#include <QMimeDatabase>
#include <QShortcut>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QSvgWidget>
#include <QTimer>
#include <QStackedLayout>
#include <QStackedLayout>
#include <QScreen>
#include <QUuid>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <unistd.h>

DWIDGET_USE_NAMESPACE

QMutex mutex;
int MainWindow::m_windowcount = 1;

MainWindow::MainWindow(QWidget *parent) : DMainWindow(parent)
{
//    setAttribute(Qt::WA_DeleteOnClose);
    qDebug() << "MainWindow::MainWindow";
    m_pArchiveModel = new ArchiveModel(this);
    m_pArchiveFilterModel = new ArchiveSortFilterModel(this);

    m_pMmainWidget = new DWidget(this);
    m_pMainLayout = new QStackedLayout(m_pMmainWidget);
    m_pHomePage = new HomePage(this);
    m_pMainLayout->addWidget(m_pHomePage);
    m_pHomePage->setAutoFillBackground(true);

    // init window flags.
    setWindowTitle(tr("Archive Manager"));
    // setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setCentralWidget(m_pMmainWidget);
    setAcceptDrops(true);

    initTitleBar();

    //m_startTimer = startTimer(500);
    InitUI();
    InitConnection();

    loadWindowState();
}

MainWindow::~MainWindow()
{
    if (m_windowcount == 0) {
        if (this->m_pMapGlobalWnd != nullptr) {
            this->m_pMapGlobalWnd->mMapGlobal.clear();
        }
        if (this->m_pCurAuxInfo != nullptr) {
            this->m_pCurAuxInfo->information.clear();
        }
    }
    saveWindowState();
}

void MainWindow::bindAdapter()
{
    m_pAdaptor = new MonitorAdaptor(this);
}

qint64 MainWindow::getMediaFreeSpace()
{
    QList< QStorageInfo > list = QStorageInfo::mountedVolumes();
    qDebug() << "Volume Num: " << list.size();
    for (QStorageInfo &si : list) {
        qDebug() << si.displayName();
        if (si.displayName().count() > 7 && si.displayName().left(6) == "/media") {
            //qDebug() << "Bytes Avaliable: " << si.bytesAvailable() / 1024 / 1024 << "MB";
            return si.bytesAvailable() / 1024 / 1024;
        }
    }

    return 0;
}

bool MainWindow::applicationQuit(CompressorApplication *p)
{
    QWidget *pWgt = p->activeWindow();
    QMainWindow *pWindow = qobject_cast<QMainWindow *>(pWgt);
    DMainWindow *pp = qobject_cast<DMainWindow *>(pWindow);

    if (pp == nullptr)
        return false;

    pp->close();

    //    if (pp == this) {
    //        //       killJob();

    //        //        if (m_pProgess->getType() == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD) {
    //        //            return true;
    //        //        }

    //        //        if (PAGE_ZIPPROGRESS == m_pMainLayout->currentIndex()) {
    //        //            if (1 != m_pProgess->showConfirmDialog()) {
    //        //                return false;
    //        //            }
    //        //            deleteCompressFile();
    //        //            QString destDirName;
    //        //            deleteDecompressFile(destDirName);
    //        //        } else if (7 == m_pMainLayout->currentIndex()) {
    //        //            deleteCompressFile();
    //        //        }
    //        close();
    //        //DApplication::quit();
    //    } else {
    //        pp->close();
    //    }

    return true;
}

//QString MainWindow::getAddFile()
//{
//    return  m_strAppendFileName;
//}

void MainWindow::saveWindowState()
{
    QSettings settings(objectName());
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::loadWindowState()
{
    QSettings settings(objectName());
    const QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        resize(620, 465);
    }

    setMinimumSize(620, 465);
}

//QString MainWindow::getLoadFile()
//{
//    return m_strLoadfile;
//}

qint64 MainWindow::getDiskFreeSpace()
{
//    QStorageInfo storage = QStorageInfo::root();
    QStorageInfo storage(m_strPathStore);
    storage.refresh();
//    qDebug() << storage.name() << storage.bytesTotal() / 1024 / 1024 << "MB";
//    qDebug() << "availableSize:" << storage.bytesAvailable() / 1024 / 1024 << "MB";
    return storage.bytesAvailable() / 1024 / 1024;
}

int MainWindow::queryDialogForClose()
{
    DDialog *dialog = new DDialog(this);
    dialog->setFixedWidth(440);
    QIcon icon = QIcon::fromTheme("deepin-compressor");
    dialog->setIcon(icon /*, QSize(32, 32)*/);
    dialog->setMessage(tr("Do you want to close the window even it has working job?"));
    dialog->addButton(tr("Cancel"));
    dialog->addButton(QObject::tr("OK"));
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
//    effect->setOffset(0, 4);
//    effect->setColor(QColor(0, 145, 255, 76));
//    effect->setBlurRadius(4);
//    dialog->getButton(2)->setGraphicsEffect(effect);

    const int mode = dialog->exec();
    delete dialog;
    qDebug() << mode;
    return mode;
}

void MainWindow::closeClean(QCloseEvent *event)
{
    if (m_pJob) {
        if (m_pJob->mType == KJob::ENUM_JOBTYPE::EXTRACTJOB) {
            this->closeExtractJobSafe();
        } else if (m_pJob->mType == KJob::ENUM_JOBTYPE::DELETEJOB) {
            DeleteJob *pJob = dynamic_cast<DeleteJob *>(m_pJob);
            pJob->archiveInterface()->extractPsdStatus = ReadOnlyArchiveInterface::ExtractPsdStatus::Canceled;
        } else {
            deleteLaterJob();
        }
    }

    event->accept();

    if (m_windowcount == 1) {
        return;
    }

    //    Archive::Entry *pRootEntry = m_pArchiveModel->getRootEntry();
    //    if (pRootEntry) {
    //        pRootEntry->clean();
    //    }

    //    SAFE_DELETE_ELE(pRootEntry);
    //    SAFE_DELETE_ELE(m_pFileWatcher);
    //    SAFE_DELETE_ELE(pEventloop);
    //    SAFE_DELETE_ELE(m_pSpinner);
    //    SAFE_DELETE_ELE(m_pWatcher);
    //    SAFE_DELETE_ELE(m_pArchiveModel);
    //    SAFE_DELETE_ELE(m_logo);
    //    SAFE_DELETE_ELE(m_titleFrame);
    //    SAFE_DELETE_ELE(m_titlelabel);
    //    SAFE_DELETE_ELE(m_pUnCompressPage);
    //    SAFE_DELETE_ELE(m_pCompressPage);
    //    SAFE_DELETE_ELE(m_pMainLayout);
    //    SAFE_DELETE_ELE(m_pHomePage);
    //    SAFE_DELETE_ELE(m_pCompressSetting);
    //    SAFE_DELETE_ELE(m_pProgess);
    //    SAFE_DELETE_ELE(m_pCompressSuccess);
    //    SAFE_DELETE_ELE(m_pCompressFail);
    //    SAFE_DELETE_ELE(m_pEncryptionpage);
    //    SAFE_DELETE_ELE(m_pProgressdialog);
    //    SAFE_DELETE_ELE(m_pSettingsDialog);
    //    SAFE_DELETE_ELE(m_pOpenLoadingPage);
    //    //SAFE_DELETE_ELE(m_encodingpage);
    //    SAFE_DELETE_ELE(m_pSettings);
    //    SAFE_DELETE_ELE(m_pMmainWidget);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    char options = OpenInfo::CLOSE;
    if (this->m_pCurAuxInfo != nullptr) {
        MainWindow_AuxInfo *curAuxInfo = this->m_pCurAuxInfo;
        QMap<QString, OpenInfo *>::iterator it = curAuxInfo->information.begin();
        if (this->m_pMapGlobalWnd != nullptr) {
            while (it != curAuxInfo->information.end()) {
                OpenInfo *pInfo = it.value();
                it++;

                MainWindow *p = qobject_cast<MainWindow *>(this->m_pMapGlobalWnd->getOne(pInfo->strWinId));
                if (p != nullptr) {
                    p->close();//close all children mainwindow
                } else {
                    continue;
                }

                if (p->m_eOption == OpenInfo::CLOSE) {
                    options |= p->m_eOption;
                } else if (p->m_eOption == OpenInfo::OPEN) {
                    options |= p->m_eOption;
                } else if (p->m_eOption == OpenInfo::QUERY_CLOSE_CANCEL) {
                    options |= p->m_eOption;
                }
            }
        }


        QMap<QString, OpenInfo *>::iterator iter;
        QString key;
        for (iter = curAuxInfo->information.begin(); iter != curAuxInfo->information.end();) {
            key = iter.key();
            iter++;             //指针移至下一个位置
            if (curAuxInfo->information[key]->option == OpenInfo::CLOSE) {
                OpenInfo *p = curAuxInfo->information.take(key);
                SAFE_DELETE_ELE(p);
            }
        }
    }

    QString destDirName;
    qDebug() << "子窗口开始关闭";
    //判断m_pJob是否结束
    int mode = -1;
    if (m_pJob == nullptr) {
        if (options == OpenInfo::QUERY_CLOSE_CANCEL) {//如果子面板取消关闭
            event->ignore();
            this->m_eOption = OpenInfo::QUERY_CLOSE_CANCEL;
            return;
        } else if (options == OpenInfo::CLOSE) {//如果子面板那正常关闭
            //event->accept();
            closeClean(event);
            this->m_eOption = OpenInfo::CLOSE;
            removeFromParentInfo(this);
            if (this->m_pMapGlobalWnd != nullptr) {
                this->m_pMapGlobalWnd->remove(QString::number(this->winId()));
            }

        }
    } else {
        if (options == OpenInfo::QUERY_CLOSE_CANCEL) {
            event->ignore();
            this->m_eOption = OpenInfo::QUERY_CLOSE_CANCEL;
            return;
        } else {//如果子面板正常关闭；并且当前面板job完成
            /*int */ mode = queryDialogForClose();
            if (mode == 0 || mode == -1) {
                event->ignore();
                this->m_eOption = OpenInfo::QUERY_CLOSE_CANCEL;
                return;
            } else if (mode == 1) {
                if (m_pJob->mType == KJob::ENUM_JOBTYPE::EXTRACTJOB) {
                    ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
                    destDirName = pExtractJob->archiveInterface()->destDirName; //获取解压目标文件(夹)名
                }

                closeClean(event);
                removeFromParentInfo(this);

                this->m_eOption = OpenInfo::CLOSE;
                if (this->m_pMapGlobalWnd != nullptr) {
                    this->m_pMapGlobalWnd->remove(QString::number(this->winId()));
                }
            }

        }

    }

    if (m_pProgess->getType() == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD) {
        if (m_pJob && m_pJob->mType == Job::ENUM_JOBTYPE::ADDJOB) {
            AddJob *pAddJob = dynamic_cast<AddJob *>(m_pJob);
            pAddJob->kill();
            pAddJob = nullptr;
        }

        deleteCompressFile();
        slotquitApp();
        return;
    }

    if (PAGE_ZIPPROGRESS == m_ePageID || PAGE_DELETEPROGRESS == m_ePageID /*m_pMainLayout->currentIndex()*/) {
        //        if (1 != m_pProgess->showConfirmDialog()) {
        //            event->ignore();
        //            return;
        //        }

        deleteCompressFile();
        slotquitApp();
        //emit sigquitApp();
    } else if (PAGE_UNZIPPROGRESS == m_ePageID) {
        deleteDecompressFile(destDirName);
        slotquitApp();
    } else if (PAGE_ZIP_FAIL == m_ePageID /*m_pMainLayout->currentIndex()*/) {
        deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_strPathStore)*/);
        slotquitApp();
    } else {
        slotquitApp();
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    /*if (m_timerId == event->timerId()) {
        m_progressTransFlag = true;
        killTimer(m_timerId);
        m_timerId = 0;
    } else */
    /*qDebug() << "timerEvent";
    if (m_startTimer == event->timerId()) {
        if (!m_initflag) {
            qDebug() << "timerEvent******************InitUI";
            InitUI();
            InitConnection();
            m_initflag = true;
        }

        killTimer(m_startTimer);
        m_startTimer = 0;
    } else*/ if (m_iWatchTimerID == event->timerId()) {     // 监控待压缩的本地文件
        if (m_pCompressPage == nullptr) {
            return;
        }

        QStringList filelist = m_pCompressPage->getCompressFilelist();

        for (int i = 0; i < filelist.count(); i++) {
            QFileInfo filein(filelist.at(i));

            // 若本地文件不存在了，则提示用户
            if (!filein.exists()) {
                QString displayName = Utils::toShortString(filein.fileName());
                QString strTips = tr("%1 was changed on the disk, please import it again.").arg(displayName);
                DDialog *dialog = new DDialog(this);
                QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
                dialog->setIcon(pixmap);
                dialog->addSpacing(32);
                dialog->setMinimumSize(380, 140);
                dialog->addButton(tr("OK"), true, DDialog::ButtonNormal);
//                QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
//                effect->setOffset(0, 4);
//                effect->setColor(QColor(0, 145, 255, 76));
//                effect->setBlurRadius(4);
//                dialog->getButton(0)->setFixedWidth(340);
//                dialog->getButton(0)->setGraphicsEffect(effect);

                DLabel *pLblContent = new DLabel(strTips, dialog);
                pLblContent->setAlignment(Qt::AlignmentFlag::AlignHCenter);
                DPalette pa;
                pa = DApplicationHelper::instance()->palette(pLblContent);
                pa.setBrush(DPalette::Text, pa.color(DPalette::ButtonText));
                DFontSizeManager::instance()->bind(pLblContent, DFontSizeManager::T6, QFont::Medium);
                pLblContent->setMinimumWidth(this->width());
                pLblContent->move(dialog->width() / 2 - pLblContent->width() / 2, 48);
                dialog->exec();

                // 从文件列表中删除已经不存在的文件
                filelist.removeAt(i);
                if (m_ePageID != PAGE_ZIP) {
                    m_ePageID = PAGE_ZIP;
                    refreshPage();
                }

                m_pCompressPage->onRefreshFilelist(filelist);   // 刷新需要压缩的文件列表

                if (filelist.isEmpty()) {   // 如果剩下的文件数目为0，则回到首页
                    m_ePageID = PAGE_HOME;
                    m_pCompressPage->setRootPathIndex();
                    refreshPage();
                } else {
                    m_pCompressPage->setRootPathIndex();
                    refreshPage();
                }

                SAFE_DELETE_ELE(dialog);
            }
        }
    }
}

void MainWindow::InitUI()
{
    m_pUnCompressPage = new UnCompressPage(this);
    m_pCompressPage = new CompressPage(this);
    m_pCompressSetting = new CompressSetting(this);
    m_pProgess = new Progress(this);
    m_pCompressSuccess = new Compressor_Success(this);
    m_pCompressFail = new Compressor_Fail(this);
    m_pEncryptionpage = new EncryptionPage(this);
    m_pProgressdialog = new ProgressDialog(this);
    m_pSettingsDialog = new SettingDialog(this);
    //m_encodingpage = new EncodingPage(this);
    m_pSettings = new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this);
    m_pOpenLoadingPage = new OpenLoadingPage(this);

    if (m_pSettings->value("dir").toString().isEmpty()) {
        m_pSettings->setValue("dir", "");
    }

    // add widget to main layout.
    m_pMainLayout->addWidget(m_pUnCompressPage);
    m_pMainLayout->addWidget(m_pCompressPage);
    m_pMainLayout->addWidget(m_pCompressSetting);
    m_pMainLayout->addWidget(m_pProgess);
    m_pMainLayout->addWidget(m_pCompressSuccess);
    m_pMainLayout->addWidget(m_pCompressFail);
    m_pMainLayout->addWidget(m_pEncryptionpage);
    //m_pMainLayout->addWidget(m_encodingpage);
    m_pMainLayout->addWidget(m_pOpenLoadingPage);
    m_pUnCompressPage->setAutoFillBackground(true);
    m_pCompressPage->setAutoFillBackground(true);
    m_pCompressSetting->setAutoFillBackground(true);
    m_pProgess->setAutoFillBackground(true);
    m_pCompressSuccess->setAutoFillBackground(true);
    m_pCompressFail->setAutoFillBackground(true);
    m_pEncryptionpage->setAutoFillBackground(true);
    //m_encodingpage->setAutoFillBackground(true);
}

QJsonObject MainWindow::creatShorcutJson()
{
    QJsonObject shortcut1;
    shortcut1.insert("name", tr("Close"));
    shortcut1.insert("value", "Alt+F4");

    QJsonObject shortcut2;
    shortcut2.insert("name", tr("Help"));
    shortcut2.insert("value", "F1");

    QJsonObject shortcut3;
    shortcut3.insert("name", tr("Select the file"));
    shortcut3.insert("value", "Ctrl+O");

    QJsonObject shortcut4;
    shortcut4.insert("name", tr("Delete"));
    shortcut4.insert("value", "Delete");

    //    QJsonObject shortcut5;
    //    shortcut5.insert("name", tr("Rename"));
    //    shortcut5.insert("value", "F2");

    QJsonObject shortcut6;
    shortcut6.insert("name", tr("Display shortcuts"));
    shortcut6.insert("value", "Ctrl+Shift+?");

    QJsonArray shortcutArray;
    shortcutArray.append(shortcut1);
    shortcutArray.append(shortcut2);
    shortcutArray.append(shortcut3);
    shortcutArray.append(shortcut4);
    // shortcutArray.append(shortcut5);
    shortcutArray.append(shortcut6);

    QJsonObject shortcut_group;
    shortcut_group.insert("groupName", tr("Shortcuts"));
    shortcut_group.insert("groupItems", shortcutArray);

    QJsonArray shortcutArrayall;
    shortcutArrayall.append(shortcut_group);

    QJsonObject main_shortcut;
    main_shortcut.insert("shortcut", shortcutArrayall);

    return main_shortcut;
}

void MainWindow::InitConnection()
{
    // connect the signals to the slot function.
    connect(m_pHomePage, &HomePage::fileSelected, this, &MainWindow::onSelected);
    connect(m_pCompressPage, &CompressPage::sigFilelistIsEmpty, this, &MainWindow::onCompressPageFilelistIsEmpty);
    connect(m_pCompressPage, &CompressPage::sigselectedFiles, this, &MainWindow::onSelected);
    connect(m_pCompressPage, &CompressPage::sigRefreshFileList, this, &MainWindow::slotCalDeleteRefreshTotalFileSize);
    connect(m_pCompressPage, &CompressPage::sigNextPress, this, &MainWindow::onCompressNext);
    connect(this, &MainWindow::sigZipAddFile, m_pCompressPage, &CompressPage::onAddfileSlot);
    connect(this, &MainWindow::sigCompressedAddFile, m_pUnCompressPage, &UnCompressPage::slotCompressedAddFile);
    connect(m_pCompressSetting, &CompressSetting::sigCompressPressed, this, &MainWindow::onCompressPressed);
    connect(m_pCompressSetting, &CompressSetting::sigUncompressStateAutoCompress, this, &MainWindow::onUncompressStateAutoCompress);
    connect(m_pCompressSetting, &CompressSetting::sigUncompressStateAutoCompressEntry, this, &MainWindow::onUncompressStateAutoCompressEntry);
    connect(m_pCompressSetting, &CompressSetting::sigFileUnreadable, this, &MainWindow::slotFileUnreadable);
    connect(m_pProgess, &Progress::sigCancelPressed, this, &MainWindow::onCancelCompressPressed);
    connect(m_pCompressSuccess, &Compressor_Success::sigQuitApp, this, &MainWindow::slotquitApp);
    connect(m_pCompressSuccess, &Compressor_Success::sigBackButtonClicked, this, &MainWindow::slotBackButtonClicked);
    connect(m_pTitleButton, &DPushButton::clicked, this, &MainWindow::onTitleButtonPressed);
    connect(this, &MainWindow::sigZipSelectedFiles, m_pCompressPage, &CompressPage::onSelectedFilesSlot);
    connect(m_pArchiveModel, &ArchiveModel::loadingFinished, this, &MainWindow::slotLoadingFinished);
    connect(m_pUnCompressPage, &UnCompressPage::sigDecompressPress, this, &MainWindow::slotextractSelectedFilesTo);
//    connect(m_pUnCompressPage, &UnCompressPage::sigRefreshFileList, this, &MainWindow::slotUncompressCalDeleteRefreshTotalFileSize);
    connect(m_pUnCompressPage, &UnCompressPage::sigRefreshEntryVector, this, &MainWindow::slotUncompressCalDeleteRefreshTotoalSize);
    connect(m_pUnCompressPage, &UnCompressPage::sigFilelistIsEmpty, this, &MainWindow::onCompressPageFilelistIsEmpty);
    connect(m_pEncryptionpage, &EncryptionPage::sigExtractPassword, this, &MainWindow::SlotExtractPassword);
    connect(m_pUnCompressPage, &UnCompressPage::sigextractfiles, this, &MainWindow::slotExtractSimpleFiles);
//    connect(this, &MainWindow::sigTipsWindowPopUp, m_pUnCompressPage, &UnCompressPage::subWindowTipsPopSig);
    connect(m_pUnCompressPage, &UnCompressPage::sigAutoCompress, m_pCompressSetting, &CompressSetting::autoCompress);
    connect(m_pUnCompressPage, &UnCompressPage::sigAutoCompressEntry, m_pCompressSetting, &CompressSetting::autoCompressEntry);
    connect(m_pUnCompressPage, &UnCompressPage::sigOpenExtractFile, this, &MainWindow::slotExtractSimpleFilesOpen);
    connect(m_pUnCompressPage, &UnCompressPage::sigDeleteArchiveFiles, this, &MainWindow::deleteFromArchive);
//    connect(m_pUnCompressPage, &UnCompressPage::sigAddArchiveFiles, this, &MainWindow::addToArchive);
    connect(m_pCompressSetting, &CompressSetting::sigMoveFilesToArchive, this, &MainWindow::moveToArchive);
    connect(this, &MainWindow::deleteJobComplete, m_pUnCompressPage, &UnCompressPage::slotDeleteJobFinished);
    connect(this, &MainWindow::sigUpdateTableView, m_pUnCompressPage, &UnCompressPage::sigUpdateUnCompreeTableView);
    connect(m_pProgressdialog, &ProgressDialog::stopExtract, this, &MainWindow::slotKillExtractJob);
    connect(m_pProgressdialog, &ProgressDialog::sigResetPercentAndTime, this, &MainWindow::slotResetPercentAndTime);
    connect(m_pCompressFail, &Compressor_Fail::sigFailRetry, this, &MainWindow::slotFailRetry);
    connect(m_pCompressFail, &Compressor_Fail::sigBackButtonClickedOnFail, this, &MainWindow::slotBackButtonClicked);
    connect(m_pCompressPage, &CompressPage::sigiscanaddfile, this, &MainWindow::onCompressAddfileSlot);

    connect(m_pProgressdialog, &ProgressDialog::extractSuccess, this, [ = ](QString msg) {
        QIcon icon = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
        this->sendMessage(icon, msg);

        // 如果设置了自动打开，执行下列操作
        if (m_pSettingsDialog->isAutoOpen()) {
            QString fullpath = m_strDecompressFilePath + "/" + m_vecExtractSimpleFiles.at(0)->property("name").toString();
            qDebug() << fullpath;
            QFileInfo fileinfo(fullpath);

            // 如果文件/文件夹存在
            if (fileinfo.exists()) {
                //                if (fileinfo.isDir()) {
                //                    DDesktopServices::showFolder(fullpath);     // 如果是文件夹
                //                } else if (fileinfo.isFile()) {
                qDebug() << "DDesktopServices start:" << fullpath;
                m_DesktopServicesThread = new DDesktopServicesThread();
                connect(m_DesktopServicesThread, SIGNAL(finished()), this, SLOT(slotKillShowFoldItem()));
                m_DesktopServicesThread->m_path = fullpath;
                m_DesktopServicesThread->start();
                //DDesktopServices::showFileItem(fullpath);   // 如果是单个文件 原BUG使用该函数，解压到桌面但文件，会出现30妙等待
                qDebug() << "DDesktopServices end:" << m_strDecompressFilePath;
                //  }
            }
        }
    });

    auto openkey = new QShortcut(QKeySequence(Qt::Key_Slash + Qt::CTRL + Qt::SHIFT), this);
    openkey->setContext(Qt::ApplicationShortcut);

    connect(openkey, &QShortcut::activated, this, [this] {
        const QRect &rect = window()->geometry();
        QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
        QStringList shortcutString;
        QJsonObject json = creatShorcutJson();

        QString param1 = "-j=" + QString(QJsonDocument(json).toJson());
        QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
        shortcutString << param1 << param2;

        QProcess *shortcutViewProcess = new QProcess(this);
        shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

        connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
    });

    connect(m_pArchiveModel, &ArchiveModel::signalUserQuery, [ = ](Query * query) {
        qDebug() << "query->execute";
        query->setParent(this);
        query->execute();
    });
}

QMenu *MainWindow::createSettingsMenu()
{
    QMenu *menu = new QMenu();

    // 创建打开Acticon
    m_pOpenAction = menu->addAction(tr("Open file"));

    connect(m_pOpenAction, &QAction::triggered, this, [this] {
        // 打开选择文件对话框
        if (m_ePageID == PAGE_UNZIP)
        {
            emit sigCompressedAddFile();
            return;
        }

        DFileDialog dialog(this);
        dialog.setAcceptMode(DFileDialog::AcceptOpen);
        dialog.setFileMode(DFileDialog::ExistingFiles);
        dialog.setAllowMixedSelection(true);

        // 设置历史目录（默认显示）
        QString historyDir = m_pSettings->value("dir").toString();
        if (historyDir.isEmpty())
        {
            historyDir = QDir::homePath();
        }
        dialog.setDirectory(historyDir);

        const int mode = dialog.exec();

        // save the directory string to config file.
        m_pSettings->setValue("dir", dialog.directoryUrl().toLocalFile());  // 获取选择目录

        // if click cancel button or close button.
        if (mode != QDialog::Accepted)
        {
            return;
        }

        onSelected(dialog.selectedFiles());

    });

    // menu->insertAction();

    // 创建设置Action
    QAction *settingsAction = menu->addAction(tr("Settings"));

    connect(settingsAction, &QAction::triggered, this, [this] { m_pSettingsDialog->exec(); });

    menu->addSeparator();

    return menu;
}

void MainWindow::initTitleBar()
{
    // 初始化标题栏菜单
    titlebar()->setMenu(createSettingsMenu());
    titlebar()->setFixedHeight(50);
    titlebar()->setTitle("");

    // 设置应用程序图标
    QIcon icon = QIcon::fromTheme("deepin-compressor");
    titlebar()->setIcon(icon);

    // 添加左上角按钮
    m_pTitleButton = new DIconButton(DStyle::SP_IncreaseElement, this);
    m_pTitleButton->setFixedSize(38, 38);
    m_pTitleButton->setVisible(false);

    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->addSpacing(6);
    leftLayout->addWidget(m_pTitleButton);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *left_frame = new QFrame(this);
    left_frame->setFixedWidth(6 + 38);
    left_frame->setContentsMargins(0, 0, 0, 0);
    left_frame->setLayout(leftLayout);

    titlebar()->addWidget(left_frame, Qt::AlignLeft);
    titlebar()->setContentsMargins(0, 0, 0, 0);
}

//void MainWindow::setQLabelText(QLabel *label, const QString &text)
//{
//    QFontMetrics cs(label->font());
//    int textWidth = cs.width(text);
//    if (textWidth > label->width()) {
//        label->setToolTip(text);
//    } else {
//        label->setToolTip("");
//    }

//    QFontMetrics elideFont(label->font());
//    label->setText(elideFont.elidedText(text, Qt::ElideMiddle, label->width()));
//}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // 判断是否有url
    if (!mime->hasUrls()) {
        return e->ignore();
    }

    // traverse.
    m_pHomePage->setIconPixmap(true);  // 设置图片
    return e->accept();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *e)
{
    m_pHomePage->setIconPixmap(false);  // 设置图片

    DMainWindow::dragLeaveEvent(e);
}

void MainWindow::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (false == mime->hasUrls()) {
        return e->ignore();
    }

    e->accept();

    // 判断本地文件
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        fileList << url.toLocalFile();
        // const QFileInfo info(localPath);
        qDebug() << fileList;
    }

    if (fileList.size() == 0) {
        return;
    }

    m_pHomePage->setIconPixmap(false);  // 设置图片
    onSelected(fileList);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

bool MainWindow::onSubWindowActionFinished(int /*mode*/, const qint64 &/*pid*/, const QStringList &/*urls*/)
{
//    qDebug() << "子界面拖拽完成！进程pid为：" << pid;
//    qDebug() << "当前进程pid为：" << getpid() << ";父类进程pid为：" << getppid();
//    qDebug() << "进程列表中进程有：" << m_gTempProcessId.size() << "个！";
//    QWriteLocker locker(&m_lock);
//    if (m_gTempProcessId.empty()) {
//        return false;
//    }
//    if (!urls.isEmpty() && pid) {
//        if (m_mapSubWinDragFiles.contains(pid)) {
//            m_mapSubWinDragFiles[pid] = urls;
//        } else {
//            m_mapSubWinDragFiles.insert(pid, urls);
//        }
//        m_iMode = mode;
//        return true;
//    }
    return false;
}

bool MainWindow::popUpChangedDialog(const qint64 &pid)
{
    qDebug() << "主界面接收到要弹出对话框消息，当前进程pid为：" << getpid();
    QWriteLocker locker(&m_lock);
    if (m_gTempProcessId.empty()) {
        qDebug() << "主子进程为空" ;
        return false;
    }
    if (m_mapSubWinDragFiles.empty()) {
        qDebug() << "要添加拖拽文件为空" ;
        return false;
    }

    if (!m_gTempProcessId.contains(pid)) {
        qDebug() << "子进程不再进程列表中" ;
        qDebug() << "列表中的子进程为：" << m_gTempProcessId[0] ;
        return false;
    }
    m_lCurOperChildPid = pid;
    //pop dialog
    emit sigTipsWindowPopUp(m_iMode, m_mapSubWinDragFiles.value(pid));
    //m_gTempProcessId.removeAll(pid);
    m_mapSubWinDragFiles.remove(pid);
    return true;
}

bool MainWindow::createSubWindow(const QStringList &urls)
{
    MainWindow *subWindow = nullptr;
    bool bStagger = false;
    QStringList inUrls;

    if (urls.length() == 0) {
        bStagger = true;
        /*MainWindow **/subWindow = new MainWindow();
        subWindow->m_pMapGlobalWnd = this->m_pMapGlobalWnd;//获取deepin-compressor进程中的全局窗口map
        //++m_windowcount;

//        subWindow->show();

//        return true;
    } else {
        QString filePath = urls[0];
        QFileInfo fileInfo(filePath);

        /*QStringList */ inUrls = std::move(const_cast<QStringList &>(urls));
        qDebug() << "=================inUrls:" << inUrls;
        //qDebug() << "=================urls:" << urls;

        QString winid = "";
        for (int i = 0; i < inUrls.length(); i++) {
            if (inUrls[i].contains(HEADBUS)) {
                winid = inUrls[i];
                inUrls.removeOne(winid);
                winid.remove(HEADBUS);
                break;
            }
        }

        MainWindow *pParentWnd = nullptr;
        if (this->m_pMapGlobalWnd != nullptr) {
            pParentWnd = qobject_cast<MainWindow *>(this->m_pMapGlobalWnd->getOne(winid));
        }

        //create sub mainwindow
        //    if (inUrls.length() == 0) {
        //        return false;
        //    }

        /*MainWindow **/subWindow = new MainWindow();
        if (fileInfo.exists() == true && (!subWindow->checkSettings(filePath))) {//判断目标文件是否合法
            return  false;
        }

        subWindow->m_pMapGlobalWnd = this->m_pMapGlobalWnd;//获取deepin-compressor进程中的全局窗口map
        subWindow->m_pChildMndExtractPath = this->m_pChildMndExtractPath;//子面板的解压路径必须和父面板的解压路径统一
        if (this->m_pMapGlobalWnd == nullptr) {
            this->m_pMapGlobalWnd = new GlobalMainWindowMap();
        }

        m_pMapGlobalWnd->insert(QString::number(subWindow->winId()), subWindow);

        if (pParentWnd != nullptr) {
            bStagger = false;
            subWindow->m_pCurAuxInfo = new MainWindow_AuxInfo();
            subWindow->m_pCurAuxInfo->parentAuxInfo = pParentWnd->m_pCurAuxInfo;

            QString strModelIndex = inUrls.takeAt(1);//第一个参数存储的有modelIndex字符串
            if (pParentWnd->m_pCurAuxInfo != nullptr &&
                    pParentWnd->m_pCurAuxInfo->information.contains(strModelIndex) == true) {
                OpenInfo *pInfo = pParentWnd->m_pCurAuxInfo->information[strModelIndex];
                //            pInfo->isHidden = false;
                pInfo->option = OpenInfo::OPEN;
                pInfo->strWinId = QString::number(subWindow->winId());
                int childCount = pParentWnd->m_pCurAuxInfo->information.size();
                subWindow->move(pParentWnd->x() + childCount * 130, pParentWnd->y() + childCount * 92);
                connect(subWindow, &MainWindow::sigTipsWindowPopUp, pParentWnd->m_pUnCompressPage, &UnCompressPage::slotSubWindowTipsPopSig);

                subWindow->m_ePageID = PAGE_ZIP;
                //        subWindow->onRightMenuSelected(inUrls);
                //QMetaObject::invokeMethod(subWindow, "onRightMenuSelected", Qt::DirectConnection, Q_ARG(QStringList, inUrls));
                //        subWindow->onSelected(inUrls);
            }
        } else {
            bStagger = true;
//            if (inUrls.length() > 0) {
//                QMetaObject::invokeMethod(subWindow, "onRightMenuSelected", Qt::DirectConnection, Q_ARG(QStringList, inUrls));
//            }
        }

        //++m_windowcount;

    }

    ++m_windowcount;

    // 对需要窗口错开的情况移动窗口
    if (bStagger) {
        int iIndex = (m_windowcount - 1) % 5;

        switch (iIndex) {
        case 0:
            Dtk::Widget::moveToCenter(subWindow);
            break;
        case 1:
            subWindow->move(0, 0);
            break;
        case 2:
            subWindow->move(QApplication::desktop()->availableGeometry().width() - subWindow->width(), 0);
            break;
        case 3:
            subWindow->move(0, QApplication::desktop()->availableGeometry().height() - subWindow->height());
            break;
        case 4:
            subWindow->move(QApplication::desktop()->availableGeometry().width() - subWindow->width(), QApplication::desktop()->availableGeometry().height() - subWindow->height());
            break;
        }
    }

    subWindow->show();

    if (inUrls.length() > 0) {
        QMetaObject::invokeMethod(subWindow, "onRightMenuSelected", Qt::DirectConnection, Q_ARG(QStringList, inUrls));
    }

    return true;
}

//void MainWindow::setEnable()
//{
//    setAcceptDrops(true);

//    // enable titlebar buttons.
//    titlebar()->setDisableFlags(Qt::Widget);
//}

//void MainWindow::setDisable()
//{
//    setAcceptDrops(false);

//    // disable titlebar buttons.
//    titlebar()->setDisableFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint
//                                | Qt::WindowSystemMenuHint);
//}

void MainWindow::refreshPage()
{
    m_pEncryptionpage->resetPage();
    qDebug() << "m_ePageID: " << m_ePageID;

    switch (m_ePageID) {
    case PAGE_HOME:         // 首页

        if (m_pFileWatcher) {
            SAFE_DELETE_ELE(m_pFileWatcher);
        }
        m_pProgess->resetProgress();
        m_pOpenAction->setEnabled(true);
        setAcceptDrops(true);
        setTitleButtonStyle(false);
        titlebar()->setTitle("");
        m_pMainLayout->setCurrentIndex(0);
        break;
    case PAGE_UNZIP:        // 解压界面
        m_pProgess->resetProgress();
        m_pOpenAction->setEnabled(true);
        setAcceptDrops(false);
        setTitleButtonStyle(true, DStyle::StandardPixmap::SP_IncreaseElement);
        titlebar()->setTitle(m_strDecompressFileName);
        m_pMainLayout->setCurrentIndex(1);
        break;
    case PAGE_ZIP:          // 压缩界面
        m_pProgess->resetProgress();
        titlebar()->setTitle(tr("Create New Archive"));
        m_pOpenAction->setEnabled(true);
        setTitleButtonStyle(true, DStyle::StandardPixmap::SP_IncreaseElement);
        setAcceptDrops(true);
        m_iWatchTimerID = startTimer(1000);
        m_pCompressPage->onPathIndexChanged();
        m_pMainLayout->setCurrentIndex(2);
        break;
    case PAGE_ZIPSET:       // 压缩设置界面
        titlebar()->setTitle(tr("Create New Archive"));
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(true, DStyle::StandardPixmap::SP_ArrowLeave);
        m_pMainLayout->setCurrentIndex(3);
        break;
    case PAGE_ZIPPROGRESS:  // 进度界面
        if (0 != m_iWatchTimerID) {
            killTimer(m_iWatchTimerID);
            m_iWatchTimerID = 0;
        }
        if (this->m_operationtype == Operation_Load) {
            int limitCounts = 10;
            int left = 5, right = 5;
            QString displayName = "";
            displayName = m_strDecompressFileName.length() > limitCounts ? m_strDecompressFileName.left(left) + "..." + m_strDecompressFileName.right(right) : m_strDecompressFileName;
            QString strTitle = tr("adding files to %1").arg(m_strDecompressFileName);
            titlebar()->setTitle(strTitle);
        } else {
            titlebar()->setTitle(tr("Compressing"));
        }
        m_pProgess->setSpeedAndTimeText(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING);
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        m_pProgess->setFilename(m_strDecompressFileName);
        m_pMainLayout->setCurrentIndex(4);
        m_pProgess->pInfo()->startTimer();
        break;
    case PAGE_UNZIPPROGRESS:  // 进度界面
        m_pProgess->setSpeedAndTimeText(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        if (!m_pProgess->getType()) {
            titlebar()->setTitle(tr("Opening"));
        } else {
            titlebar()->setTitle(tr("Extracting"));
        }
        m_pProgess->setFilename(m_strDecompressFileName);
        m_pMainLayout->setCurrentIndex(4);
        m_pProgess->pInfo()->startTimer();
        break;
    case PAGE_DELETEPROGRESS:  // 进度界面
        m_pProgess->setSpeedAndTimeText(Progress::ENUM_PROGRESS_TYPE::OP_DELETEING);
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        titlebar()->setTitle(tr("Deleteing"));
        m_pProgess->setFilename(m_strDecompressFileName);
        m_pMainLayout->setCurrentIndex(4);
        m_pProgess->pInfo()->startTimer();
        break;
    case PAGE_ZIP_SUCCESS:  // 压缩成功界面
        titlebar()->setTitle("");
        m_pCompressSuccess->setstringinfo(tr("Compression successful"));
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        m_pMainLayout->setCurrentIndex(5);
        break;
    case PAGE_ZIP_FAIL:     // 压缩失败界面
        titlebar()->setTitle("");
        m_pCompressFail->setFailStr(tr("Compression failed"));
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        m_pMainLayout->setCurrentIndex(6);
        break;
    case PAGE_UNZIP_SUCCESS:    // 解压成功界面
        if (m_pFileWatcher) {
            m_pFileWatcher->stopWatcher();
            SAFE_DELETE_ELE(m_pFileWatcher);
        }
        titlebar()->setTitle("");
        m_pCompressSuccess->setCompressPath(m_strDecompressFilePath);
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        unzipSuccessOpenFileDir();

        m_pMainLayout->setCurrentIndex(5);
        break;
    case PAGE_UNZIP_FAIL:       // 解压失败界面
        titlebar()->setTitle("");
        m_pCompressFail->setFailStr(tr("Extraction failed"));
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        m_pMainLayout->setCurrentIndex(6);
        break;
    case PAGE_ENCRYPTION:       // 解压输入密码界面
        titlebar()->setTitle(m_strDecompressFileName);
        m_pOpenAction->setEnabled(false);
        setAcceptDrops(false);
        setTitleButtonStyle(false);
        if (m_pProgressdialog->isshown()) {
            // m_pProgressdialog->reject();
            m_pProgressdialog->hide();
            m_pProgressdialog->m_extractdialog->reject();
        }
        //        m_pProgressdialog->reject();
        //        m_pProgressdialog->m_extractdialog->reject();
        m_pMainLayout->setCurrentIndex(7);
        m_pEncryptionpage->setPassowrdFocus();
        break;
    case PAGE_LOADING:      // 加载界面
        m_pMainLayout->setCurrentIndex(8);
        m_pOpenLoadingPage->start();
        break;
    default:
        break;
    }
}

//add calculate size of selected files
void MainWindow::calSelectedTotalFileSize(const QStringList &files)
{
    foreach (QString file, files) {
        QFileInfo fi(file);

        if (fi.isFile()) {  // 如果为文件，直接获取大小
            qint64 curFileSize = fi.size();

#ifdef __aarch64__
            if (maxFileSize_ < curFileSize) {
                maxFileSize_ = curFileSize;
            }
#endif
            m_pProgess->pInfo()->getTotalSize() += curFileSize;
        } else if (fi.isDir()) {    // 如果是文件夹，递归获取所有子文件大小总和
            QtConcurrent::run(this, &MainWindow::calFileSizeByThread, file);
            // m_pProgess->pInfo()->getTotalSize() += calFileSize(file);
        }
    }

    // 设置压缩总大小
    m_pCompressSetting->setSelectedFileSize(m_pProgess->pInfo()->getTotalSize());
}

void MainWindow::calSelectedTotalEntrySize(QVector<Archive::Entry *> &vectorDel)
{
    qint64 size = 0;

    // 计算每一个entry的大小
    foreach (Archive::Entry *entry, vectorDel) {
        entry->calAllSize(size);
    }
//    m_ProgressIns += size;
    m_pProgess->pInfo()->getTotalSize() += size;
}

void MainWindow::calFileSizeByThread(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return;
    // 获得文件夹中的文件列表
    QFileInfoList list = dir.entryInfoList();
    int i = 0;
    do {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
            i++;
            continue;
        }
        bool bisDir = fileInfo.isDir();
        if (bisDir) {
            // 如果是文件夹 则将此文件夹放入线程池中进行计算
            QtConcurrent::run(this, &MainWindow::calFileSizeByThread, fileInfo.filePath());
        } else {
            mutex.lock();
            // 如果是文件则直接计算大小
            qint64 curFileSize = fileInfo.size();
#ifdef __aarch64__
            if (maxFileSize_ < curFileSize) {
                maxFileSize_ = curFileSize;
            }
#endif
            m_pProgess->pInfo()->getTotalSize() += curFileSize;
            mutex.unlock();
        }
        i++;
    } while (i < list.size());
}

qint64 MainWindow::calFileSize(const QString &path)
{
    QDir dir(path);
    qint64 size = 0;

    // 获取此目录下所有文件和文件夹，如果为0,直接返回大小
    if (dir.entryInfoList().length() == 0) {
        QFileInfo file(path);
        return file.size();
    }

    // 获取此目录下所有文件，计算大小之和
    foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::Files)) {
        qint64 curFileSize = fileInfo.size();

#ifdef __aarch64__
        if (maxFileSize_ < curFileSize) {
            maxFileSize_ = curFileSize;
        }
#endif

        size += curFileSize;
    }

    // 获取此目录下所有的目录，递归计算文件大小
    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        size += calFileSize(path + QDir::separator() + subDir);
    }

    return size;
}

//void MainWindow::calSpeedAndTime(unsigned long compressPercent)
//{
//    m_pProgess->refreshSpeedAndTime(compressPercent);       // 刷新速度和剩余时间
//}

void MainWindow::onSelected(const QStringList &listSelFiles)
{
    m_pUnCompressPage->setWidth(this->width());    // 根据Maindow宽度设置压缩界面的宽度
    //calSelectedTotalFileSize(files);

    if (listSelFiles.count() == 0) {
        return;
    }

    QString strFileName = listSelFiles.at(0);

    // 首先判断是否为单个压缩文件
    if (listSelFiles.count() == 1 && Utils::isCompressed_file(strFileName)) {

        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);     // 设置进度类型为解压进度

        // 判断压缩界面是否已存在文件，若没有，则添加此文件
        if (0 == m_pCompressPage->getCompressFilelist().count()) {

            if (this->m_pArchiveModel != nullptr) {
                Archive::Entry *pRootEntry = m_pArchiveModel->getRootEntry();

                // 清空根结点数据
                if (pRootEntry) {
                    pRootEntry->clean();
                }

                this->m_pArchiveModel->resetmparent();      // 重置父节点
            }

            transSplitFileName(strFileName);    // 对7z分卷进行额外处理

            QFileInfo fileinfo(strFileName);
            m_strDecompressFileName = fileinfo.fileName();     // 压缩包名称
            m_pUnCompressPage->SetDefaultFile(fileinfo);     // 设置解压文件信息

            // 设置子压缩包解压路径为当前压缩包路径
            if (m_pChildMndExtractPath == nullptr) {
                m_pChildMndExtractPath = new QString(fileinfo.path());
            }

            // 根据设置选项设置默认解压路径
            if ("" != m_pSettingsDialog->getCurExtractPath() && m_pUnCompressPage->getExtractType() != EXTRACT_HEAR) {
                m_pUnCompressPage->setdefaultpath(m_pSettingsDialog->getCurExtractPath());
            } else {
                m_pUnCompressPage->setdefaultpath(*m_pChildMndExtractPath);
            }

            m_pUnCompressPage->getFileViewer()->setRootPathIndex();  //added by hsw 20200612 重置m_pathindex
            m_ePageID = PAGE_LOADING;
            loadArchive(strFileName);
        } else {        // 若已存在文件，则提示是添加压缩还是打开新的归档管理器进行解压查看
            DDialog *dialog = new DDialog(this);
            dialog->setFixedWidth(440);
            QIcon icon = QIcon::fromTheme("deepin-compressor");
            dialog->setIcon(icon /*, QSize(32, 32)*/);
            dialog->setMessage(tr("Do you want to add the archive to the list or open it in new window?"));
            dialog->addButton(tr("Cancel"));
            dialog->addButton(tr("Add"));
            dialog->addButton(tr("Open in new window"), true, DDialog::ButtonRecommend);
            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
            effect->setOffset(0, 4);
            effect->setColor(QColor(0, 145, 255, 76));
            effect->setBlurRadius(4);
            dialog->getButton(2)->setGraphicsEffect(effect);

            const int mode = dialog->exec();
            SAFE_DELETE_ELE(dialog);
            qDebug() << mode;
            if (1 == mode) {
                emit sigZipSelectedFiles(listSelFiles);
            } else if (2 == mode) {

                QStringList arguments;
                arguments << strFileName;
                qDebug() << arguments;
                startCmd("deepin-compressor", arguments);
            }
        }
    } else {        // 如果是多个压缩包或者文件之类的，直接进入压缩界面
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING);
        m_ePageID = PAGE_ZIP;
        emit sigZipSelectedFiles(listSelFiles);
        refreshPage();
    }
}

void MainWindow::onRightMenuSelected(const QStringList &files)
{
//    qDebug() << "onRightMenuSelected";
//    if (!m_initflag) {
//        qDebug() << "onRightMenuSelected******************InitUI";
//        InitUI();
//        InitConnection();
//        m_initflag = true;
//    }

    // 初始化子窗口默认解压路径为当前压缩包所在位置
    if (m_pChildMndExtractPath == nullptr && files.count() > 0) {
        m_pChildMndExtractPath = new QString(QFileInfo(files[0]).path());
    }

    m_pUnCompressPage->setWidth(this->width());
//    calSelectedTotalFileSize(files);
//    QString info = "";
//    for (int i = 0; i < files.length(); i++) {
//        info += files[i];
//    }
//    QMessageBox::information(nullptr, "Title", info,
//                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (files.last() == QStringLiteral("extract_here")) { //解压到当前文件夹
        m_bIsRightMenu = true;
        QFileInfo fileinfo(files.at(0));
        m_strDecompressFileName = fileinfo.fileName();
        m_pUnCompressPage->SetDefaultFile(fileinfo);
        m_pUnCompressPage->setdefaultpath(fileinfo.path());
        loadArchive(files.at(0));
        m_ePageID = PAGE_UNZIPPROGRESS;
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
        refreshPage();
    } else if (files.last() == QStringLiteral("extract_here_multi")) {
        QStringList pathlist = files;
        pathlist.removeLast();

        QStringList pathList;
        for (int i = 0; i < pathlist.size(); i++) {
            QString str = pathlist.at(i);
            //对rar分卷名单独处理
            if (pathlist.at(i).endsWith(".rar") && pathlist.at(i).contains(".part")) {
                int x = pathlist.at(i).lastIndexOf("part");
                int y = pathlist.at(i).lastIndexOf(".");

                if ((y - x) > 5) {
                    str.replace(x, y - x, "part01");
                } else {
                    str.replace(x, y - x, "part1");
                }

                QFileInfo file(str);
                if (file.exists()) {
                    pathList.append(str);
                }
            } else {
                pathList.append(str);
            }
        }

        QFileInfo fileinfo(pathList.at(0));
        m_strDecompressFileName = fileinfo.fileName();
        m_pUnCompressPage->SetDefaultFile(fileinfo);
        m_pUnCompressPage->setdefaultpath(fileinfo.path());
        m_strDecompressFilePath = fileinfo.path();

        QStringList transFiles;
        for (const QString &url : qAsConst(pathList)) {
            QString transFile = url;
            transSplitFileName(transFile);
            if (!transFiles.contains(transFile)) { //为了不重复解压
                transFiles.append(transFile);
            }
        }

        if (transFiles.count() == 1) {
            m_bIsRightMenu = true;
            loadArchive(transFiles.at(0));
        } else {
            calSelectedTotalFileSize(files);
            m_eWorkStatus = WorkProcess;
            m_operationtype = Operation_Extract;

            BatchExtract *batchJob = new BatchExtract();
            batchJob->setAutoSubfolder(true);
            batchJob->setDestinationFolder(fileinfo.path());
            batchJob->setPreservePaths(true);

            for (const QString &url : qAsConst(transFiles)) {
                batchJob->addInput(QUrl::fromLocalFile(url));
            }

            connect(batchJob, &BatchExtract::signalUserQuery, [ = ](Query * query) {
                qDebug() << "query->execute";
                query->setParent(this);
                query->execute();
            });

            connect(batchJob, SIGNAL(batchProgress(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
            connect(batchJob, &KJob::result, this, &MainWindow::slotExtractionDone);
            connect(batchJob, &BatchExtract::sendCurFile, this, &MainWindow::slotBatchExtractFileChanged);
            connect(batchJob, &BatchExtract::sendFailFile, this, &MainWindow::slotBatchExtractError);
            //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
            //                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
            //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
            //                m_pEncryptionpage, &EncryptionPage::wrongPassWordSlot);
            connect(batchJob,
                    SIGNAL(batchFilenameProgress(KJob *, const QString &)),
                    this,
                    SLOT(SlotProgressFile(KJob *, const QString &)));
            //        connect(batchJob, &BatchExtract::sigCancelled,
            //                this, &MainWindow::sigquitApp);

            qDebug() << "Starting job";
            batchJob->start();
        }

        m_ePageID = PAGE_UNZIPPROGRESS;
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
        refreshPage();

    } else if (files.last() == QStringLiteral("extract")) {
        QFileInfo fileinfo(files.at(0));
        m_strDecompressFileName = fileinfo.fileName();
        m_pUnCompressPage->SetDefaultFile(fileinfo);
        if ("" != m_pSettingsDialog->getCurExtractPath() && m_pUnCompressPage->getExtractType() != EXTRACT_HEAR) {
            m_pUnCompressPage->setdefaultpath(m_pSettingsDialog->getCurExtractPath());
        } else {
            m_pUnCompressPage->setdefaultpath(fileinfo.path());
        }

        loadArchive(files.at(0));
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
    } else if (files.last() == QStringLiteral("extract_multi")) {
        calSelectedTotalFileSize(files);
        m_eWorkStatus = WorkProcess;
        m_operationtype = Operation_Extract;
        QString defaultpath;
        QFileInfo fileinfo(files.at(0));
        if ("" != m_pSettingsDialog->getCurExtractPath() && m_pUnCompressPage->getExtractType() != EXTRACT_HEAR) {
            defaultpath = m_pSettingsDialog->getCurExtractPath();
        } else {
            defaultpath = fileinfo.path();
        }

        DFileDialog dialog;
        dialog.setAcceptMode(DFileDialog::AcceptOpen);
        dialog.setFileMode(DFileDialog::Directory);
        dialog.setWindowTitle(tr("Find directory"));
        dialog.setDirectory(defaultpath);

        const int mode = dialog.exec();

        if (mode != QDialog::Accepted) {
            QTimer::singleShot(100, this, [=] {
                slotquitApp();
            });
            return;
        }

        QList< QUrl > selectpath = dialog.selectedUrls();
        qDebug() << selectpath;
        QString curpath = selectpath.at(0).toLocalFile();

        QStringList pathlist = files;
        pathlist.removeLast();

        QStringList pathList;
        for (int i = 0; i < pathlist.size(); i++) {
            QString str = pathlist.at(i);
            if (pathlist.at(i).endsWith(".rar") && pathlist.at(i).contains(".part")) {
                int x = pathlist.at(i).lastIndexOf("part");
                int y = pathlist.at(i).lastIndexOf(".");

                if ((y - x) > 5) {
                    str.replace(x, y - x, "part01");
                } else {
                    str.replace(x, y - x, "part1");
                }

                QFileInfo file(str);
                if (file.exists()) {
                    pathList.append(str);
                }
            } else {
                pathList.append(str);
            }
        }

        m_pUnCompressPage->SetDefaultFile(fileinfo);
        m_strDecompressFileName = fileinfo.fileName();
        m_pUnCompressPage->setdefaultpath(curpath);
        m_strDecompressFilePath = curpath;
        m_ePageID = PAGE_UNZIPPROGRESS;
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
        refreshPage();

        BatchExtract *batchJob = new BatchExtract();
        batchJob->setAutoSubfolder(true);
        batchJob->setDestinationFolder(curpath);
        batchJob->setPreservePaths(true);

        QStringList transFiles;
        for (const QString &url : qAsConst(pathList)) {
            QString transFile = url;
            transSplitFileName(transFile);
            if (!transFiles.contains(transFile)) { //为了不重复解压
                transFiles.append(transFile);
                batchJob->addInput(QUrl::fromLocalFile(transFile));
            }
        }

        connect(batchJob, &BatchExtract::signalUserQuery, [ = ](Query * query) {
            qDebug() << "query->execute";
            query->setParent(this);
            query->execute();
        });

        connect(batchJob, SIGNAL(batchProgress(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
        connect(batchJob, &KJob::result, this, &MainWindow::slotExtractionDone);
        connect(batchJob, &BatchExtract::sendCurFile, this, &MainWindow::slotBatchExtractFileChanged);
        connect(batchJob, &BatchExtract::sendFailFile, this, &MainWindow::slotBatchExtractError);
        //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
        //                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
        //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
        //                m_pEncryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(batchJob,
                SIGNAL(batchFilenameProgress(KJob *, const QString &)),
                this,
                SLOT(SlotProgressFile(KJob *, const QString &)));
        //        connect(batchJob, &BatchExtract::sigCancelled,
        //                this, &MainWindow::sigquitApp);

        qDebug() << "Starting job";
        batchJob->start();
    } else if (files.last() == QStringLiteral("compress")) {
        QStringList pathlist = files;
        pathlist.removeLast();
        emit sigZipSelectedFiles(pathlist);
        m_ePageID = PAGE_ZIPSET;
        setCompressDefaultPath();
        refreshPage();
    } else if (files.last() == QStringLiteral("extract_here_split")) {
        if (files.at(0).contains(".7z.")) {
            QString filepath = files.at(0);

            transSplitFileName(filepath);

            QFileInfo fileinfo(filepath);

            if (fileinfo.exists()) {
                m_bIsRightMenu = true;
                QFileInfo fileinfo(files.at(0));
                m_strDecompressFileName = fileinfo.fileName();
                m_pUnCompressPage->SetDefaultFile(fileinfo);
                m_pUnCompressPage->setdefaultpath(fileinfo.path());
                loadArchive(filepath);
                m_ePageID = PAGE_UNZIPPROGRESS;
                m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
                refreshPage();
            } else {
                m_pCompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
                m_ePageID = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        }
    } else if (files.last() == QStringLiteral("extract_split")) {
        QString filepath = files.at(0);
        filepath = filepath.left(filepath.length() - 3) + "001";
        QFileInfo fileinfo(filepath);

        if (fileinfo.exists()) {
            m_strDecompressFileName = fileinfo.fileName();
            m_pUnCompressPage->SetDefaultFile(fileinfo);
            if ("" != m_pSettingsDialog->getCurExtractPath() && m_pUnCompressPage->getExtractType() != EXTRACT_HEAR) {
                m_pUnCompressPage->setdefaultpath(m_pSettingsDialog->getCurExtractPath());
            } else {
                m_pUnCompressPage->setdefaultpath(fileinfo.path());
            }

            loadArchive(filepath);
        } else {
            m_pCompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
            m_ePageID = PAGE_UNZIP_FAIL;
            refreshPage();
        }
    } else if (files.count() == 1 && Utils::isCompressed_file(files.at(0))) {
        QString filename;
        filename = files.at(0);

        //        if (filename.contains(".7z.")) {
        //            filename = filename.left(filename.length() - 3) + "001";
        //        }

        transSplitFileName(filename);

        QFileInfo fileinfo(filename);
        m_strDecompressFileName = fileinfo.fileName();
        m_pUnCompressPage->SetDefaultFile(fileinfo);
        if ("" != m_pSettingsDialog->getCurExtractPath() && m_pUnCompressPage->getExtractType() != EXTRACT_HEAR) {
            m_pUnCompressPage->setdefaultpath(m_pSettingsDialog->getCurExtractPath());
        } else {
//            m_pUnCompressPage->setdefaultpath(fileinfo.path());
//            m_pUnCompressPage->setdefaultpath("/home/hushiwei/Documents");//只需要在这里把路径设置为第一级窗口的解压路径，而不是临时路径。
//            if (m_pChildMndExtractPath == nullptr) {
//                m_pChildMndExtractPath = new QString(fileinfo.path());
//            }
            m_pUnCompressPage->setdefaultpath(*m_pChildMndExtractPath);
        }
        m_ePageID = Page_ID::PAGE_LOADING;
        loadArchive(filename);
    } else {
        emit sigZipSelectedFiles(files);
        m_ePageID = PAGE_ZIPSET;
        setCompressDefaultPath();
        refreshPage();
    }

    show();
}

void MainWindow::slotLoadingFinished(KJob *job)
{
    //m_pHomePage->spinnerStop();
    m_eWorkStatus = WorkNone;

    if (job->error()) {
        int errorCode = job->error();
        if (errorCode == KJob::OpenFailedError) {
            if (job->mType == KJob::ENUM_JOBTYPE::LOADJOB) {
                LoadJob *pLoadJob = dynamic_cast<LoadJob *>(job);
                ReadOnlyArchiveInterface *pFace = pLoadJob->archiveInterface();
                QString fileName = pFace->filename();
                QString tipError = tr("Failed to open archive: %1").arg(fileName);
                m_pCompressFail->setFailStrDetail(tipError);
                m_ePageID = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        } else {
            m_pCompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
            m_ePageID = PAGE_UNZIP_FAIL;
            refreshPage();
        }

        deleteLaterJob();
        return;
    }

    m_pArchiveFilterModel->setSourceModel(m_pArchiveModel);
    m_pArchiveFilterModel->setFilterKeyColumn(0);
    m_pArchiveFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pUnCompressPage->setModel(m_pArchiveFilterModel);

    if (!m_bIsRightMenu) {
        m_ePageID = PAGE_UNZIP;
        refreshPage();
        deleteLaterJob();
    } else {
        slotextractSelectedFilesTo(m_pUnCompressPage->getDecompressPath());
    }
}

bool MainWindow::isWorkProcess()
{
    return m_eWorkStatus == WorkProcess;
}

void MainWindow::loadArchive(const QString &files)
{
    QString transFile = files;
    transSplitFileName(transFile);

    WatcherFile(transFile);

    m_eWorkStatus = WorkProcess;
    m_strLoadfile = transFile;
    m_pUnCompressPage->getFileViewer()->setLoadFilePath(m_strLoadfile);
    m_operationtype = Operation_Load;
    m_ePageID = PAGE_LOADING;
    m_pJob = m_pArchiveModel->loadArchive(transFile, "", m_pArchiveModel);
    if (m_pJob == nullptr) {
        return;
    }
    LoadJob *pLoadJob = dynamic_cast<LoadJob *>(m_pJob);
    connect(pLoadJob, &LoadJob::sigLodJobPassword, this, &MainWindow::SlotNeedPassword);
    connect(pLoadJob, &LoadJob::sigWrongPassword, this, &MainWindow::SlotNeedPassword);

    m_pJob->start();
    //m_pHomePage->spinnerStart(this, static_cast<pMember_callback>(&MainWindow::isWorkProcess));
    refreshPage();
}

void MainWindow::WatcherFile(const QString &files)
{
    SAFE_DELETE_ELE(m_pFileWatcher);

    m_pFileWatcher = new DFileWatcher(files, this);
    m_pFileWatcher->startWatcher();
    qDebug() << m_pFileWatcher->startWatcher() << "=" << files;
    m_pUnCompressPage->setRootPathIndex(); //解决解压后再次打开压缩包出现返回上一级
    connect(m_pFileWatcher, &DFileWatcher::fileMoved, this, [ = ]() { //监控压缩包，重命名时提示
        DDialog *dialog = new DDialog(this);
        dialog->setFixedWidth(440);
        QIcon icon = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
        dialog->setIcon(icon);
        dialog->setMessage(tr("The archive was changed on the disk, please import it again."));
        dialog->addButton(tr("OK"), true, DDialog::ButtonNormal);
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
        effect->setOffset(0, 4);
        effect->setColor(QColor(0, 145, 255, 76));
        effect->setBlurRadius(4);
        dialog->getButton(0)->setFixedWidth(340);
        //        dialog->getButton(0)->setGraphicsEffect(effect);
        dialog->exec();
        SAFE_DELETE_ELE(dialog);

        SAFE_DELETE_ELE(m_pFileWatcher);

        m_ePageID = PAGE_HOME;
        m_pUnCompressPage->setRootPathIndex();
        this->refreshPage();
    });

}

void MainWindow::slotextractSelectedFilesTo(const QString &localPath)
{
//    m_ePageID = PAGE_UNZIPPROGRESS;
//    refreshPage();
//    m_pProgressdialog->setProcess(0);
//    m_pProgess->setprogress(0);
    m_pProgess->pInfo()->setTotalSize(0); //初始化大小
    calSelectedTotalFileSize(QStringList() << m_strLoadfile); //计算压缩包大小供解压进度使用
    qDebug() << QString("decompressedfile size: %1B").arg(m_pProgess->pInfo()->getTotalSize());
    m_pProgressdialog->setProcess(0);

    m_eWorkStatus = WorkProcess;
    m_operationtype = Operation_Extract;
    if (nullptr == m_pArchiveModel) {
        return;
    }

    if (nullptr == m_pArchiveModel->archive()) {
        return;
    }

    deleteLaterJob();

    ExtractionOptions options;
    QVector< Archive::Entry * > files;

    QString userDestination = localPath;
    QString destinationDirectory;

    m_strPathStore = userDestination;
    //m_compressDirFiles = CheckAllFiles(m_strPathStore);

    if (pSettingInfo == nullptr) {
        pSettingInfo = new Settings_Extract_Info();
    }
    pSettingInfo->b_isAutoCreateDir = m_pSettingsDialog->isAutoCreatDir();
    options.setAutoCreatDir(pSettingInfo->b_isAutoCreateDir);

    options.pSettingInfo = pSettingInfo;
    pSettingInfo->str_defaultPath = userDestination;

    QString detectedSubfolder = "";
    if (pSettingInfo->b_isAutoCreateDir) {                   //自动创建文件夹

        if (m_pArchiveModel->archive()->hasMultipleTopLevelEntries()) { //如果是顶级多个目录，则创建文件夹
            detectedSubfolder = m_pArchiveModel->archive()->subfolderName();

            pSettingInfo->str_CreateFolder = detectedSubfolder;
            if (!userDestination.endsWith(QDir::separator())) {
                userDestination.append(QDir::separator());
            }
            destinationDirectory = userDestination + detectedSubfolder;
            QDir(userDestination).mkdir(detectedSubfolder);

            m_pCompressSuccess->setCompressNewFullPath(destinationDirectory);
        } else {                        //如果是顶级单个目录，则不创建文件夹
            destinationDirectory = userDestination;
            auto rootEntry = this->m_pArchiveModel->getRootEntry();
            if (rootEntry->entries().length() > 0) {
                pSettingInfo->str_CreateFolder = rootEntry->entries().at(0)->name();
            } else {
                pSettingInfo->str_CreateFolder = detectedSubfolder;
            }

        }
    } else {
        destinationDirectory = userDestination;
        auto rootEntry = this->m_pArchiveModel->getRootEntry();
        if (rootEntry->entries().length() == 1) {
            pSettingInfo->str_CreateFolder = rootEntry->entries().at(0)->name();
        } else {
            pSettingInfo->str_CreateFolder = detectedSubfolder;
        }
    }

    qDebug() << "destinationDirectory:" << destinationDirectory;

    m_pJob = m_pArchiveModel->extractFiles(files, destinationDirectory, options);
    if (m_pJob == nullptr || m_pJob->mType != Job::ENUM_JOBTYPE::EXTRACTJOB) {
        qDebug() << "ExtractJob new failed.";
        return;
    }

    ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
    pExtractJob->archiveInterface()->extractTopFolderName = m_pArchiveModel->archive()->subfolderName();
    connect(pExtractJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
    connect(pExtractJob, &KJob::result, this, &MainWindow::slotExtractionDone);
    connect(pExtractJob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(pExtractJob, &ExtractJob::sigExtractJobPassword, m_pEncryptionpage, &EncryptionPage::wrongPassWordSlot);

    connect(pExtractJob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
    connect(pExtractJob,
            SIGNAL(percentfilename(KJob *, const QString &)),
            this,
            SLOT(SlotProgressFile(KJob *, const QString &)));
    connect(pExtractJob, &ExtractJob::sigCancelled, this, &MainWindow::slotClearTempfile);
    connect(pExtractJob, &ExtractJob::updateDestFile, this, &MainWindow::onUpdateDestFile);

    m_strDecompressFilePath = destinationDirectory;

    /*if(m_pArchiveModel->archive()->property("isPasswordProtected").toBool() == true)
    {
        if (PAGE_ENCRYPTION != m_ePageID)
        {
            m_ePageID = PAGE_ENCRYPTION;
            refreshPage();
        }
        return;
    }*/
    m_pProgess->pInfo()->startTimer();
    pExtractJob->archiveInterface()->destDirName = "";
    m_pJob->start();
}

void MainWindow::SlotProgress(KJob * /*job*/, unsigned long percent)
{

    //calSpeedAndTime(percent);
    m_pProgess->refreshSpeedAndTime(percent);       // 刷新速度和剩余时间

    if (Operation_SingleExtract == m_operationtype || Operation_DRAG == m_operationtype) {
        if (percent < 100 && WorkProcess == m_eWorkStatus) {
            if (!m_pProgressdialog->isshown()) {
                if (m_ePageID != PAGE_UNZIP) {
                    m_ePageID = PAGE_UNZIP;
                    refreshPage();
                }
                m_pProgressdialog->showdialog();
            }
            m_pProgressdialog->setProcess(percent);
        }
        return;
    } /*else if (Operation_TempExtract_Open_Choose == m_operationtype || Operation_TempExtract == m_operationtype) {
        m_ePageID = PAGE_LOADING;
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
        refreshPage();
    }*/

    // 该函数尽量使用m_pageid来判断刷新到哪种界面，尽量不要根据m_operationtype去判断。
    if (m_ePageID == PAGE_LOADING) {
        refreshPage();
    } else if (PAGE_ZIPPROGRESS == m_ePageID || PAGE_UNZIPPROGRESS == m_ePageID || PAGE_DELETEPROGRESS == m_ePageID) {
        m_pProgess->setprogress(percent);
    } else if ((PAGE_UNZIP == m_ePageID || PAGE_ENCRYPTION == m_ePageID) && (percent < 100) && m_pJob) {
        m_ePageID = PAGE_UNZIPPROGRESS;
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
        refreshPage();
    } else if ((PAGE_ZIPSET == m_ePageID) && (percent < 100)) {
        m_ePageID = PAGE_ZIPPROGRESS;
        m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING);
        refreshPage();
    }
}

void MainWindow::SlotProgressFile(KJob * /*job*/, const QString &filename)
{
    m_pProgressdialog->setCurrentFile(filename);
    m_pProgess->setProgressFilename(filename);
}

void MainWindow::slotBatchExtractFileChanged(const QString &name)
{
    qDebug() << name;
    m_pProgess->setFilename(name);
}

void MainWindow::slotBatchExtractError(const QString &name)
{
    m_pCompressFail->setFailStrDetail(name + ":" + +" " + tr("Wrong password"));
    m_ePageID = PAGE_UNZIP_FAIL;
    refreshPage();
}

void MainWindow::removeFromParentInfo(MainWindow *CurMainWnd)
{
    if (CurMainWnd->m_pCurAuxInfo != nullptr) {
        MainWindow_AuxInfo *parentInfo = CurMainWnd->m_pCurAuxInfo->parentAuxInfo;
        QString strWId = QString::number(CurMainWnd->winId());
        if (parentInfo) {
            QMap<QString, OpenInfo *>::iterator iter;
            QString key;
            for (iter = parentInfo->information.begin(); iter != parentInfo->information.end();) {
                //先存key
                key = iter.key();
                //指针移至下一个位置
                iter++;
                if (parentInfo->information[key]->strWinId == strWId) {
                    //删除当前位置数据
                    OpenInfo *p = parentInfo->information.take(key);
                    SAFE_DELETE_ELE(p);
                }
            }
        }

    }
}

void MainWindow::slotExtractionDone(KJob *job)
{
    m_eWorkStatus = WorkNone;
//    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_NONE);
    Archive::Entry *pExtractWorkEntry = nullptr;
    if (m_pJob && m_pJob->mType == Job::ENUM_JOBTYPE::EXTRACTJOB) {
        ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
        pExtractWorkEntry = pExtractJob->getWorkEntry();

        if (this->m_pWatcher != nullptr) {
            this->m_pWatcher->finishWork();
            disconnect(this->m_pWatcher, &TimerWatcher::sigBindFuncDone, pExtractJob, &ExtractJob::slotWorkTimeOut);
            SAFE_DELETE_ELE(m_pWatcher);
        }

        int errcode = this->m_pJob->error();

        deleteLaterJob();

        if (errcode == KJob::NopasswordError) { //如果需要输入密码
            m_ePageID = PAGE_ENCRYPTION;
            refreshPage();
            return;
        }

        if (errcode == 0 && m_operationtype != Operation_SingleExtract && m_operationtype != Operation_DRAG
                && m_operationtype != Operation_TempExtract_Open_Choose) {
            if (this->m_pCurAuxInfo == nullptr || this->m_pCurAuxInfo->information.size() == 0) {
                m_ePageID = PAGE_UNZIP_SUCCESS;
                m_pCompressSuccess->setstringinfo(tr("Extraction successful"));
                refreshPage();
                return;
            }
        }
    }

    int errorCode = job->error();

    if (m_ePageID == PAGE_LOADING) {
        m_pOpenLoadingPage->stop();
    }

    if (m_ePageID == PAGE_UNZIP  && m_operationtype != Operation_TempExtract_Open_Choose) { // 如果是解压界面，则返回
        if (m_pProgressdialog->isshown()) {
            m_pProgressdialog->hide();
            // m_pProgressdialog->reject();
        }

        if (m_operationtype == Operation_SingleExtract || m_operationtype == Operation_DRAG) {
            if (errorCode == KJob::UserSkiped) {
                m_pCompressSuccess->setstringinfo(tr("Skip all files"));
            } else {
                m_pProgressdialog->setFinished(m_strDecompressFilePath);
            }
        }

        return;
    } else if ((PAGE_ENCRYPTION == m_ePageID) && (errorCode && (errorCode != KJob::KilledJobError && errorCode != KJob::UserSkiped)))   {
        // do noting:wrong password
    } else if (errorCode && (errorCode != KJob::KilledJobError && errorCode != KJob::UserSkiped && errorCode != KJob::CancelError)) {
        if (m_pProgressdialog->isshown()) {
            m_pProgressdialog->hide();
            // m_pProgressdialog->reject();
        }

        if (m_strPathStore.left(6) == "/media" && getMediaFreeSpace() <= 50) {
            m_pCompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
        } else if (getDiskFreeSpace() <= 50) {
            m_pCompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
        } else {
            m_pCompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
        }
        if (KJob::UserFilenameLong == errorCode) {
            m_pCompressFail->setFailStrDetail(tr("Filename is too long, unable to extract"));
        } else if (KJob::OpenFailedError == errorCode) {
            m_pCompressFail->setFailStrDetail(tr("Failed to open archive: %1"));
        } else if (KJob::WrongPsdError == errorCode) {
            m_pCompressFail->setFailStrDetail(tr("Wrong password") + "," + tr("unable to extract"));
        }

        m_ePageID = PAGE_UNZIP_FAIL;
//        if (KJob::NopasswordError == errorCode) {
//            m_ePageID = PAGE_ENCRYPTION;
//        }
        refreshPage();
        return;
    } else if (Operation_TempExtract == m_operationtype) {
        m_pOpenLoadingPage->stop();

        QStringList arguments;
        QString programName = "xdg-open";
        /*for (int i = 0; i < m_vecExtractSimpleFiles.count(); i++)*/
//        {
        QString firstFileName = m_vecExtractSimpleFiles.at(0)->name();
        bool isCompressedFile = Utils::isCompressed_file(firstFileName);
        if (isCompressedFile == true) {
            programName = "deepin-compressor";
        }

        QFileInfo file(firstFileName);
        if (file.fileName().contains("%")/* && file.fileName().contains(".png")*/) {

            QProcess p;
            QString tempFileName = QString("%1").arg(m_iOpenTempFileLink) + "." + file.suffix();
            m_iOpenTempFileLink++;
            QString commandCreate = "ln";
            QStringList args;
            args.append(m_strPathStore + PATH_SEP + firstFileName);
            args.append(m_strPathStore + PATH_SEP + tempFileName);
            arguments << m_strPathStore + PATH_SEP + tempFileName;   //the first arg is filePath
            p.execute(commandCreate, args);
        } else {
            QString destPath = m_strPathStore + PATH_SEP + firstFileName;
            if (pExtractWorkEntry != nullptr) {
                this->m_pArchiveModel->mapFilesUpdate.insert(destPath, pExtractWorkEntry);
            }

            arguments << destPath;  //the first arg is filePath
            if (isCompressedFile == true) {
                if (m_pMapGlobalWnd == nullptr) {
                    m_pMapGlobalWnd = new GlobalMainWindowMap();
                }
                m_pMapGlobalWnd->insert(QString::number(this->winId()), this);
                arguments << HEADBUS + QString::number(this->winId());//the second arg
                if (pExtractWorkEntry == nullptr) {
                    return;
                }
                QModelIndex index = this->m_pArchiveModel->indexForEntry(pExtractWorkEntry);
                QString strIndex = modelIndexToStr(index);
                arguments << strIndex;  //the third arg
            }
        }

        qDebug() << arguments;
        startCmd(programName, arguments);
//        KProcess *cmdprocess = new KProcess;
//        cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
//        cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
//        cmdprocess->setProgram(programPath, arguments);
//        cmdprocess->start();
        m_ePageID = PAGE_UNZIP;
        refreshPage();
    } else if (Operation_SingleExtract == m_operationtype || m_operationtype == Operation_DRAG) {
        if (errorCode == KJob::UserSkiped) {
            m_bIsRightMenu = false;
            m_pProgressdialog->setMsg(tr("Skip all files"));
        } else {
            m_pProgressdialog->setFinished(m_strDecompressFilePath);
        }

        m_ePageID = PAGE_UNZIP;
        refreshPage();

        if (m_operationtype != Operation_DRAG) {
            QString fullpath = m_strDecompressFilePath + "/" + m_vecExtractSimpleFiles.at(0)->property("name").toString();
            QFileInfo fileinfo(fullpath);
            if (fileinfo.exists()) {
//                DDesktopServices::showFolder(fullpath);
            }
        }
    } else if (Operation_TempExtract_Open_Choose == m_operationtype) {
        m_pOpenLoadingPage->stop();
        QString ppp = m_strProgram;
        if (m_strProgram != tr("Choose default programma")) {
            OpenWithDialog::chooseOpen(m_strProgram, QString(m_strPathStore + PATH_SEP) + m_vecExtractSimpleFiles.at(0)->property("name").toString());
        } else {
            OpenWithDialog *dia = new OpenWithDialog(DUrl(QString(m_strPathStore + PATH_SEP) + m_vecExtractSimpleFiles.at(0)->property("name").toString()), this);
            dia->exec();
        }

        m_ePageID = PAGE_UNZIP;
        refreshPage();
    } else if (Operation_NULL == m_operationtype) {
        qDebug() << "do nothing";
    } else {
        m_ePageID = PAGE_UNZIP_SUCCESS;
        if (errorCode == KJob::UserSkiped) {
            m_bIsRightMenu = false;
            m_pCompressSuccess->setstringinfo(tr("Skip all files"));
        } else {
            m_pCompressSuccess->setstringinfo(tr("Extraction successful"));
        }
        refreshPage();
    }
}

void MainWindow::slotShowPageUnzipProgress()
{
    if (Operation_TempExtract_Open_Choose == m_operationtype || Operation_TempExtract == m_operationtype) {
        m_ePageID = PAGE_LOADING;
        m_pProgess->settype(Progress::OP_DECOMPRESSING);
        refreshPage();
    } else if (m_operationtype != Operation_SingleExtract && m_operationtype != Operation_DRAG) {
        m_ePageID = PAGE_UNZIPPROGRESS;
        refreshPage();
    }
    m_pProgressdialog->setProcess(0);
    m_pProgess->setprogress(0);
}

void MainWindow::SlotNeedPassword()
{
    if (PAGE_ENCRYPTION != m_ePageID) {
        m_ePageID = PAGE_ENCRYPTION;
        refreshPage();
    }
}

void MainWindow::SlotExtractPassword(QString password)
{
    m_pProgressdialog->clearprocess();
    // m_progressTransFlag = false;
    if (Operation_Load == m_operationtype) {
        m_ePageID = PAGE_LOADING;
        refreshPage();
        LoadPassword(password);
    } else if (Operation_Extract == m_operationtype) {
        ExtractPassword(password);
    } else if (Operation_SingleExtract == m_operationtype || Operation_TempExtract == m_operationtype || Operation_TempExtract_Open_Choose == m_operationtype || Operation_DRAG == m_operationtype) {
        ExtractSinglePassword(password);
    }
}

void MainWindow::ExtractSinglePassword(QString password)
{
    m_eWorkStatus = WorkProcess;

    deleteLaterJob();

    if (m_pJob) {
        // first  time to extract
        ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
        pExtractJob->archiveInterface()->setPassword(password);
        pExtractJob->start();
    } else {
        // second or more  time to extract
        ExtractionOptions options;
        options.setDragAndDropEnabled(true);
        m_pJob = m_pArchiveModel->extractFiles(m_vecExtractSimpleFiles, m_strDecompressFilePath, options);
        ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
        pExtractJob->archiveInterface()->setPassword(password);
        connect(pExtractJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
        connect(pExtractJob, &KJob::result, this, &MainWindow::slotExtractionDone);
        //
        connect(pExtractJob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
        connect(pExtractJob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword);
        connect(
            pExtractJob, &ExtractJob::sigExtractJobPassword, m_pEncryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(pExtractJob,
                SIGNAL(percentfilename(KJob *, const QString &)),
                this,
                SLOT(SlotProgressFile(KJob *, const QString &)));
        connect(pExtractJob, &ExtractJob::updateDestFile, this, &MainWindow::onUpdateDestFile);

        m_pJob = pExtractJob;
        m_pJob->start();
    }
}

void MainWindow::ExtractPassword(QString password)
{
    m_eWorkStatus = WorkProcess;

    ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
    if (pExtractJob) {
        // first  time to extract
        pExtractJob->archiveInterface()->setPassword(password);

        pExtractJob->start();
    } else {
        // second or more  time to extract
        ExtractionOptions options;
        options.setAutoCreatDir(m_pSettingsDialog->isAutoCreatDir());
        if (pSettingInfo == nullptr) {
            pSettingInfo = new Settings_Extract_Info();
        }
        options.pSettingInfo = pSettingInfo;

        QVector< Archive::Entry * > files;

        pExtractJob = m_pArchiveModel->extractFiles(files, m_strDecompressFilePath, options);
        pExtractJob->archiveInterface()->setPassword(password);
        connect(pExtractJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
        connect(pExtractJob, &KJob::result, this, &MainWindow::slotExtractionDone);
        //
        connect(pExtractJob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
        connect(pExtractJob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword);
        connect(
            pExtractJob, &ExtractJob::sigExtractJobPassword, m_pEncryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(pExtractJob,
                SIGNAL(percentfilename(KJob *, const QString &)),
                this,
                SLOT(SlotProgressFile(KJob *, const QString &)));
        connect(pExtractJob, &ExtractJob::sigCancelled, this, &MainWindow::slotClearTempfile);
        connect(pExtractJob, &ExtractJob::updateDestFile, this, &MainWindow::onUpdateDestFile);

        m_pJob = pExtractJob;
        m_pJob->start();
    }
}
void MainWindow::LoadPassword(QString password)
{
    m_eWorkStatus = WorkProcess;
    m_operationtype = Operation_Load;
    m_pJob = m_pArchiveModel->loadArchive(m_strLoadfile, "", m_pArchiveModel);
    LoadJob *pLoadJob = dynamic_cast<LoadJob *>(m_pJob);
    connect(pLoadJob, &LoadJob::sigWrongPassword, this, &MainWindow::slotLoadWrongPassWord);
    pLoadJob->archiveInterface()->setPassword(password);
    if (m_pJob) {
        m_pJob->start();
    }
}

void MainWindow::setCompressDefaultPath()
{
    QStringList fileslist = m_pCompressPage->getCompressFilelist();      // 获取需要压缩的文件
    m_pCompressSetting->setFilepath(fileslist);

    // 重置进度、大小等参数
    m_pProgess->resetProgress();
    calSelectedTotalFileSize(fileslist);

    QFileInfo fileinfobase(fileslist.at(0));

    QString savePath = fileinfobase.path();     // 初始化压缩包保存位置为第一个文件所在的位置

    // 若文件处于不同的位置，则压缩包保存位置默认为桌面
    for (int loop = 1; loop < fileslist.count(); loop++) {
        QFileInfo fileinfo(fileslist.at(loop));
        if (fileinfo.path() != fileinfobase.path()) {
            savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            break;
        }
    }

    m_pCompressSetting->setDefaultPath(savePath);    // 设置压缩包保存位置

    if (1 == fileslist.count()) {       // 若是单文件
        if (fileinfobase.isDir()) {
            m_pCompressSetting->setDefaultName(fileinfobase.fileName());         // 如果是文件夹，压缩包名为文件夹
        } else {
            m_pCompressSetting->setDefaultName(fileinfobase.completeBaseName()); // 如果是文件，压缩包名为完整的文件名
        }
    } else {
        m_pCompressSetting->setDefaultName(tr("Create New Archive"));            // 如果是多文件，压缩包名为新建归档文件
    }


}

void MainWindow::onCompressNext()
{
    m_ePageID = PAGE_ZIPSET;
    setCompressDefaultPath();
    refreshPage();
}

void MainWindow::onCompressPressed(QMap< QString, QString > &Args)
{
    m_pProgressdialog->setProcess(0);
    m_pProgess->setprogress(0);
    m_bIsAddArchive = false;

    QStringList filesToAdd = m_pCompressPage->getCompressFilelist();

    if (!filesToAdd.size()) {
        filesToAdd.push_back(Args[QStringLiteral("sourceFilePath")]);
        filesToAdd.push_back(Args[QStringLiteral("ToCompressFilePath")]);
    }

    QSet< QString > globalWorkDirList;
    foreach (QString file, filesToAdd) {
        QString globalWorkDir = file;
        if (globalWorkDir.right(1) == QLatin1String("/")) {
            globalWorkDir.chop(1);
        }
        globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
        globalWorkDirList.insert(globalWorkDir);
    }

    if (globalWorkDirList.count() == 1 || 0 == Args[QStringLiteral("createtar7z")].compare("true")) {
        creatArchive(Args);
    } else if (globalWorkDirList.count() > 1) {
        QMap< QString, QStringList > compressmap;
        foreach (QString workdir, globalWorkDirList) {
            QStringList filelist;
            foreach (QString file, filesToAdd) {
                QString globalWorkDir = file;
                if (globalWorkDir.right(1) == QLatin1String("/")) {
                    globalWorkDir.chop(1);
                }
                globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
                if (workdir == globalWorkDir) {
                    filelist.append(file);
                }
            }
            compressmap.insert(workdir, filelist);
        }

        qDebug() << compressmap;
        creatBatchArchive(Args, compressmap);
    } else {
        qDebug() << "Compress file count error!";
    }
}

void MainWindow::onUncompressStateAutoCompress(QMap<QString, QString> &Args)
{
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD);
    m_pProgressdialog->setProcess(0);
    m_pProgess->setprogress(0);
    m_bIsAddArchive = true;
    qDebug() << "开始添加压缩文件";
    addArchive(Args);
}

void MainWindow::onUncompressStateAutoCompressEntry(QMap<QString, QString> &Args, Archive::Entry *pWorkEntry)
{
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD);
    m_pProgressdialog->setProcess(0);
    m_pProgess->setprogress(0);
    m_bIsAddArchive = true;
    qDebug() << "开始添加压缩文件";
    addArchiveEntry(Args, pWorkEntry);
}

void MainWindow::creatBatchArchive(QMap< QString, QString > &Args, QMap< QString, QStringList > &filetoadd)
{
    m_pJob = new BatchCompress();
    BatchCompress *pBatchCompress = dynamic_cast<BatchCompress *>(m_pJob);
    pBatchCompress->setCompressArgs(Args);

    for (QString &key : filetoadd.keys()) {
        pBatchCompress->addInput(filetoadd.value(key));
    }

    connect(pBatchCompress, SIGNAL(batchProgress(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
    connect(m_pJob, &KJob::result, this, &MainWindow::slotCompressFinished);
    connect(pBatchCompress,
            SIGNAL(batchFilenameProgress(KJob *, const QString &)),
            this,
            SLOT(SlotProgressFile(KJob *, const QString &)));

    qDebug() << "Starting job";
    m_strDecompressFileName = Args[QStringLiteral("filename")];
    m_pCompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);
    m_pCompressSuccess->setCompressFullPath(Args[QStringLiteral("localFilePath")] + QDir::separator()
                                            + Args[QStringLiteral("filename")]);
    m_strPathStore = Args[QStringLiteral("localFilePath")];
    //m_compressDirFiles = CheckAllFiles(m_strPathStore);

    m_ePageID = PAGE_ZIPPROGRESS;
    m_eJobType = JOB_BATCHCOMPRESS;
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING);
    refreshPage();
    m_pJob->start();
}

void MainWindow::addArchiveEntry(QMap<QString, QString> &Args, Archive::Entry *pWorkEntry)
{
    if (!m_bIsAddArchive) {
        return;
    }
    if (!m_pArchiveModel) {
        return;
    }

    QString sourceArchivePath = Args[QStringLiteral("sourceFilePath")];
    QString filesToAddStr = Args[QStringLiteral("ToCompressFilePath")];
    QStringList filesToAdd = filesToAddStr.split("--");

    const QString fixedMimeType = Args[QStringLiteral("fixedMimeType")];
    const QString password = Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = Args[QStringLiteral("encryptHeader")];
    m_strCreateCompressFile = Args[QStringLiteral("localFilePath")] + QDir::separator() + Args[QStringLiteral("filename")];
    m_strDecompressFileName = Args[QStringLiteral("filename")];
    m_pCompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);

    ReadOnlyArchiveInterface *pIface = Archive::createInterface(m_strCreateCompressFile, fixedMimeType);

    if (m_strCreateCompressFile.isEmpty()) {
        qDebug() << "filename.isEmpty()";
        return;
    }

    //renameCompress(m_strCreateCompressFile, fixedMimeType);
    m_strDecompressFileName = QFileInfo(m_strCreateCompressFile).fileName();
    m_pCompressSuccess->setCompressFullPath(m_strCreateCompressFile);
    qDebug() << m_strCreateCompressFile;

    CompressionOptions options;
    options.setCompressionLevel(Args[QStringLiteral("compressionLevel")].toInt());
    //    options.setCompressionMethod(Args[QStringLiteral("compressionMethod")]);
    options.setEncryptionMethod(Args[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize(Args[QStringLiteral("volumeSize")].toULongLong());

    QVector< Archive::Entry * > all_entries;

    foreach (QString file, filesToAdd) {
        Archive::Entry *entry = new Archive::Entry();

        QFileInfo fi(file);
        QString externalPath = fi.path() + QDir::separator();

        QString parentPath = "";
        if (m_pArchiveModel->getParentEntry() != nullptr) {
//            parentPath = m_pArchiveModel->getParentEntry()->property("fullPath").toString();
            parentPath = pWorkEntry->getParent()->property("fullPath").toString();
        }
//        QString tempFile = file;
        entry->setFullPath(parentPath + fi.fileName());//remove external path,added by hsw
//        entry->setParent(m_pArchiveModel->getParentEntry());
        entry->setParent(pWorkEntry->getParent());
        if (fi.isDir()) {
            entry->setIsDirectory(true);
            QHash<QString, QIcon> *map = new QHash<QString, QIcon>();
            Archive::CreateEntry(fi.absoluteFilePath(), entry, externalPath, map);
            m_pArchiveModel->appendEntryIcons(*map);
            map->clear();
            delete map;
            map = nullptr;
        } else {
            entry->setProperty("size", fi.size());
        }
        entry->setFullPath(file);
        all_entries.append(entry);
        m_strAppendFileName = file;
    }

    if (all_entries.isEmpty()) {
        qDebug() << "all_entries.isEmpty()";
        return;
    }

    QFileInfo fi(sourceArchivePath);
    Archive::Entry *sourceEntry  = nullptr;
    if (fi.isAbsolute()) {
        sourceEntry = new Archive::Entry();
        if (fi.isDir()) {
            sourceEntry->setIsDirectory(true);
        }

        QString globalWorkDir = sourceArchivePath;
        if (globalWorkDir.right(1) == QLatin1String("/")) {
            globalWorkDir.chop(1);
        }
        globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
        options.setGlobalWorkDir(globalWorkDir);
    } else {
        if (!m_pUnCompressPage) {
            return;
        }
        if (fileViewer *pFViewer = m_pUnCompressPage->getFileViewer()) {
            if (MyTableView *pTableView = pFViewer->getTableView()) {
                if (!pTableView->selectionModel()) {
                    return;
                }
                for (const auto &iter :  pTableView->selectionModel()->selectedRows()) {
                    sourceEntry = m_pArchiveModel->entryForIndex(iter);
                    if (sourceEntry->name() == sourceArchivePath) {
                        break;
                    }
                }
            }
        }
        if (!sourceEntry) {
            return;
        }
        sourceEntry->setIsDirectory(false);
        options.setGlobalWorkDir(sourceArchivePath);
    }

//    if (m_pArchiveModel->getParentEntry() != sourceEntry) {
//        sourceEntry = pWorkEntry->getParent();
//    }
    if (pWorkEntry->getParent() != sourceEntry) {
        sourceEntry = pWorkEntry->getParent();
    }

    resetMainwindow();
//    calSelectedTotalEntrySize(all_entries);
    qint64 size = 0;
    sourceEntry->calAllSize(size);
//    m_ProgressIns += size;
    m_pProgess->pInfo()->getTotalSize() += size;

    m_pJob = m_pArchiveModel->addFiles(all_entries, sourceEntry, pIface, options);//this added by hsw
    if (!m_pJob) {
        return;
    }

    connect(m_pJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)), Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &CreateJob::percentfilename, this, &MainWindow::SlotProgressFile, Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &KJob::result, this, &MainWindow::slotJobFinished, Qt::ConnectionType::UniqueConnection);

    m_ePageID = PAGE_ZIPPROGRESS;
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD);
    m_pProgess->setProgressFilename(QFileInfo(filesToAddStr).fileName());
    m_eJobType = JOB_ADD;
    refreshPage();
    m_pJob->start();
    m_eWorkStatus = WorkProcess;
}

void MainWindow::addArchive(QMap<QString, QString> &Args)
{
    if (!m_bIsAddArchive) {
        return;
    }
    if (!m_pArchiveModel) {
        return;
    }

//    m_operationtype = Operation_SingleExtract;
    //m_pProgressdialog->setCurrentFile();

    QString sourceArchivePath = Args[QStringLiteral("sourceFilePath")];
    QString filesToAddStr = Args[QStringLiteral("ToCompressFilePath")];
    QStringList filesToAdd = filesToAddStr.split("--");

    const QString fixedMimeType = Args[QStringLiteral("fixedMimeType")];
    const QString password = Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = Args[QStringLiteral("encryptHeader")];
    m_strCreateCompressFile = Args[QStringLiteral("localFilePath")] + QDir::separator() + Args[QStringLiteral("filename")];
    m_strDecompressFileName = Args[QStringLiteral("filename")];
    m_pCompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);

    ReadOnlyArchiveInterface *pIface = Archive::createInterface(m_strCreateCompressFile, fixedMimeType);
    if (pIface == nullptr) {
        qDebug() << "init plugin failed.";
        return;
    }
    if (m_strCreateCompressFile.isEmpty()) {
        qDebug() << "filename.isEmpty()";
        return;
    }

    //renameCompress(m_strCreateCompressFile, fixedMimeType);
    m_strDecompressFileName = QFileInfo(m_strCreateCompressFile).fileName();
    m_pCompressSuccess->setCompressFullPath(m_strCreateCompressFile);
    //qDebug() << m_strCreateCompressFile;

    CompressionOptions options;
    options.setCompressionLevel(Args[QStringLiteral("compressionLevel")].toInt());
    //    options.setCompressionMethod(Args[QStringLiteral("compressionMethod")]);
    options.setEncryptionMethod(Args[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize(Args[QStringLiteral("volumeSize")].toULongLong());

    QVector< Archive::Entry * > all_entries;
    foreach (QString file, filesToAdd) {
        Archive::Entry *entry = new Archive::Entry();

        QFileInfo fi(file);
        QString externalPath = fi.path() + QDir::separator();

        QString parentPath = "";
        if (m_pArchiveModel->getParentEntry() != nullptr) {
            parentPath = m_pArchiveModel->getParentEntry()->property("fullPath").toString();
        }

        entry->setFullPath(parentPath + fi.fileName());//remove external path,added by hsw
        entry->setParent(m_pArchiveModel->getParentEntry());
        if (fi.isDir()) {
            entry->setIsDirectory(true);
            QHash<QString, QIcon> *map = new QHash<QString, QIcon>();
            Archive::CreateEntryNew(fi.filePath(), entry, externalPath, map);
            m_pArchiveModel->appendEntryIcons(*map);
            map->clear();
            delete map;
            map = nullptr;
        } else {
            entry->setProperty("size", fi.size());
        }
        entry->setFullPath(file);
        all_entries.append(entry);
        m_strAppendFileName = file;

    }
    if (all_entries.isEmpty()) {
        qDebug() << "all_entries.isEmpty()";
        return;
    }
    QFileInfo fi(sourceArchivePath);
    Archive::Entry *sourceEntry  = nullptr;
    if (fi.isAbsolute()) {
        sourceEntry = new Archive::Entry();
        if (fi.isDir()) {
            sourceEntry->setIsDirectory(true);
        }
        QString globalWorkDir = sourceArchivePath;
        if (globalWorkDir.right(1) == QLatin1String("/")) {
            globalWorkDir.chop(1);
        }
        QFileInfo fileInfo(globalWorkDir);
        if (fileInfo.isDir()) {
            globalWorkDir = fileInfo.filePath();
        } else {
            globalWorkDir = fileInfo.absolutePath();
        }
        options.setGlobalWorkDir(globalWorkDir);
    } else {
        if (!m_pUnCompressPage) {
            return;
        }
        if (fileViewer *pFViewer = m_pUnCompressPage->getFileViewer()) {
            if (MyTableView *pTableView = pFViewer->getTableView()) {
                if (!pTableView->selectionModel()) {
                    return;
                }
                for (const auto &iter :  pTableView->selectionModel()->selectedRows()) {
                    sourceEntry = m_pArchiveModel->entryForIndex(iter);
                    if (sourceEntry->name() == sourceArchivePath) {
                        break;
                    }
                }
            }
        }
        if (!sourceEntry) {
            return;
        }
        sourceEntry->setIsDirectory(false);
        options.setGlobalWorkDir(sourceArchivePath);
    }
    qDebug() << "开始执行添加任务12";
    if (m_pArchiveModel->getParentEntry() != nullptr && m_pArchiveModel->getParentEntry() != sourceEntry) {
        //m_pArchiveModel->mapFilesUpdate;//根据这个获取当前位于那个sourceEntry中
        sourceEntry = m_pArchiveModel->getParentEntry();
    }

    resetMainwindow();

    m_pJob = m_pArchiveModel->addFiles(all_entries, sourceEntry, pIface, options);//this added by hsw
    if (!m_pJob) {
        return;
    }

    AddJob *pAddJob = dynamic_cast<AddJob *>(m_pJob);
    if (pAddJob->archiveInterface()->mType == ReadOnlyArchiveInterface::ENUM_PLUGINTYPE::PLUGIN_CLIINTERFACE) {//7z的计算目标大小
        calSelectedTotalEntrySize(all_entries);
    } else {
        calSelectedTotalEntrySize(all_entries);
        sourceEntry->calAllSize(m_pProgess->pInfo()->getTotalSize());
    }

    connect(m_pJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)), Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &CreateJob::percentfilename, this, &MainWindow::SlotProgressFile, Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &KJob::result, this, &MainWindow::slotJobFinished, Qt::ConnectionType::UniqueConnection);
    m_ePageID = PAGE_ZIPPROGRESS;
//    m_pProgess->settype(COMPRESSING);
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD);
    m_pProgess->setProgressFilename(QFileInfo(filesToAddStr).fileName());
    m_eJobType = JOB_ADD;
    refreshPage();
    //m_strPathStore = Args[QStringLiteral("localFilePath")];
    m_pJob->start();
    m_eWorkStatus = WorkProcess;
}

void MainWindow::removeEntryVector(QVector<Archive::Entry *> &vectorDel, bool isManual)
{
    if (vectorDel.isEmpty()) {
        qDebug() << "all_entries.isEmpty()";
        return;
    }

    if (m_pArchiveModel->archive()->fileName().endsWith(".zip") || m_pArchiveModel->archive()->fileName().endsWith(".jar")) {
        if (ReadOnlyArchiveInterface *pinterface = m_pArchiveModel->getPlugin()) {
            if (pinterface->isAllEntry()) {
                foreach (Archive::Entry *p, vectorDel) {
                    m_pProgess->pInfo()->getTotalSize() += p->property("size").toLongLong();
                }
            } else {
                m_pProgess->pInfo()->getTotalSize() = pinterface->extractSize(vectorDel);
            }
        }
    } else {
        foreach (Archive::Entry *p, vectorDel) {
            m_pProgess->pInfo()->getTotalSize() += p->property("size").toLongLong();
        }
    }

    m_pJob =  m_pArchiveModel->deleteFiles(vectorDel);
    if (!m_pJob) {
        return;
    }

    connect(m_pJob, &KJob::result, this, &MainWindow::slotJobFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &DeleteJob::percentfilename, this, &MainWindow::SlotProgressFile, Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)), Qt::ConnectionType::UniqueConnection);

    m_ePageID = PAGE_DELETEPROGRESS;
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DELETEING);


    //m_pProgess->settype(DECOMPRESSING);
    if (isManual) {
        m_eJobType = JOB_DELETE_MANUAL;
    } else {
        m_eJobType = JOB_DELETE;
    }

    //重置进度条
    resetMainwindow();

    DeleteJob *pDeleteJob = dynamic_cast<DeleteJob *>(m_pJob);
    if (pDeleteJob->archiveInterface()->mType == ReadOnlyArchiveInterface::ENUM_PLUGINTYPE::PLUGIN_READWRITE_LIBARCHIVE) {//该插件(tar格式)，计算总大小时，需要减去待删除的文件的大小
        //Archive::Entry *pRootEntry = this->m_pArchiveModel->getRootEntry();
        qint64 size = 0;
        this->m_pArchiveModel->getRootEntry()->calAllSize(size);//added by hsw for valid total size
        m_pProgess->pInfo()->getTotalSize() += size;
        Archive::Entry *pFirstEntry = vectorDel[0];
        qint64 sizeCountWillDel = 0;
        pFirstEntry->calAllSize(sizeCountWillDel);
        m_pProgess->pInfo()->getTotalSize() -= sizeCountWillDel;
    } else if (pDeleteJob->archiveInterface()->mType == ReadOnlyArchiveInterface::ENUM_PLUGINTYPE::PLUGIN_CLIINTERFACE) {//7z的
        calSelectedTotalEntrySize(vectorDel);
    } else { //其他格式的是否需要减去，删除子项的大小，还待调试优化。
        //Archive::Entry *pRootEntry = this->m_pArchiveModel->getRootEntry();
        qint64 size = 0;
        this->m_pArchiveModel->getRootEntry()->calAllSize(size);
        m_pProgess->pInfo()->setTotalSize(size);//设置总大小
    }

    refreshPage();
    qDebug() << "delete job start";
    m_pJob->start();
    m_eWorkStatus = WorkProcess;
}

void MainWindow::moveToArchive(QMap<QString, QString> &Args)
{
    if (!m_pArchiveModel) {
        return;
    }

    QString sourceArchivePath = Args[QStringLiteral("sourceFilePath")];
    QString filesToAddStr = Args[QStringLiteral("ToCompressFilePath")];
    QStringList filesToAdd = filesToAddStr.split("--");

    const QString fixedMimeType = Args[QStringLiteral("fixedMimeType")];
    const QString password = Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = Args[QStringLiteral("encryptHeader")];
    m_strCreateCompressFile = Args[QStringLiteral("localFilePath")] + QDir::separator() + Args[QStringLiteral("filename")];
    m_strDecompressFileName = Args[QStringLiteral("filename")];
    m_pCompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);//relative path to base archive

    if (m_strCreateCompressFile.isEmpty()) {
        qDebug() << "filename.isEmpty()";
        return;
    }

    m_strDecompressFileName = QFileInfo(m_strCreateCompressFile).fileName();
    m_pCompressSuccess->setCompressFullPath(m_strCreateCompressFile);
    qDebug() << m_strCreateCompressFile;

    CompressionOptions options;
    options.setCompressionLevel(Args[QStringLiteral("compressionLevel")].toInt());
    //    options.setCompressionMethod(Args[QStringLiteral("compressionMethod")]);
    options.setEncryptionMethod(Args[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize(Args[QStringLiteral("volumeSize")].toULongLong());

    QVector< Archive::Entry * > all_entries;

    foreach (QString file, filesToAdd) {
        Archive::Entry *entry = new Archive::Entry();
        entry->setFullPath(file);
        QFileInfo fi(file);
        if (fi.isDir()) {
            entry->setIsDirectory(true);
        }

        all_entries.append(entry);
        m_strAppendFileName = file;
    }

    if (all_entries.isEmpty()) {
        qDebug() << "all_entries.isEmpty()";
        return;
    }

    QFileInfo fi(sourceArchivePath);
    Archive::Entry *sourceEntry  = nullptr;

    if (!m_pUnCompressPage) {
        return;
    }
    if (fileViewer *pFViewer = m_pUnCompressPage->getFileViewer()) {
        if (MyTableView *pTableView = pFViewer->getTableView()) {
            if (!pTableView->selectionModel()) {
                return;
            }
            for (const auto &iter :  pTableView->selectionModel()->selectedRows()) {
                sourceEntry = m_pArchiveModel->entryForIndex(iter);
                if (sourceEntry->name() == sourceArchivePath) {
                    break;
                }
            }
        }
    }
    if (!sourceEntry) {
        return;
    }
    sourceEntry->setIsDirectory(false);
    //options.setGlobalWorkDir(sourceArchivePath);


    qDebug() << "开始执行移动任务";
    m_pJob =  m_pArchiveModel->moveFiles(all_entries, sourceEntry, options);
    if (!m_pJob) {
        return;
    }

    connect(m_pJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)), Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &KJob::percentfilename, this, &MainWindow::SlotProgressFile, Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &KJob::result, this, &MainWindow::slotJobFinished, Qt::ConnectionType::UniqueConnection);

    m_ePageID = PAGE_ZIPPROGRESS;
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING);

    m_eJobType = JOB_MOVE;
    refreshPage();
    m_strPathStore = Args[QStringLiteral("localFilePath")];
    qDebug() << "开始执行移动任务13";
    m_pJob->start();
    m_eWorkStatus = WorkProcess;
}

void MainWindow::transSplitFileName(QString &fileName)  // *.7z.003 -> *.7z.001
{
    QRegExp reg("^([\\s\\S]*.)[0-9]{3}$"); // QRegExp reg("[*.]part\\d+.rar$"); //rar分卷不匹配

    if (reg.exactMatch(fileName) == false) {
        return;
    }

    QFileInfo fi(reg.cap(1) + "001");

    if (fi.exists() == true) {
        fileName = reg.cap(1) + "001";
    }
}

void MainWindow::renameCompress(QString &filename, QString fixedMimeType)
{
    QString localname = filename;
    int num = 2;

    if (m_pCompressSetting->onSplitChecked()) {   // 7z分卷压缩
        QFileInfo file(filename);
        bool isFirstFileExist = false;
        bool isOtherFileExist = false;

        // 以文件名为1.7z.001验证
        // 过滤 该路径下 1*.7z.*的文件
        QStringList nameFilters;
        nameFilters << file.baseName() + "*.7z.*";
        QDir dir(file.path());
        QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Dirs | QDir::Readable, QDir::Name);

        //  循环判断 files列表 1.7z文件是否存在
        foreach (QFileInfo firstFile, files) {
            if (firstFile.baseName() == file.baseName()) {
                isFirstFileExist = true;
                break;
            } else {
                continue;
            }
        }

        if (isFirstFileExist) {  // 1.7z文件已存在  文件名为1(2).7z ...
            for (int newCount = 0; newCount < files.count(); newCount++) {
                newCount += 2;
                int count = 0;
                filename = localname.remove("." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix()) + "(" + "0"
                           + QString::number(newCount) + ")" + "."
                           + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();
                for (int i = 0; i < files.count(); i++) {
                    if (files.at(i).contains(file.baseName() + "(0" + QString::number(newCount) + ").7z.")) {
                        filename = localname.remove("." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix()) + "(" + "0"
                                   + QString::number(newCount + 1) + ")" + "."
                                   + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();

                        isOtherFileExist = true;
                        break;
                    } else {
                        count++;
                        continue;
                    }
                }

                if (isOtherFileExist) {
                    isOtherFileExist = false;
                    continue;
                }
                if (files.count() == count) {
                    break;
                }
            }
        }
    } else {
        while (QFileInfo::exists(filename)) {
            filename = localname.remove("." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix()) + "(" + "0"
                       + QString::number(num) + ")" + "."
                       + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();
            num++;
        }
    }
}

//QStringList MainWindow::CheckAllFiles(QString path)
//{
//    QDir dir(path);
//    QStringList nameFilters;
//    QStringList entrys = dir.entryList(nameFilters, QDir::AllEntries | QDir::Readable, QDir::Name);

//    for (int i = 0; i < entrys.count(); i++) {
//        entrys.replace(i, path + QDir::separator() + entrys.at(i));
//    }
//    return entrys;
//}

bool clearTempFiles(const QString &temp_path)
{
    bool ret = false;
    //    qDebug()<<temp_path;
    //    QDir dir(temp_path);
    //    if(dir.isEmpty())
    //    {
    //        qDebug()<<"dir.isEmpty()";
    //        return false;
    //    }
    //    QStringList filter; //过滤器
    //    filter.append("*");
    //    QDirIterator it(temp_path, filter, QDir::Dirs | QDir::Files, QDirIterator::NoIteratorFlags);
    //    while(it.hasNext()) { //若容器中还有成员，继续执行删除操作
    //        if(it.next().contains("/..") || it.next().contains("/.") || it.next().toStdString() == "")
    //        {
    //            continue;
    //        }

    //        QFileInfo fileinfo(it.next());
    //        qDebug()<<it.next();
    //        if(fileinfo.isDir())
    //        {
    //            clearTempFiles(it.next());
    //            ret = dir.rmpath(it.next());
    //            if(false == ret)
    //            {
    //                qDebug()<<"error"<<it.next();
    //            }
    //        }
    //        else {
    //            ret = dir.remove(it.next());Q
    //            if(false == ret)
    //            {
    //                qDebug()<<"error"<<it.next();
    //            }
    //        }
    //    }
    //    qDebug()<<ret;
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-fr");
    args.append(temp_path);
    p.execute(command, args);
    p.waitForFinished();
    return ret;
}

void MainWindow::deleteCompressFile(/*QStringList oldfiles, QStringList newfiles*/)
{
    if (m_pProgess->getType() == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD) {
        QFile fi(m_strCreateCompressFile + ".tmp");
        if (fi.exists()) {
            fi.remove();
        }
        return;
    }

    QFile fi(m_strCreateCompressFile);  // 没有判断 7z分卷压缩的 文件名
    if (fi.exists()) {
        fi.remove();
    }

    if (m_pCompressSetting->onSplitChecked()) {  // 7z分卷压缩
        QFileInfo file(m_strCreateCompressFile);
        QStringList nameFilters;
        nameFilters << file.fileName() + ".0*";
        QDir dir(file.path());
        QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

        foreach (QFileInfo fi, files) {
            QFile fiRemove(fi.filePath());
            if (fiRemove.exists()) {
                fiRemove.remove();
            }
        }
    }

//    if (newfiles.count() <= oldfiles.count()) {
//        qDebug() << "No file to delete";
//        return;
//    }

//    QStringList deletefile;
//    foreach (QString newpath, newfiles) {
//        int count = 0;
//        foreach (QString oldpath, oldfiles) {
//            if (oldpath == newpath) {
//                break;
//            }
//            count++;
//        }
//        if (count == oldfiles.count()) {
//            deletefile << newpath;
//        }
//    }

//    foreach (QString path, deletefile) {
//        QFileInfo fileInfo(path);
//        if (fileInfo.isFile() || fileInfo.isSymLink()) {
//            QFile::setPermissions(path, QFile::WriteOwner);
//            if (!QFile::remove(path)) {
//                qDebug() << "delete error!!!!!!!!!";
//            }
//        } else if (fileInfo.isDir()) {
//            clearTempFiles(path);
//            qDebug() << "delete ok!!!!!!!!!!!!!!";
//            if (fileInfo.exists()) {
//                clearTempFiles(path);
//            }
//        }
    //    }
}

//解压取消时删除临时文件,这个函数好像不太安全，尽量不要使用
void MainWindow::deleteDecompressFile(QString destDirName)
{
    if (destDirName.isEmpty()) {
        return;
    }
//    qDebug() << "deleteDecompressFile" << m_strDecompressFilePath << m_strDecompressFileName << m_pUnCompressPage->getDeFileCount() << m_pArchiveModel->archive()->isSingleFile() << m_pArchiveModel->archive()->isSingleFolder();
    bool bAutoCreatDir = m_pSettingsDialog->isAutoCreatDir();
    QString tmpDecompressfilepath = m_strDecompressFilePath;
    if (!tmpDecompressfilepath.isEmpty()) {
        if (!tmpDecompressfilepath.endsWith(QDir::separator())) {
            tmpDecompressfilepath += QDir::separator();
        }
        if (m_pUnCompressPage->getDeFileCount() > 1) { //多文件(夹)
            if (bAutoCreatDir) {
                QDir fi(tmpDecompressfilepath);  //注意：若tmpDecompressfilepath为空字符串，则使用（"."）构造目录，后面会删除整个当前目录!!!
                if (fi.exists()) {
                    QString newname = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/Trash/files/") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss-") + fi.dirName();
                    fi.rename(fi.path(), newname);
                }
            } /*else {      //不自动创建文件夹，顶级多文件(夹)，未做删除临时文件处理
                auto rootEntry = this->m_pArchiveModel->getRootEntry();
                int rootEntriesNum = rootEntry->entries().length();
                for (int i = 0; i < rootEntriesNum; i++) {
                    qDebug() << rootEntry->entries().at(i)->name();
                    QDir fi(tmpDecompressfilepath + rootEntry->entries().at(i)->name());
                    if (fi.exists()) {
                        fi.removeRecursively();
                    }
                }
            }*/
        } else if (m_pUnCompressPage->getDeFileCount() == 1) {
            if (!m_pArchiveModel->archive()->isSingleFile()) { //单个文件夹
                //                QString dirName = m_pArchiveModel->archive()->subfolderName();
                //                if (dirName.isEmpty()) {
                //                    return;
                //                }
                QDir fi(tmpDecompressfilepath + destDirName);
                if (fi.exists()) {
                    QString newname = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/Trash/files/") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss-") + destDirName;
                    fi.rename(fi.path(), newname);
                }
            } else { //单个文件
                QFile fi(tmpDecompressfilepath + destDirName);
                if (fi.exists()) {
                    QString newname = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/Trash/files/") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss-") + destDirName;
                    fi.rename(newname);
                }
            }
        }
    }
}

bool MainWindow::startCmd(const QString &executeName, QStringList arguments)
{
    QString programPath = QStandardPaths::findExecutable(executeName);
    if (programPath.isEmpty()) {
        qDebug() << "error can't find xdg-mime";
        return false;
    }
    KProcess *cmdprocess = new KProcess;
    cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
    cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    cmdprocess->setProgram(programPath, arguments);
    auto func = [ = ](int)->void {
        if (cmdprocess != nullptr)
        {
            QObject::disconnect(cmdprocess);
            delete cmdprocess;
        }
    };

    QObject::connect(cmdprocess, QOverload< int, QProcess::ExitStatus >::of(&QProcess::finished), func);
    cmdprocess->start();
    return true;
}

void MainWindow::creatArchive(QMap< QString, QString > &Args)
{
    const QStringList filesToAdd = m_pCompressPage->getCompressFilelist();
    const QString fixedMimeType = Args[QStringLiteral("fixedMimeType")];
    const QString password = Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = Args[QStringLiteral("encryptHeader")];
    m_strCreateCompressFile = Args[QStringLiteral("localFilePath")] + QDir::separator() + Args[QStringLiteral("filename")];
    m_strDecompressFileName = Args[QStringLiteral("filename")];
    m_pCompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);

    if (m_strCreateCompressFile.isEmpty()) {
        qDebug() << "filename.isEmpty()";
        return;
    }

    renameCompress(m_strCreateCompressFile, fixedMimeType);
    m_strDecompressFileName = QFileInfo(m_strCreateCompressFile).fileName();
    m_pCompressSuccess->setCompressFullPath(m_strCreateCompressFile);
    qDebug() << m_strCreateCompressFile;

    CompressionOptions options;
    options.setCompressionLevel(Args[QStringLiteral("compressionLevel")].toInt());
    //    options.setCompressionMethod(Args[QStringLiteral("compressionMethod")]);
    options.setEncryptionMethod(Args[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize(Args[QStringLiteral("volumeSize")].toULongLong());
    options.setIsTar7z(0 == Args[QStringLiteral("createtar7z")].compare("true"));
    options.setFilesSize(Args[QStringLiteral("selectFilesSize")].toLongLong());

    QVector< Archive::Entry * > all_entries;

    foreach (QString file, filesToAdd) {
        Archive::Entry *entry = new Archive::Entry(this);
        entry->setFullPath(file);



        QFileInfo fi(file);
        if (fi.isDir()) {
            entry->setIsDirectory(true);
        }

        all_entries.append(entry);
    }

    if (all_entries.isEmpty()) {
        qDebug() << "all_entries.isEmpty()";
        return;
    }

    QString globalWorkDir = filesToAdd.first();
    if (globalWorkDir.right(1) == QLatin1String("/")) {
        globalWorkDir.chop(1);
    }
    globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
    options.setGlobalWorkDir(globalWorkDir);

#ifdef __aarch64__ // 华为arm平台 zip压缩 性能提升. 在多线程场景下使用7z,单线程场景下使用libarchive
    double maxFileSizeProportion = static_cast<double>(maxFileSize_) / static_cast<double>(m_pProgess->pInfo()->getTotalSize());
    m_pJob = Archive::create(m_strCreateCompressFile, fixedMimeType, all_entries, options, this, maxFileSizeProportion > 0.6);
#else
    m_pJob = Archive::create(m_strCreateCompressFile, fixedMimeType, all_entries, options, this);
#endif
//    m_createJob = Archive::create(m_strCreateCompressFile, fixedMimeType, all_entries, options, this);

    if (!password.isEmpty()) {
        if (m_pJob->mType == Job::ENUM_JOBTYPE::CREATEJOB) {
            CreateJob *pCreateJob = dynamic_cast<CreateJob *>(m_pJob);
            pCreateJob->enableEncryption(password, enableHeaderEncryption.compare("true") ? false : true);
        }
    }

    connect(m_pJob, &KJob::result, this, &MainWindow::slotCompressFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)), Qt::ConnectionType::UniqueConnection);
    connect(m_pJob, &CreateJob::percentfilename, this, &MainWindow::SlotProgressFile, Qt::ConnectionType::UniqueConnection);

    m_ePageID = PAGE_ZIPPROGRESS;
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING);
    m_eJobType = JOB_CREATE;
    refreshPage();

    m_strPathStore = Args[QStringLiteral("localFilePath")];
    //m_compressDirFiles = CheckAllFiles(m_strPathStore);

    m_pJob->start();
    m_eWorkStatus = WorkProcess;
}

void MainWindow::slotCompressFinished(KJob *job)
{
    qDebug() << "job finished" << job->error();
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_NONE);
    m_eWorkStatus = WorkNone;
    if (job->error() && (job->error() != KJob::KilledJobError)) {
        if (m_strPathStore.left(6) == "/media") {
            if (getMediaFreeSpace() <= 50) {
                m_pCompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
            } else {
                m_pCompressFail->setFailStrDetail(tr("Damaged file"));
            }
        } else {
            if (getDiskFreeSpace() <= 50) {
                m_pCompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
            } else {
                m_pCompressFail->setFailStrDetail(tr("Damaged file"));
            }
        }
        m_ePageID = PAGE_ZIP_FAIL;
        refreshPage();
        return;
    }

    m_strCreateCompressFile.clear();
    m_ePageID = PAGE_ZIP_SUCCESS;
    refreshPage();

    deleteLaterJob();
}
void MainWindow::slotJobFinished(KJob *job)
{
    if (m_eJobType == JOB_DELETE || m_eJobType == JOB_DELETE_MANUAL || m_eJobType == JOB_ADD) {
        if (m_pArchiveModel->archive()->fileName().endsWith(".zip") || m_pArchiveModel->archive()->fileName().endsWith(".jar")) {
            if (ReadOnlyArchiveInterface *pinterface = m_pArchiveModel->getPlugin()) {
                if (!pinterface->isAllEntry()) {
                    pinterface->updateListMap();
                }
            }
        }
    }

    qDebug() << "job finished" << job->error();
    m_eWorkStatus = WorkNone;
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_NONE);
    if (job->error() && (job->error() != KJob::KilledJobError && job->error() != KJob::CancelError)) {
        if (m_strPathStore.left(6) == "/media") {
            if (getMediaFreeSpace() <= 50) {
                m_pCompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
            } else {
                m_pCompressFail->setFailStrDetail(tr("Damaged file"));
            }
        } else {
            if (getDiskFreeSpace() <= 50) {
                m_pCompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
            } else {
                m_pCompressFail->setFailStrDetail(tr("Damaged file"));
            }
        }
        if (!m_bIsAddArchive && m_eJobType == JOB_CREATE) {
            m_ePageID = PAGE_ZIP_FAIL;
        } else {
            m_ePageID = PAGE_UNZIP;
        }
        refreshPage();
        return;
    }
    switch (m_eJobType) {
    case JOB_CREATE:
        m_strCreateCompressFile.clear();
        if (!m_bIsAddArchive) {
            m_ePageID = PAGE_ZIP_SUCCESS;
        }
        deleteLaterJob();
        refreshPage();
        break;
    case JOB_ADD: {
        m_strCreateCompressFile.clear();
        if (m_bIsAddArchive) {
            m_ePageID = PAGE_UNZIP;
        }

        //emit sigUpdateTableView(m_strAppendFileName);
        //reload package archive
        QString filename =   m_pArchiveModel->archive()->fileName();
        QStringList ArchivePath = QStringList() << filename;

        //onSelected(ArchivePath);

        if (m_pJob) {
            if (m_pJob->mType == KJob::ENUM_JOBTYPE::ADDJOB) {
                AddJob *pJob = dynamic_cast<AddJob *>(m_pJob);
                auto res = pJob->entries();
                if (res.length() > 0) {
                    this->m_pUnCompressPage->getFileViewer()->selectRowByEntry(res[0]);
                }
            }
            deleteLaterJob();
        }
        refreshPage();
        emit sigTipsWindowPopUp(SUBACTION_MODE::ACTION_DRAG, ArchivePath);
    }

    break;
    case JOB_DELETE: {
        m_ePageID = PAGE_UNZIP;
        //reload package archive
        QString filename =   m_pArchiveModel->archive()->fileName();
        QStringList ArchivePath = QStringList() << filename;

        if (m_pJob && m_pJob->mType == Job::ENUM_JOBTYPE::DELETEJOB) {
            DeleteJob *pDeleteJob = nullptr;
            pDeleteJob = dynamic_cast<DeleteJob *>(m_pJob);
            this->m_pUnCompressPage->getFileViewer()->getTableView()->clearSelection();// delete 后清除选中
            Archive::Entry *pWorkEntry = pDeleteJob->getWorkEntry();
            deleteLaterJob();

            refreshPage();
            //refresh valid begin
            m_pArchiveFilterModel->clear();
            m_pArchiveFilterModel->setSourceModel(m_pArchiveModel);
            //refresh valid end
            qDebug() << "自动删除完成信号" << ArchivePath;
            emit deleteJobComplete(pWorkEntry);
        }

    }
    break;
    case JOB_DELETE_MANUAL: {
        m_ePageID = PAGE_UNZIP;
        //reload package archive
        QString filename =   m_pArchiveModel->archive()->fileName();
        QStringList ArchivePath = QStringList() << filename;
        if (m_pJob->mType == Job::ENUM_JOBTYPE::DELETEJOB) {
            this->m_pUnCompressPage->getFileViewer()->getTableView()->clearSelection();// delete 后清除选中
            deleteLaterJob();
        }
        refreshPage();
        //refresh valid begin
        m_pArchiveFilterModel->clear();
        m_pArchiveFilterModel->setSourceModel(m_pArchiveModel);
        //refresh valid end
        qDebug() << "手动删除完成信号" << ArchivePath;
//        emit deleteJobComplete();
        emit sigTipsWindowPopUp(SUBACTION_MODE::ACTION_DELETE, ArchivePath);
    }
    break;
    case JOB_LOAD:
        break;
    case JOB_COPY:
        break;
    case JOB_BATCHEXTRACT:
        break;
    case JOB_EXTRACT:
        break;
    case JOB_TEMPEXTRACT:
        break;
    case JOB_MOVE: {
        m_ePageID = PAGE_UNZIP;
        deleteLaterJob();
        refreshPage();
    }
    break;
    case JOB_COMMENT:
        break;
    case JOB_BATCHCOMPRESS:
        break;
    case JOB_NULL:
        break;
    }
}

QString MainWindow::modelIndexToStr(const QModelIndex &index)
{
    return QString::number(index.row()) + QString::number(index.column()) + QString::number(index.internalId());
}

void MainWindow::slotExtractSimpleFiles(QVector< Archive::Entry * > fileList, QString path, EXTRACT_TYPE type)
{
    m_pProgess->pInfo()->startTimer();

    resetMainwindow();

    if (m_pArchiveModel->archive()->fileName().endsWith(".zip") || m_pArchiveModel->archive()->fileName().endsWith(".jar")) {
        if (ReadOnlyArchiveInterface *pinterface = m_pArchiveModel->getPlugin()) {
            if (pinterface->isAllEntry()) {
                foreach (Archive::Entry *p, fileList) {
                    m_pProgess->pInfo()->getTotalSize() += p->property("size").toLongLong();
                }
            } else {
                m_pProgess->pInfo()->getTotalSize() = pinterface->extractSize(fileList);
            }
        }
    } else {
        foreach (Archive::Entry *p, fileList) {
            m_pProgess->pInfo()->getTotalSize() += p->property("size").toLongLong();
        }
    }

//    if (type == EXTRACT_TO) {// 传递的是顶节点
//        foreach (Archive::Entry *p, fileList) {
//            p->calAllSize(m_pProgess->pInfo()->getTotalSize());
//        }
//    } else {// 传递的是所有节点
//        foreach (Archive::Entry *p, fileList) {
//            m_pProgess->pInfo()->getTotalSize() += p->getSize();
//        }
//    }

    m_pProgressdialog->setProcess(0);
    m_pProgess->setprogress(0);

    m_eWorkStatus = WorkProcess;
    m_strPathStore = path;

    m_pProgressdialog->clearprocess();
    if (!m_pArchiveModel) {
        return;
    }

    deleteLaterJob();

    ExtractionOptions options;
    options.setDragAndDropEnabled(true);
    m_vecExtractSimpleFiles = fileList;
    QString destinationDirectory = path;
    //m_compressDirFiles = CheckAllFiles(path);

    Archive::Entry *pDestEntry = fileList[0];

    QString programName = "";

    if (type == EXTRACT_TEMP) {
        m_operationtype = Operation_TempExtract;
        m_ePageID = Page_ID::PAGE_LOADING;

        // m_openType = true;
        m_pProgess->setopentype(true);
        if (m_pCurAuxInfo == nullptr) {
            m_pCurAuxInfo = new MainWindow_AuxInfo();
        }

        OpenInfo *pNewInfo = nullptr;
        QModelIndex index = this->m_pArchiveModel->indexForEntry(fileList[0]);
        QString key = modelIndexToStr(index);
        if (m_pCurAuxInfo->information.contains(key) == false) {
            pNewInfo = new OpenInfo;
        } else {
            if (this->m_pMapGlobalWnd != nullptr) {
                MainWindow *pChild = qobject_cast<MainWindow *>(this->m_pMapGlobalWnd->getOne(m_pCurAuxInfo->information[key]->strWinId));
                if (pChild != nullptr) {
                    QApplication::setActiveWindow(pChild);  // 置顶
                    m_eWorkStatus = WorkNone;
                    return;
                }
            }

            SAFE_DELETE_ELE(m_pCurAuxInfo->information[key]);
            m_pCurAuxInfo->information.remove(key);
            pNewInfo = new OpenInfo;
        }
        m_pCurAuxInfo->information.insert(key, pNewInfo);

    } else if (type == EXTRACT_TEMP_CHOOSE_OPEN) {
        m_operationtype =  Operation_TempExtract_Open_Choose;
        m_pProgess->setopentype(true);
    } else if (type == EXTRACT_DRAG) {
        m_operationtype =  Operation_DRAG;
    } else if (type == EXTRACT_HEAR) {
        programName = "deepin-compressor";
//        m_ePageID = PAGE_UNZIPPROGRESS;
        m_operationtype = Operation_SingleExtract;
        //m_strPathStore = QFileInfo(m_pArchiveModel->archive()->fileName()).absolutePath();
        m_strPathStore = *m_pChildMndExtractPath;
        destinationDirectory = m_strPathStore;
    } else if (type == EXTRACT_TO) {
        programName = "deepin-compressor";
//        m_ePageID = PAGE_UNZIPPROGRESS;
        m_operationtype = Operation_SingleExtract;
    } else {
        programName = "deepin-compressor";
        m_operationtype = Operation_SingleExtract;
    }

    if (fileList.size() == 1 && !(fileList.at(0)->fullPath().endsWith("/"))) {
        m_pProgressdialog->setCurrentFile(fileList.at(0)->fullPath());
    }

    if (!destinationDirectory.endsWith(QDir::separator())/*destinationDirectory.right(1) != QDir::separator()*/) {
        destinationDirectory = destinationDirectory + QDir::separator();
    }
    QString destEntryPath = destinationDirectory + pDestEntry->name();
    QFileInfo fileInfo(destEntryPath);

    if (fileInfo.exists() && programName == "" && (type == EXTRACT_TEMP || type == EXTRACT_TEMP_CHOOSE_OPEN)) { //判断解压文件是否已经在目标路径下已经解压出来，如果解压出来，则不再解压
        qint64 size = pDestEntry->getSize();
        qint64 size1 = calFileSize(destEntryPath);
        if (size == size1) {
            QString programName = "xdg-open";
            QString firstFileName = m_vecExtractSimpleFiles.at(0)->name();
            bool isCompressedFile = Utils::isCompressed_file(pDestEntry->fullPath());


            QStringList arguments;
            arguments << destEntryPath;//the first arg

            if (m_pMapGlobalWnd == nullptr) {
                m_pMapGlobalWnd = new GlobalMainWindowMap();
            }
            m_pMapGlobalWnd->insert(QString::number(this->winId()), this);
            if (isCompressedFile == true) {

                programName = "deepin-compressor";
                arguments << HEADBUS + QString::number(this->winId());//the second arg
                QModelIndex index = this->m_pArchiveModel->indexForEntry(pDestEntry);
                QString strIndex = modelIndexToStr(index);
                arguments << strIndex;//the third arg
            }

            startCmd(programName, arguments);
            return;
        } else {
            clearTempFiles(destEntryPath);//if file exists but diff in size,so delete it and extract again.
        }

    }
    refreshPage();
    m_pJob = m_pArchiveModel->extractFiles(fileList, destinationDirectory, options);
    if (m_pJob == nullptr || m_pJob->mType != Job::ENUM_JOBTYPE::EXTRACTJOB) {
        qDebug() << "ExtractJob new failed.";
        return;
    }
    ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
    pExtractJob->archiveInterface()->bindProgressInfo(this->m_pProgess->pInfo());
    if (this->m_pWatcher == nullptr) {
        this->m_pWatcher = new TimerWatcher();
        connect(this->m_pWatcher, &TimerWatcher::sigBindFuncDone, pExtractJob, &ExtractJob::slotWorkTimeOut);
    }

    pExtractJob->resetTimeOut();
    this->m_pWatcher->bindFunction(this, static_cast<pMember_callback>(&MainWindow::isWorkProcess));
    this->m_pWatcher->beginWork(100);

    connect(pExtractJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
    connect(pExtractJob, &KJob::result, this, &MainWindow::slotExtractionDone);
//
    connect(pExtractJob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
    connect(
        pExtractJob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(pExtractJob, &ExtractJob::sigExtractJobPassword, m_pEncryptionpage, &EncryptionPage::wrongPassWordSlot);
    connect(pExtractJob,
            SIGNAL(percentfilename(KJob *, const QString &)),
            this,
            SLOT(SlotProgressFile(KJob *, const QString &)));

    pExtractJob->start();
    m_strDecompressFilePath = destinationDirectory;

    QFileInfo file(m_strLoadfile);
    m_pProgressdialog->setCurrentTask(file.fileName());
}

void MainWindow::slotExtractSimpleFilesOpen(const QVector<Archive::Entry *> &fileList, const QString &programma)
{
    QString tmppath = TEMPDIR_NAME + PATH_SEP + QUuid::createUuid().toString();
    QDir dir(tmppath);
    if (!dir.exists()) {
        dir.mkdir(tmppath);
    }

    m_strProgram = programma;
    //lastPercent = 0;
    slotExtractSimpleFiles(fileList, tmppath, EXTRACT_TEMP_CHOOSE_OPEN);
}

void MainWindow::slotKillExtractJob()
{
//    m_openType = false;
    m_eWorkStatus = WorkNone;
    killJob();
    //deleteCompressFile(m_compressDirFiles, CheckAllFiles(m_strDecompressFilePath));
}

void MainWindow::slotFailRetry()
{
    if (PAGE_ZIP_FAIL == m_ePageID) {
        m_ePageID = PAGE_ZIPSET;
        refreshPage();
    } else if (Operation_Load == m_operationtype) {
        m_ePageID = PAGE_HOME;
        refreshPage();
        loadArchive(m_strLoadfile);
    } else if (Operation_Extract == m_operationtype) {
        slotextractSelectedFilesTo(m_pUnCompressPage->getDecompressPath());
    } else if (Operation_SingleExtract == m_operationtype) {
    }
}

void MainWindow::slotStopSpinner()
{
    if (pEventloop != nullptr) {
        pEventloop->quit();
    }
    if (m_pSpinner != nullptr) {
        m_pSpinner->stop();
        m_pSpinner->hide();
    }
    if (m_pJob) {
//        ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
//        disconnect(pExtractJob, &ExtractJob::sigExtractSpinnerFinished, this, &MainWindow::slotStopSpinner);
        Job *pJob = dynamic_cast<Job *>(m_pJob);
        disconnect(pJob, &ExtractJob::sigExtractSpinnerFinished, this, &MainWindow::slotStopSpinner);
    } /*else  {

    }*/

}

void MainWindow::slotWorkTimeOut()
{
    qDebug() << "slotWorkTimeOut";
}

void MainWindow::deleteFromArchive(const QStringList &files, const QString &/*archive*/)
{
    if (!m_pUnCompressPage) {
        return;
    }
    Archive::Entry *pEntry = nullptr;
    if (fileViewer *pFViewer = m_pUnCompressPage->getFileViewer()) {
        if (MyTableView *pTableView = pFViewer->getTableView()) {
            if (!pTableView->selectionModel()) {
                return;
            }
            for (const auto &iter :  pTableView->selectionModel()->selectedRows()) {
                pEntry = m_pArchiveModel->entryForIndex(iter);
                break;
            }
        }
    }
    if (!pEntry) {
        return;
    }


    QVector< Archive::Entry * > all_entries;

    foreach (QString file, files) {
        Archive::Entry *entry = new Archive::Entry();
        entry->setFullPath(file);

        QFileInfo fi(file);
        if (fi.isDir()) {
            entry->setIsDirectory(true);
        }

        all_entries.append(entry);
    }

    if (all_entries.isEmpty()) {
        qDebug() << "all_entries.isEmpty()";
        return;
    }

    m_pJob =  m_pArchiveModel->deleteFiles(all_entries);
    if (!m_pJob) {
        return;
    }

    connect(m_pJob, &KJob::result, this, &MainWindow::slotJobFinished, Qt::ConnectionType::UniqueConnection);
    m_ePageID = PAGE_DELETEPROGRESS;
    m_pProgess->settype(Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING);
    m_eJobType = JOB_DELETE;

    m_pJob->start();
    m_eWorkStatus = WorkProcess;
}
/**
 * @brief MainWindow::closeExtractJobSafe
 * @see 安全地退出解压过程并关闭
 */
void MainWindow::closeExtractJobSafe()
{
    slotResetPercentAndTime();
    m_bIsRightMenu = false;
    if (m_pJob && m_pJob->mType == Job::ENUM_JOBTYPE::EXTRACTJOB) {
        if (pEventloop == nullptr) {
            pEventloop = new QEventLoop(this->m_pProgess);
        }
        m_operationtype = Operation_NULL;
        ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
        pExtractJob->archiveInterface()->extractPsdStatus = ReadOnlyArchiveInterface::ExtractPsdStatus::Canceled;
        if (pEventloop->isRunning() == false) {
            connect(pExtractJob, &ExtractJob::sigExtractSpinnerFinished, this, &MainWindow::slotStopSpinner);
//            pEventloop->exec(QEventLoop::ExcludeUserInputEvents);
        }

        killJob();
    }
}

void MainWindow::slotLoadWrongPassWord()
{
    if (Operation_Load == m_operationtype) {
        m_pOpenLoadingPage->stop();
        m_ePageID = PAGE_ENCRYPTION;
        m_pMainLayout->setCurrentIndex(7);
    }

    m_pEncryptionpage->setInputflag(true);
    m_pEncryptionpage->wrongPassWordSlot();
}

//void MainWindow::addToArchive(const QStringList &files, const QString &archive)
//{
//    qDebug() << "执行添加操作" << "向" << archive << "添加文件";
//    if (!m_pCompressSetting) return;
//    if (!m_pArchiveModel) return;


//    //add to source archive
//    qDebug() << "添加路径为：" <<  m_pArchiveModel->archive()->fileName();
//    m_pCompressSetting->autoCompress(m_pArchiveModel->archive()->fileName(), files);

//    //move files to archive
//    m_pCompressSetting->autoMoveToArchive(files, archive);

//}

void MainWindow::onCancelCompressPressed(Progress::ENUM_PROGRESS_TYPE compressType)
{
//    m_compressType = compressType;
    slotResetPercentAndTime();
    m_bIsRightMenu = false;
    m_ePageID = PAGE_UNZIP;
    QString destDirName;
    if (m_pJob && m_pJob->mType == Job::ENUM_JOBTYPE::EXTRACTJOB) { // 解压取消
        ExtractJob *pExtractJob = dynamic_cast<ExtractJob *>(m_pJob);
        destDirName = pExtractJob->archiveInterface()->destDirName;

        //        if (pEventloop == nullptr) {
        //            pEventloop = new QEventLoop(this->m_pProgess);
        //        }

        pExtractJob->archiveInterface()->extractPsdStatus = ReadOnlyArchiveInterface::ExtractPsdStatus::Canceled;
        //        if (pEventloop->isRunning() == false) {
        //            connect(pExtractJob, &ExtractJob::sigExtractSpinnerFinished, this, &MainWindow::slotStopSpinner);
        //            if (m_pSpinner == nullptr) {
        //                m_pSpinner = new DSpinner(this->m_pProgess);
        //                m_pSpinner->setFixedSize(40, 40);
        //            }
        //            m_pSpinner->move(this->m_pProgess->width() / 2 - 20, this->m_pProgess->height() / 2 - 20);
        //            m_pSpinner->hide();
        //            m_pSpinner->start();
        //            m_pSpinner->show();
        //            pEventloop->exec(QEventLoop::ExcludeUserInputEvents);
        //        }
    }

    killJob();

    deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_strPathStore)*/);
    deleteDecompressFile(destDirName);

    if (compressType == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING) {
        m_ePageID = PAGE_ZIP;
    } else if (compressType == Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING) {
        m_ePageID = PAGE_UNZIP;
    } else if (compressType == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD) {
        m_ePageID = PAGE_UNZIP;
    }

    refreshPage();
    // emit sigquitApp();
    slotResetPercentAndTime();
    m_pProgess->setprogress(0);
}

void MainWindow::slotClearTempfile()
{
    m_iOpenTempFileLink = 0;
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-rf");
    args.append(TEMPDIR_NAME);
    p.execute(command, args);
    p.waitForFinished();
}

void MainWindow::slotquitApp()
{
    --m_windowcount;

    if (m_windowcount == 0) {
        QProcess p;
        QString command = "rm";
        QStringList args;
        args.append("-rf");
        args.append(TEMPDIR_NAME);
        p.execute(command, args);
        p.waitForFinished();

        emit sigquitApp();
    }

}

void MainWindow::onUpdateDestFile(QString destFile)
{
    m_pCompressSuccess->setCompressFullPath(destFile);
}

void MainWindow::onCompressPageFilelistIsEmpty()
{
    m_ePageID = PAGE_HOME;
    refreshPage();
}

void MainWindow::slotCalDeleteRefreshTotalFileSize(const QStringList &files)
{
    resetMainwindow();

    calSelectedTotalFileSize(files);
}

//void MainWindow::slotUncompressCalDeleteRefreshTotalFileSize(const QStringList &files)
//{
//    resetMainwindow();

//    calSelectedTotalFileSize(files);

//    removeFromArchive(files);
//}

void MainWindow::slotUncompressCalDeleteRefreshTotoalSize(QVector<Archive::Entry *> &vectorDel, bool isManual)
{
//    resetMainwindow();
////    calSelectedTotalEntrySize(vectorDel);
//    Archive::Entry *pRootEntry = this->m_pArchiveModel->getRootEntry();
//    this->m_pArchiveModel->getRootEntry()->calAllSize(selectedTotalFileSize);//added by hsw for valid total size
    removeEntryVector(vectorDel, isManual);
}

void MainWindow::resetMainwindow()
{
//    selectedTotalFileSize = 0;
//    lastPercent = 0;

#ifdef __aarch64__
    maxFileSize_ = 0;
#endif
    m_pProgess->pInfo()->resetProgress();
    m_pProgess->setprogress(0);
    m_pProgressdialog->setProcess(0);
}

void MainWindow::slotBackButtonClicked()
{
    resetMainwindow();

    slotResetPercentAndTime();
    m_pEncryptionpage->resetPage();
    m_pCompressSuccess->clear();

    if (m_ePageID == PAGE_ZIP_SUCCESS || m_ePageID == PAGE_UNZIP_SUCCESS) {
        m_pCompressPage->clearFiles();
        m_pProgess->setprogress(0);
        m_pProgressdialog->setProcess(0);
    }

    m_ePageID = PAGE_HOME;
    refreshPage();
}

void MainWindow::slotResetPercentAndTime()
{
    m_pProgess->setopentype(false);
    m_pProgess->pInfo()->resetProgress();
}

void MainWindow::slotFileUnreadable(QStringList &pathList, int fileIndex)
{
    pathList.removeAt(fileIndex);
    if (m_ePageID != PAGE_ZIP) {
        m_ePageID = PAGE_ZIP;
        refreshPage();
    }
    m_pCompressPage->onRefreshFilelist(pathList);
    if (pathList.isEmpty()) {
        m_ePageID = PAGE_HOME;
        refreshPage();
    }
}

void MainWindow::onTitleButtonPressed()
{
    switch (m_ePageID) {
    case PAGE_ZIP:
        emit sigZipAddFile();
        break;
    case PAGE_ZIPSET:
        emit sigZipReturn();
        m_pCompressSetting->clickTitleBtnResetAdvancedOptions();
        m_ePageID = PAGE_ZIP;
        refreshPage();
        break;
    case PAGE_ZIP_SUCCESS:
    case PAGE_ZIP_FAIL:
        m_pCompressSuccess->clear();
        m_ePageID = PAGE_ZIP;
        refreshPage();
        break;
    case PAGE_UNZIP:
        //addArchive();
        emit sigCompressedAddFile();
        break;
    case PAGE_UNZIP_SUCCESS:
    case PAGE_UNZIP_FAIL:
        m_pCompressSuccess->clear();

        if (m_pUnCompressPage->getFileCount() < 1) {
            m_ePageID = PAGE_HOME;
        } else {
            m_ePageID = PAGE_UNZIP;
        }
        refreshPage();
        break;
    default:
        break;
    }
    return;
}

void MainWindow::onCompressAddfileSlot(bool status)
{
    if (false == status) {
        setTitleButtonStyle(false);
        m_pOpenAction->setEnabled(false);
        //        setAcceptDrops(false);
    } else {
        setTitleButtonStyle(true, DStyle::StandardPixmap::SP_IncreaseElement);
        m_pOpenAction->setEnabled(true);
        //        setAcceptDrops(true);
    }
}

bool MainWindow::checkSettings(QString file)
{
    QString fileMime = Utils::judgeFileMime(file);
    bool hasSetting = true;

    bool existMime;
    if (fileMime.size() == 0) {
        existMime = true;
    } else {
        existMime = Utils::existMimeType(fileMime);
    }

    if (existMime) {
        QString defaultCompress = getDefaultApp(fileMime);

        if (defaultCompress.startsWith("dde-open.desktop")) {
            setDefaultApp(fileMime, "deepin-compressor.desktop");
        }
    } else {
        QString defaultCompress = getDefaultApp(fileMime);
        if (defaultCompress.startsWith("deepin-compressor.desktop")) {
            setDefaultApp(fileMime, "dde-open.desktop");
        }

        int re = promptDialog();
        if (re != 1) {
            hasSetting = false;
        }
    }

    return hasSetting;
}

QString MainWindow::getDefaultApp(QString mimetype)
{
    QString outInfo;
    QProcess p;
    QString command3 = "xdg-mime query default %1";
    p.start(command3.arg("application/" + mimetype));
    p.waitForFinished();
    outInfo = QString::fromLocal8Bit(p.readAllStandardOutput());

    return  outInfo;
}

void MainWindow::setDefaultApp(QString mimetype, QString desktop)
{
    QProcess p;
    QString command3 = "xdg-mime default %1 %2";
    p.start(command3.arg(desktop).arg("application/" + mimetype));
    p.waitForFinished();
}

int MainWindow::promptDialog()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect =  screen->availableVirtualGeometry();

    DDialog *dialog = new DDialog(this);
    QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);
    dialog->setMinimumSize(380, 140);
    dialog->addButton(tr("OK"), true, DDialog::ButtonNormal);
    dialog->move(((screenRect.width() / 2) - (dialog->width() / 2)), ((screenRect.height() / 2) - (dialog->height() / 2)));
    DLabel *pContent = new DLabel(tr("Please open the Archive Manager and set the file association type"), dialog);

    pContent->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(pContent);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ButtonText));
    DFontSizeManager::instance()->bind(pContent, DFontSizeManager::T6, QFont::Medium);
    pContent->setMinimumSize(293, 20);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(pContent, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(15);

    DWidget *widget = new DWidget(dialog);
    widget->setLayout(mainlayout);
    dialog->addContent(widget);
    int res = dialog->exec();
    SAFE_DELETE_ELE(dialog);

    return res;
}

void MainWindow::setTitleButtonStyle(bool bVisible, DStyle::StandardPixmap pixmap)
{
    m_pTitleButton->setVisible(bVisible);

    if (bVisible)
        m_pTitleButton->setIcon(pixmap);
}

void MainWindow::unzipSuccessOpenFileDir()
{
    if (m_bIsRightMenu) {
        if (m_pSettingsDialog->isAutoOpen()) {
            m_pCompressSuccess->showfiledirSlot(false);
        }
        //slotquitApp();
        close();
        return;
    } else {
        if (m_pSettingsDialog->isAutoOpen() && m_operationtype != Operation_NULL) {
            m_pCompressSuccess->showfiledirSlot(false);
        }
    }
}

void MainWindow::killJob()
{
    if (m_pJob) {
        m_pJob->kill();
        m_pJob = nullptr;
    }
}

void MainWindow::slotKillShowFoldItem()
{
    if (m_DesktopServicesThread) {
        delete  m_DesktopServicesThread;
        m_DesktopServicesThread = nullptr;
    }
}
void MainWindow::deleteLaterJob()
{
    if (m_pJob) {
        m_pJob->deleteLater();
        m_pJob = nullptr;
    }
}

void MainWindow::safeDelete()
{
    //    Archive::Entry *pRootEntry = m_pArchiveModel->getRootEntry();
    //    if (pRootEntry) {
    //        pRootEntry->clean();
    //    }

    //    SAFE_DELETE_ELE(pRootEntry);
    SAFE_DELETE_ELE(m_pFileWatcher);
    SAFE_DELETE_ELE(pEventloop);
    SAFE_DELETE_ELE(m_pSpinner);
    SAFE_DELETE_ELE(m_pWatcher);
    SAFE_DELETE_ELE(m_pArchiveModel);
    //    SAFE_DELETE_ELE(m_logo);
    //    SAFE_DELETE_ELE(m_titleFrame);
    //    SAFE_DELETE_ELE(m_titlelabel);
    SAFE_DELETE_ELE(m_pUnCompressPage);
    SAFE_DELETE_ELE(m_pCompressPage);
    //    SAFE_DELETE_ELE(m_pMainLayout);
    SAFE_DELETE_ELE(m_pHomePage);
    SAFE_DELETE_ELE(m_pCompressSetting);
    //    SAFE_DELETE_ELE(m_pProgess);
    SAFE_DELETE_ELE(m_pCompressSuccess);
    SAFE_DELETE_ELE(m_pCompressFail);
    SAFE_DELETE_ELE(m_pEncryptionpage);
    SAFE_DELETE_ELE(m_pProgressdialog);
    SAFE_DELETE_ELE(m_pSettingsDialog);
    SAFE_DELETE_ELE(m_pOpenLoadingPage);
    //SAFE_DELETE_ELE(m_encodingpage);
    SAFE_DELETE_ELE(m_pSettings);
    SAFE_DELETE_ELE(m_pMmainWidget);
}
