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


#include <QSvgWidget>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include "pluginmanager.h"
//#include "archivejob.h"
#include "jobs.h"


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
      m_settings("deepin", "deepin-font-installer"),
      m_themeAction(new QAction(tr("Dark theme"), this))
{
    m_encryptionjob = nullptr;
    m_encryptiontype = Encryption_NULL;
    m_model = new ArchiveModel(this);
    InitUI();
    InitConnection();
}

MainWindow::~MainWindow()
{
}

void MainWindow::InitUI()
{
//    m_extractdialog->exec();
    // add widget to main layout.
    m_mainLayout->addWidget(m_homePage);
    m_mainLayout->addWidget(m_UnCompressPage);
    m_mainLayout->addWidget(m_CompressPage);
    m_mainLayout->addWidget(m_CompressSetting);
    m_mainLayout->addWidget(m_Progess);
    m_mainLayout->addWidget(m_CompressSuccess);
    m_mainLayout->addWidget(m_CompressFail);
    m_mainLayout->addWidget(m_encryptionpage);

    // init window flags.
    setWindowTitle(tr("Deepin Archive Manager"));
    setCentralWidget(m_mainWidget);
    setAcceptDrops(true);

    // init settings.
    if (!m_settings.contains("darkTheme")) {
        m_settings.setValue("darkTheme", false);
    }

    initTitleBar();


}

void MainWindow::InitConnection()
{
    // connect the signals to the slot function.
    connect(m_homePage, &HomePage::fileSelected, this, &MainWindow::onSelected);
    connect(m_CompressPage, &CompressPage::sigNextPress, this, &MainWindow::onCompressNext);
    connect(this, &MainWindow::sigZipAddFile, m_CompressPage, &CompressPage::onAddfileSlot);
    connect(this, &MainWindow::sigZipReturn, m_CompressSetting, &CompressSetting::onRetrunPressed);
    connect(m_CompressSetting, &CompressSetting::sigCompressPressed, this, &MainWindow::onCompressPressed);
    connect(m_Progess, &Progress::sigCancelPressed, this, &MainWindow::onCancelCompressPressed);
    connect(m_CompressSuccess, &Compressor_Success::sigQuitApp, this, &MainWindow::onCancelCompressPressed);
    connect(m_titlebutton, &DPushButton::clicked, this, &MainWindow::onTitleButtonPressed);
    connect(this, &MainWindow::sigZipSelectedFiles, m_CompressPage, &CompressPage::onSelectedFilesSlot);
    connect(m_model, &ArchiveModel::loadingFinished,this, &MainWindow::slotLoadingFinished);
    connect(m_UnCompressPage, &UnCompressPage::sigDecompressPress,this, &MainWindow::slotextractSelectedFilesTo);
    connect(m_encryptionpage, &EncryptionPage::sigExtractPassword,this, &MainWindow::SlotExtractPassword);
    connect(m_UnCompressPage, &UnCompressPage::sigextractfiles,this, &MainWindow::slotExtractSimpleFiles);
    connect(m_progressdialog, &ProgressDialog::stopExtract,this, &MainWindow::slotKillExtractJob);
}

QMenu* MainWindow::createSettingsMenu()
{
    QMenu *menu = new QMenu;

    QAction *settingsAction = menu->addAction(tr("设置"));
    connect(settingsAction, &QAction::triggered, this, [this] {
//        m_settingsDialog->show();
    });

    return menu;
}


