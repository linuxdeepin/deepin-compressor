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

#include "mainwindow.h"
#include "utils.h"
#include <QSvgWidget>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <DDesktopServices>
#include <QMimeDatabase>
#include <QFileIconProvider>
#include <QShortcut>
#include <QJsonArray>
#include <QJsonDocument>
#include "pluginmanager.h"
#include <DMessageManager>
#include <QGraphicsDropShadowEffect>
#include <QStorageInfo>
#include <QTimer>
//#include "archivejob.h"
#include "jobs.h"

//static QString shortcut_json =

static bool DeleteDirectory(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }

    QDir dir(path);
    if (!dir.exists()) {
        return true;
    }

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo fi, fileList) {
        if (fi.isFile()) {
            fi.dir().remove(fi.fileName());
        } else {
            DeleteDirectory(fi.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());
}

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
      m_encryptionpage(new EncryptionPage),
      m_progressdialog(new ProgressDialog),
      m_settingsDialog(new SettingDialog),
      m_encodingpage(new EncodingPage),
      m_settings(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                               QSettings::IniFormat))
{
    m_encryptionjob = nullptr;
    m_encryptiontype = Encryption_NULL;
    m_isrightmenu = false;
    m_model = new ArchiveModel(this);
    m_filterModel = new ArchiveSortFilterModel(this);

    InitUI();
    InitConnection();
}

MainWindow::~MainWindow()
{
}

qint64 MainWindow::getMediaFreeSpace()
{
    QList<QStorageInfo> list = QStorageInfo::mountedVolumes();
    qDebug() << "Volume Num: " << list.size();
    for(QStorageInfo& si : list)
    {
        if(si.displayName().count() > 7 && si.displayName().left(6) == "/media")
        {
            qDebug() << "Bytes Avaliable: " << si.bytesAvailable()/ 1024 / 1024 << "MB";
            return si.bytesAvailable()/ 1024 / 1024;
        }
    }
}

