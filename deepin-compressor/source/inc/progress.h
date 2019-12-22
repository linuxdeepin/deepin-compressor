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
#ifndef PROGRESS_H
#define PROGRESS_H

#include <DWidget>
#include <DFileDialog>
#include <DPushButton>
#include <DLabel>
#include "utils.h"
#include <DProgressBar>
#include <DPalette>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE

enum COMPRESS_TYPE {
    COMPRESSING,
    DECOMPRESSING,
};

class Progress: public DWidget
{
    Q_OBJECT
public:
    Progress(QWidget *parent = nullptr);
    void InitUI();
    void InitConnection();
    void setprogress(uint percent);
    void setFilename(QString filename);
    void setProgressFilename(QString filename);
    void settype(COMPRESS_TYPE type);
    void setTypeImage(QString type);

    int showConfirmDialog();
private:
    DPushButton *m_cancelbutton;
    QPixmap m_compressicon;
    DLabel *m_pixmaplabel;
    DLabel *m_filenamelabel;
    DProgressBar *m_progressbar;
    DLabel *m_progressfilelabel;
    QString m_progressfile;
    DLabel *m_shadow;

    QString m_filename;
    COMPRESS_TYPE m_type;
signals:
    void  sigCancelPressed();

public slots:
    void cancelbuttonPressedSlot();
};

#endif // PROGRESS_H
