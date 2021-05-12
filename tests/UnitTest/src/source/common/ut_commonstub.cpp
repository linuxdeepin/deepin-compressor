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
#include "uitools.h"
#include "processopenthread.h"
#include "openwithdialog.h"

#include <QFileInfo>
#include <DFileDialog>
#include <DMenu>

DWIDGET_USE_NAMESPACE

bool g_QWidget_isVisible_result = false;;               // QWidget isVisible返回值
bool g_UiTools_isArchiveFile_result = false;            // UiTools isArchiveFile返回值
QObject *g_QObject_sender_result = nullptr;             // QObject sender返回值

int g_TipDialog_showDialog_result = 0;                  // TipDialog showDialog返回值
int g_SimpleQueryDialog_showDialog_result = 0;          // SimpleQueryDialog showDialog返回值
Overwrite_Result g_OverwriteQueryDialog_getDialogResult_result = OR_Cancel;  // OverwriteQueryDialog getDialogResult返回值
bool g_OverwriteQueryDialog_getApplyAll_result = false; // OverwriteQueryDialog getApplyAll返回值
int g_AppendDialog_showDialog_result = 0;               // AppendDialog showDialog返回值
QString g_OpenWithDialog_showOpenWithDialog_result = "";  // OpenWithDialog showOpenWithDialog返回值

QString g_QFileInfo_path_result = "";                   // QFileInfo path返回值
QString g_QFileInfo_filePath_result = "";               // QFileInfo filePath返回值
QString g_QFileInfo_fileName_result = "";               // QFileInfo fileName返回值
QString g_QFileInfo_completeBaseName_result = "";       // QFileInfo completeBaseName返回值
qint64 g_QFileInfo_size_result = 0;                     // QFileInfo size返回值
bool g_QFileInfo_isDir_result = false;                  // QFileInfo isDir返回值
bool g_QFileInfo_exists_result = false;                 // QFileInfo exists返回值
bool g_QFileInfo_isWritable_result = false;             // QFileInfo isWritable返回值
bool g_QFileInfo_isExecutable_result = false;           // QFileInfo isExecutable返回值
bool g_QFileInfo_isReadable_result = false;             // QFileInfo isReadable返回值
bool g_QFileInfo_isSymLink_result = false;              // QFileInfo isSymLink返回值

bool g_QFile_remove_result = false;                     // QFile remove返回值
bool g_QFile_open_result = false;                     // QFile open返回值
bool g_QFile_close_result = false;                     // QFile close返回值
QByteArray g_QFile_readAll_result;                     // QFile readAll返回值

bool g_QDir_exists_result = false;                      // QDir exists返回值
QFileInfoList g_QDir_entryInfoList_result = QFileInfoList();    // QDir entryInfoList返回值
QString g_QDir_filePath_result = "";                    // QDir filePath返回值

qint64 g_QElapsedTimer_elapsed_result = 0;              // QElapsedTimer elapsed返回值

int g_DFileDialog_exec_result = 0;                      // DFileDialog exec
QList<QUrl> g_DFileDialog_selectedUrls_result = QList<QUrl>();   // DFileDialog selectedUrls返回值

DGuiApplicationHelper::ColorType g_DGuiApplicationHelper_themeType_result;   // DGuiApplicationHelper themeType返回值

/*************************************CommonStub*************************************/
CommonStub::CommonStub()
{

}

CommonStub::~CommonStub()
{

}

int qWidget_isVisible_stub()
{
    return g_QWidget_isVisible_result;
}

QAction *menu_exec_stub(const QPoint &, QAction *)
{
    return nullptr;
}

void CommonStub::stub_QWidget_isVisible(Stub &stub, bool isVisible)
{
    g_QWidget_isVisible_result = isVisible;
    stub.set(ADDR(QWidget, isVisible), qWidget_isVisible_stub);
}

QModelIndex treeView_indexAt_stub(void *obj, const QPoint &p)
{
    QTreeView *o = (QTreeView *)obj;
    if (o) {
        return o->model()->index(0, 0);
    }
    return QModelIndex();
}

bool uiTools_isArchiveFile_stub(const QString &)
{
    return g_UiTools_isArchiveFile_result;
}

void processOpenThread_start_stub()
{
    return;
}

QObject *qObject_sender_stub()
{
    return g_QObject_sender_result;
}

void CommonStub::stub_QMenu_exec(Stub &stub)
{
    stub.set((QAction * (DMenu::*)(const QPoint &, QAction * at))ADDR(DMenu, exec), menu_exec_stub);
}

