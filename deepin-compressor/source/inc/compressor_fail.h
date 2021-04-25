/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#ifndef COMPRESSOR_FAIL_H
#define COMPRESSOR_FAIL_H


#include <DWidget>
#include <DPushButton>
#include <DLabel>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE
class CustomPushButton;
class CustomCommandLinkButton;
/**
 * @brief The Compressor_Fail class 压缩失败展示界面
 */
class Compressor_Fail: public DWidget
{
    Q_OBJECT
public:
    Compressor_Fail(QWidget *parent = nullptr);
    ~Compressor_Fail();
    /**
     * @brief InitUI 初始化界面信息
     */
    void InitUI();
    /**
     * @brief InitConnection 初始化链接信息
     */
    void InitConnection();
    /**
     * @brief setFailStr 设置显示失败之后的提示问题标题
     * @param str   标题信息
     */
    void setFailStr(const QString &str);
    /**
     * @brief setFailStrDetail 设置显示失败之后的详细信息
     * @param str     详细信息内容
     */
    void setFailStrDetail(const QString &str);
    //  CustomPushButton *getRetrybutton();

    /**
     * @brief setErrorTitle 设置错误标题
     * @param strTitle      标题
     */
    void setErrorTitle(const QString &strTitle);

private:
    CustomPushButton *m_retrybutton; // 重试按钮
    QPixmap m_compressicon; // 图标
    DLabel *m_pixmaplabel;
    DLabel *m_stringinfolabel; // 展示错误标题控件
    QString m_stringinfo; // 展示错误标题的信息

    DLabel *m_stringdetaillabel; // 展示错误详细信息的控件
    QString m_stringdetail; // 展示错误详细信息的内容

    CustomCommandLinkButton *commandLinkBackButton = nullptr;

signals:
    void sigBackButtonClickedOnFail();
    void sigFailRetry();

public slots:
    void commandLinkBackButtonClicked();

};


#endif // COMPRESSOR_FAIL_H