qint64 MainWindow::getDiskFreeSpace()
{
    QStorageInfo storage = QStorageInfo::root();
    storage.refresh();
    qDebug() << "availableSize:" << storage.bytesAvailable() / 1024 / 1024 << "MB";
    return storage.bytesAvailable()/ 1024 / 1024;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (4 == m_mainLayout->currentIndex()) { //now is progress page
        if (1 != m_Progess->showConfirmDialog()) {
            event->ignore();
            return;
        }
        deleteCompressFile(m_compressDirFiles, CheckAllFiles(m_pathstore));
    }
    event->accept();
    emit sigquitApp();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (m_timerId == event->timerId()) {
        m_progressTransFlag = true;
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

void MainWindow::InitUI()
{
    if (m_settings->value("dir").toString().isEmpty()) {
        m_settings->setValue("dir", "");
    }

    // add widget to main layout.
    m_mainLayout->addWidget(m_homePage);
    m_mainLayout->addWidget(m_UnCompressPage);
    m_mainLayout->addWidget(m_CompressPage);
    m_mainLayout->addWidget(m_CompressSetting);
    m_mainLayout->addWidget(m_Progess);
    m_mainLayout->addWidget(m_CompressSuccess);
    m_mainLayout->addWidget(m_CompressFail);
    m_mainLayout->addWidget(m_encryptionpage);
    m_mainLayout->addWidget(m_encodingpage);
    m_homePage->setAutoFillBackground(true);
    m_UnCompressPage->setAutoFillBackground(true);
    m_CompressPage->setAutoFillBackground(true);
    m_CompressSetting->setAutoFillBackground(true);
    m_Progess->setAutoFillBackground(true);
    m_CompressSuccess->setAutoFillBackground(true);
    m_CompressFail->setAutoFillBackground(true);
    m_encryptionpage->setAutoFillBackground(true);
    m_encodingpage->setAutoFillBackground(true);

    // init window flags.
    setWindowTitle(tr("Archive manager"));
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setCentralWidget(m_mainWidget);
    setAcceptDrops(true);

    initTitleBar();

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

    QJsonObject shortcut5;
    shortcut5.insert("name", tr("Show shortcut preview"));
    shortcut5.insert("value", "Ctrl+Shift+/");

    QJsonObject shortcut6;
    shortcut6.insert("name", tr("Rename"));
    shortcut6.insert("value", "F2");

    QJsonArray shortcutArray;
    shortcutArray.append(shortcut1);
    shortcutArray.append(shortcut2);
    shortcutArray.append(shortcut3);
    shortcutArray.append(shortcut4);
    shortcutArray.append(shortcut5);
    shortcutArray.append(shortcut6);

    QJsonObject shortcut_group;
    shortcut_group.insert("groupName", tr("hot key"));
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
    connect(m_CompressPage, &CompressPage::sigselectedFiles, this, &MainWindow::onSelected);
    connect(m_CompressPage, &CompressPage::sigNextPress, this, &MainWindow::onCompressNext);
    connect(this, &MainWindow::sigZipAddFile, m_CompressPage, &CompressPage::onAddfileSlot);
    connect(this, &MainWindow::sigZipReturn, m_CompressSetting, &CompressSetting::onRetrunPressed);
    connect(m_CompressSetting, &CompressSetting::sigCompressPressed, this, &MainWindow::onCompressPressed);
    connect(m_Progess, &Progress::sigCancelPressed, this, &MainWindow::onCancelCompressPressed);
    connect(m_CompressSuccess, &Compressor_Success::sigQuitApp, this, &MainWindow::sigquitApp);
    connect(m_titlebutton, &DPushButton::clicked, this, &MainWindow::onTitleButtonPressed);
    connect(this, &MainWindow::sigZipSelectedFiles, m_CompressPage, &CompressPage::onSelectedFilesSlot);
    connect(m_model, &ArchiveModel::loadingFinished, this, &MainWindow::slotLoadingFinished);
    connect(m_UnCompressPage, &UnCompressPage::sigDecompressPress, this, &MainWindow::slotextractSelectedFilesTo);
    connect(m_encryptionpage, &EncryptionPage::sigExtractPassword, this, &MainWindow::SlotExtractPassword);
    connect(m_UnCompressPage, &UnCompressPage::sigextractfiles, this, &MainWindow::slotExtractSimpleFiles);
    connect(m_progressdialog, &ProgressDialog::stopExtract, this, &MainWindow::slotKillExtractJob);
    connect(m_CompressFail, &Compressor_Fail::sigFailRetry, this, &MainWindow::slotFailRetry);
    connect(m_CompressPage, &CompressPage::sigiscanaddfile, this, &MainWindow::onCompressAddfileSlot);
    connect(m_progressdialog, &ProgressDialog::extractSuccess, this, [ = ] {
        QIcon icon = Utils::renderSVG(":/images/icon_toast_sucess.svg", QSize(30, 30));
        this->sendMessage(icon, tr("Successful extraction"));
    });

    auto openkey = new QShortcut(QKeySequence(Qt::Key_Slash + Qt::CTRL + Qt::SHIFT), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, [this] {
        QRect rect = window()->geometry();
        QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
        QStringList shortcutString;
        QJsonObject json = creatShorcutJson();

        QString param1 = "-j=" + QString(QJsonDocument(json).toJson());
        QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
        shortcutString << param1 << param2;

        QProcess *shortcutViewProcess = new QProcess();
        shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

        connect(shortcutViewProcess, SIGNAL(finished(int)),
                shortcutViewProcess, SLOT(deleteLater()));
    });
}

void MainWindow::customMessageHandler(const QString &msg)
{
    QString txt;
    txt = msg;

    QFile outFile("/home/deepin/debug.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

QMenu *MainWindow::createSettingsMenu()
{
    QMenu *menu = new QMenu;

    m_openAction = menu->addAction(tr("Open"));
    connect(m_openAction, &QAction::triggered, this, [this] {
        DFileDialog dialog;
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

    QAction *settingsAction = menu->addAction(tr("Setting"));
    connect(settingsAction, &QAction::triggered, this, [this] {
        m_settingsDialog->exec();
    });
    return menu;
}

void MainWindow::initTitleBar()
{
    titlebar()->setMenu(createSettingsMenu());
    titlebar()->setFixedHeight(50);

    QIcon icon = QIcon::fromTheme("deepin-compressor");
    m_logo = new DLabel("");
    m_logo->setPixmap(icon.pixmap(QSize(30, 30)));

    m_titlebutton = new DIconButton(DStyle::StandardPixmap::SP_IncreaseElement, this);
    m_titlebutton->setFixedSize(36, 36);
    m_titlebutton->setVisible(false);

    m_titleFrame = new QFrame;
    m_titleFrame->setObjectName("TitleBar");
    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->addSpacing(6);
    leftLayout->addWidget(m_logo);
    leftLayout->addSpacing(6);
    leftLayout->addWidget(m_titlebutton);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *left_frame = new QFrame();
    left_frame->setFixedWidth(10 + 6 + 36 + 30);
    left_frame->setContentsMargins(0, 0, 0, 0);
    left_frame->setLayout(leftLayout);

    m_titlelabel = new DLabel("");
    m_titlelabel->setMinimumSize(200, TITLE_FIXED_HEIGHT);
    m_titlelabel->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(m_titlelabel, DFontSizeManager::T6, QFont::Medium);

    m_titlelabel->setForegroundRole(DPalette::WindowText);

    QHBoxLayout *titlemainLayout = new QHBoxLayout;
    titlemainLayout->setContentsMargins(0, 0, 0, 0);
    titlemainLayout->addWidget(left_frame);
    titlemainLayout->addSpacing(50);
    titlemainLayout->addWidget(m_titlelabel, 0, Qt::AlignCenter);

    m_titleFrame->setLayout(titlemainLayout);
    m_titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);
    titlebar()->setContentsMargins(0, 0, 0, 0);
    titlebar()->setCustomWidget(m_titleFrame, false);
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
        qDebug() << fileList;


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
    titlebar()->setDisableFlags(Qt::WindowMinimizeButtonHint
                                | Qt::WindowCloseButtonHint
                                | Qt::WindowMaximizeButtonHint
                                | Qt::WindowSystemMenuHint);
}

void MainWindow::refreshPage()
{
    m_openAction->setEnabled(false);
    setAcceptDrops(false);
    m_titlebutton->setVisible(false);
    switch (m_pageid) {
    case PAGE_HOME:
        m_openAction->setEnabled(true);
        m_mainLayout->setCurrentIndex(0);
        setQLabelText(m_titlelabel, "");
        setAcceptDrops(true);
        break;
    case PAGE_UNZIP:
        m_mainLayout->setCurrentIndex(1);
//        m_titlelabel->setText(m_decompressfilename);
        setQLabelText(m_titlelabel, m_decompressfilename);
        break;
    case PAGE_ZIP:
        m_openAction->setEnabled(true);
        m_mainLayout->setCurrentIndex(2);
        setQLabelText(m_titlelabel, tr("Create New archive"));
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_IncreaseElement);
        m_titlebutton->setVisible(true);
        setAcceptDrops(true);
        m_CompressPage->onPathIndexChanged();
        break;
    case PAGE_ZIPSET:
        m_mainLayout->setCurrentIndex(3);
        setQLabelText(m_titlelabel, tr("New archive"));
        m_titlebutton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
        m_titlebutton->setVisible(true);
        break;
    case PAGE_ZIPPROGRESS:
        m_mainLayout->setCurrentIndex(4);
        setQLabelText(m_titlelabel, tr("Compressing"));
        m_Progess->setFilename(m_decompressfilename);
        break;
    case PAGE_UNZIPPROGRESS:
        m_mainLayout->setCurrentIndex(4);
        setQLabelText(m_titlelabel, tr("Extracting"));
        m_Progess->setFilename(m_decompressfilename);
        break;
    case PAGE_ZIP_SUCCESS:
        m_mainLayout->setCurrentIndex(5);
        setQLabelText(m_titlelabel, "");
        m_CompressSuccess->setstringinfo(tr("Successful compression!"));
        break;
    case PAGE_ZIP_FAIL:
        m_mainLayout->setCurrentIndex(6);
        setQLabelText(m_titlelabel, "");
        m_CompressFail->setFailStr(tr("Sorry, the compression failed!"));
        break;
    case PAGE_UNZIP_SUCCESS:
        m_mainLayout->setCurrentIndex(5);
        setQLabelText(m_titlelabel, "");
        m_CompressSuccess->setCompressPath(m_decompressfilepath);
        m_CompressSuccess->setstringinfo(tr("Successful extraction!"));
        if (m_settingsDialog->isAutoOpen()) {
            DDesktopServices::showFolder(QUrl(m_decompressfilepath, QUrl::TolerantMode));
        }
        break;
    case PAGE_UNZIP_FAIL:
        m_mainLayout->setCurrentIndex(6);
        setQLabelText(m_titlelabel, "");
        m_CompressFail->setFailStr(tr("Sorry, the extraction failed!"));
        break;
    case  PAGE_ENCRYPTION:
        m_mainLayout->setCurrentIndex(7);
        setQLabelText(m_titlelabel, m_decompressfilename);
        if (m_progressdialog->isshown()) {
            m_progressdialog->reject();
        }
        break;
    default:
        break;
    }

}

void MainWindow::onSelected(const QStringList &files)
{
    if (files.count() == 1 && Utils::isCompressed_file(files.at(0))) {
        if (0 == m_CompressPage->getCompressFilelist().count()) {
            QString filename;
            filename = files.at(0);
            if (filename.contains(".7z.")) {
                filename = filename.left(filename.length() - 3) + "001";
            }
            QFileInfo fileinfo(filename);
            m_decompressfilename = fileinfo.fileName();
            if ("" != m_settingsDialog->getCurExtractPath()) {
                m_UnCompressPage->setdefaultpath(m_settingsDialog->getCurExtractPath());
            } else {
                m_UnCompressPage->setdefaultpath(fileinfo.path());
            }

            loadArchive(filename);

        } else {
            DDialog *dialog = new DDialog;
            QIcon icon = QIcon::fromTheme("deepin-compressor");
            dialog->setIcon(icon, QSize(32, 32));
            dialog->setMessage(tr("Add a compressed file to a directory or open it in a new window?"));
            dialog->addButton(tr("Cancel"));
            dialog->addButton(tr("Add"));
            dialog->addButton(tr("Open in new window"), true, DDialog::ButtonRecommend);
            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
            effect->setOffset(0, 4);
            effect->setColor(QColor(0, 145, 255, 76));
            effect->setBlurRadius(4);
            dialog->getButton(2)->setGraphicsEffect(effect);

            const int mode = dialog->exec();
            qDebug() << mode;
            if (1 == mode) {
                emit sigZipSelectedFiles(files);
            } else if (2 == mode) {
                KProcess *cmdprocess = new KProcess;
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

        connect(batchJob, SIGNAL(batchProgress(KJob *, ulong)),
                this, SLOT(SlotProgress(KJob *, ulong)));
        connect(batchJob, &KJob::result,
                this, &MainWindow::slotExtractionDone);
        connect(batchJob, &BatchExtract::sendCurFile,
                this, &MainWindow::slotBatchExtractFileChanged);
        connect(batchJob, &BatchExtract::sendFailFile,
                this, &MainWindow::slotBatchExtractError);
//        connect(batchJob, &BatchExtract::sigExtractJobPassword,
//                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
//        connect(batchJob, &BatchExtract::sigExtractJobPassword,
//                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(batchJob, SIGNAL(batchFilenameProgress(KJob *, const QString &)),
                this, SLOT(SlotProgressFile(KJob *, const QString &)));
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
            QTimer::singleShot(100, this, [=]
            {
                emit sigquitApp();
            });
            return;
        }

        QList<QUrl> selectpath = dialog.selectedUrls();
        qDebug()<<selectpath;
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

        connect(batchJob, SIGNAL(batchProgress(KJob *, ulong)),
                this, SLOT(SlotProgress(KJob *, ulong)));
        connect(batchJob, &KJob::result,
                this, &MainWindow::slotExtractionDone);
        connect(batchJob, &BatchExtract::sendCurFile,
                this, &MainWindow::slotBatchExtractFileChanged);
        connect(batchJob, &BatchExtract::sendFailFile,
                this, &MainWindow::slotBatchExtractError);
//        connect(batchJob, &BatchExtract::sigExtractJobPassword,
//                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
//        connect(batchJob, &BatchExtract::sigExtractJobPassword,
//                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(batchJob, SIGNAL(batchFilenameProgress(KJob *, const QString &)),
                this, SLOT(SlotProgressFile(KJob *, const QString &)));
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
            filepath = filepath.left(filepath.length() - 3) + "001";
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
                m_CompressFail->setFailStrDetail(tr("Compressed file is corrupt!"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        }
    } else if (files.last() == QStringLiteral("extract_split")) {
        QString filepath = files.at(0);
        filepath = filepath.left(filepath.length() - 3) + "001";
        QFileInfo fileinfo(filepath);
        customMessageHandler(filepath);
        if (fileinfo.exists()) {
            m_decompressfilename = fileinfo.fileName();
            if ("" != m_settingsDialog->getCurExtractPath()) {
                m_UnCompressPage->setdefaultpath(m_settingsDialog->getCurExtractPath());
            } else {
                m_UnCompressPage->setdefaultpath(fileinfo.path());
            }

            loadArchive(filepath);
        } else {
            m_CompressFail->setFailStrDetail(tr("Compressed file is corrupt!"));
            m_pageid = PAGE_UNZIP_FAIL;
            refreshPage();
        }
    } else if (files.count() == 1 && Utils::isCompressed_file(files.at(0))) {
        QString filename;
        filename = files.at(0);
        if (filename.contains(".7z.")) {
            filename = filename.left(filename.length() - 3) + "001";
        }
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
    m_workstatus = WorkNone;
    if (job->error()) {
        m_CompressFail->setFailStrDetail(tr("Compressed file is corrupt!"));
        m_pageid = PAGE_UNZIP_FAIL;
        refreshPage();
        return;
    }

    m_filterModel->setSourceModel(m_model);
    m_filterModel->setFilterKeyColumn(0);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_UnCompressPage->setModel(m_filterModel);


    m_homePage->spinnerStop();
    if (!m_isrightmenu) {
        m_pageid = PAGE_UNZIP;
        refreshPage();
    } else {
        slotextractSelectedFilesTo(m_UnCompressPage->getDecompressPath());
    }
}


void MainWindow::loadArchive(const QString &files)
{
    m_workstatus = WorkProcess;
    m_loadfile = files;
    m_encryptiontype = Encryption_Load;
    m_loadjob = (LoadJob *)m_model->loadArchive(files, "", m_model);

    connect(m_loadjob, &LoadJob::sigLodJobPassword,
            this, &MainWindow::SlotNeedPassword);
    connect(m_loadjob, &LoadJob::sigWrongPassword,
            this, &MainWindow::SlotNeedPassword);

    if (m_loadjob) {
        m_loadjob->start();
        m_homePage->spinnerStart();
    }
}

void MainWindow::slotextractSelectedFilesTo(const QString &localPath)
{
    m_progressTransFlag = false;
    m_workstatus = WorkProcess;
    m_encryptiontype = Encryption_Extract;
    if (!m_model) {
        return;
    }

    if (m_encryptionjob) {
        m_encryptionjob = nullptr;
    }

    ExtractionOptions options;
    QVector<Archive::Entry *> files;

    QString userDestination = localPath;
    QString destinationDirectory;

    m_pathstore = userDestination;
    m_compressDirFiles = CheckAllFiles(m_pathstore);

    if (m_settingsDialog->isAutoCreatDir()) {
        const QString detectedSubfolder = m_model->archive()->subfolderName();
        qDebug() << "Detected subfolder" << detectedSubfolder;

        if (m_model->archive()->hasMultipleTopLevelEntries()) {
            if (!userDestination.endsWith(QDir::separator())) {
                userDestination.append(QDir::separator());
            }
            destinationDirectory = userDestination + detectedSubfolder;
            QDir(userDestination).mkdir(detectedSubfolder);
        } else {
            destinationDirectory = userDestination;
        }
    } else {
        destinationDirectory = userDestination;
    }


    qDebug() << "destinationDirectory:" << destinationDirectory;
    m_encryptionjob = m_model->extractFiles(files, destinationDirectory, options);

    connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)),
            this, SLOT(SlotProgress(KJob *, ulong)));
    connect(m_encryptionjob, &KJob::result,
            this, &MainWindow::slotExtractionDone);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
    connect(m_encryptionjob, SIGNAL(percentfilename(KJob *, const QString &)),
            this, SLOT(SlotProgressFile(KJob *, const QString &)));
    connect(m_encryptionjob, &ExtractJob::sigCancelled,
            this, &MainWindow::sigquitApp);

    m_encryptionjob->start();
    m_decompressfilepath = destinationDirectory;

}

void MainWindow::SlotProgress(KJob *job, unsigned long percent)
{
    qDebug() << percent;
    if ((Encryption_SingleExtract == m_encryptiontype)) {
        if ((percent < 100) && (percent > 0) && (WorkProcess == m_workstatus)) {
            if (!m_progressdialog->isshown()) {
                m_pageid = PAGE_UNZIP;
                refreshPage();
                m_progressdialog->showdialog();
            }
            m_progressdialog->setProcess(percent);
        }
    } else if (PAGE_ZIPPROGRESS == m_pageid || PAGE_UNZIPPROGRESS == m_pageid) {
        m_Progess->setprogress(percent);
    } else if ((PAGE_UNZIP == m_pageid || PAGE_ENCRYPTION == m_pageid) && (percent < 100) && (percent > 0)) {
        if (!m_progressTransFlag) {
            if (0 == m_timerId) {
                m_timerId = startTimer(800);
            }
        } else {
            m_pageid = PAGE_UNZIPPROGRESS;
            m_Progess->settype(DECOMPRESSING);
            refreshPage();
        }
    } else if ((PAGE_ZIPSET == m_pageid) && (percent < 100) && (percent > 0)) {
        m_pageid = PAGE_ZIPPROGRESS;
        m_Progess->settype(COMPRESSING);
        refreshPage();
    }
}

void MainWindow::SlotProgressFile(KJob *job, const QString &filename)
{
    if (Encryption_SingleExtract == m_encryptiontype && PAGE_UNZIP == m_pageid) {
        m_progressdialog->setCurrentFile(filename);
    } else if (PAGE_ZIPPROGRESS == m_pageid || PAGE_UNZIPPROGRESS == m_pageid) {
        m_Progess->setProgressFilename(filename);
    }
}

void MainWindow::slotBatchExtractFileChanged(const QString& name)
{
    qDebug()<<name;
    m_Progess->setFilename(name);
}

void MainWindow::slotBatchExtractError(const QString& name)
{
    qDebug()<<name;
    m_CompressFail->setFailStrDetail(name + ":" + tr("Wrong password!"));
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

    if ((PAGE_ENCRYPTION == m_pageid) && (job->error() && job->error() != KJob::KilledJobError)) {
        //do noting:wrong password
    } else if (job->error() && job->error() != KJob::KilledJobError) {
        if(m_pathstore.left(6) == "/media")
        {
            if(getMediaFreeSpace() <= 50)
            {
                m_CompressFail->setFailStrDetail(tr("No space left, please clean and retry"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
            else {
                m_CompressFail->setFailStrDetail(tr("Compressed file is corrupt!"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        }
        else {
            if (getDiskFreeSpace() <= 50) {
                m_CompressFail->setFailStrDetail(tr("No space left, please clean and retry"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
            else {
                m_CompressFail->setFailStrDetail(tr("Compressed file is corrupt!"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        }
        return;
    } else if (Encryption_SingleExtract == m_encryptiontype) {
        m_progressdialog->setFinished(m_decompressfilepath);
    } else {
        m_pageid = PAGE_UNZIP_SUCCESS;
        refreshPage();
    }
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
    m_progressTransFlag = false;
    if (Encryption_Load == m_encryptiontype) {
        LoadPassword(password);
    } else if (Encryption_Extract == m_encryptiontype) {
        ExtractPassword(password);
    } else if (Encryption_SingleExtract == m_encryptiontype) {
        ExtractSinglePassword(password);
    }
}

void MainWindow::ExtractSinglePassword(QString password)
{
    m_workstatus = WorkProcess;
    if (m_encryptionjob) { //first  time to extract
        m_encryptionjob->archiveInterface()->setPassword(password);

        m_encryptionjob->start();

    } else { //second or more  time to extract
        ExtractionOptions options;

        m_encryptionjob = m_model->extractFiles(m_extractSimpleFiles, m_decompressfilepath, options);
        m_encryptionjob->archiveInterface()->setPassword(password);
        connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)),
                this, SLOT(SlotProgress(KJob *, ulong)));
        connect(m_encryptionjob, &KJob::result,
                this, &MainWindow::slotExtractionDone);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                this, &MainWindow::SlotNeedPassword);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(m_encryptionjob, SIGNAL(percentfilename(KJob *, const QString &)),
                this, SLOT(SlotProgressFile(KJob *, const QString &)));

        m_encryptionjob->start();
    }
}

void MainWindow::ExtractPassword(QString password)
{
    m_workstatus = WorkProcess;
    if (m_encryptionjob) { //first  time to extract
        m_encryptionjob->archiveInterface()->setPassword(password);

        m_encryptionjob->start();

    } else { //second or more  time to extract
        ExtractionOptions options;
        QVector<Archive::Entry *> files;

        m_encryptionjob = m_model->extractFiles(files, m_decompressfilepath, options);
        m_encryptionjob->archiveInterface()->setPassword(password);
        connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)),
                this, SLOT(SlotProgress(KJob *, ulong)));
        connect(m_encryptionjob, &KJob::result,
                this, &MainWindow::slotExtractionDone);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                this, &MainWindow::SlotNeedPassword);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(m_encryptionjob, SIGNAL(percentfilename(KJob *, const QString &)),
                this, SLOT(SlotProgressFile(KJob *, const QString &)));
        connect(m_encryptionjob, &ExtractJob::sigCancelled,
                this, &MainWindow::sigquitApp);

        m_encryptionjob->start();
    }
}
void MainWindow::LoadPassword(QString password)
{
    m_workstatus = WorkProcess;
    m_encryptiontype = Encryption_Load;
    m_loadjob = (LoadJob *)m_model->loadArchive(m_loadfile, "", m_model);

    connect(m_loadjob, &LoadJob::sigWrongPassword,
            m_encryptionpage, &EncryptionPage::wrongPassWordSlot);

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
    for (int loop = 1; loop < fileslist.count(); loop++) {
        QFileInfo fileinfo(fileslist.at(loop));
        if (fileinfo.path() != fileinfobase.path()) {
            m_CompressSetting->setDefaultPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
            return;
        }
    }

    m_CompressSetting->setDefaultPath(fileinfobase.path());
    if (1 == fileslist.count()) {
        m_CompressSetting->setDefaultName(fileinfobase.baseName());
    } else {
        m_CompressSetting->setDefaultName(tr("New archive"));
    }

}

void MainWindow::onCompressNext()
{
    m_pageid = PAGE_ZIPSET;
    setCompressDefaultPath();
    refreshPage();
}

void MainWindow::onCompressPressed(QMap<QString, QString> &Args)
{
    creatArchive(Args);
}

QString MainWindow::renameCompress(QString &filename, QString fixedMimeType)
{
    QString localname = filename;
    int num = 2;
    while (QFileInfo::exists(filename)) {
        filename = localname.remove("." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix()) + "(" + "0" + QString::number(num) + ")" + "." + QMimeDatabase().mimeTypeForName(fixedMimeType).preferredSuffix();
        num++;
    }
    return filename;
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



void MainWindow::deleteCompressFile(QStringList oldfiles, QStringList newfiles)
{
    if (newfiles.count() <= oldfiles.count()) {
        qDebug() << "Nofile to delete";
        return;
    }

    QStringList deletefile;
    foreach (QString newpath, newfiles) {
        int count = 0;
        foreach (QString oldpath, oldfiles) {
            if (oldpath == newpath) {
                break;
            }
            count++;
        }
        if (count == oldfiles.count()) {
            deletefile << newpath;
        }
    }

    qDebug() << deletefile;
    foreach (QString path, deletefile) {
        QFileInfo fileInfo(path);
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            QFile::setPermissions(path, QFile::WriteOwner);
            if (!QFile::remove(path)) {
                qDebug() << "delete error!!!!!!!!!";
            }
        } else if (fileInfo.isDir()) {
            QProcess p(0);
            QString command = "rm";
            QStringList args;
            args.append("-fr");
            args.append(path);
            p.execute(command, args);
            p.waitForFinished();
        }
    }
}

void MainWindow::creatArchive(QMap<QString, QString> &Args)
{
    const QStringList filesToAdd = m_CompressPage->getCompressFilelist();
    const QString fixedMimeType = Args[QStringLiteral("fixedMimeType")];
    const QString password = Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = Args[QStringLiteral("encryptHeader")];
    QString filename = Args[QStringLiteral("localFilePath")] + "/" + Args[QStringLiteral("filename")];
    m_decompressfilename = Args[QStringLiteral("filename")];
    m_CompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);

    if (filename.isEmpty()) {
        qDebug() << "filename.isEmpty()";
        return;
    }

    renameCompress(filename, fixedMimeType);
    qDebug() << filename;


    CompressionOptions options;
    options.setCompressionLevel(Args[QStringLiteral("compressionLevel")].toInt());
//    options.setCompressionMethod(Args[QStringLiteral("compressionMethod")]);
    options.setEncryptionMethod(Args[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize(Args[QStringLiteral("volumeSize")].toULongLong());


    QVector<Archive::Entry *> all_entries;

    foreach (QString file, filesToAdd) {
        Archive::Entry *entry = new Archive::Entry();
        entry->setFullPath(file);
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

    m_createJob = Archive::create(filename, fixedMimeType, all_entries, options, this);

    if (!password.isEmpty()) {
        m_createJob->enableEncryption(password, enableHeaderEncryption.compare("true") ? false : true);
    }

    connect(m_createJob, &KJob::result, this, &MainWindow::slotCompressFinished);
    connect(m_createJob, SIGNAL(percent(KJob *, ulong)),
            this, SLOT(SlotProgress(KJob *, ulong)));
    connect(m_createJob, &CreateJob::percentfilename,
            this, &MainWindow::SlotProgressFile);

    m_pageid = PAGE_ZIPPROGRESS;
    m_Progess->settype(COMPRESSING);
    refreshPage();

    m_pathstore = Args[QStringLiteral("localFilePath")];
    m_compressDirFiles = CheckAllFiles(m_pathstore);

    m_createJob->start();
    m_workstatus = WorkProcess;



}

void MainWindow::slotCompressFinished(KJob *job)
{
    qDebug() << "job finished" << job->error();
    m_workstatus = WorkNone;
    if (job->error() &&  job->error() != KJob::KilledJobError) {
        if(m_pathstore.left(6) == "/media")
        {
            if(getMediaFreeSpace() <= 50)
            {
                m_CompressFail->setFailStrDetail(tr("No space left, please clean and retry"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
            else {
                m_CompressFail->setFailStrDetail(tr("Compressed file is corrupt!"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        }
        else {
            if (getDiskFreeSpace() <= 50) {
                m_CompressFail->setFailStrDetail(tr("No space left, please clean and retry"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
            else {
                m_CompressFail->setFailStrDetail(tr("Compressed file is corrupt!"));
                m_pageid = PAGE_UNZIP_FAIL;
                refreshPage();
            }
        }
        return;
    }
    m_pageid = PAGE_ZIP_SUCCESS;
    refreshPage();
//    if(m_createJob)
//    {
//        m_createJob->kill();
//    }
}

void MainWindow::slotExtractSimpleFiles(QVector<Archive::Entry *> fileList, QString path)
{
    m_progressTransFlag = false;
    m_workstatus = WorkProcess;
    m_encryptiontype = Encryption_SingleExtract;
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
    qDebug() << "destinationDirectory:" << destinationDirectory;
    m_encryptionjob = m_model->extractFiles(fileList, destinationDirectory, options);

    connect(m_encryptionjob, SIGNAL(percent(KJob *, ulong)),
            this, SLOT(SlotProgress(KJob *, ulong)));
    connect(m_encryptionjob, &KJob::result,
            this, &MainWindow::slotExtractionDone);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
    connect(m_encryptionjob, SIGNAL(percentfilename(KJob *, const QString &)),
            this, SLOT(SlotProgressFile(KJob *, const QString &)));

    m_encryptionjob->start();
    m_decompressfilepath = destinationDirectory;

    QFileInfo file(m_loadfile);
    m_progressdialog->setCurrentTask(file.fileName());

}

void MainWindow::slotKillExtractJob()
{
    m_workstatus = WorkNone;
    if (m_encryptionjob) {
        m_encryptionjob->Killjob();
    }
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

void MainWindow::onCancelCompressPressed()
{
    deleteCompressFile(m_compressDirFiles, CheckAllFiles(m_pathstore));

    if (m_encryptionjob) {
        m_encryptionjob->Killjob();
    }
    if (m_createJob) {
        m_createJob->kill();
    }

    emit sigquitApp();
}

void MainWindow::onTitleButtonPressed()
{
    switch (m_pageid) {
    case PAGE_ZIP:
        emit sigZipAddFile();
        break;
    case PAGE_ZIPSET:
        emit sigZipReturn();
        m_pageid = PAGE_ZIP;
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

