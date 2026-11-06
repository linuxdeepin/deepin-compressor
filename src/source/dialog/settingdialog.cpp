// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "customwidget.h"
#include "uitools.h"
#include "common/ddlog.h"

#include <DPushButton>
#include <DPaletteHelper>
#include <DSettingsWidgetFactory>
#include <DFileDialog>
#include <DLabel>
#include <DStandardPaths>

#include <QDebug>
#include <QLayout>
#include <QFileInfo>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QWidget>
#include <QCoreApplication>
#include <qsettingbackend.h>

DGUI_USE_NAMESPACE

// "打开对应文件夹后自动退出"作为"当解压完成后自动打开对应的文件夹"的二级选项，
// 在设置界面中向右缩进的像素数，用于体现两者的从属关系
static const int DEPENDENT_CHECKBOX_INDENT = 24;

// 二级复选框与上级选项的关联键
static const char *OPEN_FOLDER_OPTION_KEY = "base.decompress.open_folder";
static const char *AUTO_QUIT_OPTION_KEY = "base.decompress.auto_quit_after_open_folder";

SettingDialog::SettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    qCDebug(appLog) << "Creating SettingDialog";
    initUI();
    initConnections();
    qCDebug(appLog) << "SettingDialog initialized";
}

SettingDialog::~SettingDialog()
{
    qCDebug(appLog) << "Destroying SettingDialog";
    SAFE_DELETE_ELE(m_settings);
    qCDebug(appLog) << "SettingDialog destroyed";
}

QString SettingDialog::getDefaultExtractPath()
{
    qCDebug(appLog) << "Getting default extract path:" << m_curpath;
    return m_curpath;
}

bool SettingDialog::isAutoCreatDir()
{
    // 读取配置文件信息 是否自动创建文件夹
    bool autoCreate = m_settings->value("base.decompress.create_folder").toBool();
    qCDebug(appLog) << "Auto create folder setting:" << autoCreate;
    return autoCreate;
}

bool SettingDialog::isAutoOpen()
{
    // 读取配置文件信息 是否自动打开文件夹
    bool autoOpen = m_settings->value("base.decompress.open_folder").toBool();
    qCDebug(appLog) << "Auto open folder setting:" << autoOpen;
    return autoOpen;
}

bool SettingDialog::isAutoQuitAfterOpen()
{
    // 读取配置文件信息 打开对应文件夹后是否自动退出归档管理器
    // 该选项依赖"当解压完成后自动打开对应的文件夹"：上级未开启时，即使残留下级配置也不生效
    bool autoQuit = m_settings->value(OPEN_FOLDER_OPTION_KEY).toBool()
                    && m_settings->value(AUTO_QUIT_OPTION_KEY).toBool();
    qCDebug(appLog) << "Auto quit after open folder setting:" << autoQuit;
    return autoQuit;
}

bool SettingDialog::isAutoDeleteFile()
{
    // 读取配置文件信息 压缩后是否删除源文件
    bool autoDelete = m_settings->value("base.file_management.delete_file").toBool();
    qCDebug(appLog) << "Auto delete file after compression:" << autoDelete;
    return autoDelete;
}

QString SettingDialog::isAutoDeleteArchive()
{
    // 读取配置文件信息 解压后是否删除压缩文件
    QString deleteOption = m_deleteArchiveOption->value().toString();
    qCDebug(appLog) << "Delete archive after extraction option:" << deleteOption;
    return deleteOption;
}

bool SettingDialog::isAssociatedType(QString mime) // 暂时未被调用
{
    // 读取配置文件信息 是否是解压缩关联文件类型
    QString key = "file_association.file_association_type." + mime.remove("application/");
    bool isAssociated = m_settings->option(key)->value().toBool();
    qCDebug(appLog) << "File association for" << mime << ":" << isAssociated;
    return isAssociated;
}

