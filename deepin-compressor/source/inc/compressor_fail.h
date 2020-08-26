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

class Compressor_Fail: public DWidget
{
    Q_OBJECT
public:
    Compressor_Fail(QWidget *parent = nullptr);
    void InitUI();
    void InitConnection();

    void setFailStr(const QString &str);
    void setFailStrDetail(const QString &str);

private:
    CustomPushButton *m_retrybutton;
    QPixmap m_compressicon;
    DLabel *m_pixmaplabel;
    DLabel *m_stringinfolabel;
    QString m_stringinfo;

    DLabel *m_stringdetaillabel;
    QString m_stringdetail;

    CustomCommandLinkButton *commandLinkBackButton = nullptr;

signals:
    void sigBackButtonClickedOnFail();
    void sigFailRetry();

public slots:
    void commandLinkBackButtonClicked();

};


#endif // COMPRESSOR_FAIL_H
