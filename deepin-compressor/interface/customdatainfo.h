/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     xxx <xxx@uniontech.com>
*
* Maintainer: xxx <xxx@uniontech.com>
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
#ifndef CUSTOMDATAINFO_H
#define CUSTOMDATAINFO_H

#include <DMainWindow>
#include <QString>
#include <QMap>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE


#define TITLE_FIXED_HEIGHT 50                   // 标题栏高度
#define HEADBUS "/QtDusServer/registry"         // DBus头
#define AUTO_DELETE_NEVER "Never"               //解压后不删除源压缩文件
#define AUTO_DELETE_ASK "Ask for confirmation"  //解压后询问是否删除源压缩文件
#define AUTO_DELETE_ALWAYS "Always"             //解压后删除源压缩文件

/**
 * @brief The Page_ID enum
 * 界面ID，每一个变量对应一个切换的界面
 */
enum Page_ID {
    PAGE_HOME,              // 首页
    PAGE_UNZIP,             // 解压界面
    PAGE_ZIP,               // 压缩界面
    PAGE_ZIPSET,            // 压缩设置界面
    PAGE_ZIPPROGRESS,       // 压缩进度界面
    PAGE_UNZIPPROGRESS,     // 解压进度界面
    PAGE_ZIP_SUCCESS,       // 压缩成功界面
    PAGE_ZIP_FAIL,          // 压缩失败界面
    PAGE_UNZIP_SUCCESS,     // 解压成功界面
    PAGE_UNZIP_FAIL,        // 解压失败界面
    PAGE_ENCRYPTION,        // 解压输入密码界面
    PAGE_DELETEPROGRESS,    // 删除进度界面
    //PAGE_MAX,               //
    PAGE_LOADING            // 加载转圈界面
};

/**
 * @brief The Archive_OperationType enum
 *  针对压缩包的操作，比如解压、提取、打开内容等
 */
enum Archive_OperationType {
    Operation_NULL,                     // 无操作
    Operation_Load,                     // 加载
    Operation_Extract,                  // 解压
    Operation_SingleExtract,            // 提取
    Operation_ExtractHere,              // 解压到当前
    Operation_TempExtract,              // 临时解压
    Operation_TempExtract_Open,         // 打开
    Operation_TempExtract_Open_Choose,  // 选择打开
    Operation_DRAG                      // 拖拽
};

/**
 * @brief The WorkState enum
 * 工作状态
 */
enum WorkState {
    WorkNone,              // 无状态
    WorkProcess,           // 正在进行
};

/**
 * @brief The JobState enum
 * Job类型
 */
enum JobType {
    JOB_NULL,              // 无
    JOB_ADD,               // 添加
    JOB_DELETE,            // 删除
    JOB_DELETE_MANUAL,     // 手动delete，而非消息通知的delete
    JOB_CREATE,            // 创建
    JOB_LOAD,              // 加载
    JOB_COPY,              // 复制
    JOB_BATCHEXTRACT,      // 批量解压
    JOB_EXTRACT,           // 解压
    JOB_TEMPEXTRACT,       // 临时解压
    JOB_MOVE,              // 移动
    JOB_COMMENT,
    JOB_BATCHCOMPRESS,     // 批量压缩
};






/**
 * this can help us to get the map of all mainwindow created.
 * @brief The GlobalMainWindowMap struct
 */
struct GlobalMainWindowMap {
public:
    void insert(const QString &strWinId, DMainWindow *wnd)
    {
        if (this->mMapGlobal.contains(strWinId) == false) {
            this->mMapGlobal.insert(strWinId, wnd);
        }
    }

    DMainWindow *getOne(const QString &strWinId)
    {
        if (this->mMapGlobal.contains(strWinId) == false) {
            return nullptr;
        } else {
            return this->mMapGlobal[strWinId];
        }
    }

    void remove(const QString &strWinId)
    {
        if (this->mMapGlobal.contains(strWinId) == true) {
            this->mMapGlobal.remove(strWinId);
        }
    }

    void clear()
    {
        this->mMapGlobal.clear();
    }

    /**
     * @brief mMapGlobal
     * @ key: winId
     * @ value: pointer of mainWindow
     */
    QMap<QString, DMainWindow *> mMapGlobal = {};
};


struct OpenInfo {
    enum ENUM_OPTION {
        CLOSE = 0,              //正常关闭
        OPEN = 1,               //打开
        QUERY_CLOSE_CANCEL = 2  //询问后，关闭取消
    };

    // 逻辑子窗口的WinId
    QString strWinId = "";
    // 逻辑子窗口的状态
    ENUM_OPTION option = OPEN;
    // 逻辑子窗口的job
    //KJob *pJob = nullptr;
};


/**
 * @brief The MainWindow_AuxInfo struct
 * @see 存放MainWindow的重要辅助信息
 */
struct MainWindow_AuxInfo {

    /**
     * @brief infomation
     * @see节点详情
     * @ key :strModexIndex,see as modelIndexToStr()
     * @ value :the pointer of open info
     */
    QMap<QString, OpenInfo *> information;

    /**
     * @brief parentAuxInfo
     * @see 逻辑父面板辅助信息节点
     */
    MainWindow_AuxInfo *parentAuxInfo = nullptr;
};




#endif // CUSTOMDATAINFO_H
