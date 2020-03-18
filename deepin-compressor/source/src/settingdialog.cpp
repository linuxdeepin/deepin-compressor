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
#include "settingdialog.h"
#include <QStandardItemModel>
#include <qsettingbackend.h>
#include <QDebug>
#include <DFileDialog>
#include <QBoxLayout>
#include <DComboBox>
#include <DLabel>
#include <QMessageBox>
#include <DPalette>
#include <DSettingsWidgetFactory>
#include "DPushButton"
#include "DApplicationHelper"
#include "kprocess.h"
#include <DStandardPaths>
#include <DSuggestButton>

DGUI_USE_NAMESPACE

SettingDialog::SettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    m_associtionlist << "file_association.file_association_type.x-7z-compressed"
                     << "file_association.file_association_type.x-archive"
                     << "file_association.file_association_type.x-bcpio"
                     << "file_association.file_association_type.x-bzip"
                     << "file_association.file_association_type.x-cpio"
                     << "file_association.file_association_type.x-cpio-compressed"
                     << "file_association.file_association_type.vnd.debian.binary-package"
                     << "file_association.file_association_type.gzip"
                     << "file_association.file_association_type.x-java-archive"
                     << "file_association.file_association_type.x-lzma"
                     << "file_association.file_association_type.vnd.ms-cab-compressed"
                     << "file_association.file_association_type.vnd.rar"
                     << "file_association.file_association_type.x-rpm"
                     << "file_association.file_association_type.x-sv4cpio"
                     << "file_association.file_association_type.x-sv4crc"
                     << "file_association.file_association_type.x-tar"
                     << "file_association.file_association_type.x-bzip-compressed-tar"
                     << "file_association.file_association_type.x-compressed-tar"
                     << "file_association.file_association_type.x-lzip-compressed-tar"
                     << "file_association.file_association_type.x-lzma-compressed-tar"
                     << "file_association.file_association_type.x-tzo"
                     << "file_association.file_association_type.x-xz-compressed-tar"
                     << "file_association.file_association_type.x-tarz"
                     << "file_association.file_association_type.x-xar"
                     << "file_association.file_association_type.x-xz"
                     << "file_association.file_association_type.zip"
                     << "file_association.file_association_type.x-cd-image"
                     << "file_association.file_association_type.x-iso9660-appimage"
                     << "file_association.file_association_type.x-source-rpm";

    m_recommendedList << "file_association.file_association_type.x-7z-compressed"
                      << "file_association.file_association_type.x-archive"
                      << "file_association.file_association_type.x-bcpio"
                      << "file_association.file_association_type.x-bzip"
                      << "file_association.file_association_type.x-cpio"
                      << "file_association.file_association_type.x-cpio-compressed"
                      << "file_association.file_association_type.vnd.debian.binary-package"
                      << "file_association.file_association_type.gzip"
                      << "file_association.file_association_type.x-java-archive"
                      << "file_association.file_association_type.x-lzma"
                      << "file_association.file_association_type.vnd.ms-cab-compressed"
                      << "file_association.file_association_type.vnd.rar"
                      << "file_association.file_association_type.x-rpm"
                      << "file_association.file_association_type.x-sv4cpio"
                      << "file_association.file_association_type.x-sv4crc"
                      << "file_association.file_association_type.x-tar"
                      << "file_association.file_association_type.x-bzip-compressed-tar"
                      << "file_association.file_association_type.x-compressed-tar"
                      << "file_association.file_association_type.x-lzip-compressed-tar"
                      << "file_association.file_association_type.x-lzma-compressed-tar"
                      << "file_association.file_association_type.x-tzo"
                      << "file_association.file_association_type.x-xz-compressed-tar"
                      << "file_association.file_association_type.x-tarz"
                      << "file_association.file_association_type.x-xar"
                      << "file_association.file_association_type.x-xz"
                      << "file_association.file_association_type.zip";

    m_valuelist.clear();

    initUI();
    initConnect();

    foreach (QString key, m_associtionlist) {
        m_valuelist.append(m_settings->value(key).toBool());

    }
    m_valuelisttemp = m_valuelist;
}