void SettingDialog::initUI()
{
    qCDebug(appLog) << "Initializing SettingDialog UI";
    createSettingButton(); // 创建选择按钮
    createPathBox(); // 创建默认解压路径
    createDeleteBox(); // 创建删除选项
    createDependentCheckBox(); // 创建"打开对应文件夹后自动退出"联动复选框

    // 通过json文件创建DSettings对象
    m_settings = DSettings::fromJsonFile(":assets/data/deepin-compressor.json");
    qCDebug(appLog) << "Settings loaded from JSON file";

    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation); // 换了枚举值，待验证
    const QString confPath = confDir + QDir::separator() + "deepin-compressor.conf";
    qCDebug(appLog) << "Config path:" << confPath;

    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath, this);
    m_settings->setBackend(backend);
    qCDebug(appLog) << "Settings backend initialized";

    // 通过DSettings对象构建设置界面
    updateSettings(m_settings);
    qCDebug(appLog) << "Settings UI updated";

    writeConfbf();  // 在应用被打开的时候，写新的config文件 具体功能待验证
    qCDebug(appLog) << "Initial config file written";
}

void SettingDialog::initConnections()
{
    qCDebug(appLog) << "Initializing SettingDialog connections";
    //m_setting需要setBackend,该信号才会有效
    connect(m_settings, &DSettings::valueChanged, this, &SettingDialog::slotSettingsChanged);

    // 联动保护：取消勾选"当解压完成后自动打开对应的文件夹"时，同步取消"打开对应文件夹后自动退出"，
    // 避免残留下级配置（下级复选框的置灰状态由自定义控件联动处理）
    connect(m_settings, &DSettings::valueChanged, this, [this](const QString &strKey, const QVariant &value) {
        if (strKey == QLatin1String(OPEN_FOLDER_OPTION_KEY) && !value.toBool()) {
            DSettingsOption *pAutoQuitOption = m_settings->option(AUTO_QUIT_OPTION_KEY).data();
            if (nullptr != pAutoQuitOption && pAutoQuitOption->value().toBool()) {
                pAutoQuitOption->setValue(false);
                qCDebug(appLog) << "Auto quit disabled because auto open folder was unchecked";
            }
        }
    });
}

void SettingDialog::createSettingButton()
{
    qCDebug(appLog) << "Creating custom setting button";
    this->widgetFactory()->registerWidget("custom-button", [this](QObject * obj) -> QWidget* {
        m_customButtonOption = qobject_cast<DSettingsOption *>(obj);
        if (m_customButtonOption)
        {
            QWidget *buttonwidget = new QWidget();
            QHBoxLayout *layout = new QHBoxLayout();
            CustomPushButton *selectAllButton = new CustomPushButton(tr("Select All", "button"));
            CustomPushButton *cancelSelectButton = new CustomPushButton(tr("Clear All"));
            CustomSuggestButton *recommendedButton = new CustomSuggestButton(tr("Recommended"));
            selectAllButton->setMinimumWidth(153);
            cancelSelectButton->setMinimumWidth(153);
            recommendedButton->setMinimumWidth(153);
            layout->addStretch();
            layout->addWidget(selectAllButton);
            layout->addStretch();
            layout->addWidget(cancelSelectButton);
            layout->addStretch();
            layout->addWidget(recommendedButton);
            layout->addStretch();
            buttonwidget->setLayout(layout);

            connect(selectAllButton, &QPushButton::clicked, this, &SettingDialog::slotClickSelectAllButton);
            connect(cancelSelectButton, &QPushButton::clicked, this, &SettingDialog::slotClickCancelSelectAllButton);
            connect(recommendedButton, &QPushButton::clicked, this, &SettingDialog::slotClickRecommendedButton);
            return buttonwidget;
        }

        return nullptr;
    });
}

