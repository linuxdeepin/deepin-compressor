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
#include <DCommandLinkButton>
#include <DPalette>
#include "archivesortfiltermodel.h"
#include <DApplicationHelper>
#include <DFontSizeManager>


DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class UnCompressPage : public QWidget
{
    Q_OBJECT

public:
    UnCompressPage(QWidget *parent = 0);
    ~UnCompressPage();

    void setModel(ArchiveSortFilterModel* model);
    QString getDecompressPath();
    void setdefaultpath(QString path);


signals:
    void sigDecompressPress(const QString& localPath);
    void sigextractfiles(QVector<Archive::Entry*>, QString path);


public slots:
    void oneCompressPress();
    void onPathButoonClicked();
    void onextractfilesSlot(QVector<Archive::Entry*> fileList, EXTRACT_TYPE type, QString path);


private:

    fileViewer *m_fileviewer;
    DPushButton* m_nextbutton;
    QStringList m_filelist;
    DCommandLinkButton* m_extractpath;
    DLabel* m_pixmaplabel;
    Lib_Edit_Button* m_pathbutton;
    QString m_pathstr;

    ArchiveSortFilterModel* m_model;

};
#endif
