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
    initUI();
    initTitleBar();
    initConnections();
    initData();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    // 初始化界面
    m_pMainWidget = new QStackedWidget(this);  // 中心面板
    m_pLoadingPage = new LoadingPage(this);  // 加载界面
    m_pHomePage = new HomePage(this);            // 首页
    m_pCompressPage = new CompressPage(this);    // 压缩列表界面
    m_pCompressSettingPage = new CompressSettingPage(this);  // 压缩设置界面
    m_pUnCompressPage = new UnCompressPage(this);    // 解压列表界面
    m_pProgressPage = new ProgressPage(this);    // 进度界面
    m_pSuccessPage = new SuccessPage(this);  // 成功界面
    m_pFailurePage = new FailurePage(this);  // 失败界面

    m_pSettingDlg = new SettingDialog(this);

    m_pArchiveManager = new ArchiveManager(this);

    // 添加界面至主界面
    m_pMainWidget->addWidget(m_pHomePage);
    m_pMainWidget->addWidget(m_pCompressPage);
    m_pMainWidget->addWidget(m_pCompressSettingPage);
    m_pMainWidget->addWidget(m_pUnCompressPage);
    m_pMainWidget->addWidget(m_pProgressPage);
    m_pMainWidget->addWidget(m_pSuccessPage);
    m_pMainWidget->addWidget(m_pFailurePage);
    m_pMainWidget->addWidget(m_pLoadingPage);

    setCentralWidget(m_pMainWidget);    // 设置中心面板

    m_pMainWidget->setCurrentIndex(0);
    setMinimumSize(620, 465);
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
    }
    break;
    case PI_CompressSetting: {
        m_pMainWidget->setCurrentIndex(2);
    }
    break;
    case PI_UnCompress: {
        m_pMainWidget->setCurrentIndex(3);
    }
    break;
    case PI_CompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
    }
    break;
    case PI_UnCompressProgress: {
        m_pMainWidget->setCurrentIndex(4);
    }
    break;
    case PI_DeleteProgress: {
        m_pMainWidget->setCurrentIndex(4);
    }
    break;
    case PI_ConvertProgress: {
        m_pMainWidget->setCurrentIndex(4);
    }
    break;
    case PI_CompressSuccess: {
        m_pMainWidget->setCurrentIndex(5);
    }
    break;
    case PI_UnCompressSuccess: {
        m_pMainWidget->setCurrentIndex(5);
    }
    break;
    case PI_ConvertSuccess: {
        m_pMainWidget->setCurrentIndex(5);
    }
    break;
    case PI_CompressFailure: {
        m_pMainWidget->setCurrentIndex(6);
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
    qint64 qTotalSize = 0;

    foreach (QString file, files) {
        QFileInfo fi(file);

        if (fi.isFile()) {  // 如果为文件，直接获取大小
            qint64 curFileSize = fi.size();

#ifdef __aarch64__
            if (maxFileSize_ < curFileSize) {
                maxFileSize_ = curFileSize;
            }
#endif
            qTotalSize += curFileSize;
        } else if (fi.isDir()) {    // 如果是文件夹，递归获取所有子文件大小总和
            QtConcurrent::run(this, &MainWindow::calFileSizeByThread, file, qTotalSize);
        }
    }

    // 等待线程池结束
    QThreadPool::globalInstance()->waitForDone();

    return qTotalSize;
}

void MainWindow::calFileSizeByThread(const QString &path, qint64 &qSize)
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
            QtConcurrent::run(this, &MainWindow::calFileSizeByThread, fileInfo.filePath(), qSize);
        } else {
            mutex.lock();
            // 如果是文件则直接计算大小
            qint64 curFileSize = fileInfo.size();
#ifdef __aarch64__
            if (maxFileSize_ < curFileSize) {
                maxFileSize_ = curFileSize;
            }
#endif
            qSize += curFileSize;
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
    m_ePageID = PI_Compress;
    refreshPage();

    m_pCompressPage->addCompressFiles(listFiles);       // 添加压缩文件
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
    m_pCompressSettingPage->refreshMenu();

    // 刷新界面 切换到压缩设置界面
    m_ePageID = PI_CompressSetting;
    refreshPage();
}

void MainWindow::slotCompress()
{
    qDebug() << "点击了压缩按钮";

    QStringList listFiles = m_pCompressPage->compressFiles();   // 获取待压缩文件

    QVector<FileEntry> files;
    QString strDestination;
    CompressOptions options;
    bool bBatch = false;
    m_pArchiveManager->createArchive(files, strDestination, options, bBatch);
}
