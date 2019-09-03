/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef SINGLEFILEPAGE_H
#define SINGLEFILEPAGE_H


#include <QWidget>
#include <QLabel>
#include <DFileDialog>
#include <DPushButton>
#include <QSettings>
#include <QMessageBox>
#include <DLineEdit>

#include "fileViewer.h"
#include "jobs.h"
#include "lib_edit_button.h"

DWIDGET_USE_NAMESPACE

class UnCompressPage : public QWidget
{
    Q_OBJECT

public:
    UnCompressPage(QWidget *parent = 0);
    ~UnCompressPage();

    void setModel(QAbstractItemModel* model);
    QString getDecompressPath();
    void setdefaultpath(QString path);


signals:
    void sigDecompressPress(const QString& localPath);

public slots:
    void oneCompressPress();
    void onPathButoonClicked();


private:

    fileViewer *m_fileviewer;
    DPushButton* m_nextbutton;
    QSettings *m_settings;
    QStringList m_filelist;
    DLineEdit* m_extractpath;
    DLabel* m_pixmaplabel;
    Lib_Edit_Button* m_pathbutton;
    QString m_pathstr;

    QAbstractItemModel* m_model;

};
#endif