void SettingDialog::createPathBox()
{
    qCDebug(appLog) << "Creating path box";
    this->widgetFactory()->registerWidget("pathbox", [this](QObject * obj) -> QWidget* {
        m_extractPathOption = qobject_cast<DSettingsOption *>(obj);
        if (m_extractPathOption)
        {
            DWidget *widget = new DWidget(this);
            QHBoxLayout *layout = new QHBoxLayout();

            DLabel *label = new DLabel(widget);
            label->setForegroundRole(DPalette::WindowText);
            label->setText(tr("Extract archives to") + ":");

            CustomCombobox *combobox = new CustomCombobox(widget);
            combobox->setMinimumWidth(300);
            combobox->setEditable(false);
            QStringList list;
            list << tr("Current directory") << tr("Desktop") << tr("Other directory");
            combobox->addItems(list);

            if (QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) == m_extractPathOption->value()) {
                combobox->setCurrentIndex(1);
                m_curpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                m_index_last = 1;
            } else if ("" == m_extractPathOption->value()) {
                combobox->setCurrentIndex(0);
                m_curpath = "";
                m_index_last = 0;
            } else {
                combobox->setEditable(true);
                combobox->setCurrentIndex(2);
                m_curpath = m_extractPathOption->value().toString();
                combobox->setEditText(m_curpath);
                m_index_last = 2;
            }

            layout->addWidget(label, 0, Qt::AlignLeft);
            layout->addWidget(combobox, 0, Qt::AlignLeft);

            widget->setLayout(layout);

            connect(this, &SettingDialog::sigResetPath, this, [ = ] {
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
                    DPalette plt = DPaletteHelper::instance()->palette(combobox);

                    if (!dir.exists()) {
                        plt.setBrush(DPalette::Text, plt.color(DPalette::TextWarning));
                    } else {
                        plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));
                    }

                    combobox->setPalette(plt);
                    m_index_last = 2;
                }

                m_extractPathOption->setValue(m_curpath);
            });

            qCInfo(appLog) << m_curpath;
            return widget;
        }

        return nullptr;
    });
}

void SettingDialog::createDeleteBox()
{
    qCDebug(appLog) << "Creating delete box";
    this->widgetFactory()->registerWidget("deletebox", [this](QObject * obj) -> QWidget* {
        m_deleteArchiveOption = qobject_cast<DSettingsOption *>(obj);
        if (m_deleteArchiveOption)
        {
            DWidget *widget = new DWidget(this);
            QHBoxLayout *layout = new QHBoxLayout();

            DLabel *label = new DLabel(widget);
            label->setForegroundRole(DPalette::WindowText);
            label->setText(tr("Delete archives after extraction") + ":");

            CustomCombobox *combobox = new CustomCombobox(widget);
            combobox->setMinimumWidth(300);
            combobox->setEditable(false);
            QStringList list;
            list << tr("Never") << tr("Ask for confirmation") << tr("Always");
            combobox->addItems(list);

            if ("Always" == m_deleteArchiveOption->value()) {
                combobox->setCurrentIndex(2);
            } else if ("Ask for confirmation" == m_deleteArchiveOption->value()) {
                combobox->setCurrentIndex(1);
            } else {
                combobox->setCurrentIndex(0);
            }

            layout->addWidget(label, 0, Qt::AlignLeft);
            layout->addWidget(combobox, 0, Qt::AlignLeft);

            widget->setLayout(layout);

            connect(this, &SettingDialog::sigResetDeleteArchive, this, [ = ] {
                combobox->setCurrentIndex(0);
            });

            connect(combobox, &DComboBox::currentTextChanged, [combobox, this] {
                if (tr("Never") == combobox->currentText())
                {
                    m_autoDeleteArchive = "Never";
                } else if (tr("Ask for confirmation") == combobox->currentText())
                {
                    m_autoDeleteArchive = "Ask for confirmation";
                } else if (tr("Always") == combobox->currentText())
                {
                    m_autoDeleteArchive = "Always";
                } else
                {
                    m_autoDeleteArchive = combobox->currentText();
                }

                m_deleteArchiveOption->setValue(m_autoDeleteArchive);
            });
            if(parent()&&parent()->property(ORDER_JSON).isValid()) {
                combobox->setCurrentIndex(0);
                combobox->setEnabled(false);
            }
            return widget;
        }

        return nullptr;
    });
}

