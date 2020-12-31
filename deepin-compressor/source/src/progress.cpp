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
#include "progress.h"
#include "mainwindow.h"
#include "utils.h"
#include "archiveinterface.h"
#include "structs.h"
#include "customwidget.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DDialog>

#include <QFileIconProvider>
#include <QGraphicsDropShadowEffect>
#include <QTimerEvent>
#include <QVBoxLayout>
#include <QDebug>

Progress::Progress(DWidget *parent)
    : DWidget(parent)
{
    // 初始化压缩包文件
    m_filename = "新建归档文件.rar";
    // 初始化展示进度的文件
    m_progressfile = "设计图111.jpg";
    // 初始化界面
    InitUI();
    // 初始化信号链接
    InitConnection();
    // 设置定时任务
    m_timerTime = new QTimer(this);
    m_timerTime->setInterval(1000);
    // 链接定时任务
    connect(m_timerTime, &QTimer::timeout, this, &Progress::slotChangeTimeLeft);
    m_pInfo = new ProgressAssistant(this);
//    m_timerProgress = new QTimer(this);
//    m_timerProgress->setInterval(1000);
//    connect(m_timerProgress, &QTimer::timeout, this, &Progress::setTempProgress);
}

void Progress::InitUI()
{
//    m_compressicon = Utils::renderSVG(":/images/Compression Packet.svg", QSize(128, 128));
    m_pixmaplabel = new DLabel(this);
    m_filenamelabel = new DLabel(this);
    m_filenamelabel->setText(m_filename);
    DFontSizeManager::instance()->bind(m_filenamelabel, DFontSizeManager::T5, QFont::DemiBold);
    m_filenamelabel->setForegroundRole(DPalette::ToolTipText);

    m_progressbar = new DProgressBar(this);
    m_progressbar->setRange(0, 100);
    m_progressbar->setFixedSize(240, 8);
    m_progressbar->setValue(0);
    m_progressbar->setOrientation(Qt::Horizontal);  //水平方向
    m_progressbar->setAlignment(Qt::AlignVCenter);
    m_progressbar->setTextVisible(false);
    m_shadow = new DLabel(this);
    m_shadow->setFixedSize(8, 1);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 4);
    effect->setColor(QColor(44, 167, 248, 120));
    effect->setBlurRadius(6);
    m_shadow->setGraphicsEffect(effect);

    m_progressfilelabel = new DLabel(this);
    m_progressfilelabel->setMaximumWidth(520);
    m_progressfilelabel->setForegroundRole(DPalette::TextTips);
