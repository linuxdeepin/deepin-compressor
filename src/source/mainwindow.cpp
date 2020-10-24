/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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
#include "loadingpage.h"
#include "homepage.h"
#include "compresspage.h"
#include "compresssettingpage.h"
#include "uncompresspage.h"
#include "progresspage.h"
#include "successpage.h"
#include "failurepage.h"
#include "uitools.h"
#include "pluginmanager.h"
#include "settingdialog.h"
#include "archivemanager.h"
#include "DebugTimeManager.h"
#include "popupdialog.h"

#include <DFileDialog>
#include <DTitlebar>

#include <QStackedWidget>
#include <QKeyEvent>
#include <QSettings>
#include <QDebug>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>

static QMutex mutex;

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{

    setWindowTitle(tr("Archive Manager"));

    // 先构建初始界面
    m_pMainWidget = new QStackedWidget(this);  // 中心面板
    m_pHomePage = new HomePage(this);            // 首页
    m_pMainWidget->addWidget(m_pHomePage);
    setCentralWidget(m_pMainWidget);    // 设置中心面板
    m_pMainWidget->setCurrentIndex(0);
    setMinimumSize(620, 465);

    initTitleBar();

    m_iInitUITimer = startTimer(500);
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    // 初始化界面
    m_pLoadingPage = new LoadingPage(this);  // 加载界面

    m_pCompressPage = new CompressPage(this);    // 压缩列表界面
    m_pCompressSettingPage = new CompressSettingPage(this);  // 压缩设置界面
    m_pUnCompressPage = new UnCompressPage(this);    // 解压列表界面
    m_pProgressPage = new ProgressPage(this);    // 进度界面
    m_pSuccessPage = new SuccessPage(this);  // 成功界面
    m_pFailurePage = new FailurePage(this);  // 失败界面

    m_pSettingDlg = new SettingDialog(this);

    m_pArchiveManager = new ArchiveManager(this);

    // 添加界面至主界面
    m_pMainWidget->addWidget(m_pCompressPage);
    m_pMainWidget->addWidget(m_pCompressSettingPage);
    m_pMainWidget->addWidget(m_pUnCompressPage);
    m_pMainWidget->addWidget(m_pProgressPage);
    m_pMainWidget->addWidget(m_pSuccessPage);
    m_pMainWidget->addWidget(m_pFailurePage);
    m_pMainWidget->addWidget(m_pLoadingPage);


}