void MainWindow::initTitleBar()
{
    titlebar()->setMenu(createSettingsMenu());
    titlebar()->setFixedHeight(50);


    m_logo = new DLabel("");
    m_logoicon = Utils::renderSVG(":/images/compress-Typeface-player.svg", QSize(30, 30));
    m_logo->setPixmap(m_logoicon);


    m_titlebutton = new DPushButton();
    m_titlebutton->setText("+");
    m_titlebutton->setFixedSize(30, 30);
    m_titlebutton->setVisible(false);

    m_titleFrame = new QFrame;
    m_titleFrame->setObjectName("TitleBar");
    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    leftLayout->addSpacing(12);
    leftLayout->addWidget(m_logo);
    leftLayout->addSpacing(12);
    leftLayout->addWidget(m_titlebutton);
    leftLayout->setSpacing(0);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *left_frame = new QFrame();
    left_frame->setFixedWidth(12+12+26+30);
    left_frame->setContentsMargins(0, 0, 0, 0);
    left_frame->setLayout(leftLayout);

    m_titlelabel = new DLabel("");
    m_titlelabel->setFixedSize(242, 40);

    QHBoxLayout *titlemainLayout = new QHBoxLayout;
    titlemainLayout->setSpacing(0);
    titlemainLayout->setContentsMargins(0, 0, 0, 0);
    titlemainLayout->addWidget(left_frame);
    titlemainLayout->addSpacing(150);
    titlemainLayout->addWidget(m_titlelabel, 1, Qt::AlignCenter);
    titlemainLayout->addSpacing(50);

    m_titleFrame->setLayout(titlemainLayout);
    m_titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);
    titlebar()->setContentsMargins(0, 0, 0, 0);
    titlebar()->setCustomWidget(m_titleFrame, false);
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
    m_titlebutton->setText("");
    m_titlebutton->setVisible(false);
    switch (m_pageid) {
    case PAGE_HOME:
        m_mainLayout->setCurrentIndex(0);
        m_titlelabel->setText("");
        break;
    case PAGE_UNZIP:
        m_mainLayout->setCurrentIndex(1);
        m_titlelabel->setText(m_decompressfilename);
        break;
    case PAGE_ZIP:
        m_mainLayout->setCurrentIndex(2);
        m_titlelabel->setText(tr("新建归档文件"));
        m_titlebutton->setText("+");
        m_titlebutton->setVisible(true);
        break;
    case PAGE_ZIPSET:
        m_mainLayout->setCurrentIndex(3);
        m_titlelabel->setText(tr("新建归档文件"));
        m_titlebutton->setText("<");
        m_titlebutton->setVisible(true);
        break;
    case PAGE_ZIPPROGRESS:
        m_mainLayout->setCurrentIndex(4);
        m_titlelabel->setText(tr("正在压缩"));
        m_Progess->setFilename(m_decompressfilename);
        break;
    case PAGE_UNZIPPROGRESS:
        m_mainLayout->setCurrentIndex(4);
        m_titlelabel->setText(tr("正在解压"));
        m_Progess->setFilename(m_decompressfilename);
        break;
    case PAGE_ZIP_SUCCESS:
        m_mainLayout->setCurrentIndex(5);
        m_titlelabel->setText("");
        m_CompressSuccess->setstringinfo(tr("压缩成功!"));
        break;
    case PAGE_ZIP_FAIL:
        m_mainLayout->setCurrentIndex(6);
        m_titlelabel->setText("");
        break;
    case PAGE_UNZIP_SUCCESS:
        m_mainLayout->setCurrentIndex(5);
        m_titlelabel->setText("");
        m_CompressSuccess->setCompressPath(m_decompressfilepath);
        m_CompressSuccess->setstringinfo(tr("解压成功！"));
        break;
    case PAGE_UNZIP_FAIL:
        m_mainLayout->setCurrentIndex(6);
        m_titlelabel->setText("");
        break;
    case  PAGE_ENCRYPTION:
        m_mainLayout->setCurrentIndex(7);
        m_titlelabel->setText(m_decompressfilename);
        break;
    default:
        break;
    }

}

void MainWindow::onSelected(const QStringList &files)
{
    if(files.count() == 1 && Utils::isCompressed_file(files.at(0)))
    {

        QFileInfo fileinfo(files.at(0));
        m_decompressfilename = fileinfo.fileName();
        m_UnCompressPage->setdefaultpath(fileinfo.path());

        loadArchive(files.at(0));
    }
    else {
        m_pageid = PAGE_ZIP;
        emit sigZipSelectedFiles(files);
        refreshPage();
    }
}

