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

#pragma once

#include "fileViewer.h"
#include "progress.h"
#include "customdatainfo.h"

#include <DMainWindow>
#include <DFileWatcher>
#include <DSpinner>
#include <DIconButton>

#include <QEvent>

static QVector<qint64> m_gTempProcessId;                 // 临时进程ID

class MainWindow;
class Settings_Extract_Info;
class QStackedLayout;
class TimerWatcher;
class MonitorAdaptor;
class QSettings;
class HomePage;
class UnCompressPage;
class CompressPage;
class CompressSetting;
class Compressor_Success;
class Compressor_Fail;
class Archive;
class ArchiveModel;
class EncryptionPage;
class ProgressDialog;
class ExtractPauseDialog;
class SettingDialog;
class EncodingPage;
class ArchiveSortFilterModel;
class OpenLoadingPage;
class QEventLoop;

/**
 * @brief The MainWindow class  整个归档管理器窗口，不用类型的操作通过切换其中的界面实现
 */
class MainWindow : public DMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.archive.mainwindow.monitor")

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * @brief closeClean
     * @param event
     * @see 每次关闭窗口尽量手动释放更多的内存，因为我们的窗口close()实际上执行的是hide();
     */
    void closeClean(QCloseEvent *event);

    /**
     * @brief InitUI   初始化界面
     */
    void InitUI();

    /**
     * @brief InitConnection    初始化信号槽连接
     */
    void InitConnection();

    /**
     * @brief initTitleBar  初始化标题栏
     */
    void initTitleBar();

    /**
     * @brief createSettingsMenu    创建设置菜单Action
     * @return  设置菜单
     */
    QMenu *createSettingsMenu();

    /**
     * @brief loadArchive   加载压缩包
     * @param files 压缩包文件名（含路径）
     */
    void loadArchive(const QString &files);

    /**
     * @brief creatArchive  创建压缩包（单路径）
     * @param Args  相关设置参数
     */
    void creatArchive(QMap<QString, QString> &Args);

    /**
     * @brief creatBatchArchive  创建压缩包（多路径）
     * @param Args  相关设置参数
     * @param filetoadd 需要添加的文件（路径 - 文件名）
     */
    void creatBatchArchive(QMap<QString, QString> &Args, QMap<QString, QStringList> &filetoadd);

    /**
     * @brief addArchive    拖拽追加压缩
     * @param Args  需要追加的文件相关 参数
     */
    void addArchive(QMap<QString, QString> &Args);

    /**
     * @brief addArchiveEntry   删除之后自动追加
     * @param args  参数
     * @param pWorkEntry    删除的Entry
     */
    void addArchiveEntry(QMap<QString, QString> &args, Archive::Entry *pWorkEntry);

    /**
     * @brief removeEntryVector
     * @param vectorDel
     * @param isManual,true:by action clicked; false: by message emited.
     * @see true:手动删除；false:子面板消息通知删除。
     */
    void removeEntryVector(QVector<Archive::Entry *> &vectorDel, bool isManual);

    /**
     * @brief moveToArchive
     * @param Args
     */
    void moveToArchive(QMap<QString, QString> &Args);

    /**
     * @brief transSplitFileName    处理7z分卷压缩包名称
     * @param fileName  原始名称
     */
    void transSplitFileName(QString &fileName); // *.7z.003 -> *.7z.001

    /**
     * @brief ExtractPassword   加密解压
     * @param password  输入的密码
     */
    void ExtractPassword(QString password);

    /**
     * @brief ExtractSinglePassword 加密提取/打开等
     * @param password  输入的密码
     */
    void ExtractSinglePassword(QString password);

    /**
     * @brief LoadPassword  加密加载
     * @param password  输入的密码
     */
    void LoadPassword(QString password);

    /**
     * @brief WatcherFile   监听本地压缩包
     * @param files 压缩包名称（全路径）
     */
    void WatcherFile(const QString &files);

    /**
     * @brief renameCompress    重命名压缩包名称
     * @param filename  压缩包名称
     * @param fixedMimeType 类型
     */
    void renameCompress(QString &filename, QString fixedMimeType);

    /**
     * @brief getLoadFile
     * @return
     */

    /**
     * @brief getDiskFreeSpace  获取磁盘控件
     * @return 磁盘空间大小
     */
    qint64 getDiskFreeSpace();

    /**
     * @brief getMediaFreeSpace 获取挂载设备控件
     * @return 挂载设备空间大小
     */
    qint64 getMediaFreeSpace();

    /**
     * @brief applicationQuit   应用程序退出处理
     * @return 是否正常退出
     */
    bool applicationQuit();

    /**
     * @brief isWorkProcess 判断当前工作状态
     * @return 当前工作状态
     */
    bool isWorkProcess();

    /**
     * @brief checkSettings 检测目标文件合法性
     * @param file  文件名
     * @return
     */
    bool checkSettings(QString file);

    /**
     * @brief bindAdapter   初始化监视器
     */
    void bindAdapter();

