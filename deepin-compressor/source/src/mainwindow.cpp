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
#include "pluginmanager.h"
#include "utils.h"
#include <DDesktopServices>
#include <DMessageManager>
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
//#include "archivejob.h"
#include "jobs.h"
#include "kprocess.h"
#include <DStandardPaths>
#include <QStackedLayout>

#define DEFAUTL_PATH DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles"+ QDir::separator()


QString MainWindow::m_loadfile;

MainWindow::MainWindow(QWidget *parent) : DMainWindow(parent)
{
    m_model = new ArchiveModel(this);
    m_filterModel = new ArchiveSortFilterModel(this);

    m_mainWidget = new DWidget(this);
    m_mainLayout = new QStackedLayout(m_mainWidget);
    m_homePage = new HomePage(this);
    m_mainLayout->addWidget(m_homePage);
    m_homePage->setAutoFillBackground(true);

    // init window flags.
    setWindowTitle(tr("Archive Manager"));
    // setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setCentralWidget(m_mainWidget);
    setAcceptDrops(true);

    initTitleBar();

    m_startTimer = startTimer(500);

    loadWindowState();
}

MainWindow::~MainWindow()
{
    saveWindowState();
}

qint64 MainWindow::getMediaFreeSpace()
{
    QList< QStorageInfo > list = QStorageInfo::mountedVolumes();
    qDebug() << "Volume Num: " << list.size();
    for (QStorageInfo &si : list) {
        qDebug() << si.displayName();
        if (si.displayName().count() > 7 && si.displayName().left(6) == "/media") {
            qDebug() << "Bytes Avaliable: " << si.bytesAvailable() / 1024 / 1024 << "MB";
            return si.bytesAvailable() / 1024 / 1024;
        }
    }

    return 0;
}

bool MainWindow::applicationQuit()
{
    if (PAGE_ZIPPROGRESS == m_mainLayout->currentIndex()) {
        if (1 != m_Progess->showConfirmDialog()) {
            return false;
        }

        deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_pathstore)*/);

        if (m_encryptionjob) {
            m_encryptionjob->Killjob();
            m_encryptionjob = nullptr;
        }
        deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_pathstore)*/);
        if (m_createJob) {
            m_createJob->kill();
            m_createJob = nullptr;
        }
    } else if (7 == m_mainLayout->currentIndex()) {
        deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_pathstore)*/);
    }

    return true;
}

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

QString MainWindow::getLoadFile()
{
    return m_loadfile;
}

qint64 MainWindow::getDiskFreeSpace()
{
    QStorageInfo storage = QStorageInfo::root();
    storage.refresh();
    qDebug() << "availableSize:" << storage.bytesAvailable() / 1024 / 1024 << "MB";
    return storage.bytesAvailable() / 1024 / 1024;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (PAGE_ZIPPROGRESS == m_mainLayout->currentIndex()) {
        if (1 != m_Progess->showConfirmDialog()) {
            event->ignore();
            return;
        }

        deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_pathstore)*/);
        event->accept();

        if (m_encryptionjob) {
            m_encryptionjob->deleteLater();
            m_encryptionjob = nullptr;
        }
        deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_pathstore)*/);
        if (m_createJob) {
            m_createJob->kill();
            m_createJob = nullptr;
        }

        emit sigquitApp();
    } else if (7 == m_mainLayout->currentIndex()) {
        deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_pathstore)*/);
        event->accept();
        slotquitApp();
    } else {
        event->accept();
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
    if (m_startTimer == event->timerId()) {
        if (!m_initflag) {
            InitUI();
            InitConnection();
            m_initflag = true;
        }

        killTimer(m_startTimer);
        m_startTimer = 0;
    } else if (m_watchTimer == event->timerId()) {
        QStringList filelist = m_CompressPage->getCompressFilelist();
        for (int i = 0; i < filelist.count(); i++) {
            QFileInfo filein(filelist.at(i));
            if (!filein.exists()) {
//                DDialog *dialog = new DDialog(this);
//                dialog->setFixedWidth(440);
//                QIcon icon = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
//                dialog->setIcon(icon);
//                dialog->setMessage(tr("%1 was changed on the disk, please import it again.").arg(filein.fileName()));
//                dialog->addButton(tr("OK"), true, DDialog::ButtonRecommend);
//                QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
//                effect->setOffset(0, 4);
//                effect->setColor(QColor(0, 145, 255, 76));
//                effect->setBlurRadius(4);
//                dialog->getButton(0)->setFixedWidth(340);
//                dialog->getButton(0)->setGraphicsEffect(effect);
//                dialog->exec();
                QString displayName = Utils::toShortString(filein.fileName());
                QString strTips = tr("%1 was changed on the disk, please import it again.").arg(displayName);
                DDialog *dialog = new DDialog(this);
                QPixmap pixmap = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
                dialog->setIcon(pixmap);
                dialog->addSpacing(32);
                dialog->setFixedWidth(440);
                dialog->addButton(tr("OK"), true, DDialog::ButtonNormal);
//                QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
//                effect->setOffset(0, 4);
//                effect->setColor(QColor(0, 145, 255, 76));
//                effect->setBlurRadius(4);
                dialog->getButton(0)->setFixedWidth(340);
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

                filelist.removeAt(i);
                if (m_pageid != PAGE_ZIP) {
                    m_pageid = PAGE_ZIP;
                    refreshPage();
                }
                m_CompressPage->onRefreshFilelist(filelist);
                if (filelist.isEmpty()) {
                    m_pageid = PAGE_HOME;
                    refreshPage();
                }
                delete dialog;
            }
        }
    }
}

