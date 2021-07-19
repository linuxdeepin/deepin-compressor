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

    struct NewStr {
        QStringList strList;
        QString resultStr;
        int fontHeifht = 0;
    };

    /**
     * @brief showDialog    显示对话框
     * @param strDesText    描述内容
     * @param btnMsg        按钮内容
     * @param btnType       按钮类型
     * @param strToolTip    提示信息
     * @return              操作返回值
     */
    int showDialog(const QString &strDesText = "", const QString btnMsg = "", ButtonType btnType = ButtonNormal, const QString &strToolTip = "");

    NewStr autoCutText(const QString &text, DLabel *pDesLbl);

    /**
     * @brief autoFeed 自动换行
     * @param label
     */
    void autoFeed(DLabel *label);

protected:
    void changeEvent(QEvent *event) override;

private:
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iDialogOldHeight = 0;
};

#endif // POPUPDIALOG_H
