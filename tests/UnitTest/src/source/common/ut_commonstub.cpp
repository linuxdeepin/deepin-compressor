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
#include "ut_commonstub.h"
#include "popupdialog.h"
#include "singlejob.h"
#include "pluginmanager.h"

#include <QFileInfo>

DWIDGET_USE_NAMESPACE

int g_TipDialog_showDialog_result = 0;                  // TipDialog showDialog返回值
int g_SimpleQueryDialog_showDialog_result = 0;          // SimpleQueryDialog showDialog返回值

QString g_QFileInfo_path_result = "";                   // QFileInfo path返回值
QString g_QFileInfo_fileName_result = "";               // QFileInfo fileName返回值
QString g_QFileInfo_completeBaseName_result = "";       // QFileInfo completeBaseName返回值
bool g_QFileInfo_isDir_result = false;                  // QFileInfo isDir返回值
bool g_QFileInfo_exists_result = false;                 // QFileInfo exists返回值
bool g_QFileInfo_isWritable_result = false;             // QFileInfo isWritable返回值
bool g_QFileInfo_isExecutable_result = false;           // QFileInfo isExecutable返回值
bool g_QFileInfo_isReadable_result = false;             // QFileInfo isReadable返回值
bool g_QFileInfo_isSymLink_result = false;              // QFileInfo isSymLink返回值

bool g_QFile_remove_result = false;                     // QFile remove返回值

qint64 g_QElapsedTimer_elapsed_result = 0;              // QElapsedTimer elapsed返回值

DGuiApplicationHelper::ColorType g_DGuiApplicationHelper_themeType_result;   // DGuiApplicationHelper themeType返回值

/*************************************CommonStub*************************************/
CommonStub::CommonStub()
{

}

CommonStub::~CommonStub()
{

}


/*************************************CustomDialogStub*************************************/
CustomDialogStub::CustomDialogStub()
{

}

CustomDialogStub::~CustomDialogStub()
{

}

int tipDialog_showDialog_stub(const QString &strDesText, const QString btnMsg, DDialog::ButtonType btnType)
{
    Q_UNUSED(strDesText)
    Q_UNUSED(btnMsg)
    Q_UNUSED(btnType)
    return g_TipDialog_showDialog_result;
}

int simpleQueryDialog_showDialog_stub(const QString &strDesText, const QString btnMsg1, DDialog::ButtonType btnType1, const QString btnMsg2, DDialog::ButtonType btnType2, const QString btnMsg3, DDialog::ButtonType btnType3)
{
    Q_UNUSED(strDesText)
    Q_UNUSED(btnMsg1)
    Q_UNUSED(btnType1)
    Q_UNUSED(btnMsg2)
    Q_UNUSED(btnType2)
    Q_UNUSED(btnMsg3)
    Q_UNUSED(btnType3)
    return g_SimpleQueryDialog_showDialog_result;
}

void CustomDialogStub::stub_TipDialog_showDialog(Stub &stub, int iResult)
{
    g_TipDialog_showDialog_result = iResult;
    stub.set(ADDR(TipDialog, showDialog), tipDialog_showDialog_stub);
}

void CustomDialogStub::stub_SimpleQueryDialog_showDialog(Stub &stub, int iResult)
{
    g_SimpleQueryDialog_showDialog_result = iResult;
    stub.set(ADDR(TipDialog, showDialog), simpleQueryDialog_showDialog_stub);
}


/*************************************QFileInfoStub*************************************/
QFileInfoStub::QFileInfoStub()
{

}

QFileInfoStub::~QFileInfoStub()
{

}

QString qfileinfo_path_stub()
{
    return g_QFileInfo_path_result;
}

QString qfileinfo_fileName_stub()
{
    return g_QFileInfo_fileName_result;
}

QString qfileinfo_completeBaseName_stub()
{
    return g_QFileInfo_completeBaseName_result;
}

bool qfileinfo_isDir_stub()
{
    return g_QFileInfo_isDir_result;
}

bool qfileinfo_exists_stub()
{
    return g_QFileInfo_exists_result;
}

bool qfileinfo_isWritable_stub()
{
    return g_QFileInfo_isWritable_result;
}

bool qfileinfo_isExecutable_stub()
{
    return g_QFileInfo_isExecutable_result;
}

bool qfileinfo_isReadable_stub()
{
    return g_QFileInfo_isReadable_result;
}

bool qfileinfo_isSymLink_stub()
{
    return g_QFileInfo_isSymLink_result;
}


void QFileInfoStub::stub_QFileInfo_path(Stub &stub, const QString &strPath)
{
    g_QFileInfo_path_result = strPath;
    stub.set(ADDR(QFileInfo, path), qfileinfo_path_stub);
}

void QFileInfoStub::stub_QFileInfo_fileName(Stub &stub, const QString &strPath)
{
    g_QFileInfo_fileName_result = strPath;
    stub.set(ADDR(QFileInfo, fileName), qfileinfo_fileName_stub);
}

