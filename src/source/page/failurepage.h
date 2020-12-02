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

#ifndef FAILUREPAGE_H
#define FAILUREPAGE_H

#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class CustomPushButton;
class CustomCommandLinkButton;

// 失败界面
class FailurePage : public DWidget
{
    Q_OBJECT
public:
    explicit FailurePage(QWidget *parent = nullptr);
    ~FailurePage() override;

    /**
     * @brief setFailuerDes     设置失败描述
     * @param strDes            描述内容
     */
    void setFailuerDes(const QString &strDes);

    /**
     * @brief setFailureDetail  设置失败先详细信息
     * @param strDetail         失败详细信息
     */
    void setFailureDetail(const QString &strDetail);

    /**
     * @brief setRetryEnable    设置重试按钮是否可用
     * @param bEnable           是否可用
     */
    void setRetryEnable(bool bEnable);

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

private:
    DLabel *m_pFailurePixmapLbl; //失败图片显示
    DLabel *m_pFailureLbl; //失败文字显示
    DLabel *m_pDetailLbl; //错误原因
    CustomPushButton *m_pRetrybutton; // 重试按钮
    CustomCommandLinkButton *commandLinkBackButton; //返回按钮

signals:
    void sigBackButtonClickedOnFail();
    void sigFailRetry();
};

#endif // FAILUREPAGE_H
