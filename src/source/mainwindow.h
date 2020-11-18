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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uistruct.h"
#include "queries.h"
#include "commonstruct.h"

#include <DMainWindow>

class QStackedWidget;
class QSettings;

class LoadingPage;
class HomePage;
class CompressPage;
class CompressSettingPage;
class UnCompressPage;
class ProgressPage;
class SuccessPage;
class FailurePage;
class SettingDialog;
class ProgressDialog;
class DDesktopServicesThread;
class ArchiveManager;
class QFileSystemWatcher;

DWIDGET_USE_NAMESPACE

// 主界面
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initTitleBar  初始化标题栏
     */
    void initTitleBar();

    /**
     * @brief initData  初始化相关数据
     */
    void initData();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief loadWindowState   加载状态
     */
    void loadWindowState();

    /**
     * @brief refreshPage   刷新界面页
     */
    void refreshPage();

    /**
     * @brief calSelectedTotalFileSize  计算本地选择的文件大小
     * @param files 选择文件
     * return 文件总大小
     */
    qint64 calSelectedTotalFileSize(const QStringList &files);

    /**
     * @brief calFileSizeByThread  用多线程的方式计算文件或文件夹所有文件大小
     * @param path 文件或文件夹路径
     */
    void calFileSizeByThread(const QString &path);

    /**
     * @brief setTitleButtonStyle   设置标题栏按钮样式以及显隐状态
     * @param bVisible  显示/隐藏
     * @param pixmap    图片样式
     */
    void setTitleButtonStyle(bool bVisible, DStyle::StandardPixmap pixmap = DStyle::StandardPixmap::SP_IncreaseElement);

    /**
     * @brief loadArchive       加载压缩包
     * @param strArchiveFullPath    压缩包全路径
     */
    void loadArchive(const QString &strArchiveFullPath);
    /**
     * @brief Extract2PathFinish 提取成功提示
     * @param msg
     */
    void Extract2PathFinish(QString msg);

    /**
     * @brief createUUID    创建唯一标识
     * @return
     */
    QString createUUID();

    /**
     * @brief getExtractPath   根据设置选项是否自动创建文件夹获取解压路径
     * @param strArchiveFullPath
     * @return
     */
    QString getExtractPath(const QString &strArchiveFullPath);

    /**
     * @brief transSplitFileName 处理7z、rar分卷压缩包名称
     * @param fileName 原始名称
     */
    void transSplitFileName(QString &fileName); // *.7z.003 -> *.7z.001

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief timerEvent    定时
     */
    void timerEvent(QTimerEvent *) override;