private:
    /**
     * @brief saveWindowState   退出时保存位置
     */
    void saveWindowState();

    /**
     * @brief loadWindowState   初始化时获取上次打开的位置
     */
    void loadWindowState();

    /**
     * @brief modelIndexToStr
     * @param modelIndex
     * @return
     */
    QString modelIndexToStr(const QModelIndex &modelIndex);//added by hsw 20200525

    /**
     * @brief queryDialogForClose   关闭询问窗口
     * @return  点击操作，确认/取消
     */
    int queryDialogForClose();

    /**
     * @brief setCompressDefaultPath    设置压缩路径
     */
    void setCompressDefaultPath();

    /**
     * @brief setQLabelText
     * @param label
     * @param text
     */
    //void setQLabelText(QLabel *label, const QString &text);

    /**
     * @brief creatShorcutJson  创建快捷键配置
     * @return
     */
    QJsonObject creatShorcutJson();

    //QStringList CheckAllFiles(QString path);

    /**
     * @brief deleteCompressFile    关闭/取消/退出时删除临时压缩文件
     */
    void deleteCompressFile(/*QStringList oldfiles, QStringList newfiles*/);

    /**
     * @brief deleteDecompressFile    关闭/取消/退出时删除临时解压文件
     * @param destDirName
     */
    void deleteDecompressFile(QString destDirName = "");

    /**
     * @brief startCmd  启动命令
     * @param executeName   可执行程序名称
     * @param arguments 命令参数
     * @return 是否启动成功
     */
    bool startCmd(const QString &executeName, QStringList arguments);

    /**
     * @brief removeFromParentInfo  从父窗口中移除某窗口
     * @param CurMainWnd    需要移除的当前窗口
     */
    void removeFromParentInfo(MainWindow *CurMainWnd);

    /**
     * @brief calSelectedTotalFileSize  计算本地选择的文件大小
     * @param files 选择文件
     */
    void calSelectedTotalFileSize(const QStringList &files);

    /**
     * @brief calSelectedTotalEntrySize 计算选择的所有Entry大小
     * @param vectorDel 选择的Entry
     */
    void calSelectedTotalEntrySize(QVector<Archive::Entry *> &vectorDel);

    /**
     * @brief calFileSize   计算文件或文件夹内所有文件大小
     * @param path  文件或文件夹路径
     * @return
     */
    qint64 calFileSize(const QString &path);

//    /**
//     * @brief calSpeedAndTime   计算速度和剩余时间
//     * @param compressPercent   当前进度百分比
//     */
//    void calSpeedAndTime(unsigned long compressPercent);

    /**
     * @brief getDefaultApp 根据文件类型获取默认打开的应用的程序
     * @param mimetype  文件类型
     * @return 应用程序
     */
    QString getDefaultApp(QString mimetype);

    /**
     * @brief setDefaultApp 设置默认应用程序
     * @param mimetype  文件类型
     * @param desktop   应用程序
     */
    void setDefaultApp(QString mimetype, QString desktop);

    /**
     * @brief promptDialog  关联类型提示对话框
     * @return  点击类型
     */
    int promptDialog();

    /**
     * @brief refreshPage
     */
    void refreshPage();

    /**
     * @brief setTitleButtonStyle   设置标题栏按钮样式以及显隐状态
     * @param bVisible  显示/隐藏
     * @param pixmap    图片样式
     */
    void setTitleButtonStyle(bool bVisible, DStyle::StandardPixmap pixmap = DStyle::StandardPixmap::SP_IncreaseElement);

    /**
     * @brief unzipSuccessOpenFileDir    解压成功打开文管
     */
    void unzipSuccessOpenFileDir();

    /**
     * @brief killJob   关闭Job
     */
    void killJob();

    /**
     * @brief deleteLaterJob    删除Job
     */
    void deleteLaterJob();

