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

#ifndef SUCCESSPAGE_H
#define SUCCESSPAGE_H

#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class CustomPushButton;
class CustomCommandLinkButton;

// 成功界面
class SuccessPage : public DWidget
{
    Q_OBJECT
public:
    explicit SuccessPage(QWidget *parent = nullptr);
    ~SuccessPage() override;

    /**
     * @brief setCompressFullPath   设置压缩路径
     * @param strFullPath   压缩路径（含名称）
     */
    void setCompressFullPath(const QString &strFullPath);

    /**
     * @brief setDes    设置成功信息展示
     * @param strDes    描述内容
     */
    void setDes(const QString &strDes);

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

signals:
    void sigBackButtonClicked();

public slots:
    void slotShowfiledirSlot(bool iIsUrl = true);

private:
    DLabel *m_pSuccessPixmapLbl; //成功图片显示
    DLabel *m_pSuccessLbl;      // 成功文字显示
    CustomPushButton *m_pShowFileBtn; // 查看文件按钮
    CustomCommandLinkButton *m_pReturnBtn; // 返回按钮
    QString m_strFullPath;  // 压缩地址
};

#endif //SUCCESSPAGE_H