void MainWindow::initTitleBar()
{
    // 创建菜单
    QMenu *menu = new QMenu();
    m_pOpenAction = menu->addAction(tr("Open file"), this, &MainWindow::slotChoosefiles);
    menu->addAction(tr("Settings"), this, [ = ] { m_pSettingDlg->exec(); });
    menu->addSeparator();

    // 初始化标题栏菜单
    titlebar()->setMenu(menu);
    titlebar()->setFixedHeight(50);
    titlebar()->setTitle("");

    // 设置应用程序图标
    QIcon icon = QIcon::fromTheme("deepin-compressor");
    titlebar()->setIcon(icon);

    // 添加左上角按钮
    m_pTitleButton = new DIconButton(DStyle::SP_IncreaseElement, this);
    m_pTitleButton->setFixedSize(38, 38);
    m_pTitleButton->setVisible(true);
    m_pTitleButton->setObjectName("TitleButton");

    // 左上角按钮布局
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

void MainWindow::initData()
{
    // 初始化数据配置
    m_pSettings = new QSettings(QDir(UiTools::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this);

    if (m_pSettings->value("dir").toString().isEmpty()) {
        m_pSettings->setValue("dir", "");
    }
}

void MainWindow::initConnections()
{
    connect(m_pTitleButton, &DIconButton::clicked, this, &MainWindow::slotChoosefiles);
    connect(m_pHomePage, &HomePage::signalFileChoose, this, &MainWindow::slotChoosefiles);
    connect(m_pHomePage, &HomePage::signalDragFiles, this, &MainWindow::slotDragSelectedFiles);
    connect(m_pCompressPage, &CompressPage::signalLevelChanged, this, &MainWindow::slotCompressLevelChanged);
    connect(m_pCompressPage, &CompressPage::signalCompressNextClicked, this, &MainWindow::slotCompressNext);
    connect(m_pCompressSettingPage, &CompressSettingPage::signalCompressClicked, this, &MainWindow::slotCompress);
    connect(m_pUnCompressPage, &UnCompressPage::signalUncompress, this, &MainWindow::slotUncompressSlicked);

    connect(m_pArchiveManager, &ArchiveManager::signalJobFinished, this, &MainWindow::slotJobFinshed);
    connect(m_pArchiveManager, &ArchiveManager::signalprogress, this, &MainWindow::slotReceiveProgress);
    connect(m_pArchiveManager, &ArchiveManager::signalCurFileName, this, &MainWindow::slotReceiveCurFileName);
    connect(m_pArchiveManager, &ArchiveManager::signalQuery, this, &MainWindow::slotQuery, Qt::ConnectionType::DirectConnection);
}

void MainWindow::refreshPage()
{
    switch (m_ePageID) {
    case PI_Home: {
        m_pMainWidget->setCurrentIndex(0);
    }
    break;
    case PI_Compress: {
        m_pMainWidget->setCurrentIndex(1);
        if (m_iCompressedWatchTimerID == 0) {
            m_iCompressedWatchTimerID = startTimer(1);
        }
    }
    break;
    case PI_CompressSetting: {
        m_pMainWidget->setCurrentIndex(2);
        if (m_iCompressedWatchTimerID == 0) {
            m_iCompressedWatchTimerID = startTimer(1);
        }
    }
    break;
    case PI_UnCompress: {
        m_pMainWidget->setCurrentIndex(3);
    }
    break;
    case PI_CompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Compressing"));
    }
    break;
    case PI_UnCompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Extracting"));
    }
    break;
    case PI_DeleteProgress: {
        m_pMainWidget->setCurrentIndex(4);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Deleting"));
    }
    break;
    case PI_ConvertProgress: {
        m_pMainWidget->setCurrentIndex(4);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle("Converting");
    }
    break;
    case PI_CompressSuccess: {
        m_pMainWidget->setCurrentIndex(5);
        m_pSuccessPage->setSuccessDes(tr("Compression successful"));
        if (0 != m_iCompressedWatchTimerID) {
            killTimer(m_iCompressedWatchTimerID);
            m_iCompressedWatchTimerID = 0;
        }
    }
    break;
    case PI_UnCompressSuccess: {
        m_pMainWidget->setCurrentIndex(5);
        m_pSuccessPage->setSuccessDes(tr("Extraction successful"));
    }
    break;
    case PI_ConvertSuccess: {
        m_pMainWidget->setCurrentIndex(5);
    }
    break;
    case PI_CompressFailure: {
        m_pMainWidget->setCurrentIndex(6);
        if (0 != m_iCompressedWatchTimerID) {
            killTimer(m_iCompressedWatchTimerID);
            m_iCompressedWatchTimerID = 0;
        }
    }
    break;
    case PI_UnCompressFailure: {
        m_pMainWidget->setCurrentIndex(6);
    }
    break;
    case PI_ConvertFailure: {
        m_pMainWidget->setCurrentIndex(6);
    }
    break;
    case PI_Loading: {
        m_pMainWidget->setCurrentIndex(7);
    }
    break;
    }
}

qint64 MainWindow::calSelectedTotalFileSize(const QStringList &files)
{
    m_qTotalSize = 0;

    foreach (QString file, files) {
        QFileInfo fi(file);

        if (fi.isFile()) {  // 如果为文件，直接获取大小
            qint64 curFileSize = fi.size();

#ifdef __aarch64__
            if (maxFileSize_ < curFileSize) {
                maxFileSize_ = curFileSize;
            }
#endif
            m_qTotalSize += curFileSize;
        } else if (fi.isDir()) {    // 如果是文件夹，递归获取所有子文件大小总和
            QtConcurrent::run(this, &MainWindow::calFileSizeByThread, file);
        }
    }

    // 等待线程池结束
    QThreadPool::globalInstance()->waitForDone();

    return m_qTotalSize;
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
            m_qTotalSize += curFileSize;
            mutex.unlock();
        }

        i++;
    } while (i < list.size());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    static int index = 0;
    if (event->key() == Qt::Key_Space) {
        index++;
        if (index >= m_pMainWidget->count())
            index = 0;
        m_pMainWidget->setCurrentIndex(index);
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (m_iInitUITimer == event->timerId()) {
        // 初始化界面
        qDebug() << "初始化界面";
        initUI();
        initConnections();
        initData();

        killTimer(m_iInitUITimer);
        m_iInitUITimer = 0;
    } else if (m_iCompressedWatchTimerID == event->timerId()) {
        // 对压缩文件的监控
        QStringList listFiles = m_pCompressPage->compressFiles();

        for (int i = 0; i < listFiles.count(); i++) {
            QFileInfo info(listFiles[i]);

            if (!info.exists()) {
                QString displayName = UiTools::toShortString(info.fileName());
                QString strTips = tr("%1 was changed on the disk, please import it again.").arg(displayName);

                TipDialog dialog(this);
                dialog.showDialog(strTips, tr("OK"), DDialog::ButtonNormal);

                m_pCompressPage->refreshCompressedFiles(true, listFiles[i]);
            }
        }
    }
}

