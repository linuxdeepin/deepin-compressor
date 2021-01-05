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

#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class CustomCommandLinkButton;
class QVBoxLayout;
class QSettings;
class QShortcut;

// 首页
class HomePage : public DWidget
{
    Q_OBJECT

public:
    HomePage(QWidget *parent = nullptr);
    ~HomePage() override;


private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

protected:
    /**
     * @brief dragEnterEvent    拖拽进入
     */
    void dragEnterEvent(QDragEnterEvent *) override;

    /**
     * @brief dragMoveEvent     拖拽移动
     */
    void dragMoveEvent(QDragMoveEvent *) override;

    /**
     * @brief dropEvent 拖拽放下
     */
    void dropEvent(QDropEvent *) override;

signals:
    /**
     * @brief signalFileChoose  选择文件信号
     */
    void signalFileChoose();

    /**
     * @brief signalDragFiles   拖拽添加文件信号
     * @param listFiles 拖拽文件
     */
    void signalDragFiles(const QStringList &listFiles);

private slots:
    /**
     * @brief slotThemeChanged  系统主题变化
     */
    void slotThemeChanged();

private:
    QVBoxLayout *m_pLayout;     // 界面布局
    QPixmap m_pixmap;           // 界面图标
    DLabel *m_pIconLbl;        // 图标
    DLabel *m_pTipLbl;        // 提示语
    DLabel *m_pSplitLbl;       // 分割线
    CustomCommandLinkButton *m_pChooseBtn;     // 选择文件按钮
    QSettings *m_pSettings;     // 配置信息
};

#endif
