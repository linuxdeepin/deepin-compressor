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
#include "pluginmanager.h"
#include "settingdialog.h"
#include "archivemanager.h"
#include "DebugTimeManager.h"
#include "popupdialog.h"
#include "progressdialog.h"
#include "datamanager.h"
#include "ddesktopservicesthread.h"
#include "openFileWatcher.h"
#include "mimetypes.h"
#include "customwidget.h"
#include "treeheaderview.h"
#include "compressview.h"
#include "uncompressview.h"
#include "uitools.h"
#include "calculatesizethread.h"

#include <DFileDialog>
#include <DTitlebar>
#include <DWindowCloseButton>
#include <DWindowOptionButton>
#include <DArrowLineDrawer>
#include <DFontSizeManager>
#include <denhancedwidget.h>
#include <DSysInfo>

#include <QStackedWidget>
#include <QKeyEvent>
#include <QSettings>
#include <QDebug>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>
#include <QScreen>
#include <QFormLayout>
#include <QShortcut>

static QMutex mutex; // 静态全局变量只在定义该变量的源文件内有效

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_strProcessID(QString::number(QCoreApplication::applicationPid()))   // 获取应用进程号
{
    setWindowTitle(tr("Archive Manager"));

    // 先构建初始界面
    m_pMainWidget = new QStackedWidget(this);  // 中心面板
    m_pHomePage = new HomePage(this);            // 首页
    m_pMainWidget->addWidget(m_pHomePage);
    setCentralWidget(m_pMainWidget);    // 设置中心面板
    m_pMainWidget->setCurrentIndex(0);

    m_openkey = new QShortcut(QKeySequence(Qt::Key_Slash + Qt::CTRL + Qt::SHIFT), this); // Ctrl+Shift+/
    m_openkey->setContext(Qt::ApplicationShortcut);

    // 初始化标题栏
    initTitleBar();
    initData();
    // 开启定时器刷新界面
    m_iInitUITimer = startTimer(500);


}

MainWindow::~MainWindow()
{
    //如果窗体状态不是最大最小状态，则记录此时窗口尺寸到配置文件里，方便下次打开时恢复大小
    if (windowState() == Qt::WindowNoState) {
        saveConfigWinSize(width(), height());
    }

    ArchiveManager::get_instance()->destory_instance();

    // 清除缓存数据
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-rf");
    args.append(TEMPPATH + QDir::separator() + m_strProcessID);
    p.execute(command, args);
    p.waitForFinished();

    if (nullptr != m_mywork && m_mywork->isRunning()) {
        m_mywork->set_thread_stop(true); // 结束计算大小线程
        m_mywork->wait(); //必须等待线程结束
    }
    qInfo() << "应用正常退出";
}

bool MainWindow::checkHerePath(const QString &strPath)
{
    QFileInfo info(strPath);
    if (!(info.isWritable() && info.isExecutable())) { // 检查一选择保存路径是否有权限
        TipDialog dialog(this);
        // 屏幕居中显示
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenRect =  screen->availableVirtualGeometry();
        dialog.move(((screenRect.width() / 2) - (dialog.width() / 2)), ((screenRect.height() / 2) - (dialog.height() / 2)));
        dialog.showDialog(tr("You do not have permission to save files here, please change and retry"), tr("OK"), DDialog::ButtonNormal);
        return false;
    }

    return true;
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
    m_commentProgressDialog = new CommentProgressDialog(this); // 更新注释进度弹窗
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
    m_pOpenFileWatcher = new OpenFileWatcher(this);

    // 刷新压缩设置界面格式选项
    m_pCompressSettingPage->refreshMenu();
}

void MainWindow::initTitleBar()
{
    // 创建菜单
    QMenu *menu = new QMenu(this);
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
    m_pTitleButton->setToolTip(tr("Open file"));

    // 左上角注释信息
    m_pTitleCommentButton = new DIconButton(this);
    m_pTitleCommentButton->setFixedSize(36, 36);
    m_pTitleCommentButton->setToolTip(tr("File info"));
    slotThemeChanged();

//    m_pTitleCommentButton->setIcon(commentIcon);
//    m_pTitleCommentButton->setIconSize(QSize(15, 15));

    m_pTitleCommentButton->setVisible(false);
    m_pTitleCommentButton->setObjectName("CommentButton");
    m_pTitleCommentButton->setAccessibleName("Comment_btn");

    // 左上角按钮布局
    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->addSpacing(6);
    leftLayout->addWidget(m_pTitleButton);
    leftLayout->addSpacing(5);
    leftLayout->addWidget(m_pTitleCommentButton);
    leftLayout->addStretch();
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *left_frame = new QFrame(this);
    left_frame->setFixedWidth(6 + 36 + 5 + 36 + 6);
    left_frame->setContentsMargins(0, 0, 0, 0);
    left_frame->setLayout(leftLayout);

    titlebar()->addWidget(left_frame, Qt::AlignLeft);
    titlebar()->setContentsMargins(0, 0, 0, 0);

    setTabOrder(m_pTitleButton, m_pTitleCommentButton);
}

void MainWindow::initData()
{
    // 初始化数据配置
    m_pSettings = new QSettings(QDir(UiTools::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this);

    if (m_pSettings->value("dir").toString().isEmpty()) {
        m_pSettings->setValue("dir", "");
    }

    resize(getConfigWinSize()); // 设置窗口尺寸
    setMinimumSize(620, 465);   // 设置最小大小
}

void MainWindow::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MainWindow::slotThemeChanged);

    connect(m_pTitleButton, &DIconButton::clicked, this, &MainWindow::slotTitleBtnClicked);
    connect(m_pTitleCommentButton, &DPushButton::clicked, this, &MainWindow::slotTitleCommentButtonPressed);
    connect(m_pHomePage, &HomePage::signalFileChoose, this, &MainWindow::slotChoosefiles);
    connect(m_pHomePage, &HomePage::signalDragFiles, this, &MainWindow::slotDragSelectedFiles);
    connect(m_pCompressPage, &CompressPage::signalLevelChanged, this, &MainWindow::slotCompressLevelChanged);
    connect(m_pCompressPage, &CompressPage::signalCompressNextClicked, this, &MainWindow::slotCompressNext);
    connect(m_pCompressPage, &CompressPage::signalFileChoose, this, &MainWindow::slotChoosefiles);
    connect(m_pCompressSettingPage, &CompressSettingPage::signalCompressClicked, this, &MainWindow::slotCompress);
    connect(m_pUnCompressPage, &UnCompressPage::signalUncompress, this, &MainWindow::slotUncompressClicked);
    connect(m_pUnCompressPage, &UnCompressPage::signalExtract2Path, this, &MainWindow::slotExtract2Path);
    connect(m_pUnCompressPage, &UnCompressPage::signalDelFiles, this, &MainWindow::slotDelFiles);
    connect(m_pUnCompressPage, &UnCompressPage::signalOpenFile, this, &MainWindow::slotOpenFile);
    connect(m_pUnCompressPage, &UnCompressPage::signalAddFiles2Archive, this, &MainWindow::slotAddFiles);
    connect(m_pUnCompressPage, &UnCompressPage::signalFileChoose, this, &MainWindow::slotChoosefiles);
    connect(m_pProgressPage, &ProgressPage::signalPause, this, &MainWindow::slotPause);
    connect(m_pProgressPage, &ProgressPage::signalContinue, this, &MainWindow::slotContinue);
    connect(m_pProgressPage, &ProgressPage::signalCancel, this, &MainWindow::slotCancel);
    connect(m_pProgressdialog, &ProgressDialog::signalPause, this, &MainWindow::slotPause);
    connect(m_pProgressdialog, &ProgressDialog::signalContinue, this, &MainWindow::slotContinue);
    connect(m_pProgressdialog, &ProgressDialog::signalCancel, this, &MainWindow::slotCancel);
    connect(m_pSuccessPage, &SuccessPage::sigBackButtonClicked, this, &MainWindow::slotSuccessReturn);
    connect(m_pSuccessPage, &SuccessPage::signalViewFile, this, &MainWindow::slotSuccessView);
    connect(m_pFailurePage, &FailurePage::sigFailRetry, this, &MainWindow::slotFailureRetry);
    connect(m_pFailurePage, &FailurePage::sigBackButtonClickedOnFail, this, &MainWindow::slotFailureReturn);

    connect(ArchiveManager::get_instance(), &ArchiveManager::signalJobFinished, this, &MainWindow::slotJobFinished);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalprogress, this, &MainWindow::slotReceiveProgress);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalCurFileName, this, &MainWindow::slotReceiveCurFileName);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalFileWriteErrorName, this, &MainWindow::slotReceiveFileWriteErrorName);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalCurArchiveName, this, &MainWindow::slotReceiveCurArchiveName);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalQuery, this, &MainWindow::slotQuery);

    connect(m_pOpenFileWatcher, &OpenFileWatcher::fileChanged, this, &MainWindow::slotOpenFileChanged);

    connect(m_openkey, &QShortcut::activated, this, &MainWindow::slotShowShortcutTip);
}

void MainWindow::refreshPage()
{
    switch (m_ePageID) {
    case PI_Home: {
        resetMainwindow();
        m_pMainWidget->setCurrentIndex(0);
        setTitleButtonStyle(false, false);
        titlebar()->setTitle("");
    }
    break;
    case PI_Compress: {
        m_pMainWidget->setCurrentIndex(1);
        setTitleButtonStyle(true, false, DStyle::StandardPixmap::SP_IncreaseElement);
        if (m_iCompressedWatchTimerID == 0) {
            m_iCompressedWatchTimerID = startTimer(1);
        }

        titlebar()->setTitle(tr("Create New Archive"));
    }
    break;
    case PI_CompressSetting: {
        m_pMainWidget->setCurrentIndex(2);
        setTitleButtonStyle(true, false, DStyle::StandardPixmap::SP_ArrowLeave);
        if (m_iCompressedWatchTimerID == 0) {
            m_iCompressedWatchTimerID = startTimer(1);
        }

        titlebar()->setTitle(tr("Create New Archive"));
    }
    break;
    case PI_UnCompress: {
        m_pMainWidget->setCurrentIndex(3);
        setTitleButtonStyle(true, true, DStyle::StandardPixmap::SP_IncreaseElement);
        titlebar()->setTitle(QFileInfo(m_pUnCompressPage->archiveFullPath()).fileName());
    }
    break;
    case PI_AddCompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false, false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Adding files to %1").arg(m_pProgressPage->archiveName()));
    }
    break;
    case PI_CompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false, false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Compressing"));
    }
    break;
    case PI_UnCompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false, false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Extracting"));
    }
    break;
    case PI_DeleteProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false, false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Deleting"));
    }
    break;
    case PI_ConvertProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false, false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Converting"));
    }
    break;
    case PI_CommentProgress: {
        m_pMainWidget->setCurrentIndex(4);
        setTitleButtonStyle(false, false);
        m_pProgressPage->resetProgress();
        titlebar()->setTitle(tr("Updating comments")); // 正在更新注释
    }
    break;
    case PI_Success: {
        m_pMainWidget->setCurrentIndex(5);
        setTitleButtonStyle(false, false);
        titlebar()->setTitle("");
    }
    break;
    case PI_Failure: {
        m_pMainWidget->setCurrentIndex(6);
        setTitleButtonStyle(false, false);
        titlebar()->setTitle("");
    }
    break;
    case PI_Loading: {
        m_pMainWidget->setCurrentIndex(7);
        setTitleButtonStyle(false, false);
        titlebar()->setTitle("");
    }
    break;
    }
}

qint64 MainWindow::calSelectedTotalFileSize(const QStringList &files)
{
    QElapsedTimer time1;
    time1.start();

    m_stCompressParameter.qSize = 0;

    foreach (QString file, files) {
        QFileInfo fi(file);
        if (fi.isFile()) {  // 如果为文件，直接获取大小
            qint64 curFileSize = fi.size();

#ifdef __aarch64__
            if (maxFileSize_ < curFileSize) {
                maxFileSize_ = curFileSize;
            }
#endif
            m_stCompressParameter.qSize += curFileSize;
        } else if (fi.isDir()) {    // 如果是文件夹，递归获取所有子文件大小总和
            QtConcurrent::run(this, &MainWindow::calFileSizeByThread, file);
        }
    }

    // 等待线程池结束
    QThreadPool::globalInstance()->waitForDone();
    qInfo() << QString("计算大小线程结束，耗时:%1ms，文件总大小:%2B").arg(time1.elapsed()).arg(m_stCompressParameter.qSize);

    return m_stCompressParameter.qSize;
}

void MainWindow::calFileSizeByThread(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return;

    // 获得文件夹中的文件列表
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::System
                                           | QDir::NoDotAndDotDot | QDir::Hidden);

    for (int i = 0; i < list.count(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isDir()) {
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
            m_stCompressParameter.qSize += curFileSize;
            mutex.unlock();
        }
    }
}

void MainWindow::setTitleButtonStyle(bool bVisible, bool bVisible2, DStyle::StandardPixmap pixmap)
{
    switch (pixmap) {
    case DStyle::SP_IncreaseElement: // 列表界面
        m_pTitleButton->setToolTip(tr("Open file"));
        break;
    case DStyle::SP_ArrowLeave:  // 压缩设置界面
        m_pTitleButton->setToolTip(tr("Back"));
        break;
    default:
        break;
    }

    m_pTitleButton->setVisible(bVisible);

    if (bVisible)
        m_pTitleButton->setIcon(pixmap);

    m_pTitleCommentButton->setVisible(bVisible2);
}