void MainWindow::slotLoadingFinished(KJob *job)
{
    if (job->error()) {
        return;
    }
    m_UnCompressPage->setModel(m_model);
    m_pageid = PAGE_UNZIP;
    m_homePage->spinnerStop();
    refreshPage();
}


void MainWindow::loadArchive(const QString &files)
{
    m_loadfile = files;
    m_encryptiontype = Encryption_Load;
    m_loadjob = (LoadJob*)m_model->loadArchive(files, "", m_model);

    connect(m_loadjob, &LoadJob::sigLodJobPassword,
            this, &MainWindow::SlotNeedPassword);

    if (m_loadjob) {
        m_loadjob->start();
        m_homePage->spinnerStart();
    }
}

void MainWindow::slotextractSelectedFilesTo(const QString& localPath)
{
    m_encryptiontype = Encryption_Extract;
    if (!m_model) {
        return;
    }

    if(m_encryptionjob)
    {
        m_encryptionjob = nullptr;
    }

    ExtractionOptions options;
    QVector<Archive::Entry*> files;

    const QString destinationDirectory = localPath;
    qDebug()<<"destinationDirectory:"<<destinationDirectory;
    m_encryptionjob = m_model->extractFiles(files, destinationDirectory, options);

    connect(m_encryptionjob, SIGNAL(percent(KJob*,ulong)),
            this, SLOT(SlotProgress(KJob*,ulong)));
    connect(m_encryptionjob, &KJob::result,
            this, &MainWindow::slotExtractionDone);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
    connect(m_encryptionjob, SIGNAL(percentfilename(KJob*, const QString &)),
            this, SLOT(SlotProgressFile(KJob*, const QString &)));

    m_encryptionjob->start();
    m_decompressfilepath = destinationDirectory;

}

void MainWindow::SlotProgress(KJob *job, unsigned long percent)
{
    qDebug()<<percent;
    if((Encryption_SingleExtract == m_encryptiontype) && (percent < 100) && (percent > 0))
    {
        if(!m_progressdialog->isshown())
        {
            m_pageid = PAGE_UNZIP;
            refreshPage();
            m_progressdialog->showdialog();
        }
        m_progressdialog->setProcess(percent);

    }
    else if(PAGE_ZIPPROGRESS == m_pageid || PAGE_UNZIPPROGRESS == m_pageid)
    {
        m_Progess->setprogress(percent);
    }
    else if((PAGE_UNZIP == m_pageid || PAGE_ENCRYPTION == m_pageid) && (percent < 100) && (percent > 0))
    {
        m_pageid = PAGE_UNZIPPROGRESS;
        m_Progess->settype(DECOMPRESSING);
        refreshPage();
    }
    else if((PAGE_ZIPSET == m_pageid) && (percent < 100) && (percent > 0))
    {
        m_pageid = PAGE_ZIPPROGRESS;
        m_Progess->settype(COMPRESSING);
        refreshPage();
    }
}

void MainWindow::SlotProgressFile(KJob *job, const QString& filename)
{
    if(Encryption_SingleExtract == m_encryptiontype && PAGE_UNZIP == m_pageid)
    {
        m_progressdialog->setCurrentFile(filename);
    }
    else if(PAGE_ZIPPROGRESS == m_pageid || PAGE_UNZIPPROGRESS == m_pageid)
    {
        m_Progess->setProgressFilename(filename);
    }
}

void MainWindow::slotExtractionDone(KJob* job)
{
    if (job->error() && job->error() != KJob::KilledJobError) {

    }
    else if(Encryption_SingleExtract == m_encryptiontype)
    {
        m_progressdialog->setFinished(m_decompressfilepath);
    }
    else {
        m_pageid = PAGE_UNZIP_SUCCESS;
        refreshPage();
    }
    m_encryptionjob = nullptr;
}