//    m_progressfilelabel->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    DFontSizeManager::instance()->bind(m_progressfilelabel, DFontSizeManager::T8);
    m_progressfilelabel->setText(tr("Calculating..."));

    m_cancelbutton = new CustomPushButton(this);
    m_cancelbutton->setAccessibleName("Cancel_btn");
    m_cancelbutton->setMinimumSize(200, 36);
    m_cancelbutton->setText(tr("Cancel"));

    m_PauseContinueButton = new CustomSuggestButton(this);
    m_PauseContinueButton->setAccessibleName("PauseContinue_btn");
    m_PauseContinueButton->setMinimumSize(200, 36);
    m_PauseContinueButton->setText(tr("Pause"));
    m_PauseContinueButton->setCheckable(true);

    //add speed and time label
    m_speedLabel = new DLabel(this);
    m_speedLabel->setForegroundRole(DPalette::TextTips);        // 设置字体颜色
    DFontSizeManager::instance()->bind(m_speedLabel, DFontSizeManager::T8);

    m_restTimeLabel = new DLabel(this);
    m_restTimeLabel->setForegroundRole(DPalette::TextTips);        // 设置字体颜色
    DFontSizeManager::instance()->bind(m_restTimeLabel, DFontSizeManager::T8);

    QHBoxLayout *m_layout = new QHBoxLayout;
    m_layout->addStretch();
    m_layout->addWidget(m_speedLabel);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_restTimeLabel);
    m_layout->addStretch();

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setSpacing(0);      // 设置布局间距为0
    mainlayout->addStretch();
    mainlayout->addWidget(m_pixmaplabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(m_filenamelabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(25);
    mainlayout->addWidget(m_progressbar, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_shadow, 0, Qt::AlignLeft | Qt::AlignTop);
    mainlayout->addSpacing(10);     // 设置进度条和文件名间距为10
    mainlayout->addWidget(m_progressfilelabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    //mainlayout->addStretch();
    mainlayout->addSpacing(-2);     // 减小文件名和速度的间距

    mainlayout->addLayout(m_layout);
    mainlayout->addStretch();

    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_cancelbutton, 2);
    buttonHBoxLayout->addSpacing(10);
    buttonHBoxLayout->addWidget(m_PauseContinueButton, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);
    mainlayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
}

void Progress::InitConnection()
{
    connect(m_cancelbutton, &DPushButton::clicked, this, &Progress::cancelbuttonPressedSlot);
    connect(m_PauseContinueButton, &DPushButton::clicked, this, &Progress::pauseContinueButtonPressedSlot);
}

/**
 * @brief Progress::setSpeedAndTimeText  设置速度和剩余时间
 * @param type 进度类型
 */
void Progress::setSpeedAndTimeText(Progress::ENUM_PROGRESS_TYPE type)
{
    if (type == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING) { // 压缩
        m_speedLabel->setText(tr("Speed", "compress") + ": " + tr("Calculating..."));
    } else if (type == Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING) { // 解压
        m_speedLabel->setText(tr("Speed", "uncompress") + ": " + tr("Calculating..."));
    } else if (type == Progress::ENUM_PROGRESS_TYPE::OP_DELETEING) { // 删除
        m_speedLabel->setText(tr("Speed", "delete") + ": " + tr("Calculating..."));
    } else if (type == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD) { // 追加压缩
        m_speedLabel->setText(tr("Speed", "compress") + ": " + tr("Calculating..."));
    } else if (type == Progress::ENUM_PROGRESS_TYPE::OP_CONVERT) { // 格式转换
        m_speedLabel->setText(tr("Speed", "convert") + ": " + tr("Calculating..."));
    }

    m_restTimeLabel->setText(tr("Time left") + ": " + tr("Calculating...")); // 剩余时间计算中

    if (type == Progress::ENUM_PROGRESS_TYPE::OP_COMMENT) { // 压缩后添加注释进度
        m_speedLabel->setText("");
        m_restTimeLabel->setText("");
    }

    qDebug() << "setspeedandtimetext";
}

//void Progress::setTempProgress()
//{
//    if (m_percent >= 99) {
//        m_timerProgress->stop();
//        return;
//    }

//    m_percent += 0.3;
//    setprogress(m_percent);
//    qDebug() << "临时百分比" << m_percent;
//}

/**
 * @brief Progress::refreshSpeedAndTime 刷新速度和剩余时间
 * @param compressPercent 百分比
 * @param isConvert 是否为格式转换进度
 */
void Progress::refreshSpeedAndTime(unsigned long compressPercent, bool isConvert)
{
    double speed = this->m_pInfo->getSpeed(compressPercent, isConvert);
    qint64 timeLeft = this->m_pInfo->getLeftTime(compressPercent, isConvert);
    this->setSpeedAndTime(speed, timeLeft);
    this->m_pInfo->restartTimer();
}

ProgressAssistant *Progress::pInfo()
{
    return m_pInfo;
}

/**
 * @brief Progress::resetPauseContinueButton 改变暂停/继续按钮的标题
 */
void Progress::resetPauseContinueButton()
{
    m_PauseContinueButton->setChecked(false);
    m_PauseContinueButton->setText(tr("Pause"));
}

/**
 * @brief Progress::hidePauseContinueButton 设置暂停/继续按钮是否可见
 */
void Progress::hidePauseContinueButton()
{
    // 暂时只支持压缩解压时暂停取消
    if (Progress::OP_COMPRESSING == m_ProgressType || Progress::OP_DECOMPRESSING == m_ProgressType
            || Progress::OP_CONVERT == m_ProgressType || Progress::OP_COMMENT == m_ProgressType) {
        m_cancelbutton->setMinimumSize(200, 36);
        m_PauseContinueButton->setVisible(true);
        if (Progress::OP_COMMENT == m_ProgressType) {
            changeButtonState(false);
        } else {
            changeButtonState(true);
        }
    } else {
        m_PauseContinueButton->setVisible(false);
        m_cancelbutton->setMinimumSize(340, 36);
        changeButtonState(true);
    }
}

void Progress::changeButtonState(bool state)
{
    m_PauseContinueButton->setEnabled(state);
    m_cancelbutton->setEnabled(state);
}
/**
 * @brief Progress::slotChangeTimeLeft 定时任务定时暂时进度信息
 */
void Progress::slotChangeTimeLeft()
{
    // 如果剩余时间小于2秒自动关闭定时任务
    if (lastTimeLeft < 2) {
        m_timerTime->stop();
        return;
    }

    // 剩余时间-1
    lastTimeLeft--;
    displaySpeedAndTime(m_speed, lastTimeLeft);
}

/**
 * @brief Progress::setprogress 设置进度
 * @param percent 进度值
 */
void Progress::setprogress(double percent)
{
    //qDebug() << "setProgress(percent)" << percent;
    m_progressbar->setValue(percent);
    m_progressbar->update();
    m_percent = percent;
//    if (m_percent < 100) {
//        m_timerProgress->start();
//    } else {
//        m_timerProgress->stop();
//    }
}

/**
 * @brief Progress::setSpeedAndTime 设置速度以及剩余时间
 * @param speed
 * @param timeLeft
 */
void Progress::setSpeedAndTime(double speed, qint64 timeLeft)
{
    m_speed = speed;
    qint64 gap = timeLeft - lastTimeLeft;

    if (timeLeft > 2) {
        if (gap > 59 || gap < -59) { //时间差大于1分钟，会实时刷新
            lastTimeLeft = timeLeft;
        }
    }

    if (timeLeft < 30) {//剩余时间小于30s，会实时刷新
        lastTimeLeft = timeLeft;
    }

    // 显示速度个剩余时间
    displaySpeedAndTime(speed, lastTimeLeft);

    // 如果剩余时间大于2秒开启定时任务
    if (lastTimeLeft > 2) {
        m_timerTime->start();
    } else {
        m_timerTime->stop();
    }
}
/**
 * @brief Progress::displaySpeedAndTime 显示压缩或者解压速度，以及剩余时间
 * @param spee      速度单位 （K）
 * @param timeLeft  剩余时间 单位（s）
 */
void Progress::displaySpeedAndTime(double speed, qint64 timeLeft)
{
    // 计算剩余需要的小时。
    qint64 hour = timeLeft / 3600;
    // 计算剩余的分钟
    qint64 minute = (timeLeft - hour * 3600) / 60;
    // 计算剩余的秒数
    qint64 seconds = timeLeft - hour * 3600 - minute * 60;
    // 格式化数据
    QString hh = QString("%1").arg(hour, 2, 10, QLatin1Char('0'));
    QString mm = QString("%1").arg(minute, 2, 10, QLatin1Char('0'));
    QString ss = QString("%1").arg(seconds, 2, 10, QLatin1Char('0'));

    //add update speed and time label
    // 设置剩余时间
    m_restTimeLabel->setText(tr("Time left") + ": " + hh + ":" + mm + ":" + ss);
    if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING) {
        if (speed < 1024) {
            // 速度小于1024k， 显示速度单位为KB/s
            m_speedLabel->setText(tr("Speed", "compress") + ": " + QString::number(speed, 'f', 2) + "KB/s");
        } else if (speed > 1024 && speed < 1024 * 300) {
            // 速度大于1M/S，且小于300MB/s， 显示速度单位为MB/s
            m_speedLabel->setText(tr("Speed", "compress") + ": " + QString::number((speed / 1024), 'f', 2) + "MB/s");
        } else {
            // 速度大于300MB/s，显示速度为>300MB/s
            m_speedLabel->setText(tr("Speed", "compress") + ": " + ">300MB/s");
        }
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_DELETEING) {
        if (speed < 1024) {
            m_speedLabel->setText(tr("Speed", "delete") + ": " + QString::number(speed, 'f', 2) + "KB/s");
        } else {
            m_speedLabel->setText(tr("Speed", "delete") + ": " + QString::number((speed / 1024), 'f', 2) + "MB/s");
        }

    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD) {
//        m_speedLabel->setText(tr("Speed", "compress") + ": " + tr("Calculating..."));
        if (speed < 1024) {
            m_speedLabel->setText(tr("Speed", "compress") + ": " + QString::number(speed, 'f', 2) + "KB/s");
        } else if (speed > 1024 && speed < 1024 * 300) {
            m_speedLabel->setText(tr("Speed", "compress") + ": " + QString::number((speed / 1024), 'f', 2) + "MB/s");
        } else {
            m_speedLabel->setText(tr("Speed", "compress") + ": " + ">300MB/s");
        }
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING) {
        if (speed < 1024) {
            m_speedLabel->setText(tr("Speed", "uncompress") + ": " + QString::number(speed, 'f', 2) + "KB/s");
        } else if (speed > 1024 && speed < 1024 * 300) {
            m_speedLabel->setText(tr("Speed", "uncompress") + ": " + QString::number((speed / 1024), 'f', 2) + "MB/s");
        } else {
            m_speedLabel->setText(tr("Speed", "uncompress") + ": " + ">300MB/s");
        }
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_CONVERT) {
        if (speed < 1024) {
            m_speedLabel->setText(tr("Speed", "convert") + ": " + QString::number(speed, 'f', 2) + "KB/s");
        } else if (speed > 1024 && speed < 1024 * 300) {
            m_speedLabel->setText(tr("Speed", "convert") + ": " + QString::number((speed / 1024), 'f', 2) + "MB/s");
        } else {
            m_speedLabel->setText(tr("Speed", "convert") + ": " + ">300MB/s");
        }
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_COMMENT) {
        m_speedLabel->setText("");
        m_restTimeLabel->setText("");
    }
}