void MainWindow::loadArchive(const QString &strArchiveFullPath)
{
    if (!QFileInfo(strArchiveFullPath).isReadable()) {
        TipDialog dialog(this);
        dialog.showDialog(tr("You do not have permission to load %1").arg(strArchiveFullPath), tr("OK"), DDialog::ButtonNormal);
        return;
    }

    PERF_PRINT_BEGIN("POINT-05", "加载时间");
    m_operationtype = Operation_Load;

    //处理分卷包名称
    QString transFile = strArchiveFullPath;
    QStringList listSupportedMimeTypes = PluginManager::get_instance().supportedWriteMimeTypes(PluginManager::SortByComment);     // 获取支持的压缩格式
    CustomMimeType mimeType = determineMimeType(transFile);

    // 构建压缩包加载之后的数据
    m_stUnCompressParameter.strFullPath = strArchiveFullPath;
    UiTools::transSplitFileName(transFile, m_stUnCompressParameter.eSplitVolume);
    QFileInfo fileinfo(transFile);
    if (!fileinfo.exists()) {
        // 分卷不完整（损坏）
        // 比如打开1.7z.002时，1.7z.001不存在
        m_ePageID = PI_Failure;
        showErrorMessage(FI_Load, EI_ArchiveMissingVolume);
        return;
    }

    m_stUnCompressParameter.bCommentModifiable = (mimeType.name() == "application/zip") ? true : false;
    m_stUnCompressParameter.bMultiplePassword = (mimeType.name() == "application/zip") ? true : false;
    m_stUnCompressParameter.bModifiable = (listSupportedMimeTypes.contains(mimeType.name()) && fileinfo.isWritable()
                                           && m_stUnCompressParameter.eSplitVolume == UnCompressParameter::ST_No); // 支持压缩且文件可写的非分卷格式才能修改数据

    // 监听压缩包
    watcherArchiveFile(transFile);

    m_pUnCompressPage->setArchiveFullPath(transFile, m_stUnCompressParameter);     // 设置压缩包全路径和是否分卷

    // 根据是否可修改压缩包标志位设置打开文件选项是否可用
    m_pTitleButton->setEnabled(m_stUnCompressParameter.bModifiable);
    m_pOpenAction->setEnabled(m_stUnCompressParameter.bModifiable);

    // 设置默认解压路径
    if (m_pSettingDlg->getDefaultExtractPath().isEmpty()) {
        // 若默认为空,即设置默认解压路径为压缩包所在位置
        m_pUnCompressPage->setDefaultUncompressPath(fileinfo.absolutePath());  // 设置默认解压路径
    } else {
        // 否则,使用设置选项中路径
        m_pUnCompressPage->setDefaultUncompressPath(m_pSettingDlg->getDefaultExtractPath());  // 设置默认解压路径
    }

    // zip分卷指定使用cli7zplugin
    UiTools::AssignPluginType eType = (m_stUnCompressParameter.eSplitVolume == UnCompressParameter::ST_Zip) ?
                                      (UiTools::AssignPluginType::APT_Cli7z) : (UiTools::AssignPluginType::APT_Auto);
    // 加载操作
    if (ArchiveManager::get_instance()->loadArchive(transFile, eType)) {
        m_pLoadingPage->setDes(tr("Loading, please wait..."));
        m_pLoadingPage->startLoading();     // 开始加载
        m_ePageID = PI_Loading;
    } else {
        // 无可用插件，回到首页
        m_ePageID = PI_Home;
        refreshPage();
        show();
        // 提示无插件
        TipDialog dialog(this);
        dialog.showDialog(tr("Plugin error"), tr("OK"), DDialog::ButtonNormal);
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (m_iInitUITimer == event->timerId()) {
        if (!m_initFlag) {
            // 初始化界面
            qInfo() << "初始化界面";
            initUI();
            initConnections();
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

                // 先暂停操作
                ArchiveManager::get_instance()->pauseOperation();

                QString displayName = UiTools::toShortString(info.fileName());
                QString strTips = tr("%1 was changed on the disk, please import it again.").arg(displayName);

                TipDialog dialog(this);
                dialog.showDialog(strTips, tr("OK"), DDialog::ButtonNormal);

                m_pCompressPage->refreshCompressedFiles(true, listFiles[i]);

                ArchiveManager::get_instance()->cancelOperation();

                // 返回到列表界面
                if (m_ePageID != PI_Compress) {
                    m_ePageID = PI_Compress;
                    refreshPage();
                }


                // 如果待压缩文件列表数目为空，回到首页
                if (m_pCompressPage->compressFiles().count() == 0) {
                    m_ePageID = PI_Home;
                    refreshPage();
                }
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_operationtype != Operation_NULL) {
        // 当前还有操作正在进行
        slotPause();    // 先暂停当前操作
        // 创建询问关闭对话框
        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(tr("Are you sure you want to stop the ongoing task?"), tr("Cancel"), DDialog::ButtonNormal, tr("Confirm"), DDialog::ButtonRecommend);
        // 点击确认时，停止当前操作，关闭应用
        if (iResult == QDialog::Accepted) {
            slotCancel();       // 执行取消操作
            event->accept();
        } else {
            slotContinue();    // 继续之前的操作
            event->ignore();    // 忽略退出
        }
    } else {
        event->accept();    // 忽略退出
    }
}

bool MainWindow::checkSettings(QString file)
{
    QString strTransFileName = file;
    UnCompressParameter::SplitType type;
    UiTools::transSplitFileName(strTransFileName, type);

    QFileInfo info(strTransFileName);
    if (!info.exists()) {
        // 文件不存在
        TipDialog dialog;
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenRect =  screen->availableVirtualGeometry();
        dialog.move(((screenRect.width() / 2) - (dialog.width() / 2)), ((screenRect.height() / 2) - (dialog.height() / 2)));
        dialog.showDialog(tr("No such file or directory"), tr("OK"), DDialog::ButtonNormal);
        return false;
    } else {

        if (!info.isReadable()) {
            TipDialog dialog(this);
            dialog.showDialog(tr("You do not have permission to load %1").arg(strTransFileName), tr("OK"), DDialog::ButtonNormal);
            return false;
        }

        if (info.isDir()) {
            // 选择打开的是文件夹
            TipDialog dialog;
            QScreen *screen = QGuiApplication::primaryScreen();
            QRect screenRect =  screen->availableVirtualGeometry();
            dialog.move(((screenRect.width() / 2) - (dialog.width() / 2)), ((screenRect.height() / 2) - (dialog.height() / 2)));

            dialog.showDialog(tr("The file format is not supported by Archive Manager"), tr("OK"), DDialog::ButtonNormal);
            return false;
        } else {
            // 文件判断
            QString fileMime;

            bool existMime = false; // 在设置界面是否被勾选
            bool bArchive = false; // 是否是应用支持解压的格式
            bool mimeIsChecked = true; // 默认该格式被勾选

            // 判断内容
            if (strTransFileName.isEmpty()) {
                existMime = true;
            } else {
                fileMime = determineMimeType(strTransFileName).name();
                if (fileMime.contains("application/"))
                    fileMime = fileMime.remove("application/");

                if (fileMime.size() > 0) {
                    existMime = UiTools::isExistMimeType(fileMime, bArchive);
                } else {
                    existMime = false;
                }
            }

            // 若在关联类型中没有找到勾选的此格式
            if (!existMime) {
                QString str;
                if (bArchive) {
                    // 如果是压缩包，提示勾选关联类型
                    str = tr("Please check the file association type in the settings of Archive Manager");
                } else {
                    // 如果不是压缩包，提示非支持的压缩格式
                    str = tr("The file format is not supported by Archive Manager");
                }

                // 弹出提示对话框
                TipDialog dialog;
                QScreen *screen = QGuiApplication::primaryScreen();
                QRect screenRect =  screen->availableVirtualGeometry();
                dialog.move(((screenRect.width() / 2) - (dialog.width() / 2)), ((screenRect.height() / 2) - (dialog.height() / 2)));

                int re = dialog.showDialog(str, tr("OK"), DDialog::ButtonNormal);
                if (re != 1) { // ？
                    mimeIsChecked = false;
                }
            }

            return mimeIsChecked;
        }
    }
}

bool MainWindow::handleApplicationTabEventNotify(QObject *obj, QKeyEvent *evt)
{
    if (!m_pUnCompressPage || !m_pCompressPage /*|| !m_pCompressSetting*/) {
        return false;
    }

    int keyOfEvent = evt->key();
    if (Qt::Key_Tab == keyOfEvent) { //tab焦点顺序：从上到下，从左到右
        DWindowCloseButton *closebtn = titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
        if (obj == titlebar()) { //焦点顺序：标题栏设置按钮->标题栏按钮
            if (m_pTitleButton->isVisible() && m_pTitleButton->isEnabled()) {
                m_pTitleButton->setFocus(Qt::TabFocusReason);
                return true;
            } if (m_pTitleCommentButton->isVisible() && m_pTitleCommentButton->isEnabled()) {
                m_pTitleCommentButton->setFocus(Qt::TabFocusReason);
                return true;
            } else {
                return false;
            }
        } else if (obj->objectName() == "CommentButton") { //焦点顺序：标题栏按钮->标题栏设置按钮
            titlebar()->setFocus(Qt::TabFocusReason);
            //titlebar不截获屏蔽掉,因为让他继续往下一menubutton发送tab
            //  return  true;
        } else if (obj->objectName() == "TitleButton" && !(m_pTitleCommentButton->isVisible() && m_pTitleCommentButton->isEnabled())) { //焦点顺序：标题栏按钮->标题栏设置按钮
            titlebar()->setFocus(Qt::TabFocusReason);
            //titlebar不截获屏蔽掉,因为让他继续往下一menubutton发送tab
            //  return  true;
        } else if (obj->objectName() == "gotoPreviousLabel") { //焦点顺序：返回上一页->文件列表
            switch (m_ePageID) {
            case PI_UnCompress:
                m_pUnCompressPage->getUnCompressView()->setFocus(Qt::TabFocusReason);
                break;
            case PI_Compress:
                m_pCompressPage->getCompressView()->setFocus(Qt::TabFocusReason);
                break;
            default:
                return false;
            }
            return true;
        } else if (obj->objectName() == "TableViewFile") { //焦点顺序：文件列表->解压路径按钮/下一步按钮
            switch (m_ePageID) {
            case PI_UnCompress:
                m_pUnCompressPage->getUncompressPathBtn()->setFocus(Qt::TabFocusReason);
                break;
            case PI_Compress:
                m_pCompressPage->getNextBtn()->setFocus(Qt::TabFocusReason);
                break;
            default:
                return false;
            }
            return true;
        } else if (obj == closebtn) { //焦点顺序：关闭应用按钮->返回上一页/文件列表/压缩类型选择
            switch (m_ePageID) {
            case PI_UnCompress:
                if (m_pUnCompressPage->getUnCompressView()->getHeaderView()->getpreLbl()->isVisible()) {
                    m_pUnCompressPage->getUnCompressView()->getHeaderView()->getpreLbl()->setFocus(Qt::TabFocusReason);
                } else {
                    m_pUnCompressPage->getUnCompressView()->setFocus(Qt::TabFocusReason);
                }
                break;
            case PI_Compress:
                if (m_pCompressPage->getCompressView()->getHeaderView()->getpreLbl()->isVisible()) {
                    m_pCompressPage->getCompressView()->getHeaderView()->getpreLbl()->setFocus(Qt::TabFocusReason);
                } else {
                    m_pCompressPage->getCompressView()->setFocus(Qt::TabFocusReason);
                }
                break;
            case PI_CompressSetting:
                m_pCompressSettingPage->getClickLbl()->setFocus(Qt::TabFocusReason);
                break;
            default:
                return false;
            }
            return true;
        }
    } else if (Qt::Key_Backtab == keyOfEvent) { //shift+tab 焦点顺序，与tab焦点顺序相反
        DWindowOptionButton *optionbtn = this->titlebar()->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
        if (obj == optionbtn) {
            if (m_pTitleCommentButton->isVisible() && m_pTitleCommentButton->isEnabled()) {
                m_pTitleCommentButton->setFocus(Qt::BacktabFocusReason);
                return true;
            } else if (m_pTitleButton->isVisible() && m_pTitleButton->isEnabled()) {
                m_pTitleButton->setFocus(Qt::BacktabFocusReason);
                return true;
            } else {
                return false;
            }
        } else if (obj->objectName() == "TitleButton") {
            switch (m_ePageID) {
            case PI_UnCompress:
                m_pUnCompressPage->getUnCompressBtn()->setFocus(Qt::BacktabFocusReason);
                break;
            case PI_Compress:
                m_pCompressPage->getNextBtn()->setFocus(Qt::TabFocusReason);
                break;
            case PI_CompressSetting:
                m_pCompressSettingPage->getCompressBtn()->setFocus(Qt::BacktabFocusReason);
                break;
            default:
                return false;
            }
            return true;
        } else if (obj->objectName() == "TableViewFile") {
            if (nullptr != qobject_cast<DataTreeView *>(obj) && qobject_cast<DataTreeView *>(obj)->getHeaderView()->getpreLbl()->isVisible()) {
                qobject_cast<DataTreeView *>(obj)->getHeaderView()->getpreLbl()->setFocus(Qt::BacktabFocusReason);
            } else {
                DWindowCloseButton *closeButton = titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
                if (nullptr != closeButton) {
                    closeButton->setFocus(Qt::BacktabFocusReason);
                }
            }
            return true;
        } else if (obj->objectName() == "ClickTypeLabel" || obj->objectName() == "gotoPreviousLabel") {
            DWindowCloseButton *closeButton = titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
            if (closeButton) {
                closeButton->setFocus(Qt::BacktabFocusReason);
            }
            return  true;
        } else if (obj == m_pCompressPage->getNextBtn()) {
            m_pCompressPage->getCompressView()->setFocus(Qt::BacktabFocusReason);
            return true;
        } /*else if (obj == m_pUnCompressPage->getNextbutton()) {
            m_pUnCompressPage->getPathCommandLinkButton()->setFocus(Qt::BacktabFocusReason);
            return true;
        }*/ else if (obj == m_pUnCompressPage->getUncompressPathBtn()) {
            m_pUnCompressPage->getUnCompressView()->setFocus(Qt::BacktabFocusReason);
            return true;
        }
    } else if (Qt::Key_Left == keyOfEvent || Qt::Key_Up == keyOfEvent) { //Key_Left、Key_Up几处顺序特殊处理*/
        if (obj == m_pCompressPage->getNextBtn()) {
            m_pCompressPage->getCompressView()->setFocus(Qt::BacktabFocusReason);
            return true;
        } else if (obj == m_pUnCompressPage->getUnCompressBtn()) {
            m_pUnCompressPage->getUncompressPathBtn()->setFocus(Qt::BacktabFocusReason);
            return true;
        } else if (obj == m_pUnCompressPage->getUncompressPathBtn()) {
            m_pUnCompressPage->getUnCompressView()->setFocus(Qt::BacktabFocusReason);
            return true;
        }
    }

    return false;
}

void MainWindow::handleQuit()
{
    // 关闭处理
    close();
}

void MainWindow::slotHandleArguments(const QStringList &listParam, MainWindow::ArgumentType eType)
{
    qInfo() << listParam;
    if (!m_initFlag) {
        // 初始化界面
        qInfo() << "初始化界面";
        initUI();
        initConnections();
        m_initFlag = true;
    }

    if (listParam.count() == 0) {
        QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
            emit sigquitApp();
        });
        return;
    }

    qInfo() << "处理传入参数";

    switch (eType) {
    case AT_Open: {         // 打开操作
        if (!handleArguments_Open(listParam)) {
            QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
                emit sigquitApp();
            });
            return;
        }
    }
    break;
    case AT_RightMenu: {    // 右键操作
        if (!handleArguments_RightMenu(listParam)) {
            QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
                emit sigquitApp();
            });
            return;
        }
    }
    break;
    case AT_DragDropAdd: {       // 拖拽追加操作
        if (!handleArguments_Append(listParam)) {
            QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
                emit sigquitApp();
            });
            return;
        }
    }
    break;
    }

