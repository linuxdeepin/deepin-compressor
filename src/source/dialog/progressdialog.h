/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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
#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <DTitlebar>
//#include <DPushButton>
#include <DLabel>
#include <DProgressBar>
#include <DAbstractDialog>
#include <DDialog>

DWIDGET_USE_NAMESPACE

class ExtractPauseDialog: public Dtk::Widget::DDialog
{
    Q_OBJECT
//public:
    explicit ExtractPauseDialog(QWidget *parent = nullptr);
//    void initUI();
//    void initConnect();

//public slots:
//    void clickedSlot(int index, const QString &text);
//    void closeEvent(QCloseEvent *event) override;

//signals:
//    void sigbuttonpress(int index);
};

class ProgressDialog: public DAbstractDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    void initUI();
    void initConnect();

    void setCurrentTask(const QString &file);
    void setCurrentFile(const QString &file);
    void setProcess(int value);
    void setFinished();
    void clearprocess();

protected:
    void closeEvent(QCloseEvent *) override;

signals:
    void stopExtract();
    void extractSuccess(QString msg);
    void sigResetPercentAndTime();

public slots:
    void slotextractpress(int index);

private:
    int m_defaultWidth = 380;
    int m_defaultHeight = 120;
    int m_dPerent = 0; //保存上一次进度

    DLabel *m_tasklable;
    DLabel *m_filelable;
    DProgressBar *m_circleprogress;

public:
//    ExtractPauseDialog *m_extractdialog;

private:
    DTitlebar *m_titlebar;

};

#endif // PROGRESSDIALOG_H
