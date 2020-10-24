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

class ArchiveManager;


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



    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief timerEvent    定时
     */
    void timerEvent(QTimerEvent *) override;

private Q_SLOTS:
    /**
     * @brief slotChoosefiles   选择本地文件
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
     * @brief slotUncompressSlicked     解压按钮点击，执行解压操作
     * @param strUncompressPath         解压路径
     */
    void slotUncompressSlicked(const QString &strUncompressPath);

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
     * @brief slotFileChanged 压缩文件变化
     * @param strFileName       文件名（含路径）
     */
    void slotFileChanged(const QString &strFileName);


private:
    QStackedWidget *m_pMainWidget;  // 中心面板

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

    SettingDialog *m_pSettingDlg;       // 设置界面

    QSettings *m_pSettings;     // 默认配置信息

    Page_ID m_ePageID;      // 界面标识

    int m_iInitUITimer = 0;                           // 初始化界面定时器
    int m_iCompressedWatchTimerID = 0;            // 压缩文件监视定时器ID

    ArchiveManager *m_pArchiveManager;

    qint64 m_qTotalSize = 0;
};

#endif // MAINWINDOW_H
