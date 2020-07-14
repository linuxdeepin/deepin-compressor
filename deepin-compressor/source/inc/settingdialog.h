/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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

#pragma once

#include <DSettingsDialog>
#include <DSettings>
#include <DSettingsOption>
#include <DStandardPaths>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class KProcess;

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

private:
    void writeToConfbf();//write to disk from m_data;
    void readFromConfbf();//read data from disk and store in m_data;
public slots:
    void settingsChanged(const QString &key, const QVariant &value);
    void selectpressed();
    void cancelpressed();
    void recommandedPressed();
    virtual void done(int status) override;

signals:
    void sigReset();
private:
    DSettings *m_settings;
    KProcess *m_process = nullptr;
    QStringList m_associtionlist;
    QStringList m_recommendedList;
    QList<bool> m_valuelist;
    QList<bool> m_valuelisttemp;
    QString m_curpath;
    int m_index_last;

    DSettingsOption *m_comboboxoption;
    QMap<QString, QVariant> m_data; //keep entire data in memory

};

