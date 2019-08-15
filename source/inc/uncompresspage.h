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
#include <QPropertyAnimation>
#include <QStackedLayout>
#include <QPushButton>
#include <QLabel>
#include "dsuggestbutton.h"
#include "dspinner.h"


DWIDGET_USE_NAMESPACE

class UnCompressPage : public QWidget
{
    Q_OBJECT

public:
    UnCompressPage(QWidget *parent = nullptr);
    ~UnCompressPage();

    void updateInfo();
    void refreshPage();
    void progressBarStart();

private slots:
    void handleInstall();
    void handleRemove();
    void handleReinstall();
    void onInstallFinished();
    void onUninstallFinished();
    void onReinstallFinished();
    void viewFilePath();

private:

    QLabel *m_nameLabel;
    QLabel *m_styleLabel;
    QLabel *m_typeLabel;
    QLabel *m_versionLabel;
    QLabel *m_copyrightLabel;
    QLabel *m_descriptionLabel;
    QLabel *m_tipsLabel;
    DSuggestButton *m_installBtn;
    QPushButton *m_uninstallBtn;
    QPushButton *m_reinstallBtn;
    DSuggestButton *m_viewFileBtn;
    QPushButton *m_closeBtn;
    DSpinner *m_spinner;
    QStackedLayout *m_bottomLayout;
    QString m_filePath;
};

#endif
