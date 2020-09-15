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
    void InitUI();
    void InitConnection();
    void setstringinfo(QString str);
    void setCompressPath(QString path);
    QString getPath();
    void setCompressFullPath(const QString &path);
    void setCompressNewFullPath(const QString &path);
    void setSpilitArchive(bool isSpilit);
    bool getSpilitArchive();
    void clear();
    void setConvertType(QString type);
    CustomPushButton *getShowfilebutton();

private:
    CustomPushButton *m_showfilebutton;
    QPixmap m_compressicon;
    DLabel *m_pixmaplabel;
    DLabel *m_stringinfolabel;
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
    void showfiledirSlot(bool iIsUrl = true);
    void commandLinkBackButtonClicked();

signals:
    void sigQuitApp();
};

#endif // COMPRESSOR_SUCCESS_H