/**
 * @brief Progress::setFilename 设置文件名
 * @param filename 文件名
 */
void Progress::setFilename(QString filename)
{
    setTypeImage(filename);

    QString displayName = Utils::toShortString(filename); //文件名过长，中间用...代替
    m_filenamelabel->setText(displayName);
}

/**
 * @brief Progress::setTypeImage 根据文件类型设置显示图片
 * @param type 文件类型
 */
void Progress::setTypeImage(QString &filename)
{
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo(filename));
    m_pixmaplabel->setPixmap(icon.pixmap(128, 128));
}

DPushButton *Progress::getCancelbutton()
{
    return m_cancelbutton;
}

/**
 * @brief Progress::setProgressFilename 设置进度展示的文件名称
 * @param filename 文件名
 */
void Progress::setProgressFilename(QString filename)
{
    if (filename.isEmpty()) {
        return;
    }

    QFontMetrics elideFont(m_progressfilelabel->font());
    // 根据不同的压缩类型修改界面提示
    if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING || m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD) {
        m_progressfilelabel->setText(elideFont.elidedText(tr("Compressing") + ": " + filename, Qt::ElideMiddle, 520));
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_DELETEING) {
        m_progressfilelabel->setText(elideFont.elidedText(tr("Deleting") + ": " + filename, Qt::ElideMiddle, 520));
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_CONVERT) {
        m_progressfilelabel->setText(elideFont.elidedText(tr("Converting") + ": " + filename, Qt::ElideMiddle, 520));
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_COMMENT) { //注释进度
        m_progressfilelabel->setText(elideFont.elidedText(tr("Updating the comment..."), Qt::ElideMiddle, 520));
    } else {
        if (m_openType) {
            m_progressfilelabel->setText(elideFont.elidedText(tr("Opening") + ": " + filename, Qt::ElideMiddle, 520));
        } else {
            m_progressfilelabel->setText(elideFont.elidedText(tr("Extracting") + ": " + filename, Qt::ElideMiddle, 520));
        }
    }
}

