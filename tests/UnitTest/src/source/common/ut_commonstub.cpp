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

#include <DDialog>
#include <DMenu>

DWIDGET_USE_NAMESPACE

int g_diaolog_exec_result = 0;
int g_TipDialog_showDialog_result = 0;
QStringList g_ConvertDialog_showDialog_result;
int g_SimpleQueryDialog_showDialog_result = 0;
int g_OverwriteQueryDialog_showDialog_result = 0;   // 对话框状态
bool g_OverwriteQueryDialog_applyAll = false;  // 应用到全部文件
int g_AppendDialog_showDialog_result = 0;

CommonStub::CommonStub()
{

}

CommonStub::~CommonStub()
{

}

int diaolog_exec_stub()
{
    return g_diaolog_exec_result;
}

void CommonStub::stub_QDialog_exec(Stub &stub, int iResult)
{
    g_diaolog_exec_result = iResult;
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, diaolog_exec_stub);
}



QAction *menu_exec_stub(const QPoint &pos, QAction *at = nullptr)
{
    return nullptr;
}

void CommonStub::stub_QMenu_exec(Stub &stub)
{
    stub.set((QAction * (DMenu::*)(const QPoint &, QAction * at))ADDR(DMenu, exec), menu_exec_stub);
}



QModelIndex treeView_indexAt_stub(void *obj, const QPoint &p)
{
    QTreeView *o = (QTreeView *)obj;
    if (o) {
        return o->model()->index(0, 0);
    }
    return QModelIndex();
}

void CommonStub::stub_QTreeView_indexAt(Stub &stub)
{
    typedef QModelIndex(*fptr)(QTreeView *, int);
    fptr A_foo = (fptr)(&QTreeView::indexAt);   //获取虚函数地址
    stub.set(A_foo, treeView_indexAt_stub);
}


int tipDialog_showDialog_stub(const QString &strDesText, const QString btnMsg, DDialog::ButtonType btnType)
{
    Q_UNUSED(strDesText)
    Q_UNUSED(btnMsg)
    Q_UNUSED(btnType)
    return g_TipDialog_showDialog_result;
}

void CommonStub::stub_TipDialog_showDialog(Stub &stub, int iResult)
{
    g_TipDialog_showDialog_result = iResult;
    stub.set(ADDR(TipDialog, showDialog), tipDialog_showDialog_stub);
}


QStringList convertDialog_showDialog_stub()
{
    return g_ConvertDialog_showDialog_result;
}

void CommonStub::stub_ConvertDialog_showDialog(Stub &stub, QStringList listResult)
{
    g_ConvertDialog_showDialog_result = listResult;
    stub.set(ADDR(ConvertDialog, showDialog), convertDialog_showDialog_stub);
}


int simpleQueryDialog__showDialog_stub(const QString &strDesText, const QString btnMsg1, DDialog::ButtonType btnType1, const QString btnMsg2, DDialog::ButtonType btnType2, const QString btnMsg3, DDialog::ButtonType btnType3)
{
    return g_SimpleQueryDialog_showDialog_result;
}

void CommonStub::stub_SimpleQueryDialog_showDialog(Stub &stub, int iResult)
{
    g_SimpleQueryDialog_showDialog_result = iResult;
    stub.set(ADDR(SimpleQueryDialog, showDialog), simpleQueryDialog__showDialog_stub);
}


void overwriteshowDialog_showDialog_stub(QString file, bool bDir)
{
    return;
}

void CommonStub::stub_OverwriteQueryDialog_showDialog(Stub &stub, int iReult, bool bApplyAll)
{
    g_OverwriteQueryDialog_showDialog_result = iReult;
    g_OverwriteQueryDialog_applyAll = bApplyAll;
    stub.set(ADDR(OverwriteQueryDialog, showDialog), overwriteshowDialog_showDialog_stub);
}


int appenshowDialog_showDialog_stub(bool bMultiplePassword)
{
    return g_AppendDialog_showDialog_result;
}

void CommonStub::stub_AppendDialog_showDialog(Stub &stub, int iResult)
{
    g_AppendDialog_showDialog_result = iResult;
    stub.set(ADDR(AppendDialog, showDialog), appenshowDialog_showDialog_stub);

}


void job_start_stub()
{
    return;
}

void CommonStub::stub_SingleJob_start(Stub &stub)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void CommonStub::stub_ConvertJob_start(Stub &stub)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&ConvertJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void CommonStub::stub_UpdateJob_start(Stub &stub)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&UpdateJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}


void job_doPause_stub()
{
    return;
}

void CommonStub::stub_SingleJob_doPause(Stub &stub)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::doPause);   //获取虚函数地址
    stub.set(A_foo, job_doPause_stub);
}


void job_doContinue_stub()
{
    return;
}

void CommonStub::stub_SingleJob_doContinue(Stub &stub)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::doContinue);   //获取虚函数地址
    stub.set(A_foo, job_doContinue_stub);
}


void job_kill_stub()
{
    return;
}

void CommonStub::stub_SingleJob_kill(Stub &stub)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&SingleJob::kill);   //获取虚函数地址
    stub.set(A_foo, job_kill_stub);
}


void job_doWork_stub()
{
    return;
}

void CommonStub::stub_LoadJob_doWork(Stub &stub)
{
    typedef void (*fptr)();
    fptr A_foo = (fptr)(&LoadJob::doWork);   //获取虚函数地址
    stub.set(A_foo, job_doWork_stub);
}
