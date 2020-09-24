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
#ifndef COMPRESSOR_SUCCESS_H
#define COMPRESSOR_SUCCESS_H

#include <DWidget>
#include <DPushButton>
#include <DLabel>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class CustomPushButton;
class CustomCommandLinkButton;

class Compressor_Success: public DWidget
{
    Q_OBJECT
public:
    Compressor_Success(QWidget *parent = nullptr);
    ~Compressor_Success();
    /**
     * @brief InitUI 初始化界面信息
     */
    void InitUI();
    /**
     * @brief InitConnection 初始化信息绑定
     */
    void InitConnection();
    /**
     * @brief setstringinfo 设置成功信息
     * @param str
     */
    void setstringinfo(QString str);
    /**
     * @brief setCompressPath
     * @param path
     */
    void setCompressPath(QString path);
    QString getPath();
    /**
     * @brief setCompressFullPath
     * @param path
     */
    void setCompressFullPath(const QString &path);
    void setCompressNewFullPath(const QString &path);
    void setSpilitArchive(bool isSpilit);
    bool getSpilitArchive();
    /**
     * @brief clear 清空数据
     */
    void clear();

    void setConvertType(QString type);
    //CustomPushButton *getShowfilebutton();

private:
    CustomPushButton *m_showfilebutton;
    /**
     * @brief m_compressicon    图标
     */
    QPixmap m_compressicon;
    DLabel *m_pixmaplabel;
    /**
     * @brief m_stringinfolabel 信息展示控件
     */
    DLabel *m_stringinfolabel;
    /**
     * @brief m_stringinfo 外部获取需要展示的字符串
     */
    QString m_stringinfo;
    QString m_path;
    QString m_fullpath;
    QString newCreatePath_;
    CustomCommandLinkButton *commandLinkBackButton = nullptr;
    QString m_convertType;
    bool m_isSpilitArchive = false;

signals:
    void sigBackButtonClicked();
    void sigOpenConvertArchive(QString path);

public slots:
    /**
     * @brief showfiledirSlot 显示文件目录，点击会弹出包或者目录的当前目录位置。
     * @param iIsUrl
     */
    void showfiledirSlot(bool iIsUrl = true);
    void commandLinkBackButtonClicked();

signals:
    void sigQuitApp();
};

#endif // COMPRESSOR_SUCCESS_H
