#include "settingdialog.h"
#include "customwidget.h"

#include <DPushButton>
#include <DApplicationHelper>
#include <DSettingsWidgetFactory>
#include <DFileDialog>
#include <DLabel>
#include <DStandardPaths>

#include <QDebug>
#include <QLayout>
#include <QFileInfo>
#include <qsettingbackend.h>

DGUI_USE_NAMESPACE

SettingDialog::SettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    initUI();
    initConnections();
}

SettingDialog::~SettingDialog()
{

}

QString SettingDialog::getDefaultExtractPath()
{
    return m_curpath;
}

bool SettingDialog::isAutoCreatDir()
{
    // 读取配置文件信息
    return m_settings->value("base.decompress.create_folder").toBool();
}

bool SettingDialog::isAutoOpen()
{
    // 读取配置文件信息
    return m_settings->value("base.decompress.open_folder").toBool();
}

bool SettingDialog::isAutoDeleteFile()
{
    return m_settings->value("base.file_management.delete_file").toBool();
}

QString SettingDialog::isAutoDeleteArchive()
{
    return m_deleteArchiveOption->value().toString();
}

void SettingDialog::initUI()
{
    createSettingButton();
    createPathBox();
    createDeleteBox();

    // 通过json文件创建DSettings对象
    m_settings = DSettings::fromJsonFile(":assets/data/deepin-compressor.json");

    // 其他路径好像无效...
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    //    const QString confDir = "/home/chenglu/Desktop/refactor";
    const QString confPath = confDir + QDir::separator() + "deepin-compressor.conf";
    //    QDir dir(confDir);
    //    if (!dir.exists()) {
    //        dir.mkpath(confDir);
    //    }

    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath, this);
    m_settings->setBackend(backend);


    // 通过DSettings对象构建设置界面
    updateSettings(m_settings);
}

void SettingDialog::initConnections()
{
    //m_setting需要setBackend,该信号才会有效
    connect(m_settings, &DSettings::valueChanged, this, &SettingDialog::slotSettingsChanged);
}

void SettingDialog::createSettingButton()
{
    this->widgetFactory()->registerWidget("custom-button", [this](QObject * obj) -> QWidget* {
        m_customButtonOption = qobject_cast<DSettingsOption *>(obj);
        if (m_customButtonOption)
        {
            QWidget *buttonwidget = new QWidget();
            QHBoxLayout *layout = new QHBoxLayout();
            CustomPushButton *selectAllButton = new CustomPushButton(tr("Select All"));
            CustomPushButton *cancelSelectButton = new CustomPushButton(tr("Clear All"));
            CustomSuggestButton *recommendedButton = new CustomSuggestButton(tr("Recommended"));
            selectAllButton->setMinimumSize(153, 36);
            cancelSelectButton->setMinimumSize(153, 36);
            recommendedButton->setMinimumSize(153, 36);
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
                    DPalette plt = DApplicationHelper::instance()->palette(combobox);

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

            qDebug() << m_curpath;
            return widget;
        }

        return nullptr;
    });
}

void SettingDialog::createDeleteBox()
{
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

            return widget;
        }

        return nullptr;
    });
}

void SettingDialog::slotSettingsChanged(const QString &key, const QVariant &value)
{
    qDebug() << "click reset button";
}

void SettingDialog::slotClickSelectAllButton()
{
    qDebug() << "click select all button";
}

void SettingDialog::slotClickCancelSelectAllButton()
{
    qDebug() << "click cancel select all button";
}

void SettingDialog::slotClickRecommendedButton()
{
    qDebug() << "click recommended button";
}