protected:
    /**
     * @brief dragEnterEvent    拖拽进入
     */
    void dragEnterEvent(QDragEnterEvent *) override;

    /**
     * @brief dragLeaveEvent    拖拽离开
     */
    void dragLeaveEvent(QDragLeaveEvent *) override;

    /**
     * @brief dropEvent 拖拽放下
     */
    void dropEvent(QDropEvent *) override;

    /**
     * @brief dragMoveEvent     拖拽移动
     */
    void dragMoveEvent(QDragMoveEvent *) override;

    /**
     * @brief closeEvent    关闭
     */
    void closeEvent(QCloseEvent *) override;

    /**
     * @brief timerEvent    定时
     */
    void timerEvent(QTimerEvent *) override;

signals:
    /**
     * @brief sigquitApp
     */
    void sigquitApp();

    /**
     * @brief sigZipAddFile
     */
    void sigZipAddFile();

    /**
     * @brief sigCompressedAddFile
     */
    void sigCompressedAddFile();

    /**
     * @brief sigZipReturn
     */
    void sigZipReturn();

    /**
     * @brief sigZipSelectedFiles
     * @param files
     */
    void sigZipSelectedFiles(const QStringList &files);

    /**
     * @brief loadingStarted
     */
    void loadingStarted();

    /**
     * @brief sigUpdateTableView
     */
    void sigUpdateTableView(const QFileInfo &);

    /**
     * @brief sigTipsWindowPopUp
     */
    void sigTipsWindowPopUp(int, const QStringList &);

    /**
     * @brief sigTipsUpdateEntry
     * @param vectorDel
     */
    void sigTipsUpdateEntry(int, QVector<Archive::Entry *> &vectorDel);

    /**
     * @brief deleteJobComplete
     * @param pEntry
     */
    void deleteJobComplete(Archive::Entry *pEntry);

public slots:
    //accept subwindows drag files and return tips string
    /**
     * @brief onSubWindowActionFinished
     * @param mode
     * @param pid
     * @param urls
     * @return
     */
    bool onSubWindowActionFinished(int mode, const qint64 &pid, const QStringList &urls);

    /**
     * @brief popUpChangedDialog    主界面接收到要弹出对话框消息
     * @param pid
     * @return
     */
    bool popUpChangedDialog(const qint64 &pid);

    /**
     * @brief createSubWindow
     * @param urls
     * @return
     */
    bool createSubWindow(const QStringList &urls);

private slots:
//    /**
//     * @brief setEnable
//     */
//    void setEnable();

