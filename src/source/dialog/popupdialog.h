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

#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <DDialog>
#include <DLabel>

DWIDGET_USE_NAMESPACE

// 提示性对话框（描述 + 确定按钮）
class TipDialog : public DDialog
{
    Q_OBJECT

public:
    explicit TipDialog(QWidget *parent = nullptr);
    ~TipDialog() override;

    /**
     * @brief showDialog    显示对话框
     * @param strDesText    描述内容
     * @param btnMsg        按钮内容
     * @param btnType       按钮类型
     * @return              操作返回值
     */
    int showDialog(const QString &strDesText = "", const QString btnMsg = "", ButtonType btnType = ButtonNormal);
};


// 简单询问对话框（描述 + 多个按钮）
class SimpleQueryDialog : public DDialog
{
    Q_OBJECT

public:
    explicit SimpleQueryDialog(QWidget *parent = nullptr);
    ~SimpleQueryDialog() override;

    /**
     * @brief showDialog    显示对话框
     * @param strDesText    描述内容
     * @param btnMsg1       第一个按钮内容
     * @param btnType1      第一个按钮类型
     * @param btnMsg2       第二个按钮内容
     * @param btnType2      第二个按钮类型
     * @param btnMsg3       第三个按钮内容
     * @param btnType3      第三个按钮类型
     * @return
     */
    int showDialog(const QString &strDesText = "", const QString btnMsg1 = "", ButtonType btnType1 = ButtonNormal,
                   const QString btnMsg2 = "", ButtonType btnType2 = ButtonNormal,
                   const QString btnMsg3 = "", ButtonType btnType3 = ButtonNormal);
};

enum Overwrite_Result {
    OR_Cancel = 0,
    OR_Skip = 1,
    OR_SkipAll = 2,
    OR_Overwrite = 3,
    OR_OverwriteAll = 4,
};

class OverwriteQueryDialog : public DDialog
{
    Q_OBJECT

public:
    explicit OverwriteQueryDialog(QWidget *parent = nullptr);
    ~OverwriteQueryDialog() override;

    void showDialog(QString file);
    int getDialogResult();
    int getQueryResult();
    bool getApplyAll();

private:
    int m_ret;
    int m_retType;
    bool m_applyAll;
};

#endif // POPUPDIALOG_H
