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
/**
 * @brief The SettingDialog class 设置静态框
 */
class SettingDialog: public DSettingsDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();
    /**
     * @brief initUI 初始化界面
     */
    void initUI();
    /**
     * @brief initConnect 初始化信号
     */
    void initConnect();

    /**
     * @brief getCurExtractPath 获取当前解压目录
     * @return
     */
    QString getCurExtractPath();
    /**
     * @brief isAutoCreatDir 是否创建新的目录进行解压,
     * @return true表示创建，false表示不创建
     */
    bool isAutoCreatDir();
    /**
     * @brief isAutoOpen 是否自动打开,到对应的文件目录
     * @return true表示自动打开，false表示不打开
     */
    bool isAutoOpen();
    /**
     * @brief isAutoDeleteFile 压缩成功是否自动删除文件
     * @return true表示自动删除 false表示不自动删除
     */
    bool isAutoDeleteFile();
    /**
     * @brief isAutoDeleteArchive 解压成功是否自动删除压缩包
     * @return 表示自动删除 false表示不自动删除
     */
    QString isAutoDeleteArchive();

    void startcmd(QString &mimetype, bool state);

private:
    void writeToConfbf(); //write to disk from m_data;      将获取的配置信息，写入磁盘
    void readFromConfbf(); //read data from disk and store in m_data;    读取磁盘的配置信息
public slots:
    void settingsChanged(const QString &key, const QVariant &value); // 设置改变信号,当用户修改配置后自动，识别信息，并且将其保存至配置文件目录
    void selectpressed(); // 全选操作
    void cancelpressed(); // 全部取消操作
    void recommandedPressed(); // 重置配置信息
    virtual void done(int status) override;

signals:
    void sigResetPath();
    void sigResetDeleteArchive();

private:
    DSettings *m_settings = nullptr;
    KProcess *m_process = nullptr;
    QStringList m_associtionlist;
    QStringList m_recommendedList;
    QList<bool> m_valuelist;
    QList<bool> m_valuelisttemp;
    QString m_curpath; // 当前目录路径
    int m_index_last;

    DSettingsOption *m_comboboxoption;
    QMap<QString, QVariant> m_data; //keep entire data in memory
    DSettingsOption *m_deleteArchiveOption = nullptr;
    QString m_isAutoDeleteArchive; // 是否自动删除压缩包
};

