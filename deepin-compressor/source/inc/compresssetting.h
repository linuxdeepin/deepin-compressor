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
#ifndef COMPRESSSETTING_H
#define COMPRESSSETTING_H

#include <DWidget>
#include <DPushButton>
#include <DLabel>
#include <DLineEdit>
#include <DSwitchButton>
#include <dpasswordedit.h>
#include <QVBoxLayout>
#include "lib_edit_button.h"
#include "pluginmanager.h"
#include "dfilechooseredit.h"
#include <DCheckBox>
#include <DMenu>
#include "DSpinBox"

DWIDGET_USE_NAMESPACE

#define D_COMPRESS_SPLIT_MIX    5000
#define D_COMPRESS_SPLIT_MIN   0

class TypeLabel: public DLabel
{
    Q_OBJECT
public:
    TypeLabel(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void labelClickEvent(QMouseEvent *event);
};

class CompressSetting : public DWidget
{
    Q_OBJECT
public:
    CompressSetting(QWidget *parent = nullptr);

    void InitUI();
    void InitConnection();
    void initWidget();

    void setTypeImage(QString type);
    void setDefaultPath(QString path);
    void setDefaultName(QString name);
    void setFilepath(QStringList pathlist);
    quint64 dirFileSize(const QString &path);
    bool checkfilename(QString str);
    bool checkFilePermission(const QString &path);
    void getSelectedFileSize(qint64 size);
    void clickTitleBtnResetAdvancedOptions();

private:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void sigCompressPressed(QMap<QString, QString> &Args);
    void sigFileUnreadable(QStringList &pathList, int fileIndex);//compress file is unreadable or file is a link

public slots:
    void onNextButoonClicked();
    void onAdvanceButtonClicked(bool status);
    void ontypeChanged(QAction *action);
    void onSplitChanged(int status);
    void showRightMenu(QMouseEvent *e);

    void onThemeChanged();
    bool onSplitChecked();

private:
    int showWarningDialog(const QString &msg, int index = 0);

private:
    DPushButton *m_nextbutton = nullptr;
    QPixmap m_compressicon;
    TypeLabel *m_compresstype = nullptr;
    DLineEdit *m_filename = nullptr;
    DFileChooserEdit *m_savepath = nullptr;
    DLabel *m_pixmaplabel = nullptr;
    TypeLabel *m_clicklabel = nullptr;
    TypeLabel *typepixmap = nullptr;
//    Lib_Edit_Button* m_pathbutton;
    QVBoxLayout *m_fileLayout = nullptr;

    QHBoxLayout *m_moresetlayout = nullptr;
    DSwitchButton *m_moresetbutton = nullptr;
    DPasswordEdit *m_password = nullptr;
    QHBoxLayout *m_file_secretlayout = nullptr;
    DSwitchButton *m_file_secret = nullptr;
    QHBoxLayout *m_splitlayout = nullptr;
    DDoubleSpinBox *m_splitnumedit = nullptr;
    DPushButton *m_plusbutton = nullptr;
    DPushButton *m_minusbutton = nullptr;
    DLabel *m_encryptedlabel = nullptr;
    DCheckBox *m_splitcompress = nullptr;
    DLabel *m_encryptedfilelistlabel = nullptr;
    DMenu *m_typemenu = nullptr;

    PluginManager m_pluginManger;
    QStringList m_supportedMimeTypes;
    QStringList m_pathlist;
    bool filePermission = true;
    qint64 m_getFileSize;
    bool isSplitChecked = false;
};

#endif // COMPRESSSETTING_H