/**
 * @brief Progress::settype 设置压缩进度类型
 * @param type 进度类型
 */
void Progress::settype(Progress::ENUM_PROGRESS_TYPE type)
{
    m_ProgressType = type;
}

/**
 * @brief Progress::getType 获取当前进度类型
 * @return
 */
Progress::ENUM_PROGRESS_TYPE Progress::getType()
{
    return m_ProgressType;
}

void Progress::setopentype(bool type)
{
    m_openType = type;
}

bool Progress::getOpenType()
{
    return m_openType;
}

/**
 * @brief Progress::showConfirmDialog 显示确认按钮
 * @return
 */
int Progress::showConfirmDialog()
{
    DDialog *dialog = new DDialog(this);
    // 设置取消弹出框
    dialog->setAccessibleName("Cancel_dialog");
    QPixmap pixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);
//    DPalette pa, pa2;
//    DLabel *strlabel = new DLabel(dialog);
//    strlabel->setFixedHeight(20);
//    strlabel->setForegroundRole(DPalette::WindowText);
//    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);

    DLabel *strlabel2 = new DLabel(dialog);
    strlabel2->setMinimumHeight(18);
    strlabel2->setForegroundRole(DPalette::ToolTipText);

    strlabel2->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(strlabel2);
    pa.setBrush(DPalette::Text, pa.color(DPalette::ToolTipText));
    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T6, QFont::Medium);