void QFileInfoStub::stub_QFileInfo_completeBaseName(Stub &stub, const QString &strPath)
{
    g_QFileInfo_completeBaseName_result = strPath;
    stub.set(ADDR(QFileInfo, completeBaseName), qfileinfo_completeBaseName_stub);
}

void QFileInfoStub::stub_QFileInfo_isDir(Stub &stub, bool isDir)
{
    g_QFileInfo_isDir_result = isDir;
    stub.set(ADDR(QFileInfo, isDir), qfileinfo_isDir_stub);
}

void QFileInfoStub::stub_QFileInfo_exists(Stub &stub, bool isExists)
{
    g_QFileInfo_exists_result = isExists;

    typedef bool (QFileInfo::*fptr)()const ;
    fptr A_foo = (fptr)(&QFileInfo::exists);   //获取虚函数地址
    stub.set(A_foo, qfileinfo_exists_stub);
}

void QFileInfoStub::stub_QFileInfo_isWritable(Stub &stub, bool isWritable)
{
    g_QFileInfo_isWritable_result = isWritable;
    stub.set(ADDR(QFileInfo, isWritable), qfileinfo_isWritable_stub);

}

void QFileInfoStub::stub_QFileInfo_isExecutable(Stub &stub, bool isExecutable)
{
    g_QFileInfo_isExecutable_result = isExecutable;
    stub.set(ADDR(QFileInfo, isExecutable), qfileinfo_isExecutable_stub);
}

void QFileInfoStub::stub_QFileInfo_isReadable(Stub &stub, bool isReadable)
{
    g_QFileInfo_isReadable_result = isReadable;
    stub.set(ADDR(QFileInfo, isReadable), qfileinfo_isReadable_stub);
}

void QFileInfoStub::stub_QFileInfo_isSymLink(Stub &stub, bool isSymLink)
{
    g_QFileInfo_isSymLink_result = isSymLink;
    stub.set(ADDR(QFileInfo, isSymLink), qfileinfo_isSymLink_stub);
}


/*************************************PluginManagerStub*************************************/
PluginManagerStub::PluginManagerStub()
{

}

PluginManagerStub::~PluginManagerStub()
{

}

QStringList pluginManager_supportedWriteMimeTypes_stub(PluginManager::MimeSortingMode mode)
{
    QStringList listMimeTypes = QStringList() << "application/zip" << "application/x-7z-compressed";
    return listMimeTypes;
}

void PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(Stub &stub)
{
    stub.set(ADDR(PluginManager, supportedWriteMimeTypes), pluginManager_supportedWriteMimeTypes_stub);
}


/*************************************PluginManagerStub*************************************/
QFileStub::QFileStub()
{

}

QFileStub::~QFileStub()
{

}

bool qfile_remove_stub()
{
    return g_QFile_remove_result;
}

void QFileStub::stub_QFile_remove(Stub &stub, bool bResult)
{
    g_QFile_remove_result = bResult;
    typedef bool (QFile::*fptr)();
    fptr A_foo = (fptr)(&QFile::remove);   //获取虚函数地址
    stub.set(A_foo, qfile_remove_stub);
}


/*************************************DGuiApplicationHelperStub*************************************/
DGuiApplicationHelperStub::DGuiApplicationHelperStub()
{

}

DGuiApplicationHelperStub::~DGuiApplicationHelperStub()
{

}

DGuiApplicationHelper::ColorType dGuiApplicationHelper_themeType_stub()
{
    return g_DGuiApplicationHelper_themeType_result;
}

void DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(Stub &stub, DGuiApplicationHelper::ColorType themeType)
{
    g_DGuiApplicationHelper_themeType_result = themeType;
    stub.set(ADDR(DGuiApplicationHelper, themeType), dGuiApplicationHelper_themeType_stub);
}


/*************************************DGuiApplicationHelperStub*************************************/
QElapsedTimerStub::QElapsedTimerStub()
{

}

QElapsedTimerStub::~QElapsedTimerStub()
{

}

void qElapsedTimer_start_stub()
{
    return;
}

void qElapsedTimer_restart_stub()
{
    return;
}

qint64 qElapsedTimer_elapsed_stub()
{
    return g_QElapsedTimer_elapsed_result;
}

void QElapsedTimerStub::stub_QElapsedTimer_start(Stub &stub)
{
    stub.set(ADDR(QElapsedTimer, start), qElapsedTimer_start_stub);
}

void QElapsedTimerStub::stub_QElapsedTimer_restart(Stub &stub)
{
    stub.set(ADDR(QElapsedTimer, restart), qElapsedTimer_restart_stub);
}

void QElapsedTimerStub::stub_QElapsedTimer_elapsed(Stub &stub, qint64 qTime)
{
    g_QElapsedTimer_elapsed_result = qTime;
    stub.set(ADDR(QElapsedTimer, elapsed), qElapsedTimer_elapsed_stub);
}
/*************************************DGuiApplicationHelperStub*************************************/

