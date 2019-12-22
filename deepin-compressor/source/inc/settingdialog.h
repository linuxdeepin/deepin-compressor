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
#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H


#include <DDialog>
#include <DSettingsDialog>
#include <DStandardPaths>
#include <QDir>
#include <DSettings>
#include <DTableView>
#include <DSettingsOption>
#include "kprocess.h"
#include <DSettingsWidgetFactory>
#include <DPushButton>
#include <DApplicationHelper>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


class SettingDialog: public DSettingsDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);
    void initUI();
    void initConnect();

    QString getCurExtractPath();
    bool isAutoCreatDir();
    bool isAutoOpen();

    void startcmd(QString &mimetype, bool state);

public slots:
    void settingsChanged(const QString &key, const QVariant &value);
    void selectpressed();
    void cancelpressed();
    virtual void done(int status) override;

signals:
    void sigReset();
private:
    DSettings *m_settings;
    KProcess *m_process = nullptr;
    QStringList m_associtionlist;
    QList<bool> m_valuelist;
    QList<bool> m_valuelisttemp;
    QString m_curpath;
    int m_index_last;

    DSettingsOption *m_comboboxoption;
};

#endif // SETTINGDIALOG_H