void SettingDialog::createDependentCheckBox()
{
    qCDebug(appLog) << "Creating dependent checkbox";
    // 自定义设置控件类型：用于"打开对应文件夹后自动退出"
    // 1. 作为"当解压完成后自动打开对应的文件夹"的二级选项，界面整体向右缩进；
    // 2. 上级选项未勾选时置灰不可操作。
    this->widgetFactory()->registerWidget("checkbox-dependent", [this](QObject * obj) -> QWidget* {
        DSettingsOption *pOption = qobject_cast<DSettingsOption *>(obj);
        if (nullptr == pOption) {
            return nullptr;
        }

        // 翻译处理与内建 checkbox 保持一致：translateContext 为空时使用 QObject 上下文
        // （settings_translation.cpp 中的设置项文案均在 QObject 上下文中）
        const QByteArray translateContext = obj->property("_d_DSettingsWidgetFactory_translateContext").toByteArray();
        const QString strSourceText = pOption->data("text").toString();
        const QString strTrText = translateContext.isEmpty()
                                  ? QObject::tr(strSourceText.toUtf8().constData())
                                  : QCoreApplication::translate(translateContext.constData(), strSourceText.toUtf8().constData());

        // 使用容器承载复选框，以便通过左边距实现二级选项的缩进效果
        QWidget *pContainer = new QWidget();
        QHBoxLayout *pLayout = new QHBoxLayout(pContainer);
        pLayout->setContentsMargins(DEPENDENT_CHECKBOX_INDENT, 0, 0, 0);
        pLayout->setSpacing(0);

        QCheckBox *pCheckBox = new QCheckBox(strTrText, pContainer);
        pCheckBox->setObjectName("OptionCheckbox");
        pCheckBox->setAccessibleName("OptionCheckbox");
        pCheckBox->setChecked(pOption->value().toBool());

        pLayout->addWidget(pCheckBox, 0, Qt::AlignLeft);
        pLayout->addStretch(1);

        // 与上级选项联动：上级未勾选时置灰
        DSettingsOption *pParentOption = m_settings->option(OPEN_FOLDER_OPTION_KEY).data();
        auto updateEnabledState = [pCheckBox, pParentOption]() {
            pCheckBox->setEnabled(nullptr != pParentOption && pParentOption->value().toBool());
        };
        updateEnabledState();

        if (nullptr != pParentOption) {
            connect(pParentOption, &DSettingsOption::valueChanged, pCheckBox, [updateEnabledState](const QVariant &) {
                updateEnabledState();
            });
        }

        // 复选框与设置项双向同步
        connect(pCheckBox, &QCheckBox::toggled, pOption, [pOption](bool bChecked) {
            pOption->setValue(bChecked);
        });
        connect(pOption, &DSettingsOption::valueChanged, pCheckBox, [pCheckBox](const QVariant &value) {
            if (pCheckBox->isChecked() != value.toBool()) {
                pCheckBox->setChecked(value.toBool());
            }
        });

        return pContainer;
    });
}

void SettingDialog::writeConfbf()
{
    qCDebug(appLog) << "Writing configuration to file";
    // 设置配置文件的绝对路径
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QString confPath = confDir + QDir::separator() + "deepin-compressor.confbf";

    // 打开配置文件
    QFile file(confPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    // 写配置
    qCDebug(appLog) << "Writing configuration to file:" << confPath;
    foreach (QString key, UiTools::m_associtionlist) {
        QString bValue = m_settings->option(key)->value().toString();
        QString content = key + ":" + bValue + "\n";
        file.write(content.toUtf8());
        qCDebug(appLog) << "Config entry:" << key << "=" << bValue;
    }

    file.close();
}

void SettingDialog::slotSettingsChanged(const QString &key, const QVariant &value)
{
    qCDebug(appLog) << "Setting changed - Key:" << key << "Value:" << value;
    // 设置界面点击恢复默认选项  具体功能待确认、完善
    qCInfo(appLog) << "slotSettingsChanged:  " << key  << value;
    writeConfbf();
    qCDebug(appLog) << "Config file updated after setting change";
}

void SettingDialog::slotClickSelectAllButton()
{
    qCDebug(appLog) << "Select All button clicked";
    foreach (QString key, UiTools::m_associtionlist) {
        m_settings->setOption(key, true);
    }
    qCDebug(appLog) << "All file associations selected";
}

void SettingDialog::slotClickCancelSelectAllButton()
{
    qCDebug(appLog) << "Cancel Select All button clicked";
    foreach (QString key, UiTools::m_associtionlist) {
        m_settings->setOption(key, false);
    }
    qCDebug(appLog) << "All file associations deselected";
}

void SettingDialog::slotClickRecommendedButton()
{
    qCDebug(appLog) << "Recommended button clicked";
    foreach (QString key, UiTools::m_associtionlist) {
        if ("file_association.file_association_type.x-iso9660-image" == key
                || "file_association.file_association_type.x-iso9660-appimage" == key
                || "file_association.file_association_type.x-source-rpm" == key) {
            m_settings->setOption(key, false);
            qCDebug(appLog) << "Excluded association:" << key;
        } else {
            m_settings->setOption(key, true);
            qCDebug(appLog) << "Included association:" << key;
        }
    }
    qCDebug(appLog) << "Recommended file associations set";
}
