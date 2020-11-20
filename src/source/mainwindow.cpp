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
#include "progressdialog.h"
#include "datamanager.h"
#include "ddesktopservicesthread.h"

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
    m_strUUID = createUUID();   // 生成应用唯一标识

    setWindowTitle(tr("Archive Manager"));

    // 先构建初始界面
    m_pMainWidget = new QStackedWidget(this);  // 中心面板
    m_pHomePage = new HomePage(this);            // 首页
    m_pMainWidget->addWidget(m_pHomePage);
    setCentralWidget(m_pMainWidget);    // 设置中心面板
    m_pMainWidget->setCurrentIndex(0);

    // 初始化标题栏
    initTitleBar();

    // 开启定时器刷新界面
    m_iInitUITimer = startTimer(500);

    // 加载界面保存属性
    loadWindowState();
}

MainWindow::~MainWindow()
{
    // 清除缓存数据
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-rf");
    args.append(TEMPPATH + QDir::separator() + m_strUUID);
    p.execute(command, args);
    p.waitForFinished();

    qDebug() << "应用正常退出";
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

    m_pProgressdialog = new ProgressDialog(this); //进度弹窗
    m_pSettingDlg = new SettingDialog(this);

    // 添加界面至主界面
    m_pMainWidget->addWidget(m_pCompressPage);
    m_pMainWidget->addWidget(m_pCompressSettingPage);
    m_pMainWidget->addWidget(m_pUnCompressPage);
    m_pMainWidget->addWidget(m_pProgressPage);
    m_pMainWidget->addWidget(m_pSuccessPage);
    m_pMainWidget->addWidget(m_pFailurePage);
    m_pMainWidget->addWidget(m_pLoadingPage);

    // 创建打开文件监控
    m_pOpenFileWatcher = new QFileSystemWatcher(this);
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
    m_pTitleButton->setFixedSize(36, 36);
    m_pTitleButton->setVisible(false);
    m_pTitleButton->setObjectName("TitleButton");
    m_pTitleButton->setAccessibleName("Title_btn");

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
    // 刷新压缩设置界面格式选项
    m_pCompressSettingPage->refreshMenu();

    // 初始化数据配置
    m_pSettings = new QSettings(QDir(UiTools::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this);

    if (m_pSettings->value("dir").toString().isEmpty()) {
        m_pSettings->setValue("dir", "");
    }
}

void MainWindow::initConnections()
{
    connect(m_pTitleButton, &DIconButton::clicked, this, &MainWindow::slotTitleBtnClicked);
    connect(m_pHomePage, &HomePage::signalFileChoose, this, &MainWindow::slotChoosefiles);
    connect(m_pHomePage, &HomePage::signalDragFiles, this, &MainWindow::slotDragSelectedFiles);
    connect(m_pCompressPage, &CompressPage::signalLevelChanged, this, &MainWindow::slotCompressLevelChanged);
    connect(m_pCompressPage, &CompressPage::signalCompressNextClicked, this, &MainWindow::slotCompressNext);
    connect(m_pCompressSettingPage, &CompressSettingPage::signalCompressClicked, this, &MainWindow::slotCompress);
    connect(m_pUnCompressPage, &UnCompressPage::signalUncompress, this, &MainWindow::slotUncompressClicked);
    connect(m_pUnCompressPage, &UnCompressPage::signalExtract2Path, this, &MainWindow::slotExtract2Path);
    connect(m_pUnCompressPage, &UnCompressPage::signalDelFiles, this, &MainWindow::slotDelFiles);
    connect(m_pUnCompressPage, &UnCompressPage::signalOpenFile, this, &MainWindow::slotOpenFile);
    connect(m_pUnCompressPage, &UnCompressPage::signalAddFiles2Archive, this, &MainWindow::slotAddFiles);
    connect(m_pSuccessPage, &SuccessPage::sigBackButtonClicked, this, &MainWindow::slotSuccessReturn);
    connect(m_pProgressPage, &ProgressPage::signalPause, this, &MainWindow::slotPause);
    connect(m_pProgressPage, &ProgressPage::signalContinue, this, &MainWindow::slotContinue);
    connect(m_pProgressPage, &ProgressPage::signalCancel, this, &MainWindow::slotCancel);

    connect(ArchiveManager::get_instance(), &ArchiveManager::signalJobFinished, this, &MainWindow::slotJobFinshed);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalprogress, this, &MainWindow::slotReceiveProgress);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalCurFileName, this, &MainWindow::slotReceiveCurFileName);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalCurArchiveName, this, &MainWindow::slotReceiveCurArchiveName);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalQuery, this, &MainWindow::slotQuery);

    connect(m_pOpenFileWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::slotOpenFileChanged);
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

