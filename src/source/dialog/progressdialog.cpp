// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "progressdialog.h"
#include "popupdialog.h"
#include "common/ddlog.h"

#include <DFontSizeManager>
#include <DWindowCloseButton>

#include <QBoxLayout>
#include <QDebug>
#include <QFileInfo>
#include <QTimer>

ProgressDialog::ProgressDialog(QWidget *parent):
    DAbstractDialog(parent)
{
    qCDebug(appLog) << "Creating ProgressDialog";
    initUI();
    initConnect();
    qCDebug(appLog) << "ProgressDialog initialized with default size:"
             << m_defaultWidth << "x" << m_defaultHeight;
}

void ProgressDialog::initUI()
{
    qCDebug(appLog) << "Initializing ProgressDialog UI";
    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint /*& ~Qt::Dialog*/) | Qt::Window);
    setFixedWidth(m_defaultWidth);
    setMinimumHeight(m_defaultHeight);
    qCDebug(appLog) << "Window flags and size set";

    // 标题栏显示有几个任务正在进行
    m_titlebar = new DTitlebar(this);
    m_titlebar->setFixedHeight(50);
    m_titlebar->layout()->setContentsMargins(0, 0, 0, 0);
    m_titlebar->setMenuVisible(false);
    m_titlebar->setIcon(QIcon::fromTheme("deepin-compressor"));
    m_titlebar->setFixedWidth(m_defaultWidth);
    m_titlebar->setTitle(tr("%1 task(s) in progress").arg(1));
    m_titlebar->setBackgroundTransparent(true);

    QVBoxLayout *contentlayout = new QVBoxLayout;

    // 显示当前任务
    m_tasklable = new DLabel(this);
    m_tasklable->setText(tr("Task") + ": ");
    m_tasklable->setForegroundRole(DPalette::WindowText);
    DFontSizeManager::instance()->bind(m_tasklable, DFontSizeManager::T6, QFont::Medium);

    // 显示当前文件名
    m_filelable = new DLabel(this);
    m_filelable->setText(tr("Extracting") + ":");
    m_filelable->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_filelable, DFontSizeManager::T8, QFont::Normal);

    // 显示进度
    m_circleprogress = new  DProgressBar(this);
    m_circleprogress->setFixedSize(336, 6);
    m_circleprogress->setValue(0);

    contentlayout->setContentsMargins(20, 0, 10, 0);
    contentlayout->addWidget(m_tasklable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    contentlayout->addSpacing(7);
    contentlayout->addWidget(m_filelable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    contentlayout->addSpacing(7);
    contentlayout->addWidget(m_circleprogress, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 10, 20);
    mainlayout->setSpacing(0);
    mainlayout->addWidget(m_titlebar);
    mainlayout->addLayout(contentlayout);

    setLayout(mainlayout);
}

void ProgressDialog::initConnect()
{

}

/**
 * @brief ProgressDialog::setCurrentTask 设置当前压缩文件名
 * @param file
 */
void ProgressDialog::setCurrentTask(const QString &file)
{
    qCDebug(appLog) << "Setting current task to:" << file;
    QFileInfo fileinfo(file);
    m_tasklable->setText(tr("Task") + ":" + fileinfo.fileName());
    qCDebug(appLog) << "Task label updated";
}

/**
 * @brief ProgressDialog::setCurrentFile 设置正在提取的文件名
 * @param file
 */
void ProgressDialog::setCurrentFile(const QString &file)
{
    qCDebug(appLog) << "Setting current file to:" << file;
    // 对字符串长度进行...划分
    QFontMetrics elideFont(m_filelable->font());
    QString displayText = elideFont.elidedText(tr("Extracting") + ":" + file, Qt::ElideMiddle, 330);
    m_filelable->setText(displayText);
    qCDebug(appLog) << "File label updated to:" << displayText;
}

/**
 * @brief ProgressDialog::setProcess 设置进度
 * @param value
 */
void ProgressDialog::setProcess(double value)
{
    qCDebug(appLog) << "Setting progress to:" << value;

    if ((m_dPerent - value) == 0.0 || (m_dPerent > value)) {
        qCDebug(appLog) << "Progress not changed or decreased, skipping update";
        return;
    }

    m_dPerent = value;

    if (100 != m_circleprogress->value()) {
        m_circleprogress->setValue(qRound(value));
        qCDebug(appLog) << "Progress updated to:" << qRound(value);
    }
}

/**
 * @brief ProgressDialog::setFinished 提取结束
 * @param path
 */
void ProgressDialog::setFinished()
{
    qCDebug(appLog) << "Task finished, resetting progress";
    m_circleprogress->setValue(0);
    m_dPerent = 0.0;
    hide();
    qCDebug(appLog) << "ProgressDialog hidden";
}

/**
 * @brief ProgressDialog::clearprocess 清除进度
 */
void ProgressDialog::clearprocess()
{
    m_dPerent = 0.0;
    m_circleprogress->setValue(0);
}

void ProgressDialog::showDialog()
{
    qCDebug(appLog) << "Showing ProgressDialog";

    //获取ddialog的标题栏
    DTitlebar *titlebar = findChild<DTitlebar *>();
    if (titlebar != nullptr) {
        //获取ddialog标题栏中的关闭按钮
        DWindowCloseButton *closeBtn = titlebar->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
        if (closeBtn != nullptr) {
            //如果存在则将标题栏的焦点代理设置为关闭按钮
            closeBtn->clearFocus();
        }
        //设置ddialog的焦点代理为标题栏
        this->setFocusProxy(titlebar);
    }

    open();     // 使用open来进行模态对话框显示，不阻塞事件循环，可以一直接收进度
    qCDebug(appLog) << "ProgressDialog shown";
}

void ProgressDialog::closeEvent(QCloseEvent *)
{
    qCDebug(appLog) << "Close event received, current progress:" << m_circleprogress->value();
    if (m_dPerent < 100 && m_dPerent > 0) {
        qCDebug(appLog) << "Task in progress, pausing operation";
        // 先暂停
        emit signalPause();

        // 显示取消提示框
        QString strDesText = tr("Are you sure you want to stop the extraction?"); // 是否停止提取
        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(strDesText, tr("Cancel", "button"), DDialog::ButtonNormal, tr("Confirm", "button"), DDialog::ButtonRecommend);
        if (1 == iResult) {
            qCDebug(appLog) << "User confirmed cancellation";
            // 取消操作
            emit signalCancel();
        } else {
            qCDebug(appLog) << "User chose to continue";
            // 继续操作
            emit signalContinue();
        }
    }
}

/**
 * @brief ProgressDialog::slotextractpress 是否取消提取
 * @param index
 */
void ProgressDialog::slotextractpress(int index)
{
    qCInfo(appLog) << index;
    if (1 == index) { // 取消提取
        emit stopExtract();
        emit sigResetPercentAndTime();
    } else {
        exec();
    }
}

CommentProgressDialog::CommentProgressDialog(QWidget *parent):
    DAbstractDialog(parent)
{
    qCDebug(appLog) << "Creating CommentProgressDialog";
    initUI();
    qCDebug(appLog) << "CommentProgressDialog initialized with size 400x79";
}

void CommentProgressDialog::initUI()
{
    qCDebug(appLog) << "Initializing CommentProgressDialog UI";
    setFixedSize(400, 79);
    setWindowFlags((windowFlags() & ~ Qt::CustomizeWindowHint) | Qt::Window);
    qCDebug(appLog) << "Window flags and size set";

    // 布局
    DLabel *label = new DLabel(this);
    label->setFixedHeight(20);
    label->setText(tr("Updating the comment..."));

    m_progressBar = new DProgressBar(this);
    m_progressBar->setFixedSize(350, 6);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 20, 0, 20);
    mainLayout->addWidget(label);
    mainLayout->setSpacing(13);
    mainLayout->addWidget(m_progressBar);
}

void CommentProgressDialog::showdialog()
{
    qCDebug(appLog) << "Showing CommentProgressDialog";
    exec();
    qCDebug(appLog) << "CommentProgressDialog shown";
}

void CommentProgressDialog::setProgress(double value)
{
    qCDebug(appLog) << "Setting comment progress to:" << value;
    m_progressBar->setValue(static_cast<int>(value));
}

void CommentProgressDialog::setFinished()
{
    qCDebug(appLog) << "Comment update finished";
    m_progressBar->setValue(100);
    QTimer::singleShot(100, this, [ = ] {hide();}); // 手动延时100ms再关闭
}
