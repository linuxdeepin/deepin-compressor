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
#ifndef COMPRESSSETTING_H
#define COMPRESSSETTING_H

#include <QWidget>
#include <DPushButton>
#include <DComboBox>
#include <DLabel>
#include <DLineEdit>
#include "utils.h"
#include <DSwitchButton>
#include <dpasswordedit.h>
#include <QVBoxLayout>
#include <DSpinBox>
#include "lib_edit_button.h"
#include "pluginmanager.h"
#include "dfilechooseredit.h"
#include <DPalette>
#include <DApplicationHelper>
#include <DCheckBox>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#define D_COMPRESS_SPLIT_MIX    5000
#define D_COMPRESS_SPLIT_MIN   0

class CompressSetting :public QWidget
{
    Q_OBJECT
public:
    CompressSetting(QWidget* parent = 0);
    ~CompressSetting();

    void keyPressEvent(QKeyEvent *event) override;

    void InitUI();
    void InitConnection();

    void setTypeImage(QString type);
    void setDefaultPath(QString path);
    void setDefaultName(QString name);
    void setFilepath(QStringList pathlist);
    quint64 dirFileSize(const QString &path);

private:
    DPushButton* m_nextbutton;
    QPixmap m_compressicon;
    DComboBox* m_compresstype;
    DLineEdit* m_filename;
    DFileChooserEdit* m_savepath;
    DLabel* m_pixmaplabel;
//    Lib_Edit_Button* m_pathbutton;
    QVBoxLayout *m_fileLayout;

    QHBoxLayout *m_moresetlayout;
    DSwitchButton* m_moresetbutton;
    DPasswordEdit* m_password;
    QHBoxLayout *m_file_secretlayout;
    DSwitchButton* m_file_secret;
    QHBoxLayout *m_splitlayout;
    DDoubleSpinBox* m_splitnumedit;
    DPushButton* m_plusbutton;
    DPushButton* m_minusbutton;
    DLabel* m_encryptedlabel;
    DCheckBox* m_splitcompress;
    DLabel* m_encryptedfilelistlabel;

    PluginManager m_pluginManger;
    QStringList m_supportedMimeTypes;
    QStringList m_pathlist;


signals:
    void sigCompressPressed(QMap<QString, QString> &Args);

public slots:
    void onNextButoonClicked();
    void onAdvanceButtonClicked(bool status);
    void ontypeChanged(int index);
    void onSplitValueChanged(double value);
    void onSplitChanged(int status);

    void onRetrunPressed();
};

#endif // COMPRESSSETTING_H