//    strlabel2->setFixedHeight(18);
//    strlabel2->setForegroundRole(DPalette::ToolTipText);

//    DFontSizeManager::instance()->bind(strlabel2, DFontSizeManager::T7, QFont::Medium);

    if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING) {
        //strlabel->setText(tr("Stop compressing "));
        strlabel2->setText(tr("Are you sure you want to stop the compression?")); // 是否想要停止当天压缩
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_DELETEING) {
        strlabel2->setText(tr("Are you sure you want to stop the update?")); // 是否要停止当前更新。当删除文件是
    } else if (m_ProgressType == Progress::ENUM_PROGRESS_TYPE::OP_DELETEING) {
        strlabel2->setText(tr("Are you sure you want to stop the conversion?")); //  是否停止
    } else {
        //strlabel->setText(tr("Stop extracting "));
        if (m_openType) {
            strlabel2->setText(("Are you sure you want to stop open the file?")); // 是否停止当前文件打开
        } else {
            strlabel2->setText(tr("Are you sure you want to stop the extraction?")); // 是否停止解压
        }
    }
    // 添加取消按钮
    dialog->addButton(tr("Cancel"));
    // 添加确认按钮
    dialog->addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(dialog);
    effect->setOffset(0, 4);
    effect->setColor(QColor(0, 145, 255, 76));
    effect->setBlurRadius(4);
    dialog->getButton(1)->setGraphicsEffect(effect);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    //mainlayout->addWidget(strlabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(strlabel2, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addSpacing(15);

    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);
    // 设置窗口初始化位置
    dialog->move(mapToGlobal(this->geometry().topLeft()).x() + this->width() / 2  - dialog->width() / 2,  mapToGlobal(this->geometry().topLeft()).y() - TITLE_FIXED_HEIGHT + this->height() / 2  - dialog->height() / 2); //dialog move to center
    int res = dialog->exec();
    delete dialog;

    return res;
}

/**
 * @brief Progress::resetProgress 重新设置
 */
void Progress::resetProgress()
{
    m_progressfilelabel->setText(tr("Calculating..."));
}

/**
 * @brief Progress::cancelbuttonPressedSlot 取消按钮点击事件
 */
void Progress::cancelbuttonPressedSlot()
{
    pauseContinueButtonPressedSlot(true); //暂停
    if (DDialog::Accepted == showConfirmDialog()) {
        m_timerTime->stop();
//        m_timerProgress->stop();
        m_speed = 0;
        lastTimeLeft = 0;
        // 发送暂停信号，并且将进度的类型传递给页面。界面根据不同的进度类型进行处理。
        emit sigCancelPressed(m_ProgressType);
    } else {
        pauseContinueButtonPressedSlot(false); //继续
    }
}

/**
 * @brief 按下暂停，弹起继续
 */
void Progress::pauseContinueButtonPressedSlot(bool checked)
{
    if (checked) {
        m_PauseContinueButton->setText(tr("Continue"));
        emit sigPauseProcess();

        //暂停进度
        m_timerTime->stop();
    } else {
        m_PauseContinueButton->setText(tr("Pause"));
        emit sigContinueProcess();
    }
}