void MainWindow::SlotNeedPassword()
{

    m_pageid = PAGE_ENCRYPTION;
    refreshPage();
}

void MainWindow::SlotExtractPassword(QString password)
{
    if(Encryption_Load == m_encryptiontype)
    {
        LoadPassword(password);
    }
    else if(Encryption_Extract == m_encryptiontype)
    {
        ExtractPassword(password);
    }
    else if(Encryption_SingleExtract == m_encryptiontype)
    {
        ExtractSinglePassword(password);
    }
}

void MainWindow::ExtractSinglePassword(QString password)
{
    if(m_encryptionjob)//first  time to extract
    {
        m_encryptionjob->archiveInterface()->setPassword(password);

        m_encryptionjob->start();

    }
    else //second or more  time to extract
    {
        ExtractionOptions options;

        m_encryptionjob = m_model->extractFiles(m_extractSimpleFiles, m_decompressfilepath, options);
        m_encryptionjob->archiveInterface()->setPassword(password);
        connect(m_encryptionjob, SIGNAL(percent(KJob*,ulong)),
                this, SLOT(SlotProgress(KJob*,ulong)));
        connect(m_encryptionjob, &KJob::result,
                this, &MainWindow::slotExtractionDone);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(m_encryptionjob, SIGNAL(percentfilename(KJob*, const QString &)),
                this, SLOT(SlotProgressFile(KJob*, const QString &)));

        m_encryptionjob->start();
    }
}