void CommonStub::stub_QTreeView_indexAt(Stub &stub)
{
    typedef QModelIndex(*fptr)(QTreeView *, int);
    fptr A_foo = (fptr)(&QTreeView::indexAt);   //获取虚函数地址
    stub.set(A_foo, treeView_indexAt_stub);
}

void CommonStub::stub_UiTools_isArchiveFile(Stub &stub, bool isArchiveFile)
{
    g_UiTools_isArchiveFile_result = isArchiveFile;
    stub.set(ADDR(UiTools, isArchiveFile), uiTools_isArchiveFile_stub);
}

void CommonStub::stub_ProcessOpenThread_start(Stub &stub)
{
    stub.set(ADDR(ProcessOpenThread, start), processOpenThread_start_stub);
}

void CommonStub::stub_QObject_sender(Stub &stub, QObject *pObject)
{
    g_QObject_sender_result = pObject;
    stub.set(ADDR(QObject, sender), qObject_sender_stub);
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

void overwriteQueryDialog_showDialog_stub(QString file, bool bDir)
{
    Q_UNUSED(file)
    Q_UNUSED(bDir)
    return;
}

Overwrite_Result overwriteQueryDialog_getDialogResult_stub()
{
    return g_OverwriteQueryDialog_getDialogResult_result;
}

bool overwriteQueryDialog_getApplyAll_stub()
{
    return g_OverwriteQueryDialog_getApplyAll_result;
}

int appendDialog_showDialog_stub()
{
    return g_AppendDialog_showDialog_result;
}

QString openWithDialog_showOpenWithDialog_stub()
{
    return g_OpenWithDialog_showOpenWithDialog_result;
}

void openWithDialog_openWithProgram_stub(const QString &, const QString &)
{
    return;
}

void CustomDialogStub::stub_TipDialog_showDialog(Stub &stub, int iResult)
{
    g_TipDialog_showDialog_result = iResult;
    stub.set(ADDR(TipDialog, showDialog), tipDialog_showDialog_stub);
}

void CustomDialogStub::stub_SimpleQueryDialog_showDialog(Stub &stub, int iResult)
{
    g_SimpleQueryDialog_showDialog_result = iResult;
    stub.set(ADDR(SimpleQueryDialog, showDialog), simpleQueryDialog_showDialog_stub);
}

void CustomDialogStub::stub_OverwriteQueryDialog_showDialog(Stub &stub)
{
    stub.set(ADDR(OverwriteQueryDialog, showDialog), overwriteQueryDialog_showDialog_stub);
}

void CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(Stub &stub, Overwrite_Result iResult)
{
    g_OverwriteQueryDialog_getDialogResult_result = iResult;
    stub.set(ADDR(OverwriteQueryDialog, getDialogResult), overwriteQueryDialog_getDialogResult_stub);
}

void CustomDialogStub::stub_OverwriteQueryDialog_getApplyAll(Stub &stub, bool bApplyAll)
{
    g_OverwriteQueryDialog_getApplyAll_result = bApplyAll;
    stub.set(ADDR(OverwriteQueryDialog, getApplyAll), overwriteQueryDialog_getApplyAll_stub);
}

void CustomDialogStub::stub_AppendDialog_showDialog(Stub &stub, int iResult)
{
    g_AppendDialog_showDialog_result = iResult;
    stub.set(ADDR(AppendDialog, showDialog), appendDialog_showDialog_stub);
}

void CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(Stub &stub, const QString &strResult)
{
    g_OpenWithDialog_showOpenWithDialog_result = strResult;
    stub.set(ADDR(OpenWithDialog, showOpenWithDialog), openWithDialog_showOpenWithDialog_stub);
}

void CustomDialogStub::stub_OpenWithDialog_openWithProgram(Stub &stub)
{
    stub.set(ADDR(OpenWithDialog, openWithProgram), openWithDialog_openWithProgram_stub);
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

QString qfileinfo_filePath_stub()
{
    return g_QFileInfo_filePath_result;
}

QString qfileinfo_fileName_stub()
{
    return g_QFileInfo_fileName_result;
}

QString qfileinfo_completeBaseName_stub()
{
    return g_QFileInfo_completeBaseName_result;
}

qint64 qfileinfo_size_stub()
{
    return g_QFileInfo_size_result;
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

void QFileInfoStub::stub_QFileInfo_filePath(Stub &stub, const QString &strfilePath)
{
    g_QFileInfo_filePath_result = strfilePath;
    stub.set(ADDR(QFileInfo, filePath), qfileinfo_filePath_stub);
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

void QFileInfoStub::stub_QFileInfo_size(Stub &stub, const qint64 &size)
{
    g_QFileInfo_size_result = size;
    stub.set(ADDR(QFileInfo, size), qfileinfo_size_stub);
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

bool qfile_open_stub(QIODevice::OpenMode flags)
{
    Q_UNUSED(flags)

    return g_QFile_open_result;
}

bool qfile_close_stub()
{
    return g_QFile_close_result;
}

QByteArray qfile_readAll_stub()
{
    return g_QFile_readAll_result;
}

void QFileStub::stub_QFile_remove(Stub &stub, bool bResult)
{
    g_QFile_remove_result = bResult;
    typedef bool (QFile::*fptr)();
    fptr A_foo = (fptr)(&QFile::remove);   //获取虚函数地址
    stub.set(A_foo, qfile_remove_stub);
}

void QFileStub::stub_QFile_open(Stub &stub, bool bResult)
{
    g_QFile_open_result = bResult;
    typedef bool (*fptr)(QFile *, QIODevice::OpenMode);
    fptr A_foo = (fptr)((bool(QFile::*)(QIODevice::OpenMode))&QFile::open);   //获取虚函数地址
    stub.set(A_foo, qfile_open_stub);
}

void QFileStub::stub_QFile_close(Stub &stub, bool bResult)
{

    g_QFile_close_result = bResult;
    typedef bool (*fptr)(QFile *);
    fptr A_foo = (fptr)(&QFile::close);   //获取虚函数地址
    stub.set(A_foo, qfile_close_stub);
}

void QFileStub::stub_QFile_readAll(Stub &stub, const QByteArray &allByteArray)
{
    g_QFile_readAll_result = allByteArray;

    stub.set(ADDR(QFile, readAll), qfile_readAll_stub);
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


/*************************************DFileDialogStub*************************************/
DFileDialogStub::DFileDialogStub()
{

}

DFileDialogStub::~DFileDialogStub()
{

}

int dFileDialog_exec_stub()
{
    return g_DFileDialog_exec_result;
}

QList<QUrl> dFileDialog_selectedUrls_stub()
{
    return g_DFileDialog_selectedUrls_result;
}

void DFileDialogStub::stub_DFileDialog_exec(Stub &stub, int iResult)
{
    g_DFileDialog_exec_result = iResult;

    typedef int (*fptr)(DFileDialog *);
    fptr A_foo = (fptr)(&DFileDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dFileDialog_exec_stub);
}

void DFileDialogStub::stub_DFileDialog_selectedUrls(Stub &stub, const QList<QUrl> &listUrls)
{
    g_DFileDialog_selectedUrls_result = listUrls;
    stub.set(ADDR(DFileDialog, selectedUrls), dFileDialog_selectedUrls_stub);
}

QDirStub::QDirStub()
{

}

QDirStub::~QDirStub()
{

}

bool qdir_exists_stub()
{
    return g_QDir_exists_result;
}

QFileInfoList qdir_entryInfoList_stub(QDir::Filters filters, QDir::SortFlags sort)
{
    Q_UNUSED(filters)
    Q_UNUSED(sort)

    return g_QDir_entryInfoList_result;
}

QString qdir_filePath_stub(const QString &fileName)
{
    Q_UNUSED(fileName)

    return g_QDir_filePath_result;
}

void QDirStub::stub_QDir_exists(Stub &stub, bool isExists)
{
    g_QDir_exists_result = isExists;

    typedef bool (QDir::*fptr)()const ;
    fptr A_foo = (fptr)(&QDir::exists);   //获取虚函数地址
    stub.set(A_foo, qdir_exists_stub);
}

void QDirStub::stub_QDir_entryInfoList(Stub &stub, const QFileInfoList &entryInfoList)
{
    g_QDir_entryInfoList_result = entryInfoList;

    typedef QFileInfoList(QDir::*fptr)(QDir::Filters, QDir::SortFlags) const;
    fptr A_foo = (fptr)(&QDir::entryInfoList);   //获取虚函数地址
    stub.set(A_foo, qdir_entryInfoList_stub);
}

void QDirStub::stub_QDir_filePath(Stub &stub, const QString &strfilePath)
{
    g_QDir_filePath_result = strfilePath;

    stub.set(ADDR(QDir, filePath), qdir_filePath_stub);
}
