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

#include <DFileDialog>
#include <DTitlebar>
#include <DWindowCloseButton>
#include <DWindowOptionButton>
#include <DArrowLineDrawer>
#include <DFontSizeManager>
#include <denhancedwidget.h>

#include <QStackedWidget>
#include <QKeyEvent>
#include <QSettings>
#include <QDebug>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>
#include <QScreen>
#include <QFormLayout>

static QMutex mutex;

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    m_strProcessID = QString::number(QCoreApplication::applicationPid());   // 获取应用进程号

    setWindowTitle(tr("Archive Manager"));

    // 先构建初始界面
    m_pMainWidget = new QStackedWidget(this);  // 中心面板
    m_pHomePage = new HomePage(this);            // 首页
    m_pMainWidget->addWidget(m_pHomePage);
    setCentralWidget(m_pMainWidget);    // 设置中心面板
    m_pMainWidget->setCurrentIndex(0);

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

    // 清除缓存数据
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-rf");
    args.append(TEMPPATH + QDir::separator() + m_strProcessID);
    p.execute(command, args);
    p.waitForFinished();

    qDebug() << "应用正常退出";
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

    // 左上角注释信息
    m_pTitleCommentButton = new DIconButton(this);
    m_pTitleCommentButton->setFixedSize(36, 36);
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
    connect(m_pCompressSettingPage, &CompressSettingPage::signalCompressClicked, this, &MainWindow::slotCompress);
    connect(m_pUnCompressPage, &UnCompressPage::signalUncompress, this, &MainWindow::slotUncompressClicked);
    connect(m_pUnCompressPage, &UnCompressPage::signalExtract2Path, this, &MainWindow::slotExtract2Path);
    connect(m_pUnCompressPage, &UnCompressPage::signalDelFiles, this, &MainWindow::slotDelFiles);
    connect(m_pUnCompressPage, &UnCompressPage::signalOpenFile, this, &MainWindow::slotOpenFile);
    connect(m_pUnCompressPage, &UnCompressPage::signalAddFiles2Archive, this, &MainWindow::slotAddFiles);
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
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalCurArchiveName, this, &MainWindow::slotReceiveCurArchiveName);
    connect(ArchiveManager::get_instance(), &ArchiveManager::signalQuery, this, &MainWindow::slotQuery);

    connect(m_pOpenFileWatcher, &OpenFileWatcher::fileChanged, this, &MainWindow::slotOpenFileChanged);
}