//    /**
//     * @brief setDisable
//     */
//    void setDisable();

    /**
     * @brief onSelected    选择本地文件（通过拖拽、打开、选择打开）
     * @param listSelFiles  选择的文件
     */
    void onSelected(const QStringList &listSelFiles);

    /**
     * @brief onRightMenuSelected
     */
    void onRightMenuSelected(const QStringList &);

    /**
     * @brief onCompressNext
     */
    void onCompressNext();

    /**
     * @brief onCompressPressed
     * @param Args
     */
    void onCompressPressed(QMap<QString, QString> &Args);

    /**
     * @brief onUncompressStateAutoCompress
     * @param Args
     */
    void onUncompressStateAutoCompress(QMap<QString, QString> &Args);

    // added by hsw 20200525
    /**
     * @brief onUncompressStateAutoCompressEntry
     * @param Args
     * @param pWorkEntry
     */
    void onUncompressStateAutoCompressEntry(QMap<QString, QString> &Args, Archive::Entry *pWorkEntry = nullptr);

    /**
     * @brief onCancelCompressPressed
     * @param compressType
     */
    void onCancelCompressPressed(Progress::ENUM_PROGRESS_TYPE compressType);

    /**
     * @brief onTitleButtonPressed
     */
    void onTitleButtonPressed();

    /**
     * @brief onCompressAddfileSlot
     * @param status
     */
    void onCompressAddfileSlot(bool status);

    /**
     * @brief slotLoadingFinished   压缩包加载结束
     * @param job   加载Job
     */
    void slotLoadingFinished(KJob *job);

    /**
     * @brief slotExtractionDone
     * @param job
     */
    void slotExtractionDone(KJob *job);

    /**
     * @brief slotShowPageUnzipProgress
     */
    void slotShowPageUnzipProgress();

    /**
     * @brief slotextractSelectedFilesTo
     * @param localPath
     */
    void slotextractSelectedFilesTo(const QString &localPath);

    /**
     * @brief SlotProgress
     * @param job
     * @param percent
     */
    void SlotProgress(KJob *job, unsigned long percent);

    /**
     * @brief SlotProgressFile
     * @param job
     * @param filename
     */
    void SlotProgressFile(KJob *job, const QString &filename);

    /**
     * @brief SlotNeedPassword
     */
    void SlotNeedPassword();

    /**
     * @brief SlotExtractPassword
     * @param password
     */
    void SlotExtractPassword(QString password);

    /**
     * @brief slotCompressFinished
     * @param job
     */
    void slotCompressFinished(KJob *job);

    /**
     * @brief slotJobFinished
     * @param job
     */
    void slotJobFinished(KJob *job);

    /**
     * @brief slotExtractSimpleFiles
     * @param fileList
     * @param path
     * @param type
     */
    void slotExtractSimpleFiles(QVector<Archive::Entry *> fileList, QString path, EXTRACT_TYPE type);

    /**
     * @brief slotExtractSimpleFilesOpen
     * @param fileList
     * @param programma
     */
    void slotExtractSimpleFilesOpen(const QVector<Archive::Entry *> &fileList, const QString &programma);

    /**
     * @brief slotKillExtractJob
     */
    void slotKillExtractJob();

    /**
     * @brief slotFailRetry
     */
    void slotFailRetry();

    /**
     * @brief slotBatchExtractFileChanged
     * @param name
     */
    void slotBatchExtractFileChanged(const QString &name);

    /**
     * @brief slotBatchExtractError
     * @param name
     */
    void slotBatchExtractError(const QString &name);

    /**
     * @brief slotClearTempfile
     */
    void slotClearTempfile();

    /**
     * @brief slotquitApp
     */
    void slotquitApp();

    /**
     * @brief onUpdateDestFile
     * @param destFile
     */
    void onUpdateDestFile(QString destFile);

    /**
     * @brief onCompressPageFilelistIsEmpty
     */
    void onCompressPageFilelistIsEmpty();

    /**
     * @brief slotCalDeleteRefreshTotalFileSize
     * @param files
     */
    void slotCalDeleteRefreshTotalFileSize(const QStringList &files);

    /**
     * @brief slotUncompressCalDeleteRefreshTotoalSize
     * @param vectorDel
     * @param isManual,true:by action clicked; false: by message emited.
     */
    void slotUncompressCalDeleteRefreshTotoalSize(QVector<Archive::Entry *> &vectorDel, bool isManual);

    /**
     * @brief resetMainwindow
     */
    void resetMainwindow();

    /**
     * @brief slotBackButtonClicked
     */
    void slotBackButtonClicked();

    /**
     * @brief slotResetPercentAndTime
     */
    void slotResetPercentAndTime();

    /**
     * @brief slotFileUnreadable
     * @param pathList
     * @param fileIndex
     */
    void slotFileUnreadable(QStringList &pathList, int fileIndex);//compress file is unreadable or file is a link

    /**
     * @brief slotStopSpinner
     */
    void slotStopSpinner();

    /**
     * @brief slotWorkTimeOut
     */
    void slotWorkTimeOut();

    /**
     * @brief deleteFromArchive
     * @param files
     * @param archive
     */
    void deleteFromArchive(const QStringList &files, const QString &archive);

    /**
     * @brief closeExtractJobSafe
     */
    void closeExtractJobSafe();

    /**
     * @brief slotLoadWrongPassWord
     */
    void slotLoadWrongPassWord();

public:
    static int m_windowcount;                               // 窗口数目
    OpenInfo::ENUM_OPTION m_eOption = OpenInfo::OPEN;       // 窗口打开标志
    QString *m_pChildMndExtractPath = nullptr;              // 保存的有次级面板解压路径（用户解压路径，非临时路径），该变量其他地方用不到

