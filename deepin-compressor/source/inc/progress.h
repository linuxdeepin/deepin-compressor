/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
 *             chenglu <chenglu@uniontech.com>
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
#include <DPushButton>
#include <DLabel>
#include <DProgressBar>
#include <QTimer>

DWIDGET_USE_NAMESPACE

enum COMPRESS_TYPE {
    COMPRESSING,
    DECOMPRESSING,
};

class Progress: public DWidget
{
    Q_OBJECT
public:
    Progress(DWidget *parent = nullptr);
    void InitUI();
    void InitConnection();

    void setprogress(int percent);
    void setFilename(QString filename);
    void setProgressFilename(QString filename);
    void settype(COMPRESS_TYPE type);
    void setTypeImage(QString type);

    int showConfirmDialog();
    void resetProgress();

    void setSpeedAndTime(double speed, qint64 timeLeft);
    void displaySpeedAndTime(double speed, qint64 timeLeft);

    void setSpeedAndTimeText(COMPRESS_TYPE type);


signals:
    void  sigCancelPressed(int compressType);

public slots:
    void cancelbuttonPressedSlot();
    void slotChangeTimeLeft();

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

    //add
    DLabel *m_speedLabel = nullptr;
    DLabel *m_restTimeLabel = nullptr;

    QTimer *m_timer;
    qint64 lastTimeLeft = 0;
    double m_speed = 0;
};

#endif // PROGRESS_H