void MainWindow::ExtractPassword(QString password)
{
    if(m_encryptionjob)//first  time to extract
    {
        m_encryptionjob->archiveInterface()->setPassword(password);

        m_encryptionjob->start();

    }
    else //second or more  time to extract
    {
        ExtractionOptions options;
        QVector<Archive::Entry*> files;

        m_encryptionjob = m_model->extractFiles(files, m_decompressfilepath, options);
        m_encryptionjob->archiveInterface()->setPassword(password);
        connect(m_encryptionjob, SIGNAL(percent(KJob*,ulong)),
                this, SLOT(SlotProgress(KJob*,ulong)));
        connect(m_encryptionjob, &KJob::result,
                this, &MainWindow::slotExtractionDone);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
        connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
                m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
        connect(m_encryptionjob, SIGNAL(percentfilename(KJob*, const QString &)),
                this, SLOT(SlotProgressFile(KJob*, const QString &)));

        m_encryptionjob->start();
    }
}
void MainWindow::LoadPassword(QString password)
{
    m_encryptiontype = Encryption_Load;
    m_loadjob = (LoadJob*)m_model->loadArchive(m_loadfile, "", m_model);

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

    QFileInfo fileinfobase(fileslist.at(0));
    for(int loop = 1; loop < fileslist.count();loop++)
    {
        QFileInfo fileinfo(fileslist.at(loop));
        if(fileinfo.path() != fileinfobase.path())
        {
            m_CompressSetting->setDefaultPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
            return;
        }
    }

    m_CompressSetting->setDefaultPath(fileinfobase.path());
    if(1 == fileslist.count())
    {
        m_CompressSetting->setDefaultName(fileinfobase.baseName());
    }
    else {
        m_CompressSetting->setDefaultName(tr("新建归档文件"));
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

void MainWindow::creatArchive(QMap<QString, QString> &Args)
{
    const QStringList filesToAdd = m_CompressPage->getCompressFilelist();
    const QString fixedMimeType = Args[QStringLiteral("fixedMimeType")];
    const QString password = Args[QStringLiteral("encryptionPassword")];
    const QString enableHeaderEncryption = Args[QStringLiteral("encryptHeader")];
    const QString filename = Args[QStringLiteral("localFilePath")] + "/" + Args[QStringLiteral("filename")];
    m_decompressfilename = Args[QStringLiteral("filename")];
    m_CompressSuccess->setCompressPath(Args[QStringLiteral("localFilePath")]);

    if (filename.isEmpty()) {
        qDebug()<<"filename.isEmpty()";
        return;
    }

    CompressionOptions options;
    options.setCompressionLevel(Args[QStringLiteral("compressionLevel")].toInt());
//    options.setCompressionMethod(Args[QStringLiteral("compressionMethod")]);
    options.setEncryptionMethod(Args[QStringLiteral("encryptionMethod")]);
    options.setVolumeSize(Args[QStringLiteral("volumeSize")].toULongLong());


    QVector<Archive::Entry*> all_entries;

    foreach(QString file, filesToAdd)
    {
        Archive::Entry *entry = new Archive::Entry();
        entry->setFullPath(file);
        all_entries.append(entry);
    }


    if (all_entries.isEmpty()) {
        qDebug()<<"all_entries.isEmpty()";
        return;
    }

    QString globalWorkDir = filesToAdd.first();
    if (globalWorkDir.right(1) == QLatin1String("/")) {
        globalWorkDir.chop(1);
    }
    globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
    options.setGlobalWorkDir(globalWorkDir);

//    QString firstPath = Args[QStringLiteral("localFilePath")];
//    const QDir stripDir(firstPath);
//    for (Archive::Entry *entry : qAsConst(all_entries)) {
//        entry->setFullPath(stripDir.absoluteFilePath(entry->fullPath()));
//    }

//    qDebug() << "Setting GlobalWorkDir to " << stripDir.path();
//    options.setGlobalWorkDir(stripDir.path());

    CreateJob* createJob = Archive::create(filename, fixedMimeType, all_entries, options, this);

    if (!password.isEmpty()) {
        createJob->enableEncryption(password, enableHeaderEncryption.compare("true")?false:true);
    }

    connect(createJob, &KJob::result, this, &MainWindow::slotCompressFinished);
    connect(createJob, SIGNAL(percent(KJob*,ulong)),
            this, SLOT(SlotProgress(KJob*,ulong)));
    connect(createJob, &CreateJob::percentfilename,
            this, &MainWindow::SlotProgressFile);
    createJob->start();

}

void MainWindow::slotCompressFinished(KJob *job)
{
    qDebug() << "job finished";

    if (job->error() && !job->errorString().isEmpty()) {
    }
    m_pageid = PAGE_ZIP_SUCCESS;
    refreshPage();

}

void MainWindow::slotExtractSimpleFiles(QVector<Archive::Entry*> fileList, QString path)
{
    m_encryptiontype = Encryption_SingleExtract;
    if (!m_model) {
        return;
    }

    if(m_encryptionjob)
    {
        m_encryptionjob = nullptr;
    }

    ExtractionOptions options;
    m_extractSimpleFiles = fileList;

    const QString destinationDirectory = path;
    qDebug()<<"destinationDirectory:"<<destinationDirectory;
    m_encryptionjob = m_model->extractFiles(fileList, destinationDirectory, options);

    connect(m_encryptionjob, SIGNAL(percent(KJob*,ulong)),
            this, SLOT(SlotProgress(KJob*,ulong)));
    connect(m_encryptionjob, &KJob::result,
            this, &MainWindow::slotExtractionDone);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            this, &MainWindow::SlotNeedPassword, Qt::QueuedConnection);
    connect(m_encryptionjob, &ExtractJob::sigExtractJobPassword,
            m_encryptionpage, &EncryptionPage::wrongPassWordSlot);
    connect(m_encryptionjob, SIGNAL(percentfilename(KJob*, const QString &)),
            this, SLOT(SlotProgressFile(KJob*, const QString &)));

    m_encryptionjob->start();
    m_decompressfilepath = destinationDirectory;

    QFileInfo file(m_loadfile);
    m_progressdialog->setCurrentTask(file.fileName());

}

void MainWindow::slotKillExtractJob()
{
    if(m_encryptionjob)
    {
        m_encryptionjob->Killjob();
    }
}


void MainWindow::onCancelCompressPressed()
{
    if(m_encryptionjob)
    {
        m_encryptionjob->Killjob();
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