private:
    DWidget *m_pMmainWidget = nullptr;                      // 中心面板
    QStackedLayout *m_pMainLayout = nullptr;                // 切页

    // 界面页
    HomePage *m_pHomePage = nullptr;                        // 首页
    UnCompressPage *m_pUnCompressPage = nullptr;            // 解压界面
    CompressPage *m_pCompressPage = nullptr;                // 压缩界面
    CompressSetting *m_pCompressSetting = nullptr;          // 压缩设置界面
    Progress *m_pProgess = nullptr;                         // 进度界面
    Compressor_Success *m_pCompressSuccess = nullptr;       // 压缩成功界面
    Compressor_Fail *m_pCompressFail = nullptr;             // 压缩失败界面
    EncryptionPage *m_pEncryptionpage = nullptr;            // 解压输入密码界面
    OpenLoadingPage *m_pOpenLoadingPage = nullptr;          // 加载界面

    DIconButton *m_pTitleButton = nullptr;                  // 标题栏按钮（添加文件）
    QAction *m_pOpenAction;                                 // 菜单 - 打开
    DSpinner *m_pSpinner = nullptr;                         // 转圈加载

    // 弹窗
    ProgressDialog *m_pProgressdialog = nullptr;            // 进度对话框
    SettingDialog *m_pSettingsDialog = nullptr;             // 设置对话框

    QString m_strPathStore;                                 // 解压/压缩目标路径

    // 压缩参数
    QString m_strCreateCompressFile;                        // 创建的压缩文件名（带路径）

    // 解压参数
    ArchiveModel *m_pArchiveModel = nullptr;                // 数据模型
    ArchiveSortFilterModel *m_pArchiveFilterModel = nullptr;// 数据排序模型
    QString m_strDecompressFileName;                        // 压缩包文件名（不含路径）
    QString m_strDecompressFilePath;                        // 压缩包路径
    QString m_strLoadfile;                                  // 加载文件名（含路径）
    QVector<Archive::Entry *> m_vecExtractSimpleFiles;      // 解压文件
    QString m_strProgram;                                   // 打开方式（应用程序名称）

    // 追加压缩参数
    QString m_strAppendFileName;                            // 追加文件名（含路径）

    // DBus相关
    MonitorAdaptor *m_pAdaptor = nullptr;                   // 监视器
    GlobalMainWindowMap *m_pMapGlobalWnd = nullptr;         // winID - 面板 added by hsw 20200521
    MainWindow_AuxInfo *m_pCurAuxInfo = nullptr;            // 当前面板辅助信息 added by hsw 20200525
    QReadWriteLock m_lock;
    QMap<qint64, QStringList> m_mapSubWinDragFiles;         // 子面板拖拽文件
    int m_iMode = 0;                                        // 子面板弹出模式
    qint64 m_lCurOperChildPid = 0;                          // 当前打开的子面板winID

    // 配置相关
    QSettings *m_pSettings = nullptr;                       // 设置选项
    Settings_Extract_Info *pSettingInfo = nullptr;          // 设置信息 added by hsw 20200619
    Page_ID m_ePageID = PAGE_HOME;                          // 界面页类型
    bool m_bIsRightMenu = false;                            // 是否右键菜单引起的操作
    bool m_bIsAddArchive = false;                           // 是否追加压缩

    // 界面<->插件
    KJob *m_pJob = nullptr;                                 // 指向所有Job派生类对象
    Archive_OperationType m_operationtype = Operation_NULL; // 操作类型
    WorkState m_eWorkStatus = WorkNone;                     // Job的工作状态
    JobType m_eJobType = JOB_NULL;                          // Job的类型

    // 其它
    int m_iWatchTimerID = 0;                                // 定时器ID
    TimerWatcher *m_pWatcher = nullptr;                     // 定时器监视类
    DFileWatcher *m_pFileWatcher = nullptr;                 // 文件监控
    int m_iOpenTempFileLink = 0;                            // 打开临时文件索引
    QEventLoop *pEventloop = nullptr;                       // 事件循环


#ifdef __aarch64__
    qint64 maxFileSize_ = 0;
#endif

};