void MainWindow::slotChoosefiles()
{
    // 创建文件选择对话框
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setAllowMixedSelection(true);

    // 获取配置中历史选择路径
    QString historyDir = m_pSettings->value("dir").toString();

    // 若历史选择路径为空，则默认为主目录
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }

    dialog.setDirectory(historyDir);    // 设置对话框打开时的路径

    const int mode = dialog.exec();

    // 保存选中路径至配置中
    m_pSettings->setValue("dir", dialog.directoryUrl().toLocalFile());

    // 关闭或取消不处理
    if (mode != QDialog::Accepted) {
        return;
    }

    QStringList selectedFiles = dialog.selectedFiles();
    qDebug() << "选择的文件：" << selectedFiles;

    if (m_ePageID == PI_Compress) {
        m_pCompressPage->addCompressFiles(selectedFiles);       // 添加压缩文件
    }
}

void MainWindow::slotDragSelectedFiles(const QStringList &listFiles)
{
    // 未选择任何文件
    if (listFiles.count() == 0) {
        return;
    }

    /* 当选择的文件数目大于1时，执行压缩流程
    *  若数目为1，根据文件格式判断是压缩还是需要解压
    */
    if (listFiles.count() > 1) {
        m_ePageID = PI_Compress;
        m_pCompressPage->addCompressFiles(listFiles);       // 添加压缩文件
    } else {
        if (UiTools::isArchiveFile(listFiles[0])) {     // 压缩文件处理

            PERF_PRINT_BEGIN("POINT-05", "加载时间");
            m_pUnCompressPage->setArchiveName(listFiles[0]);

            m_ePageID = PI_Loading;
            m_pLoadingPage->startLoading();
            m_pArchiveManager->loadArchive(listFiles[0]);
            m_pUnCompressPage->setDefaultUncompressPath(QFileInfo(listFiles[0]).absolutePath());


        } else {        // 普通文件处理
            m_ePageID = PI_Compress;
            m_pCompressPage->addCompressFiles(listFiles);       // 添加压缩文件
        }
    }

    refreshPage();      // 刷新界面
}

void MainWindow::slotCompressLevelChanged(bool bRootIndex)
{
    m_pOpenAction->setEnabled(bRootIndex);
    m_pTitleButton->setVisible(bRootIndex);
}

void MainWindow::slotCompressNext()
{
    m_qTotalSize = 0;
    QStringList listCompressFiles = m_pCompressPage->compressFiles();       // 获取待压缩的文件
    m_pCompressSettingPage->setFileSize(listCompressFiles, calSelectedTotalFileSize(listCompressFiles));
    m_pCompressSettingPage->refreshMenu();

    // 刷新界面 切换到压缩设置界面
    m_ePageID = PI_CompressSetting;
    refreshPage();
}

