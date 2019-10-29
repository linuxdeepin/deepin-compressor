/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
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

#include <QWidget>
#include <DPushButton>
#include <DLabel>
#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class Compressor_Success: public QWidget
{
    Q_OBJECT
public:
    Compressor_Success(QWidget *parent = 0);
    void InitUI();
    void InitConnection();
    void setstringinfo(QString str);
    void setCompressPath(QString path);

private:
    DPushButton* m_showfilebutton;
    QPixmap m_compressicon;
    DLabel* m_pixmaplabel;
    DLabel* m_stringinfolabel;
    QString m_stringinfo;
    QString m_path;
public slots:
    void showfiledirSlot();

signals:
    void sigQuitApp();
};

#endif // COMPRESSOR_SUCCESS_H