void MainWindow::refreshPage()
{
    switch (m_ePageID) {
    case PI_Home: {
        m_pMainWidget->setCurrentIndex(0);
        setTitleButtonStyle(false);
        titlebar()->setTitle("");
    }
    break;
    case PI_Compress: {
        m_pMainWidget->setCurrentIndex(1);
        setTitleButtonStyle(true, DStyle::StandardPixmap::SP_IncreaseElement);
        if (m_iCompressedWatchTimerID == 0) {
            m_iCompressedWatchTimerID = startTimer(1);
        }
        titlebar()->setTitle(tr("Create New Archive"));
    }
    break;
    case PI_CompressSetting: {
        m_pMainWidget->setCurrentIndex(2);
        setTitleButtonStyle(true, DStyle::StandardPixmap::SP_ArrowLeave);
        if (m_iCompressedWatchTimerID == 0) {
            m_iCompressedWatchTimerID = startTimer(1);
        }
        titlebar()->setTitle(tr("Create New Archive"));
    }
    break;
    case PI_UnCompress: {
        m_pMainWidget->setCurrentIndex(3);
        setTitleButtonStyle(true, DStyle::StandardPixmap::SP_IncreaseElement);
        titlebar()->setTitle(QFileInfo(m_pUnCompressPage->archiveFullPath()).fileName());
    }
    break;
    case PI_AddCompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Adding files to %1").arg(QFileInfo(m_pUnCompressPage->archiveFullPath()).fileName()));
    }
    break;
    case PI_CompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Compressing"));
    }
    break;
    case PI_UnCompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Extracting"));
    }
    break;
    case PI_DeleteProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Deleting"));
    }
    break;
    case PI_ConvertProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false);
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
        titlebar()->setTitle("");
    }
    break;
    case PI_UnCompressSuccess: {
        m_pMainWidget->setCurrentIndex(5);
        setTitleButtonStyle(false);
        m_pSuccessPage->setSuccessDes(tr("Extraction successful"));
        titlebar()->setTitle("");
    }
    break;
    case PI_ConvertSuccess: {
        m_pMainWidget->setCurrentIndex(5);
        setTitleButtonStyle(false);
        titlebar()->setTitle("");
    }
    break;
    case PI_CompressFailure: {
        m_pMainWidget->setCurrentIndex(6);
        setTitleButtonStyle(false);
        if (0 != m_iCompressedWatchTimerID) {
            killTimer(m_iCompressedWatchTimerID);
            m_iCompressedWatchTimerID = 0;
        }
        titlebar()->setTitle("");
    }
    break;
    case PI_UnCompressFailure: {
        m_pMainWidget->setCurrentIndex(6);
        setTitleButtonStyle(false);
        titlebar()->setTitle("");
    }
    break;
    case PI_ConvertFailure: {
        m_pMainWidget->setCurrentIndex(6);
        setTitleButtonStyle(false);
        titlebar()->setTitle("");
    }
    break;
    case PI_LoadedFailure: {
        m_pMainWidget->setCurrentIndex(6);
        setTitleButtonStyle(false);
        titlebar()->setTitle("");
    }
    break;
    case PI_Loading: {
        m_pMainWidget->setCurrentIndex(7);
        setTitleButtonStyle(false);
        titlebar()->setTitle("");
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

void MainWindow::setTitleButtonStyle(bool bVisible, DStyle::StandardPixmap pixmap)
{
    m_pTitleButton->setVisible(bVisible);

    if (bVisible)
        m_pTitleButton->setIcon(pixmap);
}

void MainWindow::loadArchive(const QString &strArchiveFullPath)
{
    PERF_PRINT_BEGIN("POINT-05", "加载时间");

    //处理分卷包名称
    QString transFile = strArchiveFullPath;
    transSplitFileName(transFile);

    QFileInfo fileinfo(transFile);
    if (!fileinfo.exists()) {
        m_ePageID = PI_LoadedFailure;
        return;
    }

    m_pUnCompressPage->setArchiveFullPath(transFile);      // 设置压缩包全路径

    // 根据是否可修改压缩包标志位设置打开文件选项是否可用
    m_pTitleButton->setEnabled(m_pUnCompressPage->isModifiable());
    m_pOpenAction->setEnabled(m_pUnCompressPage->isModifiable());

    // 设置默认解压路径
    if (m_pSettingDlg->getDefaultExtractPath().isEmpty()) {
        // 若默认为空,即设置默认解压路径为压缩包所在位置
        m_pUnCompressPage->setDefaultUncompressPath(fileinfo.absolutePath());  // 设置默认解压路径
    } else {
        // 否则,使用设置选项中路径
        m_pUnCompressPage->setDefaultUncompressPath(m_pSettingDlg->getDefaultExtractPath());  // 设置默认解压路径
    }

    ArchiveManager::get_instance()->loadArchive(transFile);     // 加载操作
    m_pLoadingPage->startLoading();     // 开始加载
    m_ePageID = PI_Loading;
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
        if (!m_initFlag) {
            // 初始化界面
            qDebug() << "初始化界面";
            initUI();
            initConnections();
            initData();
            m_initFlag = true;
        }


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

void MainWindow::slotHandleRightMenuSelected(const QStringList &listParam)
{
    qDebug() << listParam;
    if (!m_initFlag) {
        // 初始化界面
        qDebug() << "初始化界面";
        initUI();
        initConnections();
        initData();
        m_initFlag = true;
    }

    if (listParam.count() == 0) {
        return;
    }

    QString strType = listParam.last();
    qDebug() << listParam;

    if ((listParam.count() == 1 && UiTools::isArchiveFile(listParam[0]))
            || strType == QStringLiteral("extract")
            || strType == QStringLiteral("extract_split")) { //右键选择解压7z压缩分卷文件
        // 加载单个压缩包数据
        loadArchive(listParam[0]);
    } else if (strType == QStringLiteral("compress")) {
        // 压缩
        // 处理选中文件
        QStringList listFiles = listParam;
        listFiles.removeLast();
        // 添加压缩文件至压缩列表
        m_pCompressPage->addCompressFiles(listFiles);
        m_pCompressSettingPage->setFileSize(listFiles, calSelectedTotalFileSize(listFiles));
        //m_pCompressSettingPage->refreshMenu();
        // 设置界面标识为压缩设置界面
        m_ePageID = PI_CompressSetting;
    } else if (strType == QStringLiteral("extract_here")) {
        // 解压单个文件到当前文件夹
        m_bRightOperation = true;
        QString filepath = listParam.at(0);
        transSplitFileName(filepath);

        QFileInfo fileinfo(filepath);
        if (fileinfo.exists()) {
            // QFileInfo fileinfo(listParam.at(0));
            ExtractionOptions options;
            // 构建解压参数
            options.strTargetPath = fileinfo.path();
            options.bAllExtract = true;
            options.bRightExtract = true;
            options.qComressSize = fileinfo.size();
            QString strAutoPath = getExtractPath(fileinfo.fileName());
            // 根据是否自动创建文件夹处理解压路径
            if (!strAutoPath.isEmpty()) {
                options.strTargetPath += QDir::separator() + strAutoPath;
            }
            m_strExtractPath = options.strTargetPath;

            // 调用解压函数
            ArchiveManager::get_instance()->extractFiles(listParam.at(0), QList<FileEntry>(), options);
            // 设置进度界面参数
            m_pProgressPage->setProgressType(PT_UnCompress);
            m_pProgressPage->setTotalSize(options.qComressSize);
            m_pProgressPage->setArchiveName(fileinfo.fileName());
            m_pProgressPage->restartTimer(); // 重启计时器
            m_ePageID = PI_UnCompressProgress;

            m_ePageID = PI_UnCompressProgress;
        } else {
            // 可能分卷文件缺失，所以解压到当前文件夹失败
            m_ePageID = PI_UnCompressFailure;
        }
    } else if (strType == QStringLiteral("extract_multi")) {
        // 批量解压
        // 处理选中文件
        QStringList listFiles = listParam;
        listFiles.removeLast();
        // 获取默认路径
        QString strDefaultPath;
        QFileInfo fileinfo(listFiles.at(0));
        // 根据设置选项设置默认解压路径
        if ("" != m_pSettingDlg->getDefaultExtractPath()) {
            strDefaultPath = m_pSettingDlg->getDefaultExtractPath();
        } else {
            strDefaultPath = fileinfo.path();
        }
        // 弹出文件选择对话框，选择解压路径
        DFileDialog dialog;
        dialog.setAcceptMode(DFileDialog::AcceptOpen);
        dialog.setFileMode(DFileDialog::Directory);
        dialog.setWindowTitle(tr("Find directory"));
        dialog.setDirectory(strDefaultPath);
        // 处理操作
        const int mode = dialog.exec();

        if (mode != QDialog::Accepted) {
            // 不选取任何路径，不做处理直接返回
            return;
        }
        // 获取解压路径
        QList< QUrl > listSelectpath = dialog.selectedUrls();
        QString strExtractPath = listSelectpath.at(0).toLocalFile();
        // 调用批量解压
        ArchiveManager::get_instance()->batchExtractFiles(listFiles, strExtractPath, m_pSettingDlg->isAutoCreatDir());
        qint64 qSize = 0;
        foreach (QString strFile, listFiles) {
            qSize += QFile(strFile).size();
        }
        // 设置进度界面参数
        m_pProgressPage->setProgressType(PT_UnCompress);
        m_pProgressPage->setTotalSize(qSize);
        m_pProgressPage->setArchiveName(QFileInfo(listFiles[0]).fileName());
        m_pProgressPage->restartTimer(); // 重启计时器
        m_ePageID = PI_UnCompressProgress;
    } else if (strType == QStringLiteral("extract_here_multi")) {
        // 批量解压到当前文件夹
        m_bRightOperation = true;
        // 处理选中文件
        QStringList listFiles = listParam;
        listFiles.removeLast();
        ArchiveManager::get_instance()->batchExtractFiles(listFiles, QFileInfo(listFiles[0]).path(), m_pSettingDlg->isAutoCreatDir());
        qint64 qSize = 0;
        foreach (QString strFile, listFiles) {
            qSize += QFile(strFile).size();
        }
        // 设置进度界面参数
        m_pProgressPage->setProgressType(PT_UnCompress);
        m_pProgressPage->setTotalSize(qSize);
        m_pProgressPage->setArchiveName(QFileInfo(listFiles[0]).fileName());
        m_pProgressPage->restartTimer(); // 重启计时器
        m_ePageID = PI_UnCompressProgress;
    } else if (strType == QStringLiteral("compress_to_zip")) {
        // 压缩成xx.zip
    } else if (strType == QStringLiteral("compress_to_7z")) {
        // 压缩成xx.7z
    } else if (strType == QStringLiteral("extract_mkdir")) {
        // 解压到xx文件夹
    } else if (strType == QStringLiteral("extract_here_split")) { // 右键选择7z压缩分卷文件，解压到当前文件夹
        // if (listParam.at(0).contains(".7z.")) {
        m_bRightOperation = true;
        QString filepath = listParam.at(0);
        transSplitFileName(filepath);

        QFileInfo fileinfo(filepath);
        if (fileinfo.exists()) {
            ExtractionOptions options;
            // 构建解压参数
            options.strTargetPath = fileinfo.path();
            options.bAllExtract = true;
            options.bRightExtract = true;
            options.qComressSize = fileinfo.size();
            // 调用解压函数
            ArchiveManager::get_instance()->extractFiles(filepath, QList<FileEntry>(), options);
            // 设置进度界面参数
            m_pProgressPage->setProgressType(PT_UnCompress);
            m_pProgressPage->setTotalSize(options.qComressSize);
            m_pProgressPage->setArchiveName(fileinfo.fileName());
            m_pProgressPage->restartTimer(); // 重启计时器
            m_ePageID = PI_UnCompressProgress;
        } else {
            // 可能分卷文件缺失，所以解压到当前文件夹失败
            m_ePageID = PI_UnCompressFailure;
        }
        // }
    }

    refreshPage();
    show();
}

void MainWindow::slotTitleBtnClicked()
{
    if (m_ePageID == PI_Home || m_ePageID == PI_Compress || m_ePageID == PI_UnCompress) {
        // 通过文件选择对话框选择文件进行操作
        slotChoosefiles();
    } else if (m_ePageID == PI_CompressSetting) {
        // 压缩设置界面点击 返回
        m_ePageID = PI_Compress;
        refreshPage();
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

    QStringList listSelFiles = dialog.selectedFiles();
    if (listSelFiles.count() == 0)
        return;

    if (m_ePageID == PI_Home) {
        if (listSelFiles.count() == 1 && UiTools::isArchiveFile(listSelFiles[0])) {
            // 压缩包加载
            loadArchive(listSelFiles[0]);
        } else {
            // 添加压缩文件
            m_pCompressPage->addCompressFiles(listSelFiles);
            m_ePageID = PI_Compress;
        }
        refreshPage();
    } else if (m_ePageID == PI_Compress) {
        // 添加压缩文件
        m_pCompressPage->addCompressFiles(listSelFiles);
    } else if (m_ePageID == PI_UnCompress) {
        // 追加压缩
        m_pUnCompressPage->addNewFiles(listSelFiles);
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

            loadArchive(listFiles[0]);
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
    QStringList listCompressFiles = m_pCompressPage->compressFiles();       // 获取待压缩的文件
    m_pCompressSettingPage->setFileSize(listCompressFiles, calSelectedTotalFileSize(listCompressFiles));
    //m_pCompressSettingPage->refreshMenu();

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
    QList<FileEntry> listEntry;
    QString strDestination;
    CompressOptions options;
    //bool bBatch = false;

    //QSet< QString > globalWorkDirList;
    // 构建压缩文件数据
    foreach (QString strFile, listFiles) {
        FileEntry stFileEntry;
        stFileEntry.strFullPath = strFile;
        listEntry.push_back(stFileEntry);

//        QString globalWorkDir = strFile;
//        if (globalWorkDir.right(1) == QLatin1String("/")) {
//            globalWorkDir.chop(1);
//        }

//        globalWorkDir = QFileInfo(globalWorkDir).dir().absolutePath();
//        globalWorkDirList.insert(globalWorkDir);
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
//    if (globalWorkDirList.count() == 1 || options.bTar_7z) {
//        bBatch = false;
//    } else {
//        bBatch = true;
//    }

    ArchiveManager::get_instance()->createArchive(listEntry, strDestination, options, false/*, bBatch*/);

    // 切换进度界面
    m_pProgressPage->setProgressType(PT_Compress);
    m_pProgressPage->setTotalSize(stCompressInfo.qSize);
    m_pProgressPage->setArchiveName(stCompressInfo.strArchiveName);
    m_pProgressPage->restartTimer();

    m_ePageID = PI_CompressProgress;
    refreshPage();
}

void MainWindow::slotJobFinshed()
{
    m_bRightOperation = false;  // 重置右键标志位
    ArchiveJob *pJob = ArchiveManager::get_instance()->archiveJob();
    if (pJob == nullptr) {
        return;
    }

    qDebug() << "结束类型：" << pJob->m_eFinishedType << "****错误类型" << pJob->m_eErrorType;

    switch (pJob->m_eFinishedType) {
    case PFT_Nomral:
        handleJobNormalFinished(pJob->m_eJobType);  // 处理job正常结束
        break;
    // 用户取消操作
    case PFT_Cancel:
        handleJobCancelFinished(pJob->m_eJobType);  // 处理job取消结束
        break;
    // 错误处理
    case PFT_Error:
        handleJobErrorFinished(pJob->m_eJobType, pJob->m_eErrorType);   // 处理job错误结束
        break;
    }

    refreshPage();
    PERF_PRINT_END("POINT-03");
    PERF_PRINT_END("POINT-04");
    PERF_PRINT_END("POINT-05");
}

void MainWindow::slotUncompressClicked(const QString &strUncompressPath)
{
    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();
    ExtractionOptions options;
    ArchiveData stArchiveData = DataManager::get_instance().archiveData();

    // 构建解压参数
    options.strTargetPath = strUncompressPath;
    options.bAllExtract = true;
    options.qSize = stArchiveData.qSize;
    options.qComressSize = stArchiveData.qComressSize;

    // 如果自动创建文件夹,解压时增加一层以压缩包名称命名的目录
    QString strAutoPath = getExtractPath(strArchiveFullPath);
    if (!strAutoPath.isEmpty()) {
        options.strTargetPath += QDir::separator() + strAutoPath;
    }
    m_strExtractPath = options.strTargetPath;

    // 调用解压函数
    ArchiveManager::get_instance()->extractFiles(strArchiveFullPath, QList<FileEntry>(), options);

    // 设置进度界面参数
    m_pProgressPage->setProgressType(PT_UnCompress);
    m_pProgressPage->setTotalSize(options.qSize);
    m_pProgressPage->setArchiveName(QFileInfo(strArchiveFullPath).fileName());
    m_pProgressPage->restartTimer(); // 重启计时器
    m_ePageID = PI_UnCompressProgress;

    m_operationtype = Operation_Extract; //解压操作

    refreshPage();
}

void MainWindow::slotReceiveProgress(double dPercentage)
{
    if (Operation_SingleExtract == m_operationtype) { //提取删除操作使用小弹窗进度
        //需要添加dPercentage < 100判断，否则会出现小文件提取进度对话框不会自动关闭
        if (m_pProgressdialog->isHidden() && dPercentage < 100) {
            m_pProgressdialog->exec();
        }

        m_pProgressdialog->setProcess(qRound(dPercentage));
    } else {
        m_pProgressPage->setProgress(dPercentage);
    }
}

void MainWindow::slotReceiveCurFileName(const QString &strName)
{
    if (Operation_SingleExtract == m_operationtype) { //提取删除操作使用小弹窗进度
        m_pProgressdialog->setCurrentFile(strName);
    } else {
        m_pProgressPage->setCurrentFileName(strName);
    }
}

void MainWindow::slotQuery(Query *query)
{
    qDebug() << " query->execute()";
    query->setParent(this);
    query->execute();
}

void MainWindow::Extract2PathFinish(QString msg)
{
    QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
    this->sendMessage(icon, msg);

//    // 如果设置了自动打开，执行下列操作
//    if (m_pSettingsDialog->isAutoOpen()) {
//        QString fullpath = m_strDecompressFilePath + "/" + m_vecExtractSimpleFiles.at(0)->property("name").toString();
//        qDebug() << fullpath;
//        QFileInfo fileinfo(fullpath);

//        // 如果文件/文件夹存在
//        if (fileinfo.exists()) {
//            //                if (fileinfo.isDir()) {
//            //                    DDesktopServices::showFolder(fullpath);     // 如果是文件夹
//            //                } else if (fileinfo.isFile()) {
//            qDebug() << "DDesktopServices start:" << fullpath;
//            m_DesktopServicesThread = new DDesktopServicesThread();
//            connect(m_DesktopServicesThread, SIGNAL(finished()), this, SLOT(slotKillShowFoldItem()));
//            m_DesktopServicesThread->m_path = fullpath;
//            m_DesktopServicesThread->start();
//            //DDesktopServices::showFileItem(fullpath);   // 如果是单个文件 原BUG使用该函数，解压到桌面但文件，会出现30妙等待
//            qDebug() << "DDesktopServices end:" << m_strDecompressFilePath;
//            //  }
//        }
    //    }
}

QString MainWindow::createUUID()
{
    QString strUUID = QUuid::createUuid().toString();   // 创建唯一标识符
    // 移除左右大括号，防止执行命令时失败
    strUUID.remove("{");
    strUUID.remove("}");
    return strUUID;
}

QString MainWindow::getExtractPath(const QString &strArchiveFullPath)
{
    QString strpath = "";
    // 根据是否自动创建文件夹获取解压最后一层路径
    if (m_pSettingDlg->isAutoCreatDir()) {
        QFileInfo info(strArchiveFullPath);
        strpath = info.completeBaseName();
        if (info.filePath().contains(".tar.")) {
            strpath = strpath.remove(".tar"); // 类似tar.gz压缩文件，创建文件夹的时候移除.tar
        } else if (info.filePath().contains(".7z.")) {
            strpath = strpath.remove(".7z"); // 7z分卷文件，创建文件夹的时候移除.7z
        } else if (info.filePath().contains(".part01.rar")) {
            strpath = strpath.remove(".part01"); // tar分卷文件，创建文件夹的时候移除part01
        } else if (info.filePath().contains(".part1.rar")) {
            strpath = strpath.remove(".part1"); // tar分卷文件，创建文件夹的时候移除.part1
        }
    }

    return strpath;
}

void MainWindow::transSplitFileName(QString &fileName)
{
    if (fileName.contains(".7z.")) {
        QRegExp reg("^([\\s\\S]*.)[0-9]{3}$"); // QRegExp reg("[*.]part\\d+.rar$"); //rar分卷不匹配

        if (reg.exactMatch(fileName) == false) {
            return;
        }
        fileName = reg.cap(1) + "001"; //例如: *.7z.003 -> *.7z.001
    } else if (fileName.contains(".part") && fileName.endsWith(".rar")) {
        int x = fileName.lastIndexOf("part");
        int y = fileName.lastIndexOf(".");

        if ((y - x) > 5) {
            fileName.replace(x, y - x, "part01");
        } else {
            fileName.replace(x, y - x, "part1");
        }
    }
}

void MainWindow::handleJobNormalFinished(ArchiveJob::JobType eType)
{
    switch (eType) {
    // 创建压缩包
    case ArchiveJob::JT_Create: {
        m_ePageID = PI_CompressSuccess;
    }
    break;
    // 添加文件至压缩包
    case ArchiveJob::JT_Add: {
        qDebug() << "添加结束";
        m_ePageID = PI_UnCompress;
        m_pUnCompressPage->refreshDataByCurrentPathDelete();
    }
    break;
    // 加载压缩包数据
    case ArchiveJob::JT_Load: {
        m_pLoadingPage->stopLoading();
        m_ePageID = PI_UnCompress;
        qDebug() << "加载结束";

        m_pUnCompressPage->refreshArchiveData();
    }
    break;
    // 批量解压
    case ArchiveJob::JT_BatchExtract:
    // 解压
    case ArchiveJob::JT_Extract: {
        if (Archive_OperationType::Operation_SingleExtract == m_operationtype) {
            qDebug() << "提取结束";
            m_ePageID = PI_UnCompress;
            Extract2PathFinish(tr("Extraction successful", "Operation_SingleExtract")); //提取成功
            m_pProgressdialog->setFinished();

            // 设置了自动打开文件夹处理流程
            if (m_pSettingDlg->isAutoOpen()) {
                if (m_pDDesktopServicesThread == nullptr) {
                    m_pDDesktopServicesThread = new DDesktopServicesThread(this);
                }

                // 打开选中第一个提取的文件/文件夹
                if (m_listOpenFiles.count() > 0)
                    m_pDDesktopServicesThread->setOpenFile(m_listExractFiles[0]);
                m_pDDesktopServicesThread->start();
            }
        } else {
            qDebug() << "解压结束";
            m_ePageID = PI_UnCompressSuccess;

            // 设置了自动打开文件夹处理流程
            if (m_pSettingDlg->isAutoOpen()) {
                if (m_pDDesktopServicesThread == nullptr) {
                    m_pDDesktopServicesThread = new DDesktopServicesThread(this);
                }

                if (m_pSettingDlg->isAutoCreatDir()) {
                    // 若设置了自动创建文件夹,显示解压路径
                    m_pDDesktopServicesThread->setOpenFile(m_strExtractPath);
                } else {
                    // 否则显示解压第一个文件/文件夹所在目录
                    ArchiveData stArchiveData = DataManager::get_instance().archiveData();
                    if (stArchiveData.listRootEntry.count() > 0)
                        m_pDDesktopServicesThread->setOpenFile(m_strExtractPath + QDir::separator() + stArchiveData.listRootEntry[0].strFullPath);
                }

                m_pDDesktopServicesThread->start();
            }
        }
    }
    break;
    // 删除
    case ArchiveJob::JT_Delete: {
        qDebug() << "删除结束";
        m_ePageID = PI_UnCompress;

        m_pUnCompressPage->refreshDataByCurrentPathDelete();
    }
    break;
    // 打开
    case ArchiveJob::JT_Open: {
        qDebug() << "打开结束";

        // 若压缩包文件可更改，打开文件之后对文件进行监控
        if (m_pUnCompressPage->isModifiable()) {
            // 打开成功之后添加到文件监控
            m_pOpenFileWatcher->addPath(m_strOpenFile);

            if (m_mapFileHasModified.find(m_strOpenFile) != m_mapFileHasModified.end()) {
                // 第一次默认文件未修改
                m_mapFileHasModified[m_strOpenFile] = true;
            } else {
                // 若已存在此监控文件，修改为未修改
                m_mapFileHasModified[m_strOpenFile] = false;
            }
        }

        m_ePageID = PI_UnCompress;
        m_pLoadingPage->stopLoading();
    }
    break;
    }
}

void MainWindow::handleJobCancelFinished(ArchiveJob::JobType eType)
{
    switch (eType) {
    // 创建压缩包
    case ArchiveJob::JT_Create: {
        m_ePageID = PI_Compress;
    }
    break;
    // 添加文件至压缩包
    case ArchiveJob::JT_Add: {
        m_ePageID = PI_UnCompress;
    }
    break;
    // 批量解压
    case ArchiveJob::JT_BatchExtract:
    // 解压
    case ArchiveJob::JT_Extract: {
        if (m_bRightOperation) {
            // 直接关闭应用
            close();
        } else {
            // 切换到解压列表界面，再执行相关操作
            m_ePageID = PI_UnCompress;
        }
    }
    break;
    // 删除
    case ArchiveJob::JT_Delete: {
        m_ePageID = PI_UnCompress;
    }
    break;
    // 转换
    case ArchiveJob::JT_Convert: {
        m_ePageID = PI_UnCompress;
    }
    break;
    default:
        break;
    }
}

void MainWindow::handleJobErrorFinished(ArchiveJob::JobType eJobType, ErrorType eErrorType)
{

}

void MainWindow::addFiles2Archive(const QStringList &listFiles, const QString &strPassword)
{
    qDebug() << "向压缩包中添加文件";

    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();  // 获取压缩包全路径
    CompressOptions options;
    QList<FileEntry> listEntry;

    options.strDestination = m_pUnCompressPage->getCurPath();   // 获取追加目录
    options.strPassword = strPassword;

    // 构建压缩文件数据
    foreach (QString strFile, listFiles) {
        FileEntry stFileEntry;
        stFileEntry.strFullPath = strFile;
        listEntry.push_back(stFileEntry);
    }

    // 调用添加文件接口
    ArchiveManager::get_instance()->addFiles(strArchiveFullPath, listEntry, options);

    // 切换进度界面
    m_pProgressPage->setProgressType(PT_CompressAdd);
    m_pProgressPage->setTotalSize(calSelectedTotalFileSize(listFiles));
    m_pProgressPage->setArchiveName(QFileInfo(strArchiveFullPath).fileName());
    m_pProgressPage->restartTimer();

    m_ePageID = PI_AddCompressProgress;
    refreshPage();
}

void MainWindow::slotExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions)
{
    qDebug() << "提取文件至:" << stOptions.strTargetPath;
    m_strExtractPath = stOptions.strTargetPath;     // 存储提取路径
    m_operationtype = Operation_SingleExtract; //提取操作
    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();

    // 存储提取之后的文件
    m_listExractFiles.clear();
    for (int i = 0; i < listSelEntry.count(); ++i) {
        QString strFullPath = listSelEntry[i].strFullPath;
        m_listExractFiles << stOptions.strTargetPath + QDir::separator() + strFullPath.remove(0, stOptions.strDestination.size());
    }

    // 提取删除操作使用小弹窗进度
    m_pProgressdialog->clearprocess();
    m_pProgressdialog->setCurrentTask(strArchiveFullPath);

    ArchiveManager::get_instance()->extractFiles2Path(strArchiveFullPath, listSelEntry, stOptions);

}

void MainWindow::slotDelFiles(const QList<FileEntry> &listSelEntry, qint64 qTotalSize)
{
    qDebug() << "删除文件:";
    m_operationtype = Operation_DELETE; //提取操作
    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();
    ArchiveManager::get_instance()->deleteFiles(strArchiveFullPath, listSelEntry);

    // 设置进度界面参数
    m_pProgressPage->setProgressType(PT_Delete);
    m_pProgressPage->setTotalSize(qTotalSize);
    m_pProgressPage->setArchiveName(QFileInfo(strArchiveFullPath).fileName());
    m_pProgressPage->restartTimer(); // 重启计时器
    m_ePageID = PI_DeleteProgress;
    refreshPage();
}

void MainWindow::slotReceiveCurArchiveName(const QString &strArchiveName)
{
    m_pProgressPage->setArchiveName(strArchiveName);
}

void MainWindow::slotOpenFile(const FileEntry &entry, const QString &strProgram)
{
    m_listOpenFiles << entry;   // 添加此文件至解压文件中

    // 设置解压临时路径
    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();
    QString strTempExtractPath =  TEMPPATH + QDir::separator() + m_strUUID + QDir::separator() + createUUID();
    m_strOpenFile = strTempExtractPath + QDir::separator() + entry.strFileName;
    m_mapOpenFils[m_strOpenFile] = entry;
    ArchiveManager::get_instance()->openFile(strArchiveFullPath, entry, strTempExtractPath, strProgram);

    // 进入打开加载界面
    m_operationtype = Operation_TempExtract_Open;
    m_pLoadingPage->startLoading();     // 开始加载
    m_ePageID = PI_Loading;
    refreshPage();
}

void MainWindow::slotOpenFileChanged(const QString &strPath)
{
    if ((m_mapFileHasModified.find(strPath) != m_mapFileHasModified.end()) && (!m_mapFileHasModified[strPath])) {
        m_mapFileHasModified[strPath] = true;

        QFileInfo file(strPath);
        QString strDesText = QObject::tr("%1 changed. Do you want to save changes to the archive?").arg(UiTools::toShortString(file.fileName()));

        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(strDesText, tr("Discard"), DDialog::ButtonNormal, tr("Update"), DDialog::ButtonRecommend);
        if (iResult == 1) {
            // 更新压缩包数据
            qDebug() << "更新压缩包中文件" << m_mapOpenFils[strPath].strFullPath;
            addFiles2Archive(QStringList() << strPath);
        }
        m_mapFileHasModified[strPath] = false;
    }
}

void MainWindow::slotSuccessReturn()
{
    switch (m_ePageID) {
    // 压缩成功
    case PI_CompressSuccess: {
        m_pCompressPage->clear();   // 清空压缩界面
    }
    break;
    // 解压成功
    case PI_UnCompressSuccess: {
        m_pUnCompressPage->clear(); // 清空解压界面
    }
    break;
    // 转换成功
    case PI_ConvertSuccess: {
        m_pUnCompressPage->clear(); // 清空解压界面
    }
    break;
    default:
        break;
    }

    m_ePageID = PI_Home;
    refreshPage();
}

void MainWindow::slotPause(Progress_Type eType)
{
    Q_UNUSED(eType)
    ArchiveManager::get_instance()->pauseOperation();
}

void MainWindow::slotContinue()
{
    ArchiveManager::get_instance()->continueOperation();
}

void MainWindow::slotCancel(Progress_Type eType)
{
    Q_UNUSED(eType)
    ArchiveManager::get_instance()->cancelOperation();
}

void MainWindow::slotAddFiles(const QStringList &listFiles, const QString &strPassword)
{
    // 向压缩包中添加文件
    addFiles2Archive(listFiles, strPassword);
}


