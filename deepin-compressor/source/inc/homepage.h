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

#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <DWidget>
#include <DLabel>

#include <QVBoxLayout>
#include <QSettings>

DWIDGET_USE_NAMESPACE

class CustomCommandLinkButton;
/**
 * @brief The HomePage class 主页
 */
class HomePage : public DWidget
{
    Q_OBJECT

public:
    HomePage(QWidget *parent = nullptr);
    ~HomePage();
    void setIconPixmap(bool isLoaded);
    CustomCommandLinkButton *getChooseBtn();

signals:
    /**
     * @brief fileSelected 文件选择信号
     * @param files
     */
    void fileSelected(const QStringList files) const;

private slots:
    /**
     * @brief themeChanged 主题改变
     */
    void themeChanged();

private:
    /**
     * @brief onChooseBtnClicked 响应选择文件按钮的槽函数
     */
    void onChooseBtnClicked();

private:
    QVBoxLayout *m_layout;
    QPixmap m_unloadPixmap;
    QPixmap m_loadedPixmap;
    DLabel *m_iconLabel;
    DLabel *m_tipsLabel;
    DLabel *m_splitLine;
    CustomCommandLinkButton *m_chooseBtn;
    QSettings *m_settings;
};

#endif