void SettingDialog::initUI()
{
    this->widgetFactory()->registerWidget("custom-button", [this](QObject * obj) -> QWidget* {
        if (/*DSettingsOption *option = */qobject_cast<DSettingsOption *>(obj))
        {
            QWidget *buttonwidget = new QWidget();
            QHBoxLayout *layout = new QHBoxLayout();
            DPushButton *button1 = new DPushButton(tr("Select All"));
            DPushButton *button2 = new DPushButton(tr("Clear All"));
            DSuggestButton *button3 = new DSuggestButton(tr("Recommended"));
            button1->setMinimumSize(153, 36);
            button2->setMinimumSize(153, 36);
            button3->setMinimumSize(153, 36);
            layout->addStretch();
            layout->addWidget(button1);
            layout->addStretch();
            layout->addWidget(button2);
            layout->addStretch();
            layout->addWidget(button3);
            layout->addStretch();
            buttonwidget->setLayout(layout);

            connect(button1, &QPushButton::clicked, this, &SettingDialog::selectpressed);
            connect(button2, &QPushButton::clicked, this, &SettingDialog::cancelpressed);
            connect(button3, &QPushButton::clicked, this, &SettingDialog::recommandedPressed);
            return buttonwidget;
        }

        return nullptr;
    });

    this->widgetFactory()->registerWidget("pathbox", [this](QObject * obj) -> QWidget* {
        m_comboboxoption = qobject_cast<DSettingsOption *>(obj);
        if (m_comboboxoption)
        {
            DWidget *widget = new DWidget(this);
            QHBoxLayout *layout = new QHBoxLayout();

            DLabel *label = new DLabel(widget);
            label->setForegroundRole(DPalette::WindowText);
            label->setText(tr("Extract archives to") + ":");

            DComboBox *combobox = new DComboBox(widget);
            combobox->setMinimumWidth(300);
            combobox->setEditable(false);
            QStringList list;
            list << tr("Current directory") << tr("Desktop") << tr("Other directory");
            combobox->addItems(list);
            qDebug() << m_comboboxoption->value();
            if (QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) == m_comboboxoption->value()) {
                combobox->setCurrentIndex(1);
                m_curpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                m_index_last = 1;
            } else if ("" == m_comboboxoption->value()) {
                combobox->setCurrentIndex(0);
                m_curpath = "";
                m_index_last = 0;
            } else {
                combobox->setEditable(true);
                combobox->setCurrentIndex(2);
                m_curpath = m_comboboxoption->value().toString();
                combobox->setEditText(m_curpath);
                m_index_last = 2;
            }

            layout->addWidget(label, 0, Qt::AlignLeft);
            layout->addWidget(combobox, 0, Qt::AlignLeft);

            widget->setLayout(layout);

            connect(this, &SettingDialog::sigReset, this, [ = ] {
                combobox->setCurrentIndex(0);
            });

            connect(combobox, &DComboBox::currentTextChanged, [combobox, this] {
                if (tr("Current directory") == combobox->currentText())
                {
                    combobox->setEditable(false);
                    m_curpath = "";
                    m_index_last = 0;
                } else if (tr("Desktop") == combobox->currentText())
                {
                    combobox->setEditable(false);
                    m_curpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                    m_index_last = 1;
                } else if (tr("Other directory") == combobox->currentText())
                {
                    combobox->setEditable(true);
                    DFileDialog dialog(this);
                    dialog.setAcceptMode(DFileDialog::AcceptOpen);
                    dialog.setFileMode(DFileDialog::Directory);
                    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

                    const int mode = dialog.exec();

                    if (mode != QDialog::Accepted) {
                        if (0 == m_index_last) {
                            combobox->setEditable(false);
                            combobox->setCurrentIndex(0);
                        } else if (1 == m_index_last) {
                            combobox->setEditable(false);
                            combobox->setCurrentIndex(1);
                        } else {
                            combobox->setEditText(m_curpath);
                        }
                        return;
                    }

                    QList<QUrl> pathlist = dialog.selectedUrls();
                    QString curpath = pathlist.at(0).toLocalFile();

                    combobox->setEditText(curpath);
                    m_curpath = curpath;
                    m_index_last = 2;
                } else
                {
                    m_curpath = combobox->currentText();
                    QDir dir(m_curpath);
                    DPalette plt = DApplicationHelper::instance()->palette(combobox);

                    if (!dir.exists()) {

                        plt.setBrush(DPalette::Text, plt.color(DPalette::TextWarning));

                    } else {
                        plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));
                    }

                    combobox->setPalette(plt);
                    m_index_last = 2;
                }
                m_comboboxoption->setValue(m_curpath);


            });


            qDebug() << m_curpath;
            return widget;
        }

        return nullptr;
    });

    const QString confDir = DStandardPaths::writableLocation(
                                QStandardPaths::AppConfigLocation);
    const QString confPath = confDir + QDir::separator() + "deepin-compressor.conf";

    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath);

    // 通过json文件创建DSettings对象
    m_settings = DSettings::fromJsonFile(":/data/deepin-compressor.json");
    // 设置DSettings存储后端
    m_settings->setBackend(backend);

    // 通过DSettings对象构建设置界面
    updateSettings(m_settings);

    //moveToCenter();
}

