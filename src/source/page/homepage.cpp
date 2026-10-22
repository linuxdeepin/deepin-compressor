// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepage.h"
#include "customwidget.h"
#include "uitools.h"
#include "common/ddlog.h"

#include <DFileDialog>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DPaletteHelper>

#include <QVBoxLayout>
#include <QSettings>
#include <QSettings>
#include <QShortcut>
#include <QDragEnterEvent>
#include <QMimeData>

DGUI_USE_NAMESPACE

HomePage::HomePage(QWidget *parent)
    : DWidget(parent)
{
    qCDebug(appLog) << "HomePage constructor called";
    initUI();           // 初始化界面配置
    initConnections();  // 初始化信号槽链接
    qCDebug(appLog) << "HomePage initialization completed";
}

HomePage::~HomePage()
{
    qCDebug(appLog) << "HomePage destructor called";
}

void HomePage::initUI()
{
    qCDebug(appLog) << "Initializing HomePage UI";
    // 初始化相关变量
    m_pIconLbl = new DLabel(this);
    m_pTipLbl = new DLabel(tr("Drag file or folder here"), this);
    m_pSplitLbl = new DLabel(this);
    m_pChooseBtn = new CustomCommandLinkButton(tr("Select File"), this);
    m_pSettings = new QSettings(QDir(UiTools::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this);

    // 初始化控件布局
    m_pLayout = new QVBoxLayout(this);
    m_pLayout->addStretch();
    m_pLayout->addWidget(m_pIconLbl, 0, Qt::AlignHCenter);
    m_pLayout->addSpacing(13);
    m_pLayout->addWidget(m_pTipLbl, 0, Qt::AlignHCenter);
    m_pLayout->addSpacing(15);
    m_pLayout->addWidget(m_pSplitLbl, 0, Qt::AlignHCenter);
    m_pLayout->addSpacing(15);
    m_pLayout->addWidget(m_pChooseBtn, 0, Qt::AlignHCenter);
    m_pLayout->addStretch();

    // 设置首页图标
    m_pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_folder_128px.svg", QSize(128, 128));
    m_pIconLbl->setFixedSize(128, 128);
    m_pIconLbl->setPixmap(m_pixmap);

    // 设置提示和按钮字体
    DFontSizeManager::instance()->bind(m_pTipLbl, DFontSizeManager::T8);
    m_pTipLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pChooseBtn, DFontSizeManager::T8);

    // 初始化配置数据
    if (m_pSettings->value("dir").toString().isEmpty()) {
        m_pSettings->setValue("dir", "");
    }

    slotThemeChanged();     // 初始化分割线图标

    // 设置快捷键
    auto openkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, &HomePage::signalFileChoose);

    setAcceptDrops(true);
    setBackgroundRole(DPalette::Base);
}

void HomePage::initConnections()
{
    qCDebug(appLog) << "Setting up HomePage connections";
    connect(m_pChooseBtn, &DCommandLinkButton::clicked, this, [this]() {
        qCInfo(appLog) << "File choose button clicked";
        emit signalFileChoose();
    });
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &HomePage::slotThemeChanged);
    qCDebug(appLog) << "HomePage connections initialized";
}

void HomePage::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();
    qCDebug(appLog) << "Drag enter event detected, has URLs:" << mime->hasUrls();

    // 判断是否有url
    if (!mime->hasUrls()) {
        qCDebug(appLog) << "No URLs in drag data, ignoring";
        e->ignore();
        return;
    }

    // 判断是否是本地设备文件，过滤 手机 网络 ftp smb 等
    for (const auto &url : mime->urls()) {
        QString localFile = url.toLocalFile();
        qCDebug(appLog) << "Checking drag file:" << localFile;
        if (!UiTools::isLocalDeviceFile(localFile)) {
            qCDebug(appLog) << "Non-local file detected:" << localFile << "ignoring";
            e->ignore();
            return;
        }
    }

    qCDebug(appLog) << "Drag accepted";
    e->accept();
}

void HomePage::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void HomePage::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();
    qCDebug(appLog) << "Drop event detected";

    if (false == mime->hasUrls()) {
        qCDebug(appLog) << "No URLs in drop data, ignoring";
        e->ignore();
        return;
    }

    e->accept();

    // 判断本地文件
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            qCDebug(appLog) << "Non-local URL dropped:" << url.toString();
            continue;
        }

        QString localFile = url.toLocalFile();
        qCDebug(appLog) << "Adding dropped file:" << localFile;
        fileList << localFile;
    }

    if (fileList.size() == 0) {
        qCDebug(appLog) << "No valid local files dropped";
        return;
    }

    qCDebug(appLog) << "Emitting signal for dropped files, count:" << fileList.size();
    emit signalDragFiles(fileList);
}

void HomePage::slotThemeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    qCDebug(appLog) << "Theme changed, new type:" << themeType;

    if (DGuiApplicationHelper::LightType == themeType) {    // 浅色
        qCDebug(appLog) << "Setting light theme split line";
        m_pSplitLbl->setPixmap(QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg"));
    } else if (DGuiApplicationHelper::DarkType == themeType) {  // 深色
        qCDebug(appLog) << "Setting dark theme split line";
        m_pSplitLbl->setPixmap(QPixmap(":assets/icons/deepin/builtin/dark/icons/split_line_dark.svg"));
    } else {        // 其它默认
        qCDebug(appLog) << "Setting default theme split line";
        m_pSplitLbl->setPixmap(QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg"));
    }
}