// 刷新界面并显示
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

    // 判断是否是本地设备文件，过滤 手机 网络 ftp smb 等
    for (const auto &url : listSelFiles) {
        if (!UiTools::isLocalDeviceFile(url)) {
            return;
        }

    }

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
    } else if (m_ePageID == PI_CompressSetting) {
        // 追加压缩
        m_pCompressPage->addCompressFiles(listSelFiles);
        m_ePageID = PI_Compress;
        refreshPage();
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
    qInfo() << "点击了压缩按钮";
    m_operationtype = Operation_Create;

    m_stCompressParameter = val.value<CompressParameter>();    // 获取压缩参数
    QStringList listFiles = m_pCompressPage->compressFiles();   // 获取待压缩文件

    if (listFiles.count() == 0) {
        qInfo() << "没有需要压缩的文件";
        return;
    }

    // 创建压缩所需相关数据，调用压缩参数
    QList<FileEntry> listEntry;
    QString strDestination;
    CompressOptions options;

    // 构建压缩文件数据
    foreach (QString strFile, listFiles) {
        FileEntry stFileEntry;
        stFileEntry.strFullPath = strFile;
        listEntry.push_back(stFileEntry);
    }

    strDestination = m_stCompressParameter.strTargetPath + QDir::separator() + m_stCompressParameter.strArchiveName;

    // 构建压缩参数
    options.bEncryption = m_stCompressParameter.bEncryption;
    options.strPassword = m_stCompressParameter.strPassword;
    options.strEncryptionMethod = m_stCompressParameter.strEncryptionMethod;
    options.strCompressionMethod = m_stCompressParameter.strCompressionMethod;
    options.bHeaderEncryption = m_stCompressParameter.bHeaderEncryption;
    options.bSplit = m_stCompressParameter.bSplit;
    options.iVolumeSize = m_stCompressParameter.iVolumeSize;
    options.iCompressionLevel = m_stCompressParameter.iCompressionLevel;
    options.qTotalSize = m_stCompressParameter.qSize;
    options.bTar_7z = m_stCompressParameter.bTar_7z;

    bool bUseLibarchive = false;
#ifdef __aarch64__ // 华为arm平台 zip压缩 性能提升. 在多线程场景下使用7z,单线程场景下使用libarchive
    double maxFileSizeProportion = static_cast<double>(maxFileSize_) / static_cast<double>(m_stCompressParameter.qSize);
    bUseLibarchive = maxFileSizeProportion > 0.6;
#else
    bUseLibarchive = false;
#endif

    // 判断zip格式是否使用了中文加密
    bool zipPasswordIsChinese = false;
    if (m_stCompressParameter.strMimeType == "application/zip") {
        if (m_stCompressParameter.strPassword.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
            zipPasswordIsChinese = true;
        }
    }

    UiTools::AssignPluginType eType = UiTools::APT_Auto;        // 默认自动选择插件
    if (zipPasswordIsChinese == true) {
        // 对zip的中文加密使用libzip插件
        eType = UiTools::APT_Libzip;
    } else if (bUseLibarchive == true && m_stCompressParameter.strMimeType == "application/zip") {  // 考虑到华为arm平台 zip压缩 性能提升，只针对zip类型的压缩才会考虑到是否特殊处理arm平台
        // 华为ARM单线程场景下使用libarchive
        eType = UiTools::APT_Libarchive;
    }

    if (ArchiveManager::get_instance()->createArchive(listEntry, strDestination, options, eType/*, bBatch*/)) {
        // 切换进度界面
        m_pProgressPage->setProgressType(PT_Compress);
        m_pProgressPage->setTotalSize(m_stCompressParameter.qSize);
        m_pProgressPage->setArchiveName(m_stCompressParameter.strArchiveName);
        m_pProgressPage->restartTimer();

        m_operationtype = Operation_Create;
        m_ePageID = PI_CompressProgress;
        refreshPage();
    } else {
        // 无可用插件
        showErrorMessage(FI_Compress, EI_NoPlugin);
    }
}

void MainWindow::slotJobFinished(ArchiveJob::JobType eJobType, PluginFinishType eFinishType, ErrorType eErrorType)
{
    qInfo() << "操作类型：" << eJobType << "****结束类型：" << eFinishType << "****错误类型" << eErrorType;

    switch (eFinishType) {
    case PFT_Nomral:
        handleJobNormalFinished(eJobType);  // 处理job正常结束
        break;
    // 用户取消操作
    case PFT_Cancel:
        handleJobCancelFinished(eJobType);  // 处理job取消结束
        break;
    // 错误处理
    case PFT_Error:
        handleJobErrorFinished(eJobType, eErrorType);   // 处理job错误结束
        break;
    }

    m_operationtype = Operation_NULL;   // 重置操作类型
    m_fileWriteErrorName.clear();    // 清空记录的创建失败文件

    refreshPage();

    // 拖拽追加或右键压缩成7z、zip，完毕自动关闭界面
    if (((PFT_Nomral == eFinishType || PFT_Cancel == eFinishType) && ArchiveJob::JT_Add == eJobType && StartupType::ST_DragDropAdd == m_eStartupType)
            || StartupType::ST_Compresstozip7z == m_eStartupType) {
        QTimer::singleShot(100, this, [ = ]() {
            close();
        });
    } else if (ArchiveJob::JT_Create == eJobType // 如果是右键压缩，压缩完毕自动关闭界面
               && StartupType::ST_Compress == m_eStartupType && eFinishType == PFT_Nomral) {
        Dtk::Core::DSysInfo::UosEdition edition =  Dtk::Core::DSysInfo::uosEditionType();
        //等于服务器行业版或欧拉版(centos)
        bool isCentos = Dtk::Core::DSysInfo::UosEuler == edition || Dtk::Core::DSysInfo::UosEnterpriseC == edition;

        if (isCentos) {
            QTimer::singleShot(100, this, [ = ]() {
                close();
            });
        }
    }

    PERF_PRINT_END("POINT-03");
    PERF_PRINT_END("POINT-04");
    PERF_PRINT_END("POINT-05");
}

void MainWindow::slotUncompressClicked(const QString &strUncompressPath)
{
    m_operationtype = Operation_Extract; //解压操作

    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();
    ExtractionOptions options;
    ArchiveData stArchiveData = DataManager::get_instance().archiveData();

    // 构建解压参数
    options.strTargetPath = strUncompressPath;
    options.bAllExtract = true;
    options.qSize = stArchiveData.qSize;
    options.qComressSize = stArchiveData.qComressSize;

    /***tar.7z格式压缩流程特殊处理***
     * 1、tar.7z本质上就是一个tar包压缩成7z包，类型依然是x-7z-compressed
     * 2、只针对7z里只有一个tar包的解压才做特殊处理，即直接解压出tar包内的文件
     * 3、对于7z里有多个文件或唯一文件不是tar包的情况，解压不做特殊处理
     * 4、后缀不为tar.7z,解压不做特殊处理
     */
    if (determineMimeType(strArchiveFullPath).name() == QLatin1String("application/x-7z-compressed")
            && strArchiveFullPath.endsWith(QLatin1String(".tar.7z"))) { // 是否为tar.7z后缀的7z压缩包
        if (1 == stArchiveData.mapFileEntry.size()
                && stArchiveData.mapFileEntry.first().strFileName.endsWith(".tar")) { // 7z里是否只有一个tar包
            options.bTar_7z = true;
        }
    }

    // 如果自动创建文件夹,解压时增加一层以压缩包名称命名的目录
    QString strAutoPath = getExtractPath(strArchiveFullPath);
    if (!strAutoPath.isEmpty()) {
        options.strTargetPath += QDir::separator() + strAutoPath;
    }

    m_stUnCompressParameter.strExtractPath = options.strTargetPath;

    // 调用解压函数
    if (ArchiveManager::get_instance()->extractFiles(strArchiveFullPath, QList<FileEntry>(), options)) {
        // 设置进度界面参数
        m_pProgressPage->setProgressType(PT_UnCompress);
        m_pProgressPage->setTotalSize(options.qSize);
        m_pProgressPage->setArchiveName(strArchiveFullPath);
        m_pProgressPage->restartTimer(); // 重启计时器
        m_ePageID = PI_UnCompressProgress;

        refreshPage();
    } /*else {
        // 无可用插件
        showErrorMessage(FI_Uncompress, EI_NoPlugin);
    }*/
}

