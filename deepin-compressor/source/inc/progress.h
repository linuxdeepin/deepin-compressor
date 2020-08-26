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
#include <DSuggestButton>
#include <DLabel>
#include <DProgressBar>

#include <QTimer>

DWIDGET_USE_NAMESPACE

//enum COMPRESS_TYPE {
//    COMPRESSING,//正常压缩
//    COMPRESSDRAGADD,//拖拽添加压缩
//    DECOMPRESSING,
//    DELETEING
//};


class CustomPushButton;
class ProgressAssistant;
class Progress: public DWidget
{
    Q_OBJECT
public:
    /**
     * @brief The ENUM_OPERATION enum
     * @see 进度的类型
     */
    enum ENUM_PROGRESS_TYPE {
        OP_NONE,            // 非进度界面
        OP_COMPRESSING,     // 正常压缩进度
        OP_COMPRESSDRAGADD, // 拖拽添加压缩进度
        OP_DECOMPRESSING,   // 解压缩进度
        OP_DELETEING,       // 删除进度
        OP_CONVERT          // 转换格式进度
    };

    Progress(DWidget *parent = nullptr);
    void InitUI();
    void InitConnection();

    void setprogress(double percent);
    void setFilename(QString filename);
    void setProgressFilename(QString filename);
    void settype(Progress::ENUM_PROGRESS_TYPE type);
    Progress::ENUM_PROGRESS_TYPE getType();
    void setopentype(bool type);
    bool getOpenType();
    void setTypeImage(QString type);
    DPushButton *getCancelbutton();

    int showConfirmDialog();
    void resetProgress();

    void setSpeedAndTime(double speed, qint64 timeLeft);
    void displaySpeedAndTime(double speed, qint64 timeLeft);
    void setSpeedAndTimeText(Progress::ENUM_PROGRESS_TYPE type);

    void setTempProgress();
    void refreshSpeedAndTime(unsigned long compressPercent, bool isConvert = false);
    ProgressAssistant *pInfo();
    void resetPauseContinueButton();

    /**
     * @brief 控制暂停继续按钮显示与隐藏
     * 暂停继续功能暂时支持压缩、解压
     * 故其他进度界面不显示该按钮
     */
    void hidePauseContinueButton();

signals:
    void sigCancelPressed(Progress::ENUM_PROGRESS_TYPE compressType);
    void sigPauseProcess();
    void sigContinueProcess();
//    void sigTypeChange(Progress::ENUM_PROGRESS_TYPE type);

public slots:
    void cancelbuttonPressedSlot();
    void pauseContinueButtonPressedSlot(bool checked);
    void slotChangeTimeLeft();

private:
    DPushButton *m_cancelbutton;
    DSuggestButton *m_PauseContinueButton; //暂停继续按钮
    QPixmap m_compressicon;
    DLabel *m_pixmaplabel;
    DLabel *m_filenamelabel;
    DProgressBar *m_progressbar;
    DLabel *m_progressfilelabel;
    QString m_progressfile;
    DLabel *m_shadow;

    QString m_filename;
    ENUM_PROGRESS_TYPE m_ProgressType;

    //add
    DLabel *m_speedLabel = nullptr;
    DLabel *m_restTimeLabel = nullptr;

    QTimer *m_timerTime;
//    QTimer *m_timerProgress;
    qint64 lastTimeLeft = 0;
    double m_speed = 0;
    double m_percent = 0;
    bool m_openType = false;
    ProgressAssistant *m_pInfo = nullptr;
};

#endif // PROGRESS_H