void MainWindow::refreshPage()
{
    switch (m_ePageID) {
    case PI_Home: {
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
        titlebar()->setTitle(tr("Adding files to %1").arg(QFileInfo(m_pUnCompressPage->archiveFullPath()).fileName()));
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
        titlebar()->setTitle(tr("Commenting"));
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

    return m_stCompressParameter.qSize;
}

void MainWindow::calFileSizeByThread(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return;
    // 获得文件夹中的文件列表
    QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);

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
    m_pTitleButton->setVisible(bVisible);

    if (bVisible)
        m_pTitleButton->setIcon(pixmap);

    m_pTitleCommentButton->setVisible(bVisible2);
}

void MainWindow::loadArchive(const QString &strArchiveFullPath)
{
    PERF_PRINT_BEGIN("POINT-05", "加载时间");
    m_operationtype = Operation_Load;

    //处理分卷包名称
    QString transFile = strArchiveFullPath;
    QStringList listSupportedMimeTypes = PluginManager::get_instance().supportedWriteMimeTypes(PluginManager::SortByComment);     // 获取支持的压缩格式
    QMimeType mimeType = determineMimeType(transFile);

    // 构建压缩包加载之后的数据
    m_stUnCompressParameter.strFullPath = strArchiveFullPath;
    transSplitFileName(transFile, m_stUnCompressParameter);
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
        // 无可用插件
        showErrorMessage(FI_Load, EI_NoPlugin);
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

                resetMainwindow();
                m_ePageID = PI_Home;
                refreshPage();
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
    QString fileMime;

    bool existMime = false; // 在设置界面是否被勾选
    bool bArchive = false; // 是否是应用支持解压的格式
    bool mimeIsChecked = true; // 默认该格式被勾选

    // 判断内容
    if (file.isEmpty()) {
        existMime = true;
    } else {
        fileMime = determineMimeType(file).name();

        if (fileMime.contains("application/"))
            fileMime = fileMime.remove("application/");

        if (fileMime.size() > 0) {
            existMime = UiTools::isExistMimeType(fileMime, bArchive);

            // 如果在设置界面找到非压缩包的类型，置为true
            if (!bArchive && !existMime) // ？
                existMime = true;
        } else {
            existMime = false;
        }
    }

    if (existMime) { // 已经是关联解压类型
        QString defaultCompress = getDefaultApp(fileMime); // 获取该类型文件的默认打开方式

        if (defaultCompress.startsWith("dde-open.desktop")) {
            // 如果默认打开方式不是归档管理器， 设置归档管理器我为默认打开方式
            setDefaultApp(fileMime, "deepin-compressor.desktop");
        }
    } else { // 不是关联解压类型
        // 如果不是归档管理器支持的压缩文件格式，设置默认打开方式为选择默认打开程序对话框
        QString defaultCompress = getDefaultApp(fileMime);
        if (defaultCompress.startsWith("deepin-compressor.desktop")) {
            setDefaultApp(fileMime, "dde-open.desktop");
        }

        TipDialog dialog;
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenRect =  screen->availableVirtualGeometry();
        dialog.move(((screenRect.width() / 2) - (dialog.width() / 2)), ((screenRect.height() / 2) - (dialog.height() / 2)));

        int re = dialog.showDialog(tr("Please check the file association type in the settings of Archive Manager"), tr("OK"), DDialog::ButtonNormal);
        if (re != 1) { // ？
            mimeIsChecked = false;
        }
    }

    return mimeIsChecked;
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

void MainWindow::slotHandleRightMenuSelected(const QStringList &listParam)
{
    qDebug() << listParam;
    if (!m_initFlag) {
        // 初始化界面
        qDebug() << "初始化界面";
        initUI();
        initConnections();
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
        // 设置界面标识为压缩设置界面
        m_ePageID = PI_CompressSetting;
    } else if (strType == QStringLiteral("extract_here")) {
        // 解压单个文件到当前文件夹
        QString filepath = listParam.at(0);
        m_stUnCompressParameter.bRightOperation = true;
        transSplitFileName(filepath, m_stUnCompressParameter);
        QFileInfo fileinfo(filepath);
        if (fileinfo.exists()) {
            // QFileInfo fileinfo(listParam.at(0));
            ExtractionOptions options;
            // 构建解压参数
            options.strTargetPath = fileinfo.path();
            options.bAllExtract = true;
            options.bRightExtract = true;
            options.qComressSize = fileinfo.size();
            options.qSize = fileinfo.size(); // 解压到当前文件夹由于没有list，不能获取压缩包原文件总大小，libarchive使用压缩包大小代替计算进度
            QString strAutoPath = getExtractPath(fileinfo.fileName());
            // 根据是否自动创建文件夹处理解压路径
            if (!strAutoPath.isEmpty()) {
                options.strTargetPath += QDir::separator() + strAutoPath;
            }

            m_stUnCompressParameter.strExtractPath = options.strTargetPath; // 解压路径
            m_stUnCompressParameter.strFullPath = filepath;     // 压缩包全路径

            m_operationtype = Operation_Extract;

            // zip分卷指定使用cli7zplugin
            UiTools::AssignPluginType eType = (m_stUnCompressParameter.eSplitVolume == UnCompressParameter::ST_Zip) ?
                                              (UiTools::AssignPluginType::APT_Cli7z) : (UiTools::AssignPluginType::APT_Auto);

            // 调用解压函数
            if (ArchiveManager::get_instance()->extractFiles(listParam.at(0), QList<FileEntry>(), options, eType)) {
                // 设置进度界面参数
                m_pProgressPage->setProgressType(PT_UnCompress);
                m_pProgressPage->setTotalSize(options.qComressSize);
                m_pProgressPage->setArchiveName(fileinfo.fileName());
                m_pProgressPage->restartTimer(); // 重启计时器
                m_ePageID = PI_UnCompressProgress;
            } else {
                // 无可用插件
                showErrorMessage(FI_Uncompress, EI_NoPlugin);
            }
        } else {
            // 可能分卷文件缺失，所以解压到当前文件夹失败
            m_ePageID = PI_Failure;
            showErrorMessage(FI_Uncompress, EI_ArchiveMissingVolume);
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

        m_operationtype = Operation_Extract;
        // 调用批量解压
        if (ArchiveManager::get_instance()->batchExtractFiles(listFiles, strExtractPath, m_pSettingDlg->isAutoCreatDir())) {
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
        } else {
            // 无可用插件
            showErrorMessage(FI_Uncompress, EI_NoPlugin);
        }
    } else if (strType == QStringLiteral("extract_here_multi")) {
        // 批量解压到当前文件夹
        m_stUnCompressParameter.bRightOperation = true;
        // 处理选中文件
        QStringList listFiles = listParam;
        listFiles.removeLast();

        m_operationtype = Operation_Extract;
        if (ArchiveManager::get_instance()->batchExtractFiles(listFiles, QFileInfo(listFiles[0]).path(), m_pSettingDlg->isAutoCreatDir())) {
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
        } else {
            // 无可用插件
            showErrorMessage(FI_Uncompress, EI_NoPlugin);
        }
    } else if (strType == QStringLiteral("compress_to_zip")) {
        // 压缩成xx.zip
    } else if (strType == QStringLiteral("compress_to_7z")) {
        // 压缩成xx.7z
    } else if (strType == QStringLiteral("extract_mkdir")) {
        // 解压到xx文件夹
    } else if (strType == QStringLiteral("extract_here_split")) { // 右键选择7z压缩分卷文件，解压到当前文件夹
        QString filepath = listParam.at(0);
        m_stUnCompressParameter.bRightOperation = true;
        transSplitFileName(filepath, m_stUnCompressParameter);

        QFileInfo fileinfo(filepath);
        if (fileinfo.exists()) {
            ExtractionOptions options;
            // 构建解压参数
            options.strTargetPath = fileinfo.path();
            options.bAllExtract = true;
            options.bRightExtract = true;
            options.qComressSize = fileinfo.size();

            m_operationtype = Operation_Extract;
            // 调用解压函数
            if (ArchiveManager::get_instance()->extractFiles(filepath, QList<FileEntry>(), options)) {
                // 设置进度界面参数
                m_pProgressPage->setProgressType(PT_UnCompress);
                m_pProgressPage->setTotalSize(options.qComressSize);
                m_pProgressPage->setArchiveName(fileinfo.fileName());
                m_pProgressPage->restartTimer(); // 重启计时器
                m_ePageID = PI_UnCompressProgress;
            } else {
                // 无可用插件
                showErrorMessage(FI_Uncompress, EI_NoPlugin);
            }
        } else {
            // 可能分卷文件缺失，所以解压到当前文件夹失败
            m_ePageID = PI_Failure;
            showErrorMessage(FI_Uncompress, EI_ArchiveMissingVolume);
        }
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
    m_operationtype = Operation_Create;

    m_stCompressParameter = val.value<CompressParameter>();    // 获取压缩参数
    QStringList listFiles = m_pCompressPage->compressFiles();   // 获取待压缩文件

    if (listFiles.count() == 0) {
        qDebug() << "没有需要压缩的文件";
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
    } else if (bUseLibarchive == true) {
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
    qDebug() << "操作类型：" << eJobType << "****结束类型：" << eFinishType << "****错误类型" << eErrorType;

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
    refreshPage();

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
        m_pProgressPage->setArchiveName(QFileInfo(strArchiveFullPath).fileName());
        m_pProgressPage->restartTimer(); // 重启计时器
        m_ePageID = PI_UnCompressProgress;

        refreshPage();
    } else {
        // 无可用插件
        showErrorMessage(FI_Uncompress, EI_NoPlugin);
    }
}

void MainWindow::slotReceiveProgress(double dPercentage)
{
    if (Operation_SingleExtract == m_operationtype) { //提取删除操作使用小弹窗进度
        //需要添加dPercentage < 100判断，否则会出现小文件提取进度对话框不会自动关闭
        if (m_pProgressdialog->isHidden() && dPercentage < 100 && dPercentage > 0) {
            m_pProgressdialog->exec();
        }

        m_pProgressdialog->setProcess(qRound(dPercentage));
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

void MainWindow::slotQuery(Query *query)
{
    qDebug() << " query->execute()";
    query->setParent(this);
    query->execute();
}

void MainWindow::Extract2PathFinish(QString msg)
{
    QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
    sendMessage(icon, msg);

    // 设置了自动打开文件夹处理流程
    if (m_pSettingDlg->isAutoOpen()) {
        if (m_pDDesktopServicesThread == nullptr) {
            m_pDDesktopServicesThread = new DDesktopServicesThread(this);
        }

        // 打开选中第一个提取的文件/文件夹
        if (m_stUnCompressParameter.listExractFiles.count() > 0)
            m_pDDesktopServicesThread->setOpenFile(m_stUnCompressParameter.listExractFiles[0]);
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

void MainWindow::transSplitFileName(QString &fileName, UnCompressParameter &unCompressPar)
{
    if (fileName.contains(".7z.")) {
        // 7z分卷处理
        QRegExp reg("^([\\s\\S]*.)[0-9]{3}$"); // QRegExp reg("[*.]part\\d+.rar$"); //rar分卷不匹配

        if (reg.exactMatch(fileName)) {
            fileName = reg.cap(1) + "001"; //例如: *.7z.003 -> *.7z.001
            unCompressPar.eSplitVolume = UnCompressParameter::ST_Other;
        }
    } else if (fileName.contains(".part") && fileName.endsWith(".rar")) {
        // rar分卷处理
        int x = fileName.lastIndexOf("part");
        int y = fileName.lastIndexOf(".");

        if ((y - x) > 5) {
            fileName.replace(x, y - x, "part01");
        } else {
            fileName.replace(x, y - x, "part1");
        }

        unCompressPar.eSplitVolume = UnCompressParameter::ST_Other;
    } else if (fileName.contains(".zip.")) { // 1.zip.001格式
        QRegExp reg("^([\\s\\S]*.)[0-9]{3}$");
        if (reg.exactMatch(fileName)) {
            QFileInfo fi(reg.cap(1) + "001");
            if (fi.exists() == true) {
                fileName = reg.cap(1) + "001";
                unCompressPar.eSplitVolume = UnCompressParameter::ST_Zip;
            }
        }
    } else if (fileName.endsWith(".zip")) { //1.zip 1.01格式
        /**
         * 例如123.zip文件，检测123.z01文件是否存在
         * 如果存在，则认定123.zip是分卷包
         */
        QFileInfo tmp(fileName.left(fileName.length() - 2) + "01");
        if (tmp.exists()) {
            unCompressPar.eSplitVolume = UnCompressParameter::ST_Zip;
        }
    }
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
        m_pDDesktopServicesThread->setOpenFile(m_stCompressParameter.strTargetPath + QDir::separator() + m_stCompressParameter.strArchiveName);

        // zip压缩包添加注释
        addArchiveComment();
    }
    break;
    // 添加文件至压缩包
    case ArchiveJob::JT_Add: {
        qDebug() << "添加结束";

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
            Extract2PathFinish(tr("SingleExtraction successful")); //提取成功
            m_pProgressdialog->setFinished();
        } else {
            qDebug() << "解压结束";
            m_ePageID = PI_Success;
            showSuccessInfo(SI_UnCompress);   // 解压成功

            // 初始化服务
            if (m_pDDesktopServicesThread == nullptr) {
                m_pDDesktopServicesThread = new DDesktopServicesThread(this);
            }

            // 构建需要查看的本地文件
            if (m_pSettingDlg->isAutoCreatDir()) {
                // 若设置了自动创建文件夹,显示解压路径
                m_pDDesktopServicesThread->setOpenFile(m_stUnCompressParameter.strExtractPath);
            } else {
                // 否则显示解压第一个文件/文件夹所在目录
                ArchiveData stArchiveData = DataManager::get_instance().archiveData();
                if (stArchiveData.listRootEntry.count() > 0)
                    m_pDDesktopServicesThread->setOpenFile(m_stUnCompressParameter.strExtractPath + QDir::separator() + stArchiveData.listRootEntry[0].strFullPath);
            }

            // 设置了自动打开文件夹处理流程
            if (m_pSettingDlg->isAutoOpen()) {
                m_pDDesktopServicesThread->start();
            }

            // 设置了压缩完成自动删除原压缩包
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
        }
    }
    break;
    // 删除
    case ArchiveJob::JT_Delete: {
        qDebug() << "删除结束";
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
        qDebug() << "打开结束";
        // 若压缩包文件可更改，打开文件之后对文件进行监控
        // 非分卷的rar可以进行格式转换
        if ((m_stUnCompressParameter.bModifiable) ||
                ((m_stUnCompressParameter.eSplitVolume == UnCompressParameter::ST_No)
                 && (determineMimeType(m_stUnCompressParameter.strFullPath).name() == "application/vnd.rar"))) {
            // 打开成功之后添加当前打开文件至文件监控中
            m_pOpenFileWatcher->addCurOpenWatchFile();
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
        qDebug() << "更新结束";
        m_pLoadingPage->stopLoading();      // 停止更新
        m_ePageID = PI_UnCompress;
        m_pUnCompressPage->refreshDataByCurrentPathChanged();
    }
    break;
    // 更新压缩包注释
    case ArchiveJob::JT_Comment: {
        if (Operation_Update_Comment == m_operationtype) {
            qDebug() << "更新注释结束";
            m_commentProgressDialog->setFinished();
        } else { /* if (Operation_Add_Comment == m_operationtype)*/ // creatjob结束的时候工作类型已经置为Operation_NULL
            qDebug() << "添加zip注释结束";
            m_ePageID = PI_Success;
            showSuccessInfo(SI_Compress);   // 显示压缩成功
        }
    }
    break;
//    default:
//        break;
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
    case ArchiveJob::JT_Load: {
        showErrorMessage(FI_Load, EI_ArchiveOpenFailed);
    }
    break;
    // 批量解压
    case ArchiveJob::JT_BatchExtract:
    // 解压
    case ArchiveJob::JT_Extract: {
        if (Archive_OperationType::Operation_SingleExtract == m_operationtype) {
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_30px.svg", QSize(30, 30));
            sendMessage(icon, tr("User Cancel"));
        } else {
            if (m_stUnCompressParameter.bRightOperation) {
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
        showErrorMessage(FI_Compress, EI_CreatArchiveFailed, true);
    }
    break;
    // 压缩包追加文件错误
    case ArchiveJob::JT_Add: {
        QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(30, 30));
        switch (eErrorType) {
        // 密码错误
        case ET_WrongPassword:
            m_ePageID = PI_UnCompress;
            sendMessage(icon, tr("Wrong password"));
            break;
        default:
            break;
        }
    }
    break;
    // 加载压缩包错误
    case ArchiveJob::JT_Load: {
        switch (eErrorType) {
        // 压缩包打开失败
        case ET_ArchiveOpenError:
            showErrorMessage(FI_Load, EI_ArchiveOpenFailed);
            break;
        // 密码错误
        case ET_WrongPassword:
            showErrorMessage(FI_Load, EI_WrongPassword);
            break;
        default:
            showErrorMessage(FI_Load, EI_ArchiveOpenFailed);
            break;
        }
    }
    break;
    // 解压错误
    case ArchiveJob::JT_Extract: {
        if (Archive_OperationType::Operation_SingleExtract == m_operationtype) {
            QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(30, 30));
            // 提取出错
            switch (eErrorType) {
            // 压缩包打开失败
            case ET_ArchiveOpenError:
                sendMessage(icon, tr("Failed to open compressed package"));
                break;
            // 密码错误
            case ET_WrongPassword:
                sendMessage(icon, tr("Wrong password"));
                break;
            // 文件名过长
            case ET_LongNameError:
                sendMessage(icon, tr("File name too long"));
                break;
            // 创建文件失败
            case ET_FileWriteError:
                sendMessage(icon, tr("Failed to create file"));
                break;
            default:
                break;
            }
        } else {
            // 解压出错
            switch (eErrorType) {
            // 压缩包打开失败
            case ET_ArchiveOpenError:
                showErrorMessage(FI_Uncompress, EI_ArchiveOpenFailed, !m_stUnCompressParameter.bRightOperation);
                break;
            // 密码错误
            case ET_WrongPassword:
                showErrorMessage(FI_Uncompress, EI_WrongPassword, !m_stUnCompressParameter.bRightOperation);
                break;
            // 文件名过长
            case ET_LongNameError:
                showErrorMessage(FI_Uncompress, EI_LongFileName, !m_stUnCompressParameter.bRightOperation);
                break;
            // 创建文件失败
            case ET_FileWriteError:
                showErrorMessage(FI_Uncompress, EI_CreatFileFailed, !m_stUnCompressParameter.bRightOperation);
                break;
            case ET_MissingVolume:
                showErrorMessage(FI_Uncompress, EI_ArchiveMissingVolume, !m_stUnCompressParameter.bRightOperation);
                break;
            default:
                break;
            }
        }
    }
    break;
    // 删除错误
    case ArchiveJob::JT_Delete: {
        m_ePageID = PI_UnCompress;
        QIcon icon = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(30, 30));
        switch (eErrorType) {
        // 压缩包打开失败
        case ET_ArchiveOpenError:
            sendMessage(icon, tr("Failed to open compressed package"));
            break;
        // 密码错误
        case ET_WrongPassword:
            sendMessage(icon, tr("Wrong password"));
            break;
        default:
            break;
        }
    }
    break;
    // 批量解压错误
    case ArchiveJob::JT_BatchExtract: {
        switch (eErrorType) {
        // 压缩包打开失败
        case ET_ArchiveOpenError:
            showErrorMessage(FI_Uncompress, EI_ArchiveOpenFailed, !m_stUnCompressParameter.bRightOperation);
            break;
        // 密码错误
        case ET_WrongPassword:
            showErrorMessage(FI_Uncompress, EI_WrongPassword, !m_stUnCompressParameter.bRightOperation);
            break;
        // 文件名过长
        case ET_LongNameError:
            showErrorMessage(FI_Uncompress, EI_LongFileName, !m_stUnCompressParameter.bRightOperation);
            break;
        // 创建文件失败
        case ET_FileWriteError:
            showErrorMessage(FI_Uncompress, EI_CreatFileFailed, !m_stUnCompressParameter.bRightOperation);
            break;
        default:
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
                sendMessage(icon, tr("Wrong password"));
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
    qDebug() << "向压缩包中添加文件";
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

    // 调用添加文件接口
    if (ArchiveManager::get_instance()->addFiles(strArchiveFullPath, listEntry, options)) {
        // 切换进度界面
        m_pProgressPage->setProgressType(PT_CompressAdd);
        m_pProgressPage->setArchiveName(QFileInfo(strArchiveFullPath).fileName());

        m_operationtype = Operation_Create;
        m_ePageID = PI_AddCompressProgress;
        refreshPage();

        // 设置更新选项
        m_stUpdateOptions.reset();
        m_stUpdateOptions.strParentPath = options.strDestination;
        m_stUpdateOptions.eType = UpdateOptions::Add;
        ConstructAddOptions(listFiles);

        m_pProgressPage->setTotalSize(m_stUpdateOptions.qSize);
        m_pProgressPage->restartTimer();
    } else {
        // 无可用插件
        showErrorMessage(FI_Add, EI_NoPlugin);
    }
}

void MainWindow::resetMainwindow()
{
#ifdef __aarch64__
    maxFileSize_ = 0;
#endif

    m_operationtype = Operation_NULL;   // 重置操作类型
    m_iCompressedWatchTimerID = 0;      // 初始化定时器返回值
    m_pProgressPage->resetProgress();   // 重置进度
    m_pOpenFileWatcher->reset();

    // 重置数据
    m_stCompressParameter = CompressParameter();
    m_stUnCompressParameter = UnCompressParameter();

    // 清空压缩包监听数据
    SAFE_DELETE_ELE(m_pFileWatcher);
}

void MainWindow::deleteWhenJobFinish(ArchiveJob::JobType eType)
{
    switch (eType) {
    // 压缩完成删除原文件
    case ArchiveJob::JT_Create: {
        QStringList listCompressFiles = m_pCompressPage->compressFiles();   // 获取所有压缩文件

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
        QString strArchivePath = m_pUnCompressPage->archiveFullPath();  // 获取压缩包全路径
        QFile file(strArchivePath);
        // 若压缩包存在，将之移动到回收站中（Qt5.15完美支持）
        if (file.exists()) {
            //QString strFileName = QFileInfo(strArchivePath).fileName();
            //file.rename(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/Trash/files/") + strFileName);

            // 删除文件
            file.remove();
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
    QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);

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
        m_pSuccessPage->setSuccessDes(tr("Extraction successful"));
        break;
    case SI_Convert:
        m_pSuccessPage->setSuccessDes(tr("Conversion successful"));
        break;
    }
}

void MainWindow::showErrorMessage(FailureInfo fFailureInfo, ErrorInfo eErrorInfo, bool bShowRetry)
{
    m_pFailurePage->setRetryEnable(bShowRetry);     // 设置重试按钮是否可用
    m_pFailurePage->setFailureInfo(fFailureInfo);   // 设置失败信息

    switch (fFailureInfo) {
    // 压缩失败
    case FI_Compress: {
        m_pFailurePage->setFailuerDes(tr("Compression failed"));
        switch (eErrorInfo) {
        case EI_NoPlugin: {
            m_pFailurePage->setFailureDetail(tr("No plugin available"));
        }
        break;
        case EI_CreatArchiveFailed: {
            m_pFailurePage->setFailureDetail(tr("Failed to create file"));
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
            m_pFailurePage->setFailureDetail(tr("No plugin available"));
        }
        break;
        case EI_ArchiveOpenFailed: {
            m_pFailurePage->setFailureDetail(tr("Failed to open compressed package"));
        }
        break;
        case EI_WrongPassword: {
            m_pFailurePage->setFailureDetail(tr("Wrong password"));
        }
        break;
        case EI_ArchiveMissingVolume: {
            m_pFailurePage->setFailureDetail(tr("Missing volume"));
        }
        break;
        default:
            break;
        }
    }
    break;
    // 解压失败
    case FI_Uncompress: {
        m_pFailurePage->setFailuerDes(tr("Extraction failed"));
        switch (eErrorInfo) {
        case EI_NoPlugin: {
            m_pFailurePage->setFailureDetail(tr("No plugin available"));
        }
        break;
        case EI_ArchiveOpenFailed: {
            m_pFailurePage->setFailureDetail(tr("Failed to open compressed package"));
        }
        break;
        case EI_ArchiveDamaged: {
            m_pFailurePage->setFailureDetail(tr("Damaged file"));
        }
        break;
        case EI_ArchiveMissingVolume: {
            m_pFailurePage->setFailureDetail(tr("Missing volume"));
        }
        break;
        case EI_WrongPassword: {
            m_pFailurePage->setFailureDetail(tr("Wrong password"));
        }
        break;
        case EI_LongFileName: {
            m_pFailurePage->setFailureDetail(tr("File name too long, unable to extract"));
        }
        break;
        case EI_CreatFileFailed: {
            m_pFailurePage->setFailureDetail(tr("Failed to create file"));
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
    qDebug() << "对压缩包进行格式转换" << convertType;
    m_operationtype = Operation_CONVERT;

    QString oldArchivePath = m_stUnCompressParameter.strFullPath; // 需要进行格式转换的压缩包的全路径
    QFileInfo oldArchive(oldArchivePath);

    QString newArchivePath = oldArchive.filePath().remove(oldArchive.suffix()) + convertType; // 转换后压缩包的全路径，还未判断该文件名是否存在
    // 压缩后的文件名
    int num = 2;
    while (QFileInfo::exists(newArchivePath)) { // 如果文件名存在自动重命名 文件名+（2）...
        newArchivePath = oldArchive.absolutePath() + QDir::separator() + QFileInfo(m_pUnCompressPage->archiveFullPath()).completeBaseName()
                         + "(" + QString::number(num) + ")" + "." + convertType;
        num++;
    }

    m_strFinalConvertFile = newArchivePath;  // 记录格式转换后的文件名，在转换成功之后打开文件使用

    // 创建格式转换的job
    if (ArchiveManager::get_instance()->convertArchive(oldArchivePath, TEMPPATH + QDir::separator() + m_strProcessID + createUUID(), newArchivePath)) {
        m_pProgressPage->setProgressType(PT_Convert);
        m_pProgressPage->setTotalSize(oldArchive.size() + DataManager::get_instance().archiveData().qSize);
        m_pProgressPage->setArchiveName(QFileInfo(newArchivePath).fileName());
        m_pProgressPage->restartTimer();

        m_ePageID = PI_ConvertProgress;
        refreshPage();
    }
}

void MainWindow::updateArchiveComment()
{
    m_operationtype = Operation_Update_Comment;
    if (ArchiveManager::get_instance()->updateArchiveComment(m_stUnCompressParameter.strFullPath, m_comment)) {
        qDebug() << "更新压缩包的注释信息";
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
            qDebug() << "添加zip压缩包的注释信息";
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

void MainWindow::slotExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions)
{
    qDebug() << "提取文件至:" << stOptions.strTargetPath;
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
    qDebug() << "删除文件:";
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
        m_pProgressPage->setArchiveName(QFileInfo(strArchiveFullPath).fileName());
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

    if ((mapStatus.find(strPath) != mapStatus.end()) && (!mapStatus[strPath])) {
        mapStatus[strPath] = true;

        QFileInfo file(strPath);
        QString strDesText = QObject::tr("%1 changed. Do you want to save changes to the archive?").arg(UiTools::toShortString(file.fileName()));

        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(strDesText, tr("Discard"), DDialog::ButtonNormal, tr("Update"), DDialog::ButtonRecommend);
        if (iResult == 1) {
            if (!m_stUnCompressParameter.bModifiable) { // 不支持修改文件的压缩包
                ConvertDialog dialog(this); // 询问是否进行格式转换
                QStringList ret = dialog.showDialog();

                if (ret.at(0) == "true") { // 进行格式转换
                    convertArchive(ret.at(1));
                }

                return;
            }

            // 更新压缩包数据
            addFiles2Archive(QStringList() << strPath);
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
        if (m_pDDesktopServicesThread)
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
        if (m_stUnCompressParameter.bRightOperation) {
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
    //    qDebug() << __FUNCTION__;
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

        //        commentTextedit->setPlaceholderText(tr("No more than %1 characters please").arg(MAXCOMMENTLEN));
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