void MainWindow::slotReceiveProgress(double dPercentage)
{
    if (Operation_SingleExtract == m_operationtype) { //提取删除操作使用小弹窗进度
        //需要添加dPercentage < 100判断，否则会出现小文件提取进度对话框不会自动关闭
        if (m_pProgressdialog->isHidden() && dPercentage < 100 && dPercentage > 0) {
            m_pProgressdialog->exec();
        }

        m_pProgressdialog->setProcess(dPercentage);
    } else if (Operation_Update_Comment == m_operationtype) { // 更新压缩包注释的进度
        if (!m_commentProgressDialog->isVisible()) {
            m_commentProgressDialog->exec();
        }

        m_commentProgressDialog->setProgress(dPercentage);
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

void MainWindow::slotReceiveFileWriteErrorName(const QString &strName)
{
    m_fileWriteErrorName = strName;
}

void MainWindow::slotQuery(Query *query)
{
    qInfo() << " query->execute()";
    query->setParent(this);
    query->execute();
}

void MainWindow::Extract2PathFinish(QString msg)
{
    QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
    sendMessage(new CustomFloatingMessage(icon, msg, 1000, this));

    // 设置了自动打开文件夹处理流程
    if (m_pSettingDlg->isAutoOpen()) {
        if (m_pDDesktopServicesThread == nullptr) {
            m_pDDesktopServicesThread = new DDesktopServicesThread(this);
        }

        // 打开选中第一个提取的文件/文件夹
        if (m_stUnCompressParameter.listExractFiles.count() > 0)
            m_pDDesktopServicesThread->setOpenFiles(QStringList() << m_stUnCompressParameter.listExractFiles[0]);
        m_pDDesktopServicesThread->start();
    }
}

QString MainWindow::createUUID()
{
    // 创建临时路径
    QTemporaryDir dir;
    QString strTempPath =  dir.path();
    strTempPath = strTempPath.remove(TEMPPATH);     // 移除/tmp
    return strTempPath;
}

QString MainWindow::getExtractPath(const QString &strArchiveFullPath)
{
    QString strpath = "";
    // 根据是否自动创建文件夹获取解压最后一层路径
    if (m_pSettingDlg->isAutoCreatDir()) {
        QFileInfo info(strArchiveFullPath);
        strpath = UiTools::handleFileName(info.filePath());
//        if (info.filePath().contains(".tar.")) {
//            strpath = strpath.remove(".tar"); // 类似tar.gz压缩文件，创建文件夹的时候移除.tar
//        } else if (info.filePath().contains(".7z.")) {
//            strpath = strpath.remove(".7z"); // 7z分卷文件，创建文件夹的时候移除.7z
//        } else if (info.filePath().contains(".part01.rar")) {
//            strpath = strpath.remove(".part01"); // tar分卷文件，创建文件夹的时候移除part01
//        } else if (info.filePath().contains(".part1.rar")) {
//            strpath = strpath.remove(".part1"); // rar分卷文件，创建文件夹的时候移除.part1
//        } else if (info.filePath().contains(".zip.")) {
//            strpath = strpath.remove(".zip"); // zip分卷文件，创建文件夹的时候移除.zip
//        }
    }

    return strpath;
}

void MainWindow::handleJobNormalFinished(ArchiveJob::JobType eType)
{
    switch (eType) {
    // 创建压缩包
    case ArchiveJob::JT_Create: {
        m_ePageID = PI_Success;
        showSuccessInfo(SI_Compress);   // 显示压缩成功
        // 删除文件监听定时器
        if (0 != m_iCompressedWatchTimerID) {
            killTimer(m_iCompressedWatchTimerID);
            m_iCompressedWatchTimerID = 0;
        }
        // 设置了压缩完成自动删除原文件
        if (m_pSettingDlg->isAutoDeleteFile()) {
            deleteWhenJobFinish(ArchiveJob::JT_Create);
        }

        // 初始化文件服务
        if (m_pDDesktopServicesThread == nullptr) {
            m_pDDesktopServicesThread = new DDesktopServicesThread(this);
        }

        // 设置需要查看的文件为压缩包
        QString name = m_stCompressParameter.bSplit ? m_stCompressParameter.strArchiveName + ".001" : m_stCompressParameter.strArchiveName;
        m_pDDesktopServicesThread->setOpenFiles(QStringList() << m_stCompressParameter.strTargetPath + QDir::separator() + name);

        // zip压缩包添加注释
        addArchiveComment();
    }
    break;
    // 添加文件至压缩包
    case ArchiveJob::JT_Add: {
        qInfo() << "添加结束";

        //拖拽追加成功后不需要刷新
        if (StartupType::ST_DragDropAdd != m_eStartupType) {
            // 追加成功tip提示
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
            sendMessage(new CustomFloatingMessage(icon, tr("Adding successful"), 1000, this));

            // 追加完成更新压缩包数据
            m_operationtype = Operation_UpdateData;
            if (ArchiveManager::get_instance()->updateArchiveCacheData(m_stUpdateOptions)) {
                // 开始更新
                m_pLoadingPage->setDes(tr("Updating, please wait..."));
                m_pLoadingPage->startLoading();     // 开始加载
                m_ePageID = PI_Loading;
            } else {
                // 无可用插件
                showErrorMessage(FI_Add, EI_NoPlugin);
            }
        } else {
            m_ePageID = PI_Success;
            showSuccessInfo(SI_Compress);   // 显示压缩成功
        }
    }
    break;
    // 加载压缩包数据
    case ArchiveJob::JT_Load: {
        qInfo() << "加载结束";
        m_pLoadingPage->stopLoading();

        // 判断压缩包是否有数据
        if (DataManager::get_instance().archiveData().listRootEntry.count() == 0) {
            // 回到首页
            m_ePageID = PI_Home;
            m_pMainWidget->setCurrentIndex(0);
            resetMainwindow();
            // 提示用户无数据
            TipDialog dialog(this);
            dialog.showDialog(tr("No data in it"), tr("OK"), DDialog::ButtonNormal);
        } else {
            // 有数据的情况下切换到解压列表界面，刷新数据
            m_ePageID = PI_UnCompress;
            m_pUnCompressPage->refreshArchiveData();
        }
    }
    break;
    // 批量解压
    case ArchiveJob::JT_BatchExtract:
    // 解压
    case ArchiveJob::JT_StepExtract:
    case ArchiveJob::JT_Extract: {
        if (Archive_OperationType::Operation_SingleExtract == m_operationtype) {
            qInfo() << "提取结束";
            m_ePageID = PI_UnCompress;
            Extract2PathFinish(tr("Extraction successful", "提取成功")); //提取成功
            m_pProgressdialog->setFinished();
        } else {
            qInfo() << "解压结束";
            ArchiveData stArchiveData = DataManager::get_instance().archiveData();

            if (stArchiveData.listRootEntry.count() == 0) {
                // 压缩包无数据的情况下，跳转到错误界面，提示解压失败，无数据
                m_ePageID = PI_Failure;
                showErrorMessage(FI_Uncompress, EI_ArchiveNoData);
            } else {
                // 正常解压完成的情况，解压成功
                m_ePageID = PI_Success;
                showSuccessInfo(SI_UnCompress);

                // 初始化服务
                if (m_pDDesktopServicesThread == nullptr) {
                    m_pDDesktopServicesThread = new DDesktopServicesThread(this);
                }

                if (m_stUnCompressParameter.bBatch) {
                    // 批量解压
                    QStringList listFiles;
                    QString strFile = m_stUnCompressParameter.strExtractPath;
                    if (m_pSettingDlg->isAutoCreatDir()) {
                        // 自动创建文件夹的情况下显示创建的文件夹内容
                        for (int i = 0; i < m_stUnCompressParameter.listBatchFiles.count(); ++i) {
                            listFiles << m_stUnCompressParameter.strExtractPath + QDir::separator() + UiTools::handleFileName(m_stUnCompressParameter.listBatchFiles[i]);
                        }
                    } else {
                        // 未自动创建文件夹的情况下，显示每个压缩包解压出的第一个文件
                        for (int i = 0; i < stArchiveData.listRootEntry.count(); ++i) {
                            listFiles << m_stUnCompressParameter.strExtractPath + QDir::separator() + stArchiveData.listRootEntry[i].strFullPath;
                            qInfo() << "**********" << m_stUnCompressParameter.strExtractPath + QDir::separator() + stArchiveData.listRootEntry[i].strFullPath;
                        }
                    }
                    // 设置最终需要打开的文件
                    m_pDDesktopServicesThread->setOpenFiles(listFiles);
                } else {
                    // 单压缩包解压
                    QString strFile = m_stUnCompressParameter.strExtractPath;
                    // 未自动创建文件夹且有解压出数据的情况
                    if (!m_pSettingDlg->isAutoCreatDir() && stArchiveData.listRootEntry.count() > 0)
                        strFile += QDir::separator() + stArchiveData.listRootEntry[0].strFullPath;
                    // 设置最终需要打开的文件
                    qInfo() << "单压缩包解压 设置最终需要打开的文件*********************" << strFile;
                    m_pDDesktopServicesThread->setOpenFiles(QStringList() << strFile);
                }

                // 设置了自动打开文件夹
                if (m_pSettingDlg->isAutoOpen()) {
                    m_pDDesktopServicesThread->start();
                }

                // 设置了解压完成自动删除原压缩包
                if (m_pSettingDlg->isAutoDeleteArchive() == AUTO_DELETE_ALWAYS) {
                    // 总是自动删除原压缩包
                    deleteWhenJobFinish(ArchiveJob::JT_Extract);
                } else if (m_pSettingDlg->isAutoDeleteArchive() == AUTO_DELETE_ASK) {
                    // 创建询问删除对话框
                    SimpleQueryDialog dialog(this);
                    int iResult = dialog.showDialog(tr("Do you want to delete the archive?"), tr("Cancel"), DDialog::ButtonNormal, tr("Confirm"), DDialog::ButtonRecommend);
                    // 点击确认时，删除原压缩包
                    if (iResult == QDialog::Accepted) {
                        deleteWhenJobFinish(ArchiveJob::JT_Extract);
                    }
                }

                if (StartupType::ST_ExtractHere == m_eStartupType || StartupType::ST_Extractto == m_eStartupType) {
                    m_operationtype = Operation_NULL;   // 首先将操作置空，防止关闭时提示有任务进行
                    // 右键解压到当前文件夹，关闭界面（延时100ms，显示明了）
                    QTimer::singleShot(100, this, [ = ]() {
                        close();;
                    });
                }
            }
        }
    }
    break;
// 删除
    case ArchiveJob::JT_Delete: {
        qInfo() << "删除结束";
        // 追加完成更新压缩包数据
        m_operationtype = Operation_UpdateData;
        if (ArchiveManager::get_instance()->updateArchiveCacheData(m_stUpdateOptions)) {
            // 开始更新
            m_pLoadingPage->setDes(tr("Updating, please wait..."));
            m_pLoadingPage->startLoading();
            m_ePageID = PI_Loading;
        } else {
            // 无可用插件
            showErrorMessage(FI_Delete, EI_NoPlugin);
        }
    }
    break;
// 打开
    case ArchiveJob::JT_Open: {
        qInfo() << "打开结束";
        // 若压缩包文件可更改，打开文件之后对文件进行监控
        // 非分卷的rar可以进行格式转换
        if ((m_stUnCompressParameter.bModifiable) ||
                ((m_stUnCompressParameter.eSplitVolume == UnCompressParameter::ST_No)
                 && (determineMimeType(m_stUnCompressParameter.strFullPath).name() == "application/vnd.rar"))) {
            // 打开成功之后添加当前打开文件至文件监控中
            m_pOpenFileWatcher->addCurOpenWatchFile();
            m_pOpenFileWatcher->setCurFilePassword(ArchiveManager::get_instance()->getCurFilePassword());
        }

        m_ePageID = PI_UnCompress;
        m_pLoadingPage->stopLoading();      // 停止更新
    }
    break;
// 格式转换
    case ArchiveJob::JT_Convert: {
        m_ePageID = PI_Success;
        showSuccessInfo(SI_Convert);   // 显示压缩成功
    }
    break;
// 追加/删除更新
    case ArchiveJob::JT_Update: {
        qInfo() << "更新结束";
        m_pLoadingPage->stopLoading();      // 停止更新

        if (DataManager::get_instance().archiveData().listRootEntry.count() == 0) {
            QFile::remove(m_stUnCompressParameter.strFullPath); // 删除原始压缩包
            m_pUnCompressPage->clear(); // 清空解压界面
            // 压缩包数据为空时，回到首页，且删除原始压缩包
            resetMainwindow();
            m_ePageID = PI_Home;
        } else {
            // 刷新解压列表数据
            m_ePageID = PI_UnCompress;
            m_pUnCompressPage->refreshDataByCurrentPathChanged();
        }
    }
    break;
// 更新压缩包注释
    case ArchiveJob::JT_Comment: {
        if (Operation_Update_Comment == m_operationtype) {
            qInfo() << "更新注释结束";
            m_commentProgressDialog->setFinished();
        } else { /* if (Operation_Add_Comment == m_operationtype)*/ // creatjob结束的时候工作类型已经置为Operation_NULL
            qInfo() << "添加zip注释结束";
            m_ePageID = PI_Success;
            showSuccessInfo(SI_Compress);   // 显示压缩成功
        }
    }
    break;
    default:
        break;
    }
}

void MainWindow::handleJobCancelFinished(ArchiveJob::JobType eType)
{
    switch (eType) {
    // 创建压缩包
    case ArchiveJob::JT_Create: {
        if (StartupType::ST_Compresstozip7z == m_eStartupType) { // 右键快捷压缩不需要返回到界面，应该直接关闭
            // 避免重复提示停止任务
            m_operationtype = Operation_NULL;
            // 直接关闭应用
            close();
        } else {
            m_ePageID = PI_Compress;
        }
    }
    break;
    // 添加文件至压缩包
    case ArchiveJob::JT_Add: {
        //拖拽追加取消后不需要返回列表界面
        if (StartupType::ST_DragDropAdd != m_eStartupType) {
            m_ePageID = PI_UnCompress;
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
            sendMessage(new CustomFloatingMessage(icon, tr("Adding canceled"), 1000, this));
        } else {
            // 避免重复提示停止任务
            m_operationtype = Operation_NULL;
            // 直接关闭应用
            close();
        }
    }
    break;
    // 打开压缩包
    case ArchiveJob::JT_Load: {
        m_ePageID = PI_Home;
    }
    break;
    // 批量解压
    case ArchiveJob::JT_BatchExtract:
    // 解压
    case ArchiveJob::JT_StepExtract:
    case ArchiveJob::JT_Extract: {
        if (Archive_OperationType::Operation_SingleExtract == m_operationtype) {
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
            sendMessage(new CustomFloatingMessage(icon, tr("Extraction canceled", "取消提取"), 1000, this));
        } else {
            if (StartupType::ST_ExtractHere == m_eStartupType || StartupType::ST_Extractto == m_eStartupType) {
                // 避免重复提示停止任务
                m_operationtype = Operation_NULL;
                // 直接关闭应用
                close();
            } else {
                // 切换到解压列表界面，再执行相关操作
                m_ePageID = PI_UnCompress;
            }
        }
    }
    break;
    // 删除
    case ArchiveJob::JT_Delete: {
        m_ePageID = PI_UnCompress;
    }
    break;
    case ArchiveJob::JT_Open: {
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
    switch (eJobType) {
    // 创建压缩包错误
    case ArchiveJob::JT_Create: {
        // 删除文件监听定时器
        if (0 != m_iCompressedWatchTimerID) {
            killTimer(m_iCompressedWatchTimerID);
            m_iCompressedWatchTimerID = 0;
        }

        switch (eErrorType) {
        case ET_InsufficientDiskSpace: {
            showErrorMessage(FI_Compress, EI_InsufficientDiskSpace, true);
            break;
        }
        default: {
            showErrorMessage(FI_Compress, EI_CreatArchiveFailed, true);
            break;
        }
        }

    }
    break;
    // 压缩包追加文件错误
    case ArchiveJob::JT_Add: {
        //拖拽追加失败后需要跳转到失败界面，
        if (StartupType::ST_DragDropAdd != m_eStartupType) {
            m_ePageID = PI_UnCompress;
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(30, 30));
            switch (eErrorType) {
            // 密码错误
            case ET_WrongPassword: {
                sendMessage(new CustomFloatingMessage(icon, tr("Wrong password"), 1000, this));
                break;
            }
            default: {
                sendMessage(new CustomFloatingMessage(icon, tr("Adding failed"), 1000, this));
                break;
            }
            }
        } else {
            switch (eErrorType) {
            // 密码错误
            case ET_WrongPassword: {
                showErrorMessage(FI_Uncompress, EI_WrongPassword);
                break;
            }
            default: {
                showErrorMessage(FI_Uncompress, EI_ArchiveDamaged);
                break;
            }
            }
        }
    }
    break;
    // 加载压缩包错误
    case ArchiveJob::JT_Load: {
        switch (eErrorType) {
        // 压缩包损坏
        case ET_ArchiveDamaged:
            showErrorMessage(FI_Load, EI_ArchiveDamaged);
            break;
        // 密码错误
        case ET_WrongPassword:
            showErrorMessage(FI_Load, EI_WrongPassword);
            break;
        default:
            showErrorMessage(FI_Load, EI_ArchiveDamaged);
            break;
        }
    }
    break;
    // 解压错误
    case ArchiveJob::JT_StepExtract:
    case ArchiveJob::JT_Extract: {
        if (Archive_OperationType::Operation_SingleExtract == m_operationtype) {
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(30, 30));

            if (m_pProgressdialog->isVisible()) {
                m_pProgressdialog->setFinished();
            }

#if 0 // 提取失败详细提示
            // 提取出错
            switch (eErrorType) {
            // 压缩包损坏
            case ET_ArchiveDamaged : {
                sendMessage(new CustomFloatingMessage(icon, tr("The archive is damaged"), 1000, this));
                break;
            }
            // 密码错误
            case ET_WrongPassword: {
                sendMessage(new CustomFloatingMessage(icon, tr("Wrong password"), 1000, this));
                break;
            }
            // 文件名过长
            case ET_LongNameError: {
                sendMessage(new CustomFloatingMessage(icon, tr("File name too long"), 1000, this));
                break;
            }
            // 创建文件失败
            case ET_FileWriteError: {
                sendMessage(new CustomFloatingMessage(icon, tr("Failed to create \"%1\"").arg(UiTools::toShortString(m_fileWriteErrorName)), 1000, this));
                break;
            }
            default:
                break;
            }
#endif
            sendMessage(new CustomFloatingMessage(icon, tr("Extraction failed", "提取失败"), 1000, this)); // 提取失败提示
        } else {
            // 解压出错
            switch (eErrorType) {
            // 压缩包损坏
            case ET_ArchiveDamaged :
                showErrorMessage(FI_Uncompress, EI_ArchiveDamaged,
                                 !(StartupType::ST_ExtractHere == m_eStartupType || StartupType::ST_Extractto == m_eStartupType));
                break;
            // 密码错误
            case ET_WrongPassword:
                showErrorMessage(FI_Uncompress, EI_WrongPassword,
                                 !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
                break;
            // 文件名过长
            case ET_LongNameError:
                showErrorMessage(FI_Uncompress, EI_LongFileName,
                                 !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
                break;
            // 创建文件失败
            case ET_FileWriteError:
                showErrorMessage(FI_Uncompress, EI_CreatFileFailed,
                                 !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
                break;
            case ET_MissingVolume:
                showErrorMessage(FI_Uncompress, EI_ArchiveMissingVolume,
                                 !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
                break;
            case ET_InsufficientDiskSpace: {
                showErrorMessage(FI_Uncompress, EI_InsufficientDiskSpace,
                                 !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
                break;
            }
            case ET_PluginError: {
                // 无可用插件
                showErrorMessage(FI_Uncompress, EI_NoPlugin);
                break;
            }
            default:
                showErrorMessage(FI_Uncompress, EI_ArchiveDamaged);
                break;
            }
        }
    }
    break;
    // 删除错误
    case ArchiveJob::JT_Delete: {
        m_ePageID = PI_UnCompress;
#if 0 // 删除错误提示暂时不需要
        QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(30, 30));
        switch (eErrorType) {
        // 压缩包损坏
        case ET_ArchiveDamaged : {
            sendMessage(new CustomFloatingMessage(icon, tr("The archive is damaged"), 1000, this));
            break;
        }
        // 密码错误
        case ET_WrongPassword: {
            sendMessage(new CustomFloatingMessage(icon, tr("Wrong password"), 1000, this));
            break;
        }
        default:
//            sendMessage(new CustomFloatingMessage(icon, tr("Delete failed"), 1000, this));
            break;
        }
#endif
    }
    break;
    // 批量解压错误
    case ArchiveJob::JT_BatchExtract: {
        switch (eErrorType) {
        // 压缩包损坏
        case ET_ArchiveDamaged :
            showErrorMessage(FI_Uncompress, EI_ArchiveDamaged,
                             !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
            break;
        // 密码错误
        case ET_WrongPassword:
            showErrorMessage(FI_Uncompress, EI_WrongPassword,
                             !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
            break;
        // 文件名过长
        case ET_LongNameError:
            showErrorMessage(FI_Uncompress, EI_LongFileName,
                             !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
            break;
        // 创建文件失败
        case ET_FileWriteError:
            showErrorMessage(FI_Uncompress, EI_CreatFileFailed,
                             !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
            break;
        case ET_InsufficientDiskSpace: {
            showErrorMessage(FI_Uncompress, EI_InsufficientDiskSpace,
                             !(StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/));
            break;
        }
        case ET_PluginError: {
            // 无可用插件
            showErrorMessage(FI_Uncompress, EI_NoPlugin);
            break;
        }
        default:
            showErrorMessage(FI_Uncompress, EI_ArchiveDamaged);
            break;
        }
    }
    break;
    // 打开压缩包中的文件错误
    case ArchiveJob::JT_Open:
        if (Archive_OperationType::Operation_TempExtract_Open == m_operationtype) {
            m_ePageID = PI_UnCompress;
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(30, 30));
            if (eErrorType == ET_WrongPassword) { // 打开压缩包中文件密码错误
                sendMessage(new CustomFloatingMessage(icon, tr("Wrong password"), 1000, this));
            }
        }
        break;
    // 转换错误
    case ArchiveJob::JT_Convert:

        break;
    // 更新压缩包数据错误
    case ArchiveJob::JT_Update:

        break;
    default:
        break;
    }
}

void MainWindow::addFiles2Archive(const QStringList &listFiles, const QString &strPassword)
{
    // 没有需要追加的文件时，直接返回，防止出现追加根目录的现象
    if (listFiles.isEmpty())
        return;

    qInfo() << "向压缩包中添加文件";
    m_operationtype = Operation_Add;

    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();  // 获取压缩包全路径
    CompressOptions options;
    QList<FileEntry> listEntry;

    options.strDestination = m_pUnCompressPage->getCurPath();   // 获取追加目录
    options.strPassword = strPassword;
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    options.qTotalSize = stArchiveData.qSize; // 原压缩包内文件总大小，供libarchive追加进度使用

    // 构建压缩文件数据
    foreach (QString strFile, listFiles) {
        FileEntry stFileEntry;
        stFileEntry.strFullPath = strFile;
        if (!QFileInfo(strFile).isDir()) {
            stFileEntry.qSize = QFileInfo(strFile).size(); // 原文件大小，供libarchive追加进度使用
        }

        listEntry.push_back(stFileEntry);
    }

    // 切换进度界面
    m_pProgressPage->setProgressType(PT_CompressAdd);
    m_pProgressPage->setArchiveName(strArchiveFullPath);
    m_operationtype = Operation_Add;
    m_ePageID = PI_AddCompressProgress;
    m_pProgressPage->setPushButtonCheckable(false, false);
    refreshPage();

    // 计算大小
    m_mywork = new CalculateSizeThread(listFiles, m_stUnCompressParameter.strFullPath, listEntry, options, this);
    connect(m_mywork, &CalculateSizeThread::signalFinishCalculateSize, this, &MainWindow::slotFinishCalculateSize);
    m_mywork->start();
}

void MainWindow::resetMainwindow()
{
#ifdef __aarch64__
    maxFileSize_ = 0;
#endif

    m_ePageID = PI_Home;
    m_operationtype = Operation_NULL;   // 重置操作类型
    m_iCompressedWatchTimerID = 0;      // 初始化定时器返回值
    m_pProgressPage->resetProgress();   // 重置进度
    m_pOpenFileWatcher->reset();

    // 重置数据
    m_stUpdateOptions = UpdateOptions();
    m_stCompressParameter = CompressParameter();
    m_stUnCompressParameter = UnCompressParameter();
    m_comment.clear();
    m_isFirstViewComment = true;
    m_strFinalConvertFile.clear();
    m_eStartupType = StartupType::ST_Normal;

    // 清空压缩包监听数据
    SAFE_DELETE_ELE(m_pFileWatcher);
}

void MainWindow::deleteWhenJobFinish(ArchiveJob::JobType eType)
{
    switch (eType) {
    // 压缩完成删除原文件
    case ArchiveJob::JT_Create: {
        QStringList listCompressFiles = (m_eStartupType == StartupType::ST_Compresstozip7z) ? m_stCompressParameter.listCompressFiles : m_pCompressPage->compressFiles(); // 获取所有压缩文件

        for (int i = 0; i < listCompressFiles.count(); ++i) {
            QFileInfo file(listCompressFiles[i]);
            if (file.exists()) {
                // 文件/文件夹若存在，将之移动到回收站中（Qt5.15完美支持）
//                QString strFileName = QFileInfo(listCompressFiles[i]).fileName();
//                file.rename(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/Trash/files/") + strFileName);

                // 删除文件
                if (file.isDir()) {
                    QDir dir(listCompressFiles[i]);
                    dir.removeRecursively();
                } else {
                    QFile fi(listCompressFiles[i]);
                    fi.remove();
                }
            }
        }
    }
    break;
    // 压缩完成删除原压缩包
    case ArchiveJob::JT_Extract: {
        QStringList listArchivePath;

        if (m_eStartupType == StartupType::ST_ExtractHere || m_eStartupType == StartupType::ST_Extractto) { // 右键快捷解压
            if (m_stUnCompressParameter.bBatch) {   // 批量删除
                listArchivePath = m_stUnCompressParameter.listBatchFiles;
            } else {    // 单个压缩包
                listArchivePath << m_stUnCompressParameter.strFullPath;
            }
        } else {    // 打开解压
            listArchivePath << m_pUnCompressPage->archiveFullPath();
        }

        for (int i = 0; i < listArchivePath.count(); ++i) {
            // 获取压缩包全路径
            QFile file(listArchivePath[i]);
            // 若压缩包存在，将之移动到回收站中（Qt5.15完美支持）
            if (file.exists()) {
                //QString strFileName = QFileInfo(strArchivePath).fileName();
                //file.rename(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/Trash/files/") + strFileName);

                // 删除文件
                file.remove();
            }
        }


    }
    break;
    default:
        break;
    }
}

void MainWindow::ConstructAddOptions(const QStringList &files)
{
    foreach (QString file, files) {
        QFileInfo fileInfo(file);

        FileEntry entry;
        entry.strFullPath = fileInfo.filePath();    // 文件全路径
        entry.strFileName = fileInfo.fileName();    // 文件名
        entry.isDirectory = fileInfo.isDir();   // 是否是文件夹
        entry.qSize = fileInfo.size();   // 大小
        entry.uLastModifiedTime = fileInfo.lastModified().toTime_t();   // 最后一次修改时间

        m_stUpdateOptions.listEntry << entry;

        if (!entry.isDirectory) {  // 如果为文件，直接获取大小
            qint64 curFileSize = entry.qSize;
            m_stUpdateOptions.qSize += curFileSize;
        } else {    // 如果是文件夹，递归获取所有子文件大小总和
            QtConcurrent::run(this, &MainWindow::ConstructAddOptionsByThread, file);
        }
    }

    // 等待线程池结束
    QThreadPool::globalInstance()->waitForDone();
}

void MainWindow::ConstructAddOptionsByThread(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return;
    // 获得文件夹中的文件列表
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::System
                                           | QDir::NoDotAndDotDot | QDir::Hidden);

    for (int i = 0; i < list.count(); ++i) {
        QFileInfo fileInfo = list.at(i);

        FileEntry entry;
        entry.strFullPath = fileInfo.filePath();    // 文件全路径
        entry.strFileName = fileInfo.fileName();    // 文件名
        entry.isDirectory = fileInfo.isDir();   // 是否是文件夹
        entry.qSize = fileInfo.size();   // 大小
        entry.uLastModifiedTime = fileInfo.lastModified().toTime_t();   // 最后一次修改时间

        if (entry.isDirectory) {
            mutex.lock();
            m_stUpdateOptions.listEntry << entry;
            mutex.unlock();
            // 如果是文件夹 则将此文件夹放入线程池中进行计算
            QtConcurrent::run(this, &MainWindow::ConstructAddOptionsByThread, entry.strFullPath);
        } else {
            mutex.lock();
            // 如果是文件则直接计算大小
            m_stUpdateOptions.qSize += entry.qSize;
            m_stUpdateOptions.listEntry << entry;
            mutex.unlock();
        }
    }
}

void MainWindow::showSuccessInfo(SuccessInfo eSuccessInfo)
{
    m_pSuccessPage->setSuccessType(eSuccessInfo);

    switch (eSuccessInfo) {
    // 压缩成功
    case SI_Compress:
        m_pSuccessPage->setSuccessDes(tr("Compression successful"));
        break;
    // 解压成功
    case SI_UnCompress:
        m_pSuccessPage->setSuccessDes(tr("Extraction successful", "解压成功"));
        break;
    case SI_Convert:
        m_pSuccessPage->setSuccessDes(tr("Conversion successful"));
        break;
    }
}

void MainWindow::showErrorMessage(FailureInfo fFailureInfo, ErrorInfo eErrorInfo, bool bShowRetry)
{
    m_operationtype = Operation_NULL;   // 重置操作类型
    m_pFailurePage->setRetryEnable(bShowRetry);     // 设置重试按钮是否可用
    m_pFailurePage->setFailureInfo(fFailureInfo);   // 设置失败信息

    switch (fFailureInfo) {
    // 压缩失败
    case FI_Compress: {
        m_pFailurePage->setFailuerDes(tr("Compression failed"));
        switch (eErrorInfo) {
        case EI_NoPlugin: {
            m_pFailurePage->setFailureDetail(tr("Plugin error"));
        }
        break;
        case EI_CreatArchiveFailed: {
            m_pFailurePage->setFailureDetail(tr("Failed to create file"));
        }
        break;
        case EI_InsufficientDiskSpace: {
            m_pFailurePage->setFailureDetail(tr("Insufficient disk space"));
        }
        break;
        default:
            break;
        }
    }
    break;
    // 加载失败
    case FI_Load: {
        m_pLoadingPage->stopLoading();
        m_pFailurePage->setFailuerDes(tr("Open failed"));
        switch (eErrorInfo) {
        case EI_NoPlugin: {
            m_pFailurePage->setFailureDetail(tr("Plugin error"));
        }
        break;
        case EI_ArchiveDamaged: {
            m_pFailurePage->setFailureDetail(tr("The archive is damaged"));
        }
        break;
        case EI_WrongPassword: {
            m_pFailurePage->setFailureDetail(tr("Wrong password"));
        }
        break;
        case EI_ArchiveMissingVolume: {
            m_pFailurePage->setFailureDetail(tr("Some volumes are missing"));
        }
        break;
        default:
            break;
        }
    }
    break;
    // 解压失败
    case FI_Uncompress: {
        m_pFailurePage->setFailuerDes(tr("Extraction failed", "解压失败"));
        switch (eErrorInfo) {
        case EI_NoPlugin: {
            m_pFailurePage->setFailureDetail(tr("Plugin error"));
        }
        break;
        case EI_ArchiveDamaged: {
            m_pFailurePage->setFailureDetail(tr("The archive is damaged"));
        }
        break;
        case EI_ArchiveMissingVolume: {
            m_pFailurePage->setFailureDetail(tr("Some volumes are missing"));
        }
        break;
        case EI_WrongPassword: {
            m_pFailurePage->setFailureDetail(tr("Wrong password, please retry")); // 解压密码错误，请重试
        }
        break;
        case EI_LongFileName: {
            m_pFailurePage->setFailureDetail(tr("File name too long")); // 文件名过长
        }
        break;
        case EI_CreatFileFailed: {
            m_pFailurePage->setFailureDetail(tr("Failed to create \"%1\"").arg(UiTools::toShortString(m_fileWriteErrorName)));
        }
        break;
        case EI_ArchiveNoData: {
            m_pFailurePage->setFailureDetail(tr("No data in it"));
        }
        break;
        case EI_InsufficientDiskSpace: {
            m_pFailurePage->setFailureDetail(tr("Insufficient disk space"));
        }
        break;
        default:
            break;
        }
    }
    break;
    default:
        break;
    }

    // 刷新错误界面显示
    m_ePageID = PI_Failure;
    refreshPage();
}

QSize MainWindow::getConfigWinSize()
{
    // 获取界面宽、高
    QVariant tempWidth = m_pSettings->value(MAINWINDOW_WIDTH_NAME);
    QVariant tempHeight = m_pSettings->value(MAINWINDOW_HEIGHT_NAME);
    int winWidth = MAINWINDOW_DEFAULTW;
    int winHeight = MAINWINDOW_DEFAULTH;

    // 设置界面宽度
    if (tempWidth.isValid()) {

        winWidth = tempWidth.toInt();
        winWidth = winWidth > MAINWINDOW_DEFAULTW ? winWidth : MAINWINDOW_DEFAULTW;
    }

    // 设置界面高度
    if (tempHeight.isValid()) {
        winHeight = tempHeight.toInt();
        winHeight = winHeight > MAINWINDOW_DEFAULTH ? winHeight : MAINWINDOW_DEFAULTH;
    }

    return QSize(winWidth, winHeight);
}

void MainWindow::saveConfigWinSize(int w, int h)
{
    int winWidth = w > MAINWINDOW_DEFAULTW ? w : MAINWINDOW_DEFAULTW;
    int winHeight = h > MAINWINDOW_DEFAULTH ? h : MAINWINDOW_DEFAULTH;
    m_pSettings->setValue(MAINWINDOW_HEIGHT_NAME, winHeight);
    m_pSettings->setValue(MAINWINDOW_WIDTH_NAME, winWidth);
    m_pSettings->sync();
}

QString MainWindow::getDefaultApp(QString mimetype)
{
    QString outInfo;
    QProcess p;
    QString command3 = "xdg-mime query default %1"; // eg: xdg-mime query default application/vnd.rar
    p.start(command3.arg("application/" + mimetype)); // 获取默认打开方式
    p.waitForFinished();
    outInfo = QString::fromLocal8Bit(p.readAllStandardOutput());

    return  outInfo;
}

void MainWindow::setDefaultApp(QString mimetype, QString desktop)
{
    QProcess p;
    QString command3 = "xdg-mime default %1 %2"; // eg: xdg-mime default deepin-compressor.desktop application/vnd.rar
    p.start(command3.arg(desktop).arg("application/" + mimetype)); // 设置默认打开方式
    p.waitForFinished();
}

void MainWindow::convertArchive(QString convertType)
{
    qInfo() << "对压缩包进行格式转换" << convertType;
    m_operationtype = Operation_CONVERT;

    QString oldArchivePath = m_stUnCompressParameter.strFullPath; // 需要进行格式转换的压缩包的全路径
    QFileInfo oldArchive(oldArchivePath);

    QString newArchivePath = oldArchive.filePath().remove(oldArchive.suffix()) + convertType; // 转换后压缩包的全路径，还未判断该文件名是否存在
    // 压缩后的文件名
    int num = 2;
    while (QFileInfo::exists(newArchivePath)) { // 如果文件名存在自动重命名 文件名+（2）...
        newArchivePath = oldArchive.absolutePath() + QDir::separator() + UiTools::handleFileName(m_pUnCompressPage->archiveFullPath())
                         + "(" + QString::number(num) + ")" + "." + convertType;
        num++;
    }

    m_strFinalConvertFile = newArchivePath;  // 记录格式转换后的文件名，在转换成功之后打开文件使用

    // 创建格式转换的job
    if (ArchiveManager::get_instance()->convertArchive(oldArchivePath, TEMPPATH + QDir::separator() + m_strProcessID + createUUID(), newArchivePath)) {
        m_pProgressPage->setProgressType(PT_Convert);
        m_pProgressPage->setTotalSize(oldArchive.size() + DataManager::get_instance().archiveData().qSize);
        m_pProgressPage->setArchiveName(newArchivePath);
        m_pProgressPage->restartTimer();

        m_ePageID = PI_ConvertProgress;
        refreshPage();
    }
}

void MainWindow::updateArchiveComment()
{
    m_operationtype = Operation_Update_Comment;
    if (ArchiveManager::get_instance()->updateArchiveComment(m_stUnCompressParameter.strFullPath, m_comment)) {
        qInfo() << "更新压缩包的注释信息";
    }
}

void MainWindow::addArchiveComment()
{
    m_operationtype = Operation_Add_Comment;
    if (!m_pCompressSettingPage->getComment().isEmpty()) {
        if (ArchiveManager::get_instance()->updateArchiveComment(m_stCompressParameter.strTargetPath + QDir::separator() + m_stCompressParameter.strArchiveName, m_pCompressSettingPage->getComment())) {
            m_operationtype = Operation_Add_Comment;
            m_ePageID = PI_CommentProgress;
            m_pProgressPage->setProgressType(PT_Comment);
            qInfo() << "添加zip压缩包的注释信息";
        }
    }
}

void MainWindow::watcherArchiveFile(const QString &strFullPath)
{
    SAFE_DELETE_ELE(m_pFileWatcher);

    m_pFileWatcher = new DFileWatcher(strFullPath, this);
    m_pFileWatcher->startWatcher();

    connect(m_pFileWatcher, &DFileWatcher::fileMoved, this, [ = ]() { //监控压缩包，重命名时提示
        // 显示提示对话框
        TipDialog dialog(this);
        dialog.showDialog(tr("The archive was changed on the disk, please import it again."), tr("OK"));

        resetMainwindow();
        m_ePageID = PI_Home;
        refreshPage();
    });
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
    shortcut3.insert("name", tr("Select file"));
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

bool MainWindow::handleArguments_Open(const QStringList &listParam)
{
    qInfo() << "打开文件";
    m_eStartupType = StartupType::ST_Normal;
    // 加载单个压缩包数据
    QString strFileName = listParam[0];
    UnCompressParameter::SplitType type;
    UiTools::transSplitFileName(strFileName, type);
    loadArchive(strFileName);

    return true;
}

bool MainWindow::handleArguments_RightMenu(const QStringList &listParam)
{
    qInfo() << "右键操作";

    QString strType = listParam.last();     // 获取最后一个参数
    // 获取所有文件名
    QStringList listFiles = listParam;
    listFiles.removeLast();

    if (listFiles.count() == 0) {
        return false;
    }

    if (strType == "compress") {
        m_eStartupType = StartupType::ST_Compress;
        // 压缩
        m_pCompressPage->addCompressFiles(listFiles);
        m_pCompressSettingPage->setFileSize(listFiles, calSelectedTotalFileSize(listFiles));
        // 设置界面标识为压缩设置界面
        m_ePageID = PI_CompressSetting;
    } else if (strType == "compress_to_7z" || strType == "compress_to_zip") {
        m_eStartupType = StartupType::ST_Compresstozip7z;

        // 压缩成指定格式
        QString strSuffix = (strType == "compress_to_7z") ? ".7z" : ".zip";
        QFileInfo info = QFileInfo(listFiles[0]);
        QString strArchivePath = info.path();

        if (listFiles.count() == 1) {
            strArchivePath += QDir::separator() + UiTools::handleFileName(info.filePath()) + strSuffix;
        } else {
            QString strpath = info.absolutePath();
            int iIndex = strpath.lastIndexOf(QDir::separator());
            strArchivePath += QDir::separator() + strpath.mid(iIndex) + strSuffix;
        }

        // 判断本地是否存在此压缩包
        QFileInfo archiveInfo(strArchivePath);
        if (archiveInfo.exists()) {
            SimpleQueryDialog dialog(this);
            int iResult = dialog.showDialog(tr("Another file with the same name already exists, replace it?"), tr("Cancel"), DDialog::ButtonNormal, tr("Replace"), DDialog::ButtonWarning);
            if (iResult == 1) {     // 如果点击替换，先移除本地压缩包
                QFile file(archiveInfo.filePath());
                file.remove();
            } else {    // 点击关闭或者取消，不操作
                QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
                    emit sigquitApp();
                });
                return false;
            }
        }

        QStringList listCompressFiles = listFiles;
//        listCompressFiles.removeLast();   // 获取所有压缩包全路径

        // 创建压缩所需相关数据，调用压缩参数
        QList<FileEntry> listEntry;
        QString strDestination = strArchivePath;
        CompressOptions options;

        // 构建压缩文件数据
        foreach (QString strFile, listCompressFiles) {
            FileEntry stFileEntry;
            stFileEntry.strFullPath = strFile;
            listEntry.push_back(stFileEntry);
        }

        m_stCompressParameter.listCompressFiles = listCompressFiles;
        m_stCompressParameter.qSize = calSelectedTotalFileSize(listCompressFiles);
        m_stCompressParameter.strTargetPath = archiveInfo.filePath();
        m_stCompressParameter.strArchiveName = archiveInfo.fileName();

        options.qTotalSize = m_stCompressParameter.qSize;

        if (ArchiveManager::get_instance()->createArchive(listEntry, strDestination, options, UiTools::APT_Auto)) {
            // 切换进度界面
            m_pProgressPage->setProgressType(PT_Compress);
            m_pProgressPage->setTotalSize(m_stCompressParameter.qSize);
            m_pProgressPage->setArchiveName(m_stCompressParameter.strArchiveName);
            m_pProgressPage->restartTimer();

            m_operationtype = Operation_Create;
            m_ePageID = PI_CompressProgress;
        } else {
            // 无可用插件
            showErrorMessage(FI_Compress, EI_NoPlugin);
        }

    } else if (strType == "extract") {
        m_eStartupType = StartupType::ST_Extract;
        // 解压缩
        QStringList listTransFiles;
        // 对压缩包文件名进行处理
        for (int i = 0; i < listFiles.count(); ++i) {
            QString strFileName = listFiles[i];
            UnCompressParameter::SplitType eSplitVolume;
            UiTools::transSplitFileName(strFileName, eSplitVolume);
            listTransFiles << strFileName;
        }
        listTransFiles = listTransFiles.toSet().toList();   // 处理重复文件，防止出现重复的分卷文件名

        if (listTransFiles.count() == 1) {
            // 单个压缩包，打开
            loadArchive(listTransFiles[0]);
        } else {
            // 多个压缩包，显示文件选择对话框，选择解压路径
            QFileInfo fileInfo(listTransFiles[0]);
            QString strDefaultPath;
            // 根据设置选项设置默认解压路径
            if ("" != m_pSettingDlg->getDefaultExtractPath()) {
                strDefaultPath = m_pSettingDlg->getDefaultExtractPath();
            } else {
                strDefaultPath = fileInfo.path();
            }
            // 弹出文件选择对话框，选择解压路径
            DFileDialog dialog(this);
            dialog.setAcceptMode(DFileDialog::AcceptOpen);
            dialog.setFileMode(DFileDialog::Directory);
            dialog.setWindowTitle(tr("Find directory"));
            dialog.setDirectory(strDefaultPath);
            // 处理操作
            int mode = dialog.exec();

            if (mode != QDialog::Accepted) { // 没有选择解压路径
                QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
                    emit sigquitApp();
                });
                return false;
            }

            // 获取解压路径
            QList< QUrl > listSelectpath = dialog.selectedUrls();
            QString strExtractPath = listSelectpath.at(0).toLocalFile();

            // 构建批量解压参数
            m_stUnCompressParameter.bBatch = true;
            m_stUnCompressParameter.listBatchFiles = listFiles;
            m_stUnCompressParameter.strExtractPath = strExtractPath;

            m_operationtype = Operation_Extract;
            // 调用批量解压
            if (ArchiveManager::get_instance()->batchExtractFiles(listTransFiles, strExtractPath/*, m_pSettingDlg->isAutoCreatDir()*/)) {
                qint64 qSize = 0;
                foreach (QString strFile, listTransFiles) {
                    qSize += QFile(strFile).size();
                }
                // 设置进度界面参数
                m_pProgressPage->setProgressType(PT_UnCompress);
                m_pProgressPage->setTotalSize(qSize);
                m_pProgressPage->setArchiveName(listTransFiles[0]);
                m_pProgressPage->restartTimer(); // 重启计时器
                m_ePageID = PI_UnCompressProgress;
            } else {
                // 无可用插件
                showErrorMessage(FI_Uncompress, EI_NoPlugin);
            }
        }
    } else if (strType == "extract_here") {
        m_eStartupType = StartupType::ST_ExtractHere;               // 启动类型
        // 解压到当前文件夹
        rightExtract2Path(m_eStartupType, listFiles/*, QFileInfo(listFiles[0]).path()*/);
    } else if (strType == "extract_to_specifypath") {
        m_eStartupType = StartupType::ST_Extractto;
        // 解压到xx文件夹（倒数第二个参数为解压路径）
        QString strPath = listFiles.last();     // 获取解压全路径
        QStringList listArchives = listFiles;
//        listArchives.removeLast();   // 获取所有压缩包全路径
        // 解压到指定路径
        rightExtract2Path(m_eStartupType, listArchives/*, strPath*/);
    }

    return true;
}

bool MainWindow::handleArguments_Append(const QStringList &listParam)
{
    if (listParam.size() < 3) {
        QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
            emit sigquitApp();
        });
        return false;
    }

    QString archiveName = listParam.first();
    //处理分卷包名称
    QString transFile = archiveName;
    UiTools::transSplitFileName(transFile, m_stUnCompressParameter.eSplitVolume);

    QStringList listFiles;
    for (int i = 1; i < listParam.size() - 1; ++i) {
        listFiles.push_back(listParam.at(i));
    }
    QStringList listSupportedMimeTypes = PluginManager::get_instance().supportedWriteMimeTypes(PluginManager::SortByComment);     // 获取支持的压缩格式
    CustomMimeType mimeType = determineMimeType(transFile);
    // 构建压缩包加载之后的数据
    m_stUnCompressParameter.strFullPath = archiveName;
    QFileInfo fileinfo(transFile);
    m_stUnCompressParameter.bCommentModifiable = (mimeType.name() == "application/zip") ? true : false;
    m_stUnCompressParameter.bMultiplePassword = false;
    m_stUnCompressParameter.bModifiable = (listSupportedMimeTypes.contains(mimeType.name()) && fileinfo.isWritable()
                                           && m_stUnCompressParameter.eSplitVolume == UnCompressParameter::ST_No); // 支持压缩且文件可写的非分卷格式才能修改数据
    if (!m_stUnCompressParameter.bModifiable) { // 不支持修改数据的压缩包进行提示
        TipDialog dialog(this);
        dialog.showDialog(tr("You cannot add files to archives in this file type"), tr("OK"), DDialog::ButtonNormal);
        QTimer::singleShot(100, this, [ = ] { // 发信号退出应用
            emit sigquitApp();
        });
        return false;
    }

    qInfo() << "向压缩包中添加文件";
    m_eStartupType = StartupType::ST_DragDropAdd;

    CompressOptions options;
    QList<FileEntry> listEntry;

    options.qTotalSize = fileinfo.size(); // 拖拽追加由于没有list，不能获取压缩包原文件总大小，libarchive使用压缩包大小代替计算进度
// 构建压缩文件数据
    foreach (QString strFile, listFiles) {
        FileEntry stFileEntry;
        stFileEntry.strFullPath = strFile;
        if (!QFileInfo(strFile).isDir()) {
            stFileEntry.qSize = QFileInfo(strFile).size(); // 原文件大小，供libarchive追加进度使用
        } else {
            stFileEntry.isDirectory = true;
        }

        listEntry.push_back(stFileEntry);
    }

// 切换进度界面
    m_pProgressPage->setProgressType(PT_CompressAdd);
    m_pProgressPage->setArchiveName(archiveName);
    m_operationtype = Operation_Add;
    m_ePageID = PI_AddCompressProgress;
    m_pProgressPage->setPushButtonCheckable(false, false);

// 计算大小
    m_mywork = new CalculateSizeThread(listFiles, transFile, listEntry, options/*, this*/);
    connect(m_mywork, &CalculateSizeThread::signalFinishCalculateSize, this, &MainWindow::slotFinishCalculateSize);
    m_mywork->start();

    return true;
}

void MainWindow::rightExtract2Path(StartupType eType, const QStringList &listFiles/*, const QString &strTargetPath*/)
{
    if (listFiles.count() == 0) {
        return;
    }

    QString strExtractPath;
    if (m_eStartupType == StartupType::ST_ExtractHere) {
        // 解压到当前文件夹时获取当前文件夹路径
        strExtractPath = QFileInfo(listFiles[0]).path();
    } else {
        // 解压到指定路径时根据压缩包数量判断
        if (listFiles.count() == 1) {
            strExtractPath = QFileInfo(listFiles[0]).path() + QDir::separator() + UiTools::handleFileName(listFiles[0]);
        } else {
            QFileInfo info(listFiles[0]);
            QString strpath = info.absolutePath();
            strExtractPath = info.path();
            int iIndex = strpath.lastIndexOf(QDir::separator());
            strExtractPath += QDir::separator() + strpath.mid(iIndex);
        }
    }

    // 对压缩包文件名进行处理
    QStringList listTransFiles;
    QMap<QString, UnCompressParameter::SplitType> mapType;
    for (int i = 0; i < listFiles.count(); ++i) {
        QString strFileName = listFiles[i];
        UnCompressParameter::SplitType eSplitVolume;
        UiTools::transSplitFileName(strFileName, eSplitVolume);
        listTransFiles << strFileName;
        mapType[strFileName] = eSplitVolume;
    }
    listTransFiles = listTransFiles.toSet().toList();   // 处理重复文件，防止出现重复的分卷文件名

    if (listTransFiles.count() == 1) {
        // 单个压缩包解压
        m_stUnCompressParameter.eSplitVolume = mapType[listTransFiles[0]];
        // 单个压缩包，解压到当前文件夹
        QFileInfo fileinfo(listTransFiles[0]);
        if (fileinfo.exists()) {
            ExtractionOptions options;
            // 构建解压参数
            options.strTargetPath = strExtractPath;
            options.bExistList = false;
            options.bAllExtract = true;
            options.qComressSize = fileinfo.size();
            options.qSize = fileinfo.size(); // 解压到当前文件夹由于没有list，不能获取压缩包原文件总大小，libarchive使用压缩包大小代替计算进度

            // 如果是右键解压到当前文件夹或者右键解压缩，根据设置选项判断是否自动创建文件夹，给出解压路径
            if (eType == ST_ExtractHere || eType == ST_Extract) {
                QString strAutoPath = getExtractPath(fileinfo.fileName());
                // 根据是否自动创建文件夹处理解压路径
                if (!strAutoPath.isEmpty()) {
                    options.strTargetPath += QDir::separator() + strAutoPath;
                }
            }

            m_stUnCompressParameter.strExtractPath = options.strTargetPath; // 解压路径
            m_stUnCompressParameter.strFullPath = listTransFiles[0];     // 压缩包全路径

            m_operationtype = Operation_Extract;

            // zip分卷指定使用cli7zplugin
            UiTools::AssignPluginType eType = (m_stUnCompressParameter.eSplitVolume == UnCompressParameter::ST_Zip) ?
                                              (UiTools::AssignPluginType::APT_Cli7z) : (UiTools::AssignPluginType::APT_Auto);

            // tar.7z特殊处理，右键解压缩到当前文件夹使用cli7zplugin
            if (listTransFiles[0].endsWith(QLatin1String(".tar.7z"))
                    && determineMimeType(listTransFiles[0]).name() == QLatin1String("application/x-7z-compressed")) {
                options.bTar_7z = true;
                eType = UiTools::AssignPluginType::APT_Cli7z;
            }

            // 调用解压函数-----------------------------------7z非001卷解压到当前文件夹需要使用处理后的文件名
            if (ArchiveManager::get_instance()->extractFiles(listTransFiles[0], QList<FileEntry>(), options, eType)) {
                // 设置进度界面参数
                m_pProgressPage->setProgressType(PT_UnCompress);
                m_pProgressPage->setTotalSize(options.qComressSize);
                m_pProgressPage->setArchiveName(listTransFiles[0]);
                m_pProgressPage->restartTimer(); // 重启计时器
                m_ePageID = PI_UnCompressProgress;
            }
        } else {
            // 可能分卷文件缺失，所以解压到当前文件夹失败
            m_ePageID = PI_Failure;
            showErrorMessage(FI_Uncompress, EI_ArchiveMissingVolume);
        }
    } else {
        // 批量解压（无论有没有设置自动创建文件夹，内部解压时都需要自动创建文件夹）
        // 多个压缩包解压到当前文件夹
        // 构建批量解压参数
        m_stUnCompressParameter.bBatch = true;
        m_stUnCompressParameter.listBatchFiles = listTransFiles;
        m_stUnCompressParameter.strExtractPath = strExtractPath;

        m_operationtype = Operation_Extract;
        if (ArchiveManager::get_instance()->batchExtractFiles(listTransFiles, m_stUnCompressParameter.strExtractPath/*, m_pSettingDlg->isAutoCreatDir()*/)) {
            qint64 qSize = 0;
            foreach (QString strFile, listTransFiles) {
                qSize += QFile(strFile).size();
            }
            // 设置进度界面参数
            m_pProgressPage->setProgressType(PT_UnCompress);
            m_pProgressPage->setTotalSize(qSize);
            m_pProgressPage->setArchiveName(listTransFiles[0]);
            m_pProgressPage->restartTimer(); // 重启计时器
            m_ePageID = PI_UnCompressProgress;
        } else {
            // 无可用插件
            showErrorMessage(FI_Uncompress, EI_NoPlugin);
        }
    }
}

void MainWindow::slotExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions)
{
    qInfo() << "提取文件至:" << stOptions.strTargetPath;
    m_stUnCompressParameter.strExtractPath = stOptions.strTargetPath;     // 存储提取路径
    m_operationtype = Operation_SingleExtract; //提取操作
    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();

    // 存储提取之后的文件
    m_stUnCompressParameter.listExractFiles.clear();
    for (int i = 0; i < listSelEntry.count(); ++i) {
        QString strFullPath = listSelEntry[i].strFullPath;
        m_stUnCompressParameter.listExractFiles << stOptions.strTargetPath + QDir::separator() + strFullPath.remove(0, stOptions.strDestination.size());
    }

    // 提取删除操作使用小弹窗进度
    m_pProgressdialog->clearprocess();
    m_pProgressdialog->setCurrentTask(strArchiveFullPath);

    if (!ArchiveManager::get_instance()->extractFiles2Path(strArchiveFullPath, listSelEntry, stOptions)) {
        // 无可用插件
        showErrorMessage(FI_Uncompress, EI_NoPlugin);
    }
}

void MainWindow::slotDelFiles(const QList<FileEntry> &listSelEntry, qint64 qTotalSize)
{
    qInfo() << "删除文件:";
    m_operationtype = Operation_DELETE; //提取操作
    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();
    if (ArchiveManager::get_instance()->deleteFiles(strArchiveFullPath, listSelEntry)) {
        // 设置更新选项
        m_stUpdateOptions.reset();
        m_stUpdateOptions.eType = UpdateOptions::Delete;
        m_stUpdateOptions.listEntry << listSelEntry;
        m_stUpdateOptions.qSize = qTotalSize;

        // 设置进度界面参数
        m_pProgressPage->setProgressType(PT_Delete);
        m_pProgressPage->setTotalSize(qTotalSize);
        m_pProgressPage->setArchiveName(strArchiveFullPath);
        m_pProgressPage->restartTimer(); // 重启计时器
        m_ePageID = PI_DeleteProgress;
        refreshPage();
    } else {
        // 无可用插件
        showErrorMessage(FI_Delete, EI_NoPlugin);
    }
}

void MainWindow::slotReceiveCurArchiveName(const QString &strArchiveName)
{
    m_pProgressPage->setArchiveName(strArchiveName);
}

void MainWindow::slotOpenFile(const FileEntry &entry, const QString &strProgram)
{
    m_operationtype = Operation_TempExtract_Open;
    // 设置解压临时路径
    QString strArchiveFullPath = m_pUnCompressPage->archiveFullPath();
    QString strTempExtractPath = TEMPPATH + QDir::separator() + m_strProcessID + createUUID();  // 拼接临时路径
    QString strOpenFile =  strTempExtractPath + QDir::separator() + entry.strFileName;     // 临时解压文件全路径
    m_pOpenFileWatcher->setCurOpenFile(strOpenFile);
    if (ArchiveManager::get_instance()->openFile(strArchiveFullPath, entry, strTempExtractPath, strProgram)) {
        // 进入打开加载界面
        m_pLoadingPage->setDes(tr("Loading, please wait..."));
        m_pLoadingPage->startLoading();     // 开始加载
        m_ePageID = PI_Loading;
        refreshPage();
    } else {
        // 无可用插件
        showErrorMessage(FI_Uncompress, EI_NoPlugin);
    }
}

void MainWindow::slotOpenFileChanged(const QString &strPath)
{
    QMap<QString, bool> &mapStatus = m_pOpenFileWatcher->getFileHasModified();
    QMap<QString, QString> mapPassword = m_pOpenFileWatcher->getFilePassword();

    if ((mapStatus.find(strPath) != mapStatus.end()) && (!mapStatus[strPath])) {

        // 对重命名或删除导致的文件不存在的情况，不给出任何提示，且从文件监控中去掉此文件
        if (!QFile::exists(strPath)) {
            mapStatus.remove(strPath);
            mapPassword.remove(strPath);
            return;
        }

        mapStatus[strPath] = true;

        QFileInfo file(strPath);
        QString strDesText = QObject::tr("%1 changed. Do you want to save changes to the archive?").arg(UiTools::toShortString(file.fileName()));

        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(strDesText, tr("Cancel"), DDialog::ButtonNormal, tr("Update"), DDialog::ButtonRecommend);
        if (iResult == 1) {
            if (!m_stUnCompressParameter.bModifiable) { // 不支持修改文件的压缩包
                ConvertDialog dialogConvert(this); // 询问是否进行格式转换
                QStringList ret = dialogConvert.showDialog();

                if (ret.at(0) == "true") { // 进行格式转换
                    convertArchive(ret.at(1));
                }

                return;
            }

            // 更新压缩包数据
            addFiles2Archive(QStringList() << strPath, mapPassword[strPath]);
        }

        mapStatus[strPath] = false;

        // 这里需要再次添加文件监控，因为某些应用修改文件是先删除再创建，所以需要再次监听
        m_pOpenFileWatcher->addWatchFile(strPath);
    }
}

void MainWindow::slotPause()
{
    ArchiveManager::get_instance()->pauseOperation();
}

void MainWindow::slotContinue()
{
    ArchiveManager::get_instance()->continueOperation();
}

void MainWindow::slotCancel()
{
    ArchiveManager::get_instance()->cancelOperation();
}

void MainWindow::slotAddFiles(const QStringList &listFiles, const QString &strPassword)
{
    // 向压缩包中添加文件
    addFiles2Archive(listFiles, strPassword);
}

void MainWindow::slotSuccessView()
{
    if (m_pSuccessPage->getSuccessType() == SI_Convert) { // 格式转换点击查看文件重新加载转换后的压缩文件
        loadArchive(m_strFinalConvertFile);
    } else {
        // 若文件服务线程不为空，查看相应的文件
        if (m_pDDesktopServicesThread && m_pDDesktopServicesThread->hasFiles())
            m_pDDesktopServicesThread->start();
    }
}

void MainWindow::slotSuccessReturn()
{
    switch (m_pSuccessPage->getSuccessType()) {
    // 压缩成功
    case SI_Compress: {
        m_pCompressPage->clear();   // 清空压缩界面
    }
    break;
    // 解压成功
    case SI_UnCompress: {
        m_pUnCompressPage->clear(); // 清空解压界面
    }
    break;
    // 转换成功
    case SI_Convert: {
        m_pUnCompressPage->clear(); // 清空解压界面
    }
    break;
//    default:
//        break;
    }

    resetMainwindow();
    m_ePageID = PI_Home;
    refreshPage();
}

void MainWindow::slotFailureRetry()
{
    switch (m_pFailurePage->getFailureInfo()) {
    case FI_Compress: {
        m_ePageID = PI_CompressSetting;  // 返回到列表设置界面
    }
    break;
    case FI_Uncompress: {
        if (StartupType::ST_ExtractHere == m_eStartupType/* || StartupType::SST_Extractto == m_eStartupType*/) {
            // 右键解压到当前文件夹
        } else {
            // 普通加载解压
            m_ePageID = PI_UnCompress;  // 返回到列表界面
        }
    }
    break;
    case FI_Convert: {
        m_ePageID = PI_UnCompress;  // 返回到列表界面
    }
    break;
    default:
        break;
    }

    refreshPage();
}

void MainWindow::slotFailureReturn()
{
    switch (m_pFailurePage->getFailureInfo()) {
    case FI_Compress: {
        m_pCompressPage->clear();   // 清空压缩界面
    }
    break;
    case FI_Uncompress: {
        m_pUnCompressPage->clear(); // 清空解压界面
    }
    break;
    case FI_Convert: {
        m_pUnCompressPage->clear(); // 清空解压界面
    }
    break;
    default:
        break;
    }

    resetMainwindow();
    m_ePageID = PI_Home;
    refreshPage();
}

void MainWindow::slotTitleCommentButtonPressed()
{
    //    qInfo() << __FUNCTION__;
    // 文件名
    QFileInfo file(m_stUnCompressParameter.strFullPath);

    if (m_isFirstViewComment) {
        // 记录修改前压缩包本身的注释
        m_comment = DataManager::get_instance().archiveData().strComment;
    }

    if (m_ePageID == PI_UnCompress) {
        DDialog *dialog = new DDialog(this);
        dialog->setWindowTitle(tr("File info"));
        dialog->setFixedWidth(300);
        DFontSizeManager::instance()->bind(dialog, DFontSizeManager::T6, QFont::Medium);

        // 整体布局
        QVBoxLayout *mainLayout = new QVBoxLayout;

        // 控件标题的字体
        QFont titleFont;
        titleFont.setWeight(QFont::Medium);

        // 压缩包具体信息内容的字体
        QFont infoFont;
        infoFont.setPixelSize(12);
        infoFont.setWeight(QFont::Normal);

        QList<DArrowLineDrawer *> expandGroup;

        // 基本信息控件的布局
        QVBoxLayout *basicInfoLayout = new QVBoxLayout;

        // 基本信息控件
        DArrowLineDrawer *basicInfoDrawer = new DArrowLineDrawer;
        basicInfoDrawer->setFont(titleFont);
        basicInfoDrawer->setTitle(tr("Basic info"));
        basicInfoDrawer->setFixedHeight(30);
        basicInfoDrawer->setExpand(true);
        basicInfoDrawer->setSeparatorVisible(false);  // 无效！
        basicInfoDrawer->setExpandedSeparatorVisible(false);
        expandGroup.append(basicInfoDrawer);

        // 基本信息Frame
        DFrame *basicInforFrame = new DFrame;

        // 基本信息具体内容布局
        QFormLayout *basicInfoFormLayout = new QFormLayout;
        basicInfoFormLayout->setHorizontalSpacing(35);
        basicInfoFormLayout->setVerticalSpacing(7);
        basicInfoFormLayout->setLabelAlignment(Qt::AlignLeft);

        DLabel *left1 = new DLabel(tr("Size"));
        DLabel *left2 = new DLabel(tr("Type"));
        DLabel *left3 = new DLabel(tr("Location"));
        DLabel *left4 = new DLabel(tr("Time created"));
        DLabel *left5 = new DLabel(tr("Time accessed"));
        DLabel *left6 = new DLabel(tr("Time modified"));

        left1->setFont(infoFont);
        left2->setFont(infoFont);
        left3->setFont(infoFont);
        left4->setFont(infoFont);
        left5->setFont(infoFont);
        left6->setFont(infoFont);

        DLabel *right1 = new DLabel(UiTools::humanReadableSize(file.size(), 1));
        DLabel *right2 = new DLabel(tr("Archive"));
        DLabel *right3 = new DLabel(file.filePath());
        QString str2 = fontMetrics().elidedText(file.filePath(), Qt::ElideMiddle, 150);
        right3->setText(str2);
        right3->setToolTip(file.filePath());
        DLabel *right4 = new DLabel(file.created().toString("yyyy/MM/dd hh:mm:ss"));
        DLabel *right5 = new DLabel(file.lastRead().toString("yyyy/MM/dd hh:mm:ss"));
        DLabel *right6 = new DLabel(file.lastModified().toString("yyyy/MM/dd hh:mm:ss"));

        right1->setFont(infoFont);
        right2->setFont(infoFont);
        right3->setFont(infoFont);
        right4->setFont(infoFont);
        right5->setFont(infoFont);
        right6->setFont(infoFont);

        basicInfoFormLayout->addRow(left1, right1);
        basicInfoFormLayout->addRow(left2, right2);
        basicInfoFormLayout->addRow(left3, right3);
        basicInfoFormLayout->addRow(left4, right4);
        basicInfoFormLayout->addRow(left5, right5);
        basicInfoFormLayout->addRow(left6, right6);

        basicInforFrame->setLayout(basicInfoFormLayout);
        basicInfoDrawer->setContent(basicInforFrame);
        basicInfoLayout->setContentsMargins(0, 0, 0, 0);
        basicInfoLayout->addWidget(basicInfoDrawer, Qt::AlignTop);

        // 注释的控件
        DArrowLineDrawer *commentDrawer = new DArrowLineDrawer;
        commentDrawer->setFont(titleFont);
        commentDrawer->setTitle(tr("Comment"));
        commentDrawer->setFixedHeight(30);
        commentDrawer->setExpand(true);
        commentDrawer->setSeparatorVisible(false);  // 无效！
        commentDrawer->setExpandedSeparatorVisible(false);
        expandGroup.append(commentDrawer);

        // 注释Frame布局
        DFrame *commentFrame = new DFrame;

        // 显示注释内容的布局
        QVBoxLayout *texteditLayout = new QVBoxLayout;

        // 显示注释内容的控件
        DTextEdit *commentTextedit = new DTextEdit(commentDrawer);
        commentTextedit->setTabChangesFocus(true); //使用tab按键切换焦点功能
        bool isReadOnly = false;
        if (m_stUnCompressParameter.bCommentModifiable) { // 只有zip格式支持修改注释(注:zip分卷也不支持修改注释)
            if (m_stUnCompressParameter.strFullPath.endsWith(".zip")) {
                /**
                 * 例如123.zip文件，检测123.z01文件是否存在
                 * 如果存在，则认定123.zip是分卷包
                 */
                QFileInfo tmp(m_stUnCompressParameter.strFullPath.left(m_stUnCompressParameter.strFullPath.length() - 2) + "01");
                if (tmp.exists()) {
                    isReadOnly = true;
                }
            } else if (m_stUnCompressParameter.strFullPath.endsWith(".zip.001")) {
                isReadOnly = true;
            } else {
                isReadOnly = false;
            }
        } else {
            isReadOnly = true;
        }

        commentTextedit->setReadOnly(isReadOnly);

        commentTextedit->setPlaceholderText(tr("Enter up to %1 characters").arg(MAXCOMMENTLEN)); // 注释内容不得超过MAXCOMMENTLEN字符
        commentTextedit->setFixedHeight(80);
        commentTextedit->setText(m_comment);
        commentTextedit->setFont(infoFont);

        QString newComment = m_comment;
        connect(commentTextedit, &DTextEdit::textChanged, this, [ & ] {
            newComment = commentTextedit->toPlainText();

            if (newComment.size() > MAXCOMMENTLEN)   //限制最多注释MAXCOMMENTLEN个字
            {
                // 保留前MAXCOMMENTLEN个注释字符
                commentTextedit->setText(newComment.left(MAXCOMMENTLEN));

                //设定鼠标位置，将鼠标放到最后的地方
                QTextCursor cursor = commentTextedit->textCursor();
                cursor.setPosition(MAXCOMMENTLEN);
                commentTextedit->setTextCursor(cursor);
            }
        });

        texteditLayout->setContentsMargins(10, 5, 10, 16);
        texteditLayout->addWidget(commentTextedit, Qt::AlignCenter);

        commentFrame->setLayout(texteditLayout);
        commentDrawer->setContent(commentFrame);

        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->addLayout(basicInfoLayout);
        mainLayout->addWidget(commentDrawer, 0,  Qt::AlignTop);

        DWidget *widget = new DWidget;
        widget->setLayout(mainLayout);

        dialog->addContent(widget);
        dialog->move(this->geometry().topLeft().x() + this->width() / 2  - dialog->width() / 2,
                     this->geometry().topLeft().y() + this->height() / 2 - 200); // 200是dialog展开的高度的一半

        // DArrowLineDrawer收缩展开设置大小
        DEnhancedWidget *basicInfoWidget = new DEnhancedWidget(basicInfoDrawer, basicInfoDrawer);
        connect(basicInfoWidget, &DEnhancedWidget::heightChanged, basicInfoWidget, [ = ]() {
            QRect rc = dialog->geometry();
            rc.setHeight(basicInfoDrawer->height() + commentDrawer->height()
                         + widget->contentsMargins().top() + widget->contentsMargins().bottom());
            dialog->setGeometry(rc);
        });

        // comment布局加margin
        DEnhancedWidget *commentWidget = new DEnhancedWidget(commentDrawer, commentDrawer);
        connect(commentWidget, &DEnhancedWidget::heightChanged, commentWidget, [ = ]() {
            QRect rc = dialog->geometry();
            rc.setHeight(basicInfoDrawer->height() + commentDrawer->height() + 70
                         + dialog->contentsMargins().top() + dialog->contentsMargins().bottom());
            dialog->setGeometry(rc);
        });

        int mode = dialog->exec();
        delete dialog;
        if (mode == -1 && m_comment != newComment) {
            m_isFirstViewComment = false;
            m_comment = newComment;
            updateArchiveComment();
        }
    }
}

void MainWindow::slotThemeChanged()
{
    QIcon icon;
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {    // 浅色
        icon.addFile(":assets/icons/deepin/builtin/icons/compress_information_15px.svg");
    } else if (themeType == DGuiApplicationHelper::DarkType) {  // 深色
        icon.addFile(":assets/icons/deepin/builtin/icons/compress_information_dark.svg");
    } else {        // 其它默认
        icon.addFile(":assets/icons/deepin/builtin/icons/compress_information_15px.svg");
    }

    m_pTitleCommentButton->setIcon(icon);
    m_pTitleCommentButton->setIconSize(QSize(15, 15));
}

void MainWindow::slotShowShortcutTip()
{
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
}

void MainWindow::slotFinishCalculateSize(qint64 size, QString strArchiveFullPath, QList<FileEntry> listAddEntry, CompressOptions stOptions, QList<FileEntry> listEntry)
{
    // 调用添加文件接口
    if (ArchiveManager::get_instance()->addFiles(strArchiveFullPath, listAddEntry, stOptions)) {
        // 切换进度界面
        m_pProgressPage->setTotalSize(size);
        m_pProgressPage->setPushButtonCheckable(true, true);
        m_pProgressPage->setProgressType(PT_CompressAdd);
        m_pProgressPage->setArchiveName(m_stUnCompressParameter.strFullPath);
        m_pProgressPage->restartTimer();

        // 设置更新选项
        m_stUpdateOptions.reset();
        m_stUpdateOptions.qSize = size;
        m_stUpdateOptions.listEntry = listEntry;
        m_stUpdateOptions.strParentPath = stOptions.strDestination;
        m_stUpdateOptions.eType = UpdateOptions::Add;
    } else {
        // 无可用插件
        showErrorMessage(FI_Add, EI_NoPlugin);
    }

    if (nullptr != m_mywork) {
        m_mywork->deleteLater();
        m_mywork = nullptr;
    }
}