void MainWindow::InitUI()
{
    m_UnCompressPage = new UnCompressPage(this);
    m_CompressPage = new CompressPage(this);
    m_CompressSetting = new CompressSetting(this);
    m_Progess = new Progress(this);
    m_CompressSuccess = new Compressor_Success(this);
    m_CompressFail = new Compressor_Fail(this);
    m_encryptionpage = new EncryptionPage(this);
    m_progressdialog = new ProgressDialog(this);
    m_settingsDialog = new SettingDialog(this);
    m_encodingpage = new EncodingPage(this);
    m_settings = new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"), QSettings::IniFormat);

    if (m_settings->value("dir").toString().isEmpty()) {
        m_settings->setValue("dir", "");
    }

    // add widget to main layout.
    m_mainLayout->addWidget(m_UnCompressPage);
    m_mainLayout->addWidget(m_CompressPage);
    m_mainLayout->addWidget(m_CompressSetting);
    m_mainLayout->addWidget(m_Progess);
    m_mainLayout->addWidget(m_CompressSuccess);
    m_mainLayout->addWidget(m_CompressFail);
    m_mainLayout->addWidget(m_encryptionpage);
    m_mainLayout->addWidget(m_encodingpage);
    m_UnCompressPage->setAutoFillBackground(true);
    m_CompressPage->setAutoFillBackground(true);
    m_CompressSetting->setAutoFillBackground(true);
    m_Progess->setAutoFillBackground(true);
    m_CompressSuccess->setAutoFillBackground(true);
    m_CompressFail->setAutoFillBackground(true);
    m_encryptionpage->setAutoFillBackground(true);
    m_encodingpage->setAutoFillBackground(true);
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
    connect(m_homePage, &HomePage::fileSelected, this, &MainWindow::onSelected);
    connect(m_CompressPage, &CompressPage::sigFilelistIsEmpty, this, &MainWindow::onCompressPageFilelistIsEmpty);
    connect(m_CompressPage, &CompressPage::sigselectedFiles, this, &MainWindow::onSelected);
    connect(m_CompressPage, &CompressPage::sigRefreshFileList, this, &MainWindow::slotCalDeleteRefreshTotalFileSize);
    connect(m_CompressPage, &CompressPage::sigNextPress, this, &MainWindow::onCompressNext);
    connect(this, &MainWindow::sigZipAddFile, m_CompressPage, &CompressPage::onAddfileSlot);
    connect(m_CompressSetting, &CompressSetting::sigCompressPressed, this, &MainWindow::onCompressPressed);
    connect(m_CompressSetting, &CompressSetting::sigFileUnreadable, this, &MainWindow::slotFileUnreadable);
    connect(m_Progess, &Progress::sigCancelPressed, this, &MainWindow::onCancelCompressPressed);
    connect(m_CompressSuccess, &Compressor_Success::sigQuitApp, this, &MainWindow::slotquitApp);
    connect(m_CompressSuccess, &Compressor_Success::sigBackButtonClicked, this, &MainWindow::slotBackButtonClicked);
    connect(m_titlebutton, &DPushButton::clicked, this, &MainWindow::onTitleButtonPressed);
    connect(this, &MainWindow::sigZipSelectedFiles, m_CompressPage, &CompressPage::onSelectedFilesSlot);
    connect(m_model, &ArchiveModel::loadingFinished, this, &MainWindow::slotLoadingFinished);
    connect(m_UnCompressPage, &UnCompressPage::sigDecompressPress, this, &MainWindow::slotextractSelectedFilesTo);
    connect(m_encryptionpage, &EncryptionPage::sigExtractPassword, this, &MainWindow::SlotExtractPassword);
    connect(m_UnCompressPage, &UnCompressPage::sigextractfiles, this, &MainWindow::slotExtractSimpleFiles);
    connect(m_UnCompressPage, &UnCompressPage::sigOpenExtractFile, this, &MainWindow::slotExtractSimpleFilesOpen);
    connect(m_progressdialog, &ProgressDialog::stopExtract, this, &MainWindow::slotKillExtractJob);
    connect(m_progressdialog, &ProgressDialog::sigResetPercentAndTime, this, &MainWindow::slotResetPercentAndTime);
    connect(m_CompressFail, &Compressor_Fail::sigFailRetry, this, &MainWindow::slotFailRetry);
    connect(m_CompressFail, &Compressor_Fail::sigBackButtonClickedOnFail, this, &MainWindow::slotBackButtonClicked);
    connect(m_CompressPage, &CompressPage::sigiscanaddfile, this, &MainWindow::onCompressAddfileSlot);
    connect(m_progressdialog, &ProgressDialog::extractSuccess, this, [ = ](QString msg) {
        QIcon icon = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
        this->sendMessage(icon, msg);
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
}

QMenu *MainWindow::createSettingsMenu()
{
    QMenu *menu = new QMenu();

    m_openAction = menu->addAction(tr("Open file"));
    connect(m_openAction, &QAction::triggered, this, [this] {
        DFileDialog dialog(this);
        dialog.setAcceptMode(DFileDialog::AcceptOpen);
        dialog.setFileMode(DFileDialog::ExistingFiles);
        dialog.setAllowMixedSelection(true);

        QString historyDir = m_settings->value("dir").toString();
        if (historyDir.isEmpty())
        {
            historyDir = QDir::homePath();
        }
        dialog.setDirectory(historyDir);

        const int mode = dialog.exec();

        // save the directory string to config file.
        m_settings->setValue("dir", dialog.directoryUrl().toLocalFile());

        // if click cancel button or close button.
        if (mode != QDialog::Accepted)
        {
            return;
        }

        onSelected(dialog.selectedFiles());
    });

    // menu->insertAction();

    QAction *settingsAction = menu->addAction(tr("Settings"));
    connect(settingsAction, &QAction::triggered, this, [this] { m_settingsDialog->exec(); });

    menu->addSeparator();

    return menu;
}

void MainWindow::initTitleBar()
{
    titlebar()->setMenu(createSettingsMenu());
    titlebar()->setFixedHeight(50);

    QIcon icon = QIcon::fromTheme("deepin-compressor");
    m_logo = new DLabel("", this);
    m_logo->setPixmap(icon.pixmap(QSize(30, 30)));

    m_titlebutton = new DIconButton(DStyle::StandardPixmap::SP_IncreaseElement, this);
    m_titlebutton->setFixedSize(36, 36);
    m_titlebutton->setVisible(false);

    m_titleFrame = new QFrame(this);
    m_titleFrame->setObjectName("TitleBar");
    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->addSpacing(6);
    leftLayout->addWidget(m_logo);
    leftLayout->addSpacing(6);
    leftLayout->addWidget(m_titlebutton);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *left_frame = new QFrame(this);
    left_frame->setFixedWidth(10 + 6 + 36 + 30);
    left_frame->setContentsMargins(0, 0, 0, 0);
    left_frame->setLayout(leftLayout);

    m_titlelabel = new DLabel(this);
    m_titlelabel->setMinimumSize(315, TITLE_FIXED_HEIGHT);
    // m_titlelabel->setMinimumHeight(TITLE_FIXED_HEIGHT);
    m_titlelabel->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(m_titlelabel, DFontSizeManager::T6, QFont::Medium);

    m_titlelabel->setForegroundRole(DPalette::WindowText);

    QHBoxLayout *titlemainLayout = new QHBoxLayout;
    titlemainLayout->setContentsMargins(0, 0, 0, 0);
    titlemainLayout->addWidget(left_frame);
    titlemainLayout->addSpacing(5);
    titlemainLayout->addWidget(m_titlelabel, 0, Qt::AlignCenter);

    m_titleFrame->setLayout(titlemainLayout);
    m_titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);
    titlebar()->setContentsMargins(0, 0, 0, 0);
    titlebar()->setCustomWidget(m_titleFrame, false);

    // m_titlelabel->setText( tr("%1 task(s) in progress").arg(1) );
}

void MainWindow::setQLabelText(QLabel *label, const QString &text)
{
    QFontMetrics cs(label->font());
    int textWidth = cs.width(text);
    if (textWidth > label->width()) {
        label->setToolTip(text);
    } else {
        label->setToolTip("");
    }

    QFontMetrics elideFont(label->font());
    label->setText(elideFont.elidedText(text, Qt::ElideMiddle, label->width()));
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

    if (false == mime->hasUrls()) {
        return e->ignore();
    }

    e->accept();

    // find font files.
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

    m_homePage->setIconPixmap(false);
    onSelected(fileList);
    //    onRightMenuSelected(fileList);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
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
    titlebar()->setDisableFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint
                                | Qt::WindowSystemMenuHint);
}