private Q_SLOTS:
    /**
     * @brief slotHandleRightMenuSelected   处理文管操作
     * @param listParam     参数
     */
    void slotHandleRightMenuSelected(const QStringList &listParam);

    /**
     * @brief slotTitleBtnClicked   标题栏按钮点击
     */
    void slotTitleBtnClicked();

    /**
     * @brief slotTitleBtnClicked   选择文件触发
     */
    void slotChoosefiles();

    /**
     * @brief slotDragSelectedFiles     拖拽添加文件
     * @param listFiles     拖拽的文件
     */
    void slotDragSelectedFiles(const QStringList &listFiles);

    /**
     * @brief compressLevelChanged  处理压缩层级变化
     * @param bRootIndex    是否是根目录
     */
    void slotCompressLevelChanged(bool bRootIndex);

    /**
     * @brief slotCompressNext  压缩界面点击下一步按钮处理操作
     */
    void slotCompressNext();

    /**
     * @brief slotCompress  压缩点击
     */
    void slotCompress(const QVariant &val);

    /**
     * @brief slotJobFinshed    操作结束处理
     */
    void slotJobFinshed();

    /**
     * @brief slotUncompressClicked     解压按钮点击，执行解压操作
     * @param strUncompressPath         解压路径
     */
    void slotUncompressClicked(const QString &strUncompressPath);

    /**
     * @brief slotReceiveProgress   进度信号处理
     * @param dPercentage   进度值
     */
    void slotReceiveProgress(double dPercentage);

    /**
     * @brief slotReceiveCurFileName    当前操作的文件名显示处理
     * @param strName       当前文件名
     */
    void slotReceiveCurFileName(const QString &strName);

    /**
     * @brief slotQuery   发送询问信号
     * @param query 询问类型
     */
    void slotQuery(Query *query);

    /**
     * @brief slotExtract2Path  提取压缩包中文件
     * @param listSelEntry      选中的提取文件
     * @param stOptions         提取参数
     */
    void slotExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions);

    /**
     * @brief slotDelFiels    删除压缩包中文件
     * @param listSelEntry      当前选中的文件
     * @param qTotalSize        删除文件总大小
     */
    void slotDelFiles(const QList<FileEntry> &listSelEntry, qint64 qTotalSize);

    /**
     * @brief slotReceiveCurArchiveName  当前正在操作的压缩包名称显示
     * @param strArchiveName        压缩包名称
     */
    void slotReceiveCurArchiveName(const QString &strArchiveName);

    /**
     * @brief slotOpenFile    打开压缩包中文件
     * @param entry             待打开的文件
     * @param strProgram        应用程序名（为空时，用默认应用程序打开）
     */
    void slotOpenFile(const FileEntry &entry, const QString &strProgram = "");

    /**
     * @brief slotOpenFileChanged   打开文件变化通知
     * @param strPath               文件全路径
     */
    void slotOpenFileChanged(const QString &strPath);

    /**
     * @brief slotSuccessReturn     解压成功返回
     */
    void slotSuccessReturn();

    /**
     * @brief slotPause       暂停
     * @param eType             操作类型
     */
    void slotPause(Progress_Type eType);

    /**
     * @brief slotContinue    继续
     */
    void slotContinue();

    /**
     * @brief slotCancel       取消
     * @param eType             操作类型
     */
    void slotCancel(Progress_Type eType);

private:
    QString m_strUUID;              // 应用唯一标识（用于退出应用时清除缓存文件）
    bool m_initFlag = false;        // 界面是否初始化标志

    QStackedWidget *m_pMainWidget;  // 中心面板

    // 界面页
    HomePage *m_pHomePage;            // 首页
    CompressPage *m_pCompressPage;    // 压缩列表界面
    CompressSettingPage *m_pCompressSettingPage;  // 压缩设置界面
    UnCompressPage *m_pUnCompressPage;    // 解压列表界面
    ProgressPage *m_pProgressPage;    // 进度界面
    SuccessPage *m_pSuccessPage;  // 成功界面
    FailurePage *m_pFailurePage;  // 失败界面
    LoadingPage *m_pLoadingPage;  // 加载界面

    DIconButton *m_pTitleButton;                  // 标题栏按钮（添加文件）
    QAction *m_pOpenAction;                                 // 菜单 - 打开

    // 弹窗
    ProgressDialog *m_pProgressdialog; // 进度对话框
    SettingDialog *m_pSettingDlg;       // 设置界面

    QSettings *m_pSettings;     // 默认配置信息

    Page_ID m_ePageID = PI_Home;      // 界面标识
    Archive_OperationType m_operationtype = Operation_NULL; // 操作类型

    int m_iInitUITimer = 0;                       // 初始化界面定时器
    int m_iCompressedWatchTimerID = 0;            // 压缩文件监视定时器ID

    qint64 m_qTotalSize = 0;            // 压缩文件总大小
    QString m_strExtractPath;           // 解压路径
    QList<QString> m_listExractFiles; // 存储提取文件,用来结束之后自动打开文件夹时选中
    DDesktopServicesThread *m_pDDesktopServicesThread = nullptr;    // 打开文管界面线程服务

    // 打开压缩包文件
    QFileSystemWatcher *m_pOpenFileWatcher;       // 对打开的文件监控
    QList<FileEntry> m_listOpenFiles;      // 所有的打开的文件数据
    QString m_strOpenFile;                 // 最后一次打开的文件（真实全路径 用来添加到文件监控中）
    QMap<QString, bool> m_mapFileHasModified;   // 文件是否更改
    QMap<QString, FileEntry> m_mapOpenFils;   // 本地文件 - 压缩包文件数据（方式同名文件寻找失败）

    bool m_bRightOperation = false;     // 右键快捷解压
};

#endif // MAINWINDOW_H
