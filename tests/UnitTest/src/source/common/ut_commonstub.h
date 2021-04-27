/*
* Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
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
#ifndef UT_COMMONSTUB_H
#define UT_COMMONSTUB_H

#include "gtest/src/stub.h"

#include <DGuiApplicationHelper>

#include <QObject>
#include <QTreeView>


DGUI_USE_NAMESPACE

extern int g_TipDialog_showDialog_result;               // TipDialog showDialog返回值
extern int g_SimpleQueryDialog_showDialog_result;       // SimpleQueryDialog showDialog返回值

extern QString g_QFileInfo_path_result;                 // QFileInfo path返回值
extern QString g_QFileInfo_fileName_result;             // QFileInfo fileName返回值
extern QString g_QFileInfo_completeBaseName_result;     // QFileInfo completeBaseName返回值
extern bool g_QFileInfo_isDir_result;                   // QFileInfo isDir返回值
extern bool g_QFileInfo_exists_result;                  // QFileInfo exists返回值
extern bool g_QFileInfo_isWritable_result;              // QFileInfo isWritable返回值
extern bool g_QFileInfo_isExecutable_result;            // QFileInfo isExecutable返回值
extern bool g_QFileInfo_isReadable_result;              // QFileInfo isExecutable返回值
extern bool g_QFileInfo_isSymLink_result;               // QFileInfo isSymLink返回值

extern bool g_QFile_remove_result;                      // QFile remove返回值

extern qint64 g_QElapsedTimer_elapsed_result;           // QElapsedTimer elapsed返回值

extern DGuiApplicationHelper::ColorType g_DGuiApplicationHelper_themeType_result;   // DGuiApplicationHelper themeType返回值

// 通用打桩
class CommonStub : public QObject
{
public:
    CommonStub();
    ~CommonStub();
};


// 针对自定义Dialog的打桩
class CustomDialogStub : public QObject
{
public:
    CustomDialogStub();
    ~CustomDialogStub();

    /**
     * @brief stub_TipDialog_showDialog     针对TipDialog的showDialog打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_TipDialog_showDialog(Stub &stub, int iResult);

    /**
     * @brief stub_SimpleQueryDialog_showDialog     针对SimpleQueryDialog的showDialog打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_SimpleQueryDialog_showDialog(Stub &stub, int iResult);
};


// 针对QFileInfo的打桩
class QFileInfoStub : public QObject
{
public:
    QFileInfoStub();
    ~QFileInfoStub();


    /**
     * @brief stub_QFileInfo_path     针对QFileInfo的path打桩
     * @param stub
     * @param strPath
     * @return
     */
    static void stub_QFileInfo_path(Stub &stub, const QString &strPath);

    /**
     * @brief stub_QFileInfo_fileName     针对QFileInfo的fileName打桩
     * @param stub
     * @param strPath
     * @return
     */
    static void stub_QFileInfo_fileName(Stub &stub, const QString &strPath);

    /**
     * @brief stub_QFileInfo_completeBaseName     针对QFileInfo的completeBaseName打桩
     * @param stub
     * @param strPath
     * @return
     */
    static void stub_QFileInfo_completeBaseName(Stub &stub, const QString &strPath);

    /**
     * @brief stub_QFileInfo_isDir     针对QFileInfo的isDir打桩
     * @param stub
     * @param isDir
     * @return
     */
    static void stub_QFileInfo_isDir(Stub &stub, bool isDir);

    /**
     * @brief stub_QFileInfo_exists     针对QFileInfo的exists打桩
     * @param stub
     * @param isExists
     * @return
     */
    static void stub_QFileInfo_exists(Stub &stub, bool isExists);

    /**
     * @brief stub_QFileInfo_isWritable     针对QFileInfo的isWritable打桩
     * @param stub
     * @param isWritable
     * @return
     */
    static void stub_QFileInfo_isWritable(Stub &stub, bool isWritable);

    /**
     * @brief stub_QFileInfo_isExecutable     针对QFileInfo的isExecutable打桩
     * @param stub
     * @param isExecutable
     * @return
     */
    static void stub_QFileInfo_isExecutable(Stub &stub, bool isExecutable);

    /**
     * @brief stub_QFileInfo_isReadable     针对QFileInfo的isReadable打桩
     * @param stub
     * @param isReadable
     * @return
     */
    static void stub_QFileInfo_isReadable(Stub &stub, bool isReadable);

    /**
     * @brief stub_QFileInfo_isSymLink     针对QFileInfo的isSymLink打桩
     * @param stub
     * @param isSymLink
     * @return
     */
    static void stub_QFileInfo_isSymLink(Stub &stub, bool isSymLink);


};

// 针对PluginManager的打桩
class PluginManagerStub : public QObject
{
public:
    PluginManagerStub();
    ~PluginManagerStub();

    /**
     * @brief stub_PluginManager_supportedWriteMimeTypes     针对PluginManager的supportedWriteMimeTypes打桩
     * @param stub
     * @return
     */
    static void stub_PluginManager_supportedWriteMimeTypes(Stub &stub);
};


// 针对QFile的打桩
class QFileStub : public QObject
{
public:
    QFileStub();
    ~QFileStub();

    /**
     * @brief stub_QFile_remove     针对QFile的remove打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_QFile_remove(Stub &stub, bool bResult);
};


// 针对DGuiApplicationHelper的打桩
class DGuiApplicationHelperStub : public QObject
{
public:
    DGuiApplicationHelperStub();
    ~DGuiApplicationHelperStub();

    /**
     * @brief stub_DGuiApplicationHelper_themeType     针对DGuiApplicationHelper的themeType打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_DGuiApplicationHelper_themeType(Stub &stub, DGuiApplicationHelper::ColorType themeType);
};


// 针对QElapsedTimer的打桩
class QElapsedTimerStub : public QObject
{
public:
    QElapsedTimerStub();
    ~QElapsedTimerStub();

    /**
     * @brief stub_QElapsedTimer_start     针对QElapsedTimer的start打桩
     * @param stub
     * @return
     */
    static void stub_QElapsedTimer_start(Stub &stub);

    /**
     * @brief stub_QElapsedTimer_restart     针对QElapsedTimer的restart打桩
     * @param stub
     * @return
     */
    static void stub_QElapsedTimer_restart(Stub &stub);

    /**
     * @brief stub_QElapsedTimer_elapsed     针对QElapsedTimer的elapsed打桩
     * @param stub
     * @return
     */
    static void stub_QElapsedTimer_elapsed(Stub &stub, qint64 qTime);
};


#endif // UT_COMMONSTUB_H