void MainWindow::refreshPage()
{
    //    m_openAction->setEnabled(false);
    //    setAcceptDrops(false);
    //    m_titlebutton->setVisible(false);
    qDebug() << m_pageid;

    m_encryptionpage->resetPage();

    switch (m_pageid) {
    case PAGE_HOME:

        if (m_fileManager) {
            delete m_fileManager;
            m_fileManager = nullptr;
        }
        m_Progess->resetProgress();
        m_openAction->setEnabled(true);
        setAcceptDrops(true);
        m_titlebutton->setVisible(false);
        setQLabelText(m_titlelabel, "");
        m_mainLayout->setCurrentIndex(0);
        break;
    case PAGE_UNZIP:
        m_Progess->resetProgress();
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        setQLabelText(m_titlelabel, m_decompressfilename);
        m_mainLayout->setCurrentIndex(1);
        break;
    case PAGE_ZIP:
        m_Progess->resetProgress();
        setQLabelText(m_titlelabel, tr("Create New Archive"));
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_IncreaseElement);
        m_openAction->setEnabled(true);
        m_titlebutton->setVisible(true);
        setAcceptDrops(true);
        m_watchTimer = startTimer(1000);
        m_CompressPage->onPathIndexChanged();
        m_mainLayout->setCurrentIndex(2);
        break;
    case PAGE_ZIPSET:
        setQLabelText(m_titlelabel, tr("Create New Archive"));
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(true);
        m_mainLayout->setCurrentIndex(3);
        break;
    case PAGE_ZIPPROGRESS:
        if (0 != m_watchTimer) {
            killTimer(m_watchTimer);
            m_watchTimer = 0;
        }
        m_Progess->setSpeedAndTimeText(COMPRESSING);
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        setQLabelText(m_titlelabel, tr("Compressing"));
        m_Progess->setFilename(m_decompressfilename);
        m_mainLayout->setCurrentIndex(4);
        m_timer.start();
        break;
    case PAGE_UNZIPPROGRESS:
        m_Progess->setSpeedAndTimeText(DECOMPRESSING);
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        setQLabelText(m_titlelabel, tr("Extracting"));
        m_Progess->setFilename(m_decompressfilename);
        m_mainLayout->setCurrentIndex(4);
        m_timer.start();
        break;
    case PAGE_ZIP_SUCCESS:
        setQLabelText(m_titlelabel, "");
        m_CompressSuccess->setstringinfo(tr("Compression successful"));
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        m_mainLayout->setCurrentIndex(5);
        break;
    case PAGE_ZIP_FAIL:
        setQLabelText(m_titlelabel, "");
        m_CompressFail->setFailStr(tr("Compression failed"));
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        m_mainLayout->setCurrentIndex(6);
        break;
    case PAGE_UNZIP_SUCCESS:
        setQLabelText(m_titlelabel, "");
        m_CompressSuccess->setCompressPath(m_decompressfilepath);
        //m_CompressSuccess->setstringinfo(tr("Extraction successful"));
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        if (m_settingsDialog->isAutoOpen()) {
            DDesktopServices::showFolder(QUrl(m_decompressfilepath, QUrl::TolerantMode));
        }

        if (m_isrightmenu) {
            m_CompressSuccess->showfiledirSlot();
            slotquitApp();
            return;
        }

        m_mainLayout->setCurrentIndex(5);
        break;
    case PAGE_UNZIP_FAIL:
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
        setQLabelText(m_titlelabel, "");
        m_CompressFail->setFailStr(tr("Extraction failed"));
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        m_mainLayout->setCurrentIndex(6);
        break;
    case PAGE_ENCRYPTION:
        setQLabelText(m_titlelabel, m_decompressfilename);
        m_openAction->setEnabled(false);
        setAcceptDrops(false);
        m_titlebutton->setVisible(false);
        if (m_progressdialog->isshown()) {
            // m_progressdialog->reject();
            m_progressdialog->hide();
            m_progressdialog->m_extractdialog->reject();
        }
        //        m_progressdialog->reject();
        //        m_progressdialog->m_extractdialog->reject();
        m_mainLayout->setCurrentIndex(7);
        m_encryptionpage->setPassowrdFocus();
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

        if (fi.isFile()) {
            qint64 curFileSize = fi.size();

#ifdef __aarch64__
            if (maxFileSize_ < curFileSize) {
                maxFileSize_ = curFileSize;
            }
#endif

            selectedTotalFileSize += curFileSize;
        } else if (fi.isDir()) {
            selectedTotalFileSize += calFileSize(file);
        }
    }
    m_CompressSetting->getSelectedFileSize(selectedTotalFileSize);
}

qint64 MainWindow::calFileSize(const QString &path)
{
    QDir dir(path);
    qint64 size = 0;

    foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::Files)) {
        qint64 curFileSize = fileInfo.size();

#ifdef __aarch64__
        if (maxFileSize_ < curFileSize) {
            maxFileSize_ = curFileSize;
        }
#endif

        size += curFileSize;
    }

    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        size += calFileSize(path + QDir::separator() + subDir);
    }

    return size;
}

void MainWindow::calSpeedAndTime(unsigned long compressPercent)
{
    qDebug() << "size" << selectedTotalFileSize;
    compressTime += m_timer.elapsed();
    qDebug() << "compresstime" << compressTime;

    double m_compressSpeed = ((selectedTotalFileSize / 1024.0) * (compressPercent / 100.0)) / compressTime * 1000;
    double m_sizeLeft = (selectedTotalFileSize / 1024.0) * (100 - compressPercent) / 100;
    qint64 m_timeLeft = (qint64)(m_sizeLeft / m_compressSpeed);

    qDebug() << "m_sizeLeft" << m_sizeLeft;
    qDebug() << "m_compressSpeed" << m_compressSpeed;
    qDebug() << "m_timeLeft" << m_timeLeft;

//    if (lastPercent != 100 && m_timeLeft == 0)
//    {
//        m_timeLeft = 1;
//    }

    m_Progess->setSpeedAndTime(m_compressSpeed, m_timeLeft);
    m_timer.restart();
}

void MainWindow::onSelected(const QStringList &files)
{
    calSelectedTotalFileSize(files);

    if (files.count() == 1 && Utils::isCompressed_file(files.at(0))) {
        if (0 == m_CompressPage->getCompressFilelist().count()) {
            QString filename;
            filename = files.at(0);

            //            if (filename.contains(".7z.")) {
            //                filename = filename.left(filename.length() - 3) + "001";
            //            }

            transSplitFileName(filename);

            QFileInfo fileinfo(filename);
            m_decompressfilename = fileinfo.fileName();
            if ("" != m_settingsDialog->getCurExtractPath()) {
                m_UnCompressPage->setdefaultpath(m_settingsDialog->getCurExtractPath());
            } else {
                m_UnCompressPage->setdefaultpath(fileinfo.path());
            }

            loadArchive(filename);
        } else {
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
            delete dialog;
            qDebug() << mode;
            if (1 == mode) {
                emit sigZipSelectedFiles(files);
            } else if (2 == mode) {
                KProcess *cmdprocess = new KProcess(this);
                QStringList arguments;

                QString programPath = QStandardPaths::findExecutable("deepin-compressor");
                if (programPath.isEmpty()) {
                    qDebug() << "error can't find xdg-mime";
                    return;
                }

                arguments << files.at(0);

                qDebug() << arguments;

                cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
                cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
                cmdprocess->setProgram(programPath, arguments);
                cmdprocess->start();
            }
        }
    } else {
        m_pageid = PAGE_ZIP;
        emit sigZipSelectedFiles(files);
        refreshPage();
    }
}

