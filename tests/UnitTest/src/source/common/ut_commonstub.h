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

#include <QObject>
#include <QTreeView>

class CommonStub : public QObject
{
public:
    CommonStub();
    ~CommonStub();

    /**
     * @brief stub_QDialog_exec     针对QDialog的exec打桩
     * @param stub
     * @param iResult               期望返回值
     * @return
     */
    static void stub_QDialog_exec(Stub &stub, int iResult);


    /**
     * @brief stub_QMenu_exec     针对QMenu的exec打桩
     * @param stub
     * @return
     */
    static void stub_QMenu_exec(Stub &stub);


    /**
     * @brief stub_QTreeView_indexAt     针对QTreeView的indexAt打桩
     * @param stub
     * @return
     */
    static void stub_QTreeView_indexAt(Stub &stub);


    /**
     * @brief stub_TipDialog_showDialog     针对TipDialog的showDialog打桩
     * @param stub
     * @return
     */
    static void stub_TipDialog_showDialog(Stub &stub, int iResult);


    /**
     * @brief stub_ConvertDialog_showDialog     针对ConvertDialog的showDialog打桩
     * @param stub
     * @return
     */
    static void stub_ConvertDialog_showDialog(Stub &stub, QStringList listResult);


    /**
     * @brief stub_SimpleQueryDialog_showDialog     针对SimpleQueryDialog的showDialog打桩
     * @param stub
     * @return
     */
    static void stub_SimpleQueryDialog_showDialog(Stub &stub, int iResult);


    /**
     * @brief stub_OverwriteQueryDialog_showDialog     针对OverwriteQueryDialog的showDialog打桩
     * @param stub
     * @return
     */
    static void stub_OverwriteQueryDialog_showDialog(Stub &stub, int iReult, bool bApplyAll);


    /**
     * @brief stub_AppendDialog_showDialog     针对AppendDialog的showDialog打桩
     * @param stub
     * @return
     */
    static void stub_AppendDialog_showDialog(Stub &stub, int iResult);


    /**
     * @brief stub_SingleJob_start     针对SingleJob的start打桩
     * @param stub
     * @return
     */
    static void stub_SingleJob_start(Stub &stub);


    /**
     * @brief stub_ConvertJob_start     针对ConvertJob的start打桩
     * @param stub
     * @return
     */
    static void stub_ConvertJob_start(Stub &stub);


    /**
     * @brief stub_UpdateJob_start     针对UpdateJob的start打桩
     * @param stub
     * @return
     */
    static void stub_UpdateJob_start(Stub &stub);


    /**
     * @brief stub_SingleJob_doPause     针对SingleJob的doPause打桩
     * @param stub
     * @return
     */
    static void stub_SingleJob_doPause(Stub &stub);


    /**
     * @brief stub_SingleJob_doContinue     针对SingleJob的doContinue打桩
     * @param stub
     * @return
     */
    static void stub_SingleJob_doContinue(Stub &stub);


    /**
     * @brief stub_SingleJob_kill     针对SingleJob的kill打桩
     * @param stub
     * @return
     */
    static void stub_SingleJob_kill(Stub &stub);


    /**
     * @brief stub_LoadJob_doWork     针对LoadJob的doWork打桩
     * @param stub
     * @return
     */
    static void stub_LoadJob_doWork(Stub &stub);
};


#endif // UT_COMMONSTUB_H