void MainWindow::slotCompress(const QVariant &val)
{
    qDebug() << "点击了压缩按钮";

    CompressParameter stCompressInfo = val.value<CompressParameter>();    // 获取压缩参数
    QStringList listFiles = m_pCompressPage->compressFiles();   // 获取待压缩文件

    if (listFiles.count() == 0) {
        qDebug() << "没有需要压缩的文件";
        return;
    }

    // 创建压缩所需相关数据，调用压缩参数
    QVector<FileEntry> vecFiles;
    QString strDestination;
    CompressOptions options;
    bool bBatch = false;

    QSet< QString > globalWorkDirList;
    // 构建压缩文件数据
    foreach (QString strFile, listFiles) {
        FileEntry stFileEntry;
        stFileEntry.strFullPath = strFile;
        vecFiles.push_back(stFileEntry);

        QString globalWorkDir = strFile;
        if (globalWorkDir.right(1) == QLatin1String("/")) {
            globalWorkDir.chop(1);
        }

        globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
        globalWorkDirList.insert(globalWorkDir);
    }

    strDestination = stCompressInfo.strTargetPath + QDir::separator() + stCompressInfo.strArchiveName;

    // 构建压缩参数
    options.bEncryption = stCompressInfo.bEncryption;
    options.strPassword = stCompressInfo.strPassword;
    options.strEncryptionMethod = stCompressInfo.strEncryptionMethod;
    options.strCompressionMethod = stCompressInfo.strCompressionMethod;
    options.bHeaderEncryption = stCompressInfo.bHeaderEncryption;
    options.bSplit = stCompressInfo.bSplit;
    options.iVolumeSize = stCompressInfo.iVolumeSize;
    options.iCompressionLevel = stCompressInfo.iCompressionLevel;
    options.qTotalSize = m_qTotalSize;
    options.bTar_7z = stCompressInfo.bTar_7z;

    // 判断是否批量压缩（多路径）
    if (globalWorkDirList.count() == 1 || options.bTar_7z) {
        bBatch = false;
    } else {
        bBatch = true;
    }

    m_pArchiveManager->createArchive(vecFiles, strDestination, options, false, bBatch);


    m_pProgressPage->setProgressType(PT_Compress);
    m_pProgressPage->setArchiveName(stCompressInfo.strArchiveName, stCompressInfo.qSize);
    m_pProgressPage->restartTimer();

    m_ePageID = PI_CompressProgress;
    refreshPage();
}

void MainWindow::slotJobFinshed()
{
    ArchiveJob *pJob = m_pArchiveManager->archiveJob();
    if (pJob == nullptr) {
        return;
    }

    switch (pJob->m_eJobType) {
    case ArchiveJob::JT_Create: {
        qDebug() << "创建结束";
        m_ePageID = PI_CompressSuccess;
    }
    break;
    case ArchiveJob::JT_Add: {
        qDebug() << "添加结束";
    }
    break;
    case ArchiveJob::JT_Load: {
        m_pLoadingPage->stopLoading();
        m_ePageID = PI_UnCompress;
        qDebug() << "加载结束";

        ArchiveData stArchiveData;
        m_pArchiveManager->getLoadArchiveData(stArchiveData);
        m_pUnCompressPage->setArchiveData(stArchiveData);
    }
    break;
    case ArchiveJob::JT_Extract: {
        qDebug() << "解压结束";
        m_ePageID = PI_UnCompressSuccess;
    }
    break;
    }

    refreshPage();
    PERF_PRINT_END("POINT-03");
    PERF_PRINT_END("POINT-04");
    PERF_PRINT_END("POINT-05");
}

void MainWindow::slotUncompressSlicked(const QString &strUncompressPath)
{
    QString strArchiveName = m_pUnCompressPage->archiveName();
    ExtractionOptions options;
    ArchiveData stArchiveData;

    // 获取压缩包数据
    m_pArchiveManager->getLoadArchiveData(stArchiveData);

    // 构建解压参数
    options.strTargetPath = strUncompressPath;
    options.bAllExtract = true;
    options.qSize = stArchiveData.qSize;
    options.qComressSize = stArchiveData.qComressSize;

    // 调用解压函数
    m_pArchiveManager->extractArchive(QVector<FileEntry>(), strArchiveName, options);

    // 设置进度界面参数
    m_pProgressPage->setProgressType(PT_UnCompress);
    m_pProgressPage->setArchiveName(QFileInfo(strArchiveName).fileName(), options.qSize);
    m_pProgressPage->restartTimer(); // 重启计时器
    m_ePageID = PI_UnCompressProgress;
    refreshPage();
}

void MainWindow::slotReceiveProgress(double dPercentage)
{
//    qDebug() << "Percentage=" << dPercentage;
    m_pProgressPage->setProgress(qRound(dPercentage));
}

void MainWindow::slotReceiveCurFileName(const QString &strName)
{
//    qDebug() << "filename=" << strName;
    m_pProgressPage->setCurrentFileName(strName);
}

void MainWindow::slotQuery(Query *query)
{
    qDebug() << " query->execute()";
    query->setParent(this);
    query->execute();
}

void MainWindow::slotFileChanged(const QString &strFileName)
{
    QString displayName = UiTools::toShortString(QFileInfo(strFileName).fileName());
    QString strTips = tr("%1 was changed on the disk, please import it again.").arg(displayName);

    TipDialog dialog(this);
    dialog.showDialog(strTips, tr("OK"), DDialog::ButtonNormal);

    m_pCompressPage->refreshCompressedFiles(true, strFileName);
}