void SettingDialog::initConnect()
{
    connect(m_settings, &DSettings::valueChanged,
            this, &SettingDialog::settingsChanged);
}

void SettingDialog::done(int status)
{
    Q_UNUSED(status)

    QDir dir(m_curpath);

    if (!dir.exists()) {
        QMessageBox box;
        box.setText(tr("The default extraction path does not exist, please retry"));
        box.exec();
        return;
    }

    QDialog::done(status);

    int loop = 0;
    foreach (bool value, m_valuelisttemp) {
        if (m_valuelist.at(loop) != value) {
            QString key = m_associtionlist.at(loop);
            QString mimetype = "application/" + key.remove("file_association.file_association_type.");
            startcmd(mimetype, m_valuelisttemp.at(loop));
        }

        loop++;
    }
    m_valuelist = m_valuelisttemp;
}


QString SettingDialog::getCurExtractPath()
{
    qDebug() << m_curpath;
    return m_curpath;
}

bool SettingDialog::isAutoCreatDir()
{
    return m_settings->value("base.decompress.create_folder").toBool();
}

bool SettingDialog::isAutoOpen()
{
    return m_settings->value("base.decompress.open_folder").toBool();
}


void SettingDialog::settingsChanged(const QString &key, const QVariant &value)
{
    qDebug() << key << value;

    if (key.contains("file_association_type")) {
        int index = m_associtionlist.indexOf(key);
        if (index > -1) {
            m_valuelisttemp.replace(index, value.toBool());
        }
    } else if (key.contains("default_path") && value.toString() == "") {
        emit sigReset();
    }
}

void SettingDialog::selectpressed()
{
    foreach (QString key, m_associtionlist) {
        m_settings->setOption(key, true);
    }
}

void SettingDialog::cancelpressed()
{
    foreach (QString key, m_associtionlist) {
        m_settings->setOption(key, false);
    }
}

void SettingDialog::recommandedPressed()
{
    foreach (QString key, m_associtionlist) {
        m_settings->setOption(key, false);
    }
    foreach (QString key, m_recommendedList) {
        m_settings->setOption(key, true);
    }
}

void SettingDialog::startcmd(QString &mimetype, bool state)
{
    QString programPath = QStandardPaths::findExecutable("xdg-mime");
    if (programPath.isEmpty()) {
        qDebug() << "error can't find xdg-mime";
        return;
    }

    if ( nullptr == m_process) {
        m_process = new KProcess(this);
    }

    QStringList arguments;

    if (state) {
        arguments << "default" << "deepin-compressor.desktop" << mimetype;
    } else {
        arguments << "default" << ".desktop" << mimetype;
    }

    qDebug() << mimetype << arguments;

    m_process->setOutputChannelMode(KProcess::MergedChannels);
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    m_process->setProgram(programPath, arguments);
    m_process->start();
    m_process->waitForStarted();
    m_process->waitForFinished();
}