void MainWindow::onRightMenuSelected(const QStringList &files)
{
    if (!m_initflag) {
        InitUI();
        InitConnection();
        m_initflag = true;
    }

    calSelectedTotalFileSize(files);

    if (files.last() == QStringLiteral("extract_here")) {
        m_isrightmenu = true;
        QFileInfo fileinfo(files.at(0));
        m_decompressfilename = fileinfo.fileName();
        m_UnCompressPage->setdefaultpath(fileinfo.path());
        loadArchive(files.at(0));
        m_pageid = PAGE_UNZIPPROGRESS;
        m_Progess->settype(DECOMPRESSING);
        refreshPage();
    } else if (files.last() == QStringLiteral("extract_here_multi")) {
        QStringList pathlist = files;
        pathlist.removeLast();
        QFileInfo fileinfo(pathlist.at(0));
        m_decompressfilename = fileinfo.fileName();
        m_UnCompressPage->setdefaultpath(fileinfo.path());
        m_decompressfilepath = fileinfo.path();
        m_pageid = PAGE_UNZIPPROGRESS;
        m_Progess->settype(DECOMPRESSING);
        refreshPage();

        BatchExtract *batchJob = new BatchExtract();
        batchJob->setAutoSubfolder(true);
        batchJob->setDestinationFolder(fileinfo.path());
        batchJob->setPreservePaths(true);

        for (const QString &url : qAsConst(pathlist)) {
            batchJob->addInput(QUrl::fromLocalFile(url));
        }

        connect(batchJob, SIGNAL(batchProgress(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
        connect(batchJob, &KJob::result, this, &MainWindow::slotExtractionDone);
        connect(batchJob, &BatchExtract::sendCurFile, this, &MainWindow::slotBatchExtractFileChanged);
        connect(batchJob, &BatchExtract::sendFailFile, this, &MainWindow::slotBatchExtractError);
        //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
        //                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
        //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
        //                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(batchJob,
                SIGNAL(batchFilenameProgress(KJob *, const QString &)),
                this,
                SLOT(SlotProgressFile(KJob *, const QString &)));
        //        connect(batchJob, &BatchExtract::sigCancelled,
        //                this, &MainWindow::sigquitApp);

        qDebug() << "Starting job";
        batchJob->start();
    } else if (files.last() == QStringLiteral("extract")) {
        QFileInfo fileinfo(files.at(0));
        m_decompressfilename = fileinfo.fileName();
        if ("" != m_settingsDialog->getCurExtractPath()) {
            m_UnCompressPage->setdefaultpath(m_settingsDialog->getCurExtractPath());
        } else {
            m_UnCompressPage->setdefaultpath(fileinfo.path());
        }

        loadArchive(files.at(0));
    } else if (files.last() == QStringLiteral("extract_multi")) {
        QString defaultpath;
        QFileInfo fileinfo(files.at(0));
        if ("" != m_settingsDialog->getCurExtractPath()) {
            defaultpath = m_settingsDialog->getCurExtractPath();
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
            QTimer::singleShot(100, this, [ = ] { slotquitApp(); });
            return;
        }

        QList< QUrl > selectpath = dialog.selectedUrls();
        qDebug() << selectpath;
        QString curpath = selectpath.at(0).toLocalFile();

        QStringList pathlist = files;
        pathlist.removeLast();
        m_decompressfilename = fileinfo.fileName();
        m_UnCompressPage->setdefaultpath(curpath);
        m_decompressfilepath = curpath;
        m_pageid = PAGE_UNZIPPROGRESS;
        m_Progess->settype(DECOMPRESSING);
        refreshPage();

        BatchExtract *batchJob = new BatchExtract();
        batchJob->setAutoSubfolder(true);
        batchJob->setDestinationFolder(curpath);
        batchJob->setPreservePaths(true);

        for (const QString &url : qAsConst(pathlist)) {
            batchJob->addInput(QUrl::fromLocalFile(url));
        }

        connect(batchJob, SIGNAL(batchProgress(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
        connect(batchJob, &KJob::result, this, &MainWindow::slotExtractionDone);
        connect(batchJob, &BatchExtract::sendCurFile, this, &MainWindow::slotBatchExtractFileChanged);
        connect(batchJob, &BatchExtract::sendFailFile, this, &MainWindow::slotBatchExtractError);
        //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
        //                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
        //        connect(batchJob, &BatchExtract::sigExtractJobPassword,
        //                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
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
        m_pageid = PAGE_ZIPSET;
        setCompressDefaultPath();
        refreshPage();
    } else if (files.last() == QStringLiteral("extract_here_split")) {
        if (files.at(0).contains(".7z.")) {
            QString filepath = files.at(0);

            transSplitFileName(filepath);

            QFileInfo fileinfo(filepath);

            if (fileinfo.exists()) {
                m_isrightmenu = true;
                QFileInfo fileinfo(files.at(0));
                m_decompressfilename = fileinfo.fileName();
                m_UnCompressPage->setdefaultpath(fileinfo.path());
                loadArchive(filepath);
                m_pageid = PAGE_UNZIPPROGRESS;
                m_Progess->settype(DECOMPRESSING);
                refreshPage();
            } else {
                m_CompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        }
    } else if (files.last() == QStringLiteral("extract_split")) {
        QString filepath = files.at(0);
        filepath = filepath.left(filepath.length() - 3) + "001";
        QFileInfo fileinfo(filepath);

        if (fileinfo.exists()) {
            m_decompressfilename = fileinfo.fileName();
            if ("" != m_settingsDialog->getCurExtractPath()) {
                m_UnCompressPage->setdefaultpath(m_settingsDialog->getCurExtractPath());
            } else {
                m_UnCompressPage->setdefaultpath(fileinfo.path());
            }

            loadArchive(filepath);
        } else {
            m_CompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
            m_pageid = PAGE_UNZIP_FAIL;
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
        m_decompressfilename = fileinfo.fileName();
        if ("" != m_settingsDialog->getCurExtractPath()) {
            m_UnCompressPage->setdefaultpath(m_settingsDialog->getCurExtractPath());
        } else {
            m_UnCompressPage->setdefaultpath(fileinfo.path());
        }

        loadArchive(filename);
    } else {
        emit sigZipSelectedFiles(files);
        m_pageid = PAGE_ZIPSET;
        setCompressDefaultPath();
        refreshPage();
    }
}

void MainWindow::slotLoadingFinished(KJob *job)
{
    m_homePage->spinnerStop();
    m_workstatus = WorkNone;
    if (job->error()) {
        m_CompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
        m_pageid = PAGE_UNZIP_FAIL;
        refreshPage();
        return;
    }

    m_filterModel->setSourceModel(m_model);
    m_filterModel->setFilterKeyColumn(0);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_UnCompressPage->setModel(m_filterModel);

    if (!m_isrightmenu) {
        m_pageid = PAGE_UNZIP;
        refreshPage();
    } else {
        slotextractSelectedFilesTo(m_UnCompressPage->getDecompressPath());
    }
}

void MainWindow::loadArchive(const QString &files)
{
    QString transFile = files;
    transSplitFileName(transFile);

    WatcherFile(transFile);

    m_workstatus = WorkProcess;
    m_loadfile = transFile;
    m_encryptiontype = Encryption_Load;
    m_loadjob = dynamic_cast< LoadJob * >(m_model->loadArchive(transFile, "", m_model));

    if (m_loadjob == nullptr) {
        return;
    }

    m_Progess->settype(DECOMPRESSING);

    connect(m_loadjob, &LoadJob::sigLodJobPassword, this, &MainWindow::SlotNeedPassword);
    connect(m_loadjob, &LoadJob::sigWrongPassword, this, &MainWindow::SlotNeedPassword);

    m_loadjob->start();
    m_homePage->spinnerStart();
}

void MainWindow::WatcherFile(const QString &files)
{
    if (m_fileManager) {
        delete m_fileManager;
        m_fileManager = nullptr;
    }

    m_fileManager = new DFileWatcher(files, this);
    m_fileManager->startWatcher();
    qDebug() << m_fileManager->startWatcher() << "=" << files;
    connect(m_fileManager, &DFileWatcher::fileMoved, this, [ = ]() { //监控压缩包，重命名时提示
        DDialog *dialog = new DDialog(this);
        dialog->setFixedWidth(440);
        QIcon icon = Utils::renderSVG(":/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
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
        delete dialog;

        delete m_fileManager;
        m_fileManager = nullptr;

        m_pageid = PAGE_HOME;
        this->refreshPage();
    });
}

void MainWindow::slotextractSelectedFilesTo(const QString &localPath)
{
//    m_pageid = PAGE_UNZIPPROGRESS;
//    refreshPage();
//    m_progressdialog->setProcess(0);
//    m_Progess->setprogress(0);

    m_workstatus = WorkProcess;
    m_encryptiontype = Encryption_Extract;
    if (nullptr == m_model) {
        return;
    }

    if (nullptr == m_model->archive()) {
        return;
    }

    if (m_encryptionjob) {
        m_encryptionjob->deleteLater();
        m_encryptionjob = nullptr;
    }

    ExtractionOptions options;
    QVector< Archive::Entry * > files;

    QString userDestination = localPath;
    QString destinationDirectory;

    m_pathstore = userDestination;
    //m_compressDirFiles = CheckAllFiles(m_pathstore);

    if (m_settingsDialog->isAutoCreatDir()) {
        const QString detectedSubfolder = m_model->archive()->subfolderName();
        qDebug() << "Detected subfolder" << detectedSubfolder;
        if (m_model->archive()->hasMultipleTopLevelEntries()) {
            if (!userDestination.endsWith(QDir::separator())) {
                userDestination.append(QDir::separator());
            }
            destinationDirectory = userDestination + detectedSubfolder;
            QDir(userDestination).mkdir(detectedSubfolder);
            m_CompressSuccess->setCompressNewFullPath(destinationDirectory);
        } else {
            destinationDirectory = userDestination;
        }
    } else {
        destinationDirectory = userDestination;
    }

    qDebug() << "destinationDirectory:" << destinationDirectory;

    m_encryptionjob = m_model->extractFiles(files, destinationDirectory, options);
    m_encryptionjob->archiveInterface()->extractTopFolderName = m_model->archive()->subfolderName();
    connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
    connect(m_encryptionjob, &KJob::result, this, &MainWindow::slotExtractionDone);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword, m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
    //
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
    connect(m_encryptionjob,
            SIGNAL(percentfilename(KJob *, const QString &)),
            this,
            SLOT(SlotProgressFile(KJob *, const QString &)));
    connect(m_encryptionjob, &ExtractJob::sigCancelled, this, &MainWindow::slotClearTempfile);
    connect(m_encryptionjob, &ExtractJob::updateDestFile, this, &MainWindow::onUpdateDestFile);

    m_decompressfilepath = destinationDirectory;

    /*if(m_model->archive()->property("isPasswordProtected").toBool() == true)
    {
        if (PAGE_ENCRYPTION != m_pageid)
        {
            m_pageid = PAGE_ENCRYPTION;
            refreshPage();
        }
        return;
    }*/
    m_encryptionjob->archiveInterface()->destDirName = "";
    m_encryptionjob->start();
}

void MainWindow::SlotProgress(KJob * /*job*/, unsigned long percent)
{
    if (percent > lastPercent) {
        calSpeedAndTime(percent);
        lastPercent = percent;
    }

    qDebug() << "percent" << percent;
    if ((Encryption_SingleExtract == m_encryptiontype)) {
        if (percent < 100 && WorkProcess == m_workstatus) {
            if (!m_progressdialog->isshown()) {
                if (m_pageid != PAGE_UNZIP) {
                    m_pageid = PAGE_UNZIP;
                    refreshPage();
                }
                m_progressdialog->showdialog();
            }
            m_progressdialog->setProcess(percent);
        }
    } else if (PAGE_ZIPPROGRESS == m_pageid || PAGE_UNZIPPROGRESS == m_pageid) {
        m_Progess->setprogress(percent);
    } else if ((PAGE_UNZIP == m_pageid || PAGE_ENCRYPTION == m_pageid) && (percent < 100) && m_encryptionjob) {
        /*if (!m_progressTransFlag) {
            if (0 == m_timerId) {
                m_timerId = startTimer(800);
            }
        } else*/
        {
            m_pageid = PAGE_UNZIPPROGRESS;
            m_Progess->settype(DECOMPRESSING);
            refreshPage();
        }
    } else if ((PAGE_ZIPSET == m_pageid) && (percent < 100)) {
        m_pageid = PAGE_ZIPPROGRESS;
        m_Progess->settype(COMPRESSING);
        refreshPage();
    }
}

void MainWindow::SlotProgressFile(KJob * /*job*/, const QString &filename)
{
    // if (Encryption_SingleExtract == m_encryptiontype && PAGE_UNZIP == m_pageid) {
    m_progressdialog->setCurrentFile(filename);
    //} else if (PAGE_ZIPPROGRESS == m_pageid || PAGE_UNZIPPROGRESS == m_pageid) {
    m_Progess->setProgressFilename(filename);
    //}
}

void MainWindow::slotBatchExtractFileChanged(const QString &name)
{
    qDebug() << name;
    m_Progess->setFilename(name);
}

void MainWindow::slotBatchExtractError(const QString &name)
{
    m_CompressFail->setFailStrDetail(name + ":" + +" " + tr("Wrong password"));
    m_pageid = PAGE_UNZIP_FAIL;
    refreshPage();
}

void MainWindow::slotExtractionDone(KJob *job)
{
    m_workstatus = WorkNone;
    if (m_encryptionjob) {
        m_encryptionjob->deleteLater();
        m_encryptionjob = nullptr;
    }

    int errorCode = job->error();


    if ((PAGE_ENCRYPTION == m_pageid) && (errorCode && (errorCode != KJob::KilledJobError && errorCode != KJob::UserSkiped)))   {

        // do noting:wrong password
    } else if (errorCode && (errorCode != KJob::KilledJobError && errorCode != KJob::UserSkiped)) {
        if (m_progressdialog->isshown()) {
            m_progressdialog->hide();
            // m_progressdialog->reject();
        }

        if (m_pathstore.left(6) == "/media" && getMediaFreeSpace() <= 50) {
            m_CompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
        } else if (getDiskFreeSpace() <= 50) {
            m_CompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
        } else {
            m_CompressFail->setFailStrDetail(tr("Damaged file, unable to extract"));
        }

        m_pageid = PAGE_UNZIP_FAIL;
        refreshPage();
        return;
    } else if (Encryption_TempExtract == m_encryptiontype) {
        KProcess *cmdprocess = new KProcess;
        QStringList arguments;
        QString programPath = QStandardPaths::findExecutable("xdg-open");
        /*for (int i = 0; i < m_extractSimpleFiles.count(); i++)*/
        {
            QFileInfo file = m_extractSimpleFiles.at(0)->name();


            if (file.fileName().contains("%")/* && file.fileName().contains(".png")*/) {

                QProcess p;
//                QString tempFileName = QString("%1.png").arg(openTempFileLink);
                QString tempFileName = QString("%1").arg(openTempFileLink) + "." + file.suffix();
                openTempFileLink++;
                QString commandCreate = "ln";
                QStringList args;
                args.append(DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles"
                            + QDir::separator() + m_extractSimpleFiles.at(0)->name());
                args.append(DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles"
                            + QDir::separator() + tempFileName);
                arguments << DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles"
                          + QDir::separator() + tempFileName;
                p.execute(commandCreate, args);
            } else {
                arguments << DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles"
                          + QDir::separator() + m_extractSimpleFiles.at(0)->name();
            }
        }
        qDebug() << arguments;
        cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
        cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
        cmdprocess->setProgram(programPath, arguments);
        cmdprocess->start();
        m_pageid = PAGE_UNZIP;
        refreshPage();
    } else if (Encryption_SingleExtract == m_encryptiontype) {
        if (errorCode == KJob::UserSkiped) {
            m_isrightmenu = false;
            m_progressdialog->setMsg(tr("Skip all files"));
        } else {
            m_progressdialog->setFinished(m_decompressfilepath);
        }

        DDesktopServices::showFileItem(QUrl(m_decompressfilepath + "/" + m_extractSimpleFiles.at(0)->property("name").toString(), QUrl::TolerantMode));

        m_pageid = PAGE_UNZIP;
        refreshPage();
    }
    //    else if( job->error() && job->error() == KJob::KilledJobError )
    //    {
    //        m_pageid = PAGE_UNZIP;
    //        refreshPage();
    //    }
//    else if (Encryption_TempExtract_Open == m_encryptiontype)
//    {


//        m_pageid = PAGE_UNZIP;
//        refreshPage();

//    }
    else if (Encryption_TempExtract_Open_Choose == m_encryptiontype) {
        QString ppp = program;
        if (program != tr("Choose default programma")) {
            OpenWithDialog::chooseOpen(program, QString(DEFAUTL_PATH) + m_extractSimpleFiles.at(0)->property("name").toString());
        } else {
            OpenWithDialog *dia = new OpenWithDialog(DUrl(QString(DEFAUTL_PATH) + m_extractSimpleFiles.at(0)->property("name").toString()), this);
            dia->exec();
        }

        m_pageid = PAGE_UNZIP;
        refreshPage();

    }

    else {
        m_pageid = PAGE_UNZIP_SUCCESS;
        if (errorCode == KJob::UserSkiped) {
            m_isrightmenu = false;
            m_CompressSuccess->setstringinfo(tr("Skip all files"));
        } else {
            m_CompressSuccess->setstringinfo(tr("Extraction successful"));
        }
        refreshPage();
    }
}

void MainWindow::slotShowPageUnzipProgress()
{
    m_pageid = PAGE_UNZIPPROGRESS;
    refreshPage();
    m_progressdialog->setProcess(0);
    m_Progess->setprogress(0);

}

void MainWindow::SlotNeedPassword()
{
    if (PAGE_ENCRYPTION != m_pageid) {
        m_pageid = PAGE_ENCRYPTION;
        refreshPage();
    }
}

void MainWindow::SlotExtractPassword(QString password)
{
    // m_progressTransFlag = false;
    if (Encryption_Load == m_encryptiontype) {
        LoadPassword(password);
    } else if (Encryption_Extract == m_encryptiontype) {
        ExtractPassword(password);
    } else if (Encryption_SingleExtract == m_encryptiontype) {
        ExtractSinglePassword(password);
    } else if (Encryption_TempExtract == m_encryptiontype) {
        ExtractSinglePassword(password);
    }
}

void MainWindow::ExtractSinglePassword(QString password)
{
    m_workstatus = WorkProcess;
    if (m_encryptionjob) {
        // first  time to extract
        m_encryptionjob->archiveInterface()->setPassword(password);

        m_encryptionjob->start();
    } else {
        // second or more  time to extract
        ExtractionOptions options;

        m_encryptionjob = m_model->extractFiles(m_extractSimpleFiles, m_decompressfilepath, options);
        m_encryptionjob->archiveInterface()->setPassword(password);
        connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
        connect(m_encryptionjob, &KJob::result, this, &MainWindow::slotExtractionDone);
        //
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword);
        connect(
            m_encryptionjob, &ExtractJob::sigExtractJobPassword, m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(m_encryptionjob,
                SIGNAL(percentfilename(KJob *, const QString &)),
                this,
                SLOT(SlotProgressFile(KJob *, const QString &)));
        connect(m_encryptionjob, &ExtractJob::updateDestFile, this, &MainWindow::onUpdateDestFile);

        m_encryptionjob->start();
    }
}

void MainWindow::ExtractPassword(QString password)
{
    m_workstatus = WorkProcess;
    if (m_encryptionjob) {
        // first  time to extract
        m_encryptionjob->archiveInterface()->setPassword(password);

        m_encryptionjob->start();
    } else {
        // second or more  time to extract
        ExtractionOptions options;
        QVector< Archive::Entry * > files;

        m_encryptionjob = m_model->extractFiles(files, m_decompressfilepath, options);
        m_encryptionjob->archiveInterface()->setPassword(password);
        connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
        connect(m_encryptionjob, &KJob::result, this, &MainWindow::slotExtractionDone);
        //
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword);
        connect(
            m_encryptionjob, &ExtractJob::sigExtractJobPassword, m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(m_encryptionjob,
                SIGNAL(percentfilename(KJob *, const QString &)),
                this,
                SLOT(SlotProgressFile(KJob *, const QString &)));
        connect(m_encryptionjob, &ExtractJob::sigCancelled, this, &MainWindow::slotClearTempfile);
        connect(m_encryptionjob, &ExtractJob::updateDestFile, this, &MainWindow::onUpdateDestFile);

        m_encryptionjob->start();
    }
}
void MainWindow::LoadPassword(QString password)
{
    m_workstatus = WorkProcess;
    m_encryptiontype = Encryption_Load;
    m_loadjob = dynamic_cast< LoadJob * >(m_model->loadArchive(m_loadfile, "", m_model));

    connect(m_loadjob, &LoadJob::sigWrongPassword, m_encryptionpage, &EncryptionPage::wrongPassWordSlot);

    m_loadjob->archiveInterface()->setPassword(password);
    if (m_loadjob) {
        m_loadjob->start();
    }
}

void MainWindow::setCompressDefaultPath()
{
    QString path;
    QStringList fileslist = m_CompressPage->getCompressFilelist();
    m_CompressSetting->setFilepath(fileslist);

    QFileInfo fileinfobase(fileslist.at(0));

    QString savePath = fileinfobase.path();

    for (int loop = 1; loop < fileslist.count(); loop++) {
        QFileInfo fileinfo(fileslist.at(loop));
        if (fileinfo.path() != fileinfobase.path()) {
            savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            break;
        }
    }

    m_CompressSetting->setDefaultPath(savePath);

    if (1 == fileslist.count()) {
        m_CompressSetting->setDefaultName(fileinfobase.baseName());
    } else {
        m_CompressSetting->setDefaultName(tr("Create New Archive"));
    }
}

void MainWindow::onCompressNext()
{
    m_pageid = PAGE_ZIPSET;
    setCompressDefaultPath();
    refreshPage();
}

void MainWindow::onCompressPressed(QMap< QString, QString > &Args)
{
    m_progressdialog->setProcess(0);
    m_Progess->setprogress(0);

    const QStringList filesToAdd = m_CompressPage->getCompressFilelist();

    QSet< QString > globalWorkDirList;
    foreach (QString file, filesToAdd) {
        QString globalWorkDir = file;
        if (globalWorkDir.right(1) == QLatin1String("/")) {
            globalWorkDir.chop(1);
        }
        globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
        globalWorkDirList.insert(globalWorkDir);
    }

    if (globalWorkDirList.count() == 1) {
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

void MainWindow::creatBatchArchive(QMap< QString, QString > &Args, QMap< QString, QStringList > &filetoadd)
{
    batchJob = new BatchCompress();
    batchJob->setCompressArgs(Args);

    for (QString &key : filetoadd.keys()) {
        batchJob->addInput(filetoadd.value(key));
    }

    connect(batchJob, SIGNAL(batchProgress(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
    connect(batchJob, &KJob::result, this, &MainWindow::slotCompressFinished);
    connect(batchJob,
            SIGNAL(batchFilenameProgress(KJob *, const QString &)),
            this,
            SLOT(SlotProgressFile(KJob *, const QString &)));

    qDebug() << "Starting job";
    m_decompressfilename = Args[QStringLiteral("filename")];
    m_CompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);
    m_CompressSuccess->setCompressFullPath(Args[QStringLiteral("localFilePath")] + QDir::separator()
                                           + Args[QStringLiteral("filename")]);
    m_pathstore = Args[QStringLiteral("localFilePath")];
    //m_compressDirFiles = CheckAllFiles(m_pathstore);

    m_pageid = PAGE_ZIPPROGRESS;
    m_Progess->settype(COMPRESSING);
    refreshPage();
    batchJob->start();
}

void MainWindow::transSplitFileName(QString &fileName)  // *.7z.003 -> *.7z.001
{
    QRegExp reg("^([\\s\\S]*.)[0-9]{3}$");

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
    while (QFileInfo::exists(filename)) {
        filename = localname.remove("." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix()) + "(" + "0"
                   + QString::number(num) + ")" + "."
                   + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();
        num++;
    }
}

QStringList MainWindow::CheckAllFiles(QString path)
{
    QDir dir(path);
    QStringList nameFilters;
    QStringList entrys = dir.entryList(nameFilters, QDir::AllEntries | QDir::Readable, QDir::Name);

    for (int i = 0; i < entrys.count(); i++) {
        entrys.replace(i, path + QDir::separator() + entrys.at(i));
    }
    return entrys;
}

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
    QFile fi(createCompressFile_);
    if (fi.exists()) {
        fi.remove();
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

void MainWindow::creatArchive(QMap< QString, QString > &Args)
{
    const QStringList filesToAdd = m_CompressPage->getCompressFilelist();
    const QString fixedMimeType = Args[QStringLiteral("fixedMimeType")];
    const QString password = Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = Args[QStringLiteral("encryptHeader")];
    createCompressFile_ = Args[QStringLiteral("localFilePath")] + QDir::separator() + Args[QStringLiteral("filename")];
    m_decompressfilename = Args[QStringLiteral("filename")];
    m_CompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);

    if (createCompressFile_.isEmpty()) {
        qDebug() << "filename.isEmpty()";
        return;
    }

    renameCompress(createCompressFile_, fixedMimeType);
    m_decompressfilename = QFileInfo(createCompressFile_).fileName();
    m_CompressSuccess->setCompressFullPath(createCompressFile_);
    qDebug() << createCompressFile_;

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

//#ifdef __aarch64__ // 华为arm平台 zip压缩 性能提升. 在多线程场景下使用7z,单线程场景下使用libarchive
//    double maxFileSizeProportion = static_cast<double>(maxFileSize_) / static_cast<double>(selectedTotalFileSize);
//    m_createJob = Archive::create(createCompressFile_, fixedMimeType, all_entries, options, this, maxFileSizeProportion > 0.6 );
//#else
//    m_createJob = Archive::create(createCompressFile_, fixedMimeType, all_entries, options, this );
//#endif
    m_createJob = Archive::create(createCompressFile_, fixedMimeType, all_entries, options, this);

    if (!password.isEmpty()) {
        m_createJob->enableEncryption(password, enableHeaderEncryption.compare("true") ? false : true);
    }

    connect(m_createJob, &KJob::result, this, &MainWindow::slotCompressFinished, Qt::ConnectionType::UniqueConnection);
    connect(m_createJob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)), Qt::ConnectionType::UniqueConnection);
    connect(m_createJob, &CreateJob::percentfilename, this, &MainWindow::SlotProgressFile, Qt::ConnectionType::UniqueConnection);

    m_pageid = PAGE_ZIPPROGRESS;
    m_Progess->settype(COMPRESSING);
    refreshPage();

    m_pathstore = Args[QStringLiteral("localFilePath")];
    //m_compressDirFiles = CheckAllFiles(m_pathstore);

    m_createJob->start();
    m_workstatus = WorkProcess;
}

void MainWindow::slotCompressFinished(KJob *job)
{
    qDebug() << "job finished" << job->error();
    m_workstatus = WorkNone;
    if (job->error() && (job->error() != KJob::KilledJobError)) {
        if (m_pathstore.left(6) == "/media") {
            if (getMediaFreeSpace() <= 50) {
                m_CompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
            } else {
                m_CompressFail->setFailStrDetail(tr("Damaged file"));
            }
        } else {
            if (getDiskFreeSpace() <= 50) {
                m_CompressFail->setFailStrDetail(tr("Insufficient space, please clear and retry"));
            } else {
                m_CompressFail->setFailStrDetail(tr("Damaged file"));
            }
        }
        m_pageid = PAGE_ZIP_FAIL;
        refreshPage();
        return;
    }

    createCompressFile_.clear();
    m_pageid = PAGE_ZIP_SUCCESS;
    refreshPage();

    if (m_createJob) {
        m_createJob->deleteLater();
        m_createJob = nullptr;
    }
}

void MainWindow::slotExtractSimpleFiles(QVector< Archive::Entry * > fileList, QString path, EXTRACT_TYPE type)
{
    m_timer.start();
    QStringList m_tempFileList;
    m_tempFileList.insert(0, path);
    calSelectedTotalFileSize(m_tempFileList);
    m_progressdialog->setProcess(0);
    m_Progess->setprogress(0);
    // m_progressTransFlag = false;
    m_workstatus = WorkProcess;

    if (type == EXTRACT_TEMP) {
        m_encryptiontype = Encryption_TempExtract;
    } else if (type == EXTRACT_TEMP_CHOOSE_OPEN) {
        m_encryptiontype =  Encryption_TempExtract_Open_Choose;
    } else {
        m_encryptiontype = Encryption_SingleExtract;
    }
    m_progressdialog->clearprocess();
    if (!m_model) {
        return;
    }

    if (m_encryptionjob) {
        m_encryptionjob = nullptr;
    }

    ExtractionOptions options;
    options.setDragAndDropEnabled(true);
    m_extractSimpleFiles = fileList;
    const QString destinationDirectory = path;

    //m_compressDirFiles = CheckAllFiles(path);

    m_encryptionjob = m_model->extractFiles(fileList, destinationDirectory, options);

    connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)), this, SLOT(SlotProgress(KJob *, ulong)));
    connect(m_encryptionjob, &KJob::result, this, &MainWindow::slotExtractionDone);
    //
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPwdCheckDown, this, &MainWindow::slotShowPageUnzipProgress);
    connect(
        m_encryptionjob, &ExtractJob::sigExtractJobPassword, this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword, m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
    connect(m_encryptionjob,
            SIGNAL(percentfilename(KJob *, const QString &)),
            this,
            SLOT(SlotProgressFile(KJob *, const QString &)));

    m_encryptionjob->start();
    m_decompressfilepath = destinationDirectory;

    QFileInfo file(m_loadfile);
    m_progressdialog->setCurrentTask(file.fileName());
}

void MainWindow::slotExtractSimpleFilesOpen(const QVector<Archive::Entry *> &fileList, const QString &programma)
{
    QString tmppath = DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles";
    QDir dir(tmppath);
    if (!dir.exists()) {
        dir.mkdir(tmppath);
    }

    program = programma;

    slotExtractSimpleFiles(fileList, tmppath, EXTRACT_TEMP_CHOOSE_OPEN);

}

void MainWindow::slotKillExtractJob()
{
    m_workstatus = WorkNone;
    if (m_encryptionjob) {
        m_encryptionjob->Killjob();
        m_encryptionjob = nullptr;
    }
    //deleteCompressFile(m_compressDirFiles, CheckAllFiles(m_decompressfilepath));
}

void MainWindow::slotFailRetry()
{
    if (PAGE_ZIP_FAIL == m_pageid) {
        m_pageid = PAGE_ZIPSET;
        refreshPage();
    } else if (Encryption_Load == m_encryptiontype) {
        m_pageid = PAGE_HOME;
        refreshPage();
        loadArchive(m_loadfile);
    } else if (Encryption_Extract == m_encryptiontype) {
        slotextractSelectedFilesTo(m_UnCompressPage->getDecompressPath());
    } else if (Encryption_SingleExtract == m_encryptiontype) {
    }
}

void MainWindow::slotStopSpinner()
{
    if (pEventloop != nullptr) {
        pEventloop->quit();
    }
    if (m_spinner != nullptr) {
        m_spinner->stop();
        m_spinner->hide();
    }
    disconnect(m_encryptionjob, &ExtractJob::sigExtractSpinnerFinished, this, &MainWindow::slotStopSpinner);
}


void MainWindow::onCancelCompressPressed(int compressType)
{
    slotResetPercentAndTime();

    if (m_encryptionjob) {
        m_encryptionjob->Killjob();
        //append the spiner animation to the eventloop, so can play the spinner animation
        if (pEventloop == nullptr) {
            pEventloop = new QEventLoop(this->m_Progess);
        }
//        QString name = m_model->archive()->completeBaseName();
        m_model->archive()->subfolderName();
//        m_encryptionjob->archiveInterface()->extractTopFolderName = m_model->archive()->subfolderName();
        m_encryptionjob->archiveInterface()->extractPsdStatus = ReadOnlyArchiveInterface::ExtractPsdStatus::Canceled;
        if (pEventloop->isRunning() == false) {
            connect(m_encryptionjob, &ExtractJob::sigExtractSpinnerFinished, this, &MainWindow::slotStopSpinner);
            if (m_spinner == nullptr) {
                m_spinner = new DSpinner(this->m_Progess);
                m_spinner->setFixedSize(40, 40);
            }
            m_spinner->move(this->m_Progess->width() / 2 - 20, this->m_Progess->height() / 2 - 20);
            m_spinner->hide();
            m_spinner->start();
            m_spinner->show();
//            m_encryptionjob->Killjob();
            m_encryptionjob = nullptr;
            pEventloop->exec(QEventLoop::ExcludeUserInputEvents);
        } else {
            m_encryptionjob->Killjob();
            m_encryptionjob = nullptr;
        }
    }

    deleteCompressFile(/*m_compressDirFiles, CheckAllFiles(m_pathstore)*/);

    if (compressType == COMPRESSING) {
        if (m_createJob) {
            m_createJob->deleteLater();
            m_createJob = nullptr;
        } else if (batchJob != nullptr) {
//            batchJob->doKill();
            batchJob->kill();
        }
        m_pageid = PAGE_ZIP;
    } else if (compressType == DECOMPRESSING) {
        m_pageid = PAGE_UNZIP;
    }
    refreshPage();
    // emit sigquitApp();
}

void MainWindow::slotClearTempfile()
{
    openTempFileLink = 0;
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-rf");
    args.append(DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles");
    p.execute(command, args);
    p.waitForFinished();
}

void MainWindow::slotquitApp()
{
    openTempFileLink = 0;
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-rf");
    args.append(DStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "tempfiles");
    p.execute(command, args);
    p.waitForFinished();

    emit sigquitApp();
}

void MainWindow::onUpdateDestFile(QString destFile)
{
    m_CompressSuccess->setCompressFullPath(destFile);
}

void MainWindow::onCompressPageFilelistIsEmpty()
{
    m_pageid = PAGE_HOME;
    refreshPage();
}

void MainWindow::slotCalDeleteRefreshTotalFileSize(const QStringList &files)
{
    resetMainwindow();

    calSelectedTotalFileSize(files);
}

void MainWindow::resetMainwindow()
{
    selectedTotalFileSize = 0;
    lastPercent = 0;

#ifdef __aarch64__
    maxFileSize_ = 0;
#endif
}

void MainWindow::slotBackButtonClicked()
{
    resetMainwindow();

    slotResetPercentAndTime();
    m_CompressSuccess->clear();

    if (m_pageid == PAGE_ZIP_SUCCESS || m_pageid == PAGE_UNZIP_SUCCESS) {
        m_CompressPage->clearFiles();
        m_Progess->setprogress(0);
        m_progressdialog->setProcess(0);
    }

    m_pageid = PAGE_HOME;
    refreshPage();
}

void MainWindow::slotResetPercentAndTime()
{
    lastPercent = 0;
    compressTime = 0;
    m_timer.elapsed();
}

void MainWindow::slotFileUnreadable(QStringList &pathList, int fileIndex)
{
    pathList.removeAt(fileIndex);
    if (m_pageid != PAGE_ZIP) {
        m_pageid = PAGE_ZIP;
        refreshPage();
    }
    m_CompressPage->onRefreshFilelist(pathList);
    if (pathList.isEmpty()) {
        m_pageid = PAGE_HOME;
        refreshPage();
    }
}

void MainWindow::onTitleButtonPressed()
{
    switch (m_pageid) {
    case PAGE_ZIP:
        emit sigZipAddFile();
        break;
    case PAGE_ZIPSET:
        emit sigZipReturn();
        m_CompressSetting->clickTitleBtnResetAdvancedOptions();
        m_pageid = PAGE_ZIP;
        refreshPage();
        break;
    case PAGE_ZIP_SUCCESS:
    case PAGE_ZIP_FAIL:
        m_CompressSuccess->clear();
        m_pageid = PAGE_ZIP;
        refreshPage();
        break;
    case PAGE_UNZIP_SUCCESS:
    case PAGE_UNZIP_FAIL:
        m_CompressSuccess->clear();

        if (m_UnCompressPage->getFileCount() < 1) {
            m_pageid = PAGE_HOME;
        } else {
            m_pageid = PAGE_UNZIP;
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
        m_titlebutton->setVisible(false);
        m_openAction->setEnabled(false);
        //        setAcceptDrops(false);
    } else {
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_IncreaseElement);
        m_titlebutton->setVisible(true);
        m_openAction->setEnabled(true);
        //        setAcceptDrops(true);
    }
}
