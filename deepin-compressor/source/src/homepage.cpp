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

#include "homepage.h"
#include "mainwindow.h"
#include "utils.h"
#include "customwidget.h"

#include <DFileDialog>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QShortcut>

DGUI_USE_NAMESPACE

HomePage::HomePage(QWidget *parent)
    : DWidget(parent),
      m_layout(new QVBoxLayout(this)),
      m_iconLabel(new DLabel(this)),
      m_tipsLabel(new DLabel(tr("Drag file or folder here"), this)),
      m_splitLine(new DLabel(this)),
      m_chooseBtn(new CustomCommandLinkButton(tr("Select File"), this)),
      m_settings(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this))
{
    m_chooseBtn->setAccessibleName("Choose_btn");
    m_unloadPixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_folder_128px.svg", QSize(128, 128));
    m_loadedPixmap = Utils::renderSVG(":assets/icons/deepin/builtin/icons/compress_folder_128px.svg", QSize(128, 128));

    m_iconLabel->setFixedSize(128, 128);
    m_iconLabel->setPixmap(m_unloadPixmap);

    DFontSizeManager::instance()->bind(m_tipsLabel, DFontSizeManager::T8);
    m_tipsLabel->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_chooseBtn, DFontSizeManager::T8);

    // initalize the configuration file.
    if (m_settings->value("dir").toString().isEmpty()) {
        m_settings->setValue("dir", "");
    }

    DLabel *buttomlabel = new DLabel(this);
    buttomlabel->setFixedHeight(50);

    m_layout->addStretch();
    m_layout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    m_layout->addSpacing(13);
    m_layout->addWidget(m_tipsLabel, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_splitLine, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    m_layout->addWidget(m_chooseBtn, 0, Qt::AlignHCenter);
    m_layout->addWidget(buttomlabel, 0, Qt::AlignHCenter);
    m_layout->addStretch();

    connect(m_chooseBtn, &DCommandLinkButton::clicked, this, &HomePage::onChooseBtnClicked);

    auto openkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, &HomePage::onChooseBtnClicked);

    themeChanged();
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &HomePage::themeChanged);

    setBackgroundRole(DPalette::Background);
}

HomePage::~HomePage()
{
    SAFE_DELETE_ELE(m_layout);
    SAFE_DELETE_ELE(m_iconLabel);
    SAFE_DELETE_ELE(m_tipsLabel);
    SAFE_DELETE_ELE(m_splitLine);
    SAFE_DELETE_ELE(m_chooseBtn);
    SAFE_DELETE_ELE(m_settings);
}
/**
 * @brief HomePage::setIconPixmap 设置图标
 * @param isLoaded  true设置加载图标。false:设置非加载图标
 */
void HomePage::setIconPixmap(bool isLoaded)
{
    if (isLoaded) {
        m_iconLabel->setPixmap(m_loadedPixmap);
    } else {
        m_iconLabel->setPixmap(m_unloadPixmap);
    }
}

CustomCommandLinkButton *HomePage::getChooseBtn()
{
    return m_chooseBtn;
}
/**
 * @brief HomePage::onChooseBtnClicked 点击按钮选择需要压缩或者解压的文件
 */
void HomePage::onChooseBtnClicked()
{
    // 初始化文件选择对话框
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setAllowMixedSelection(true);
    //    dialog.setNameFilter(Utils::suffixList());
    // 从设置中获取获取历史文件目录
    QString historyDir = m_settings->value("dir").toString();
    // 设置如果历史目录为空自动设置主目录作为历史目录
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    // 设置目录位置
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    m_settings->setValue("dir", dialog.directoryUrl().toLocalFile());

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }
    // 发送目录选择信号。
    emit fileSelected(dialog.selectedFiles());
}
/**
 * @brief HomePage::themeChanged 主题改变，主题改变以后线条也改变
 */
void HomePage::themeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType)
        m_splitLine->setPixmap(QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg"));
    else if (themeType == DGuiApplicationHelper::DarkType)
        m_splitLine->setPixmap(QPixmap(":assets/icons/deepin/builtin/dark/icons/split_line_dark.svg"));
    else
        m_splitLine->setPixmap(QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg"));
}
