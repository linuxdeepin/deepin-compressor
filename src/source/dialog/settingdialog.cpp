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
    m_associtionList << "file_association.file_association_type.x-7z-compressed"
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

bool SettingDialog::isAssociatedType(QString mime)
{
    return m_settings->option("file_association.file_association_type." + mime.remove("application/"))->value().toBool();
}

void SettingDialog::initUI()
{
    createSettingButton();
    createPathBox();
    createDeleteBox();

    // 通过json文件创建DSettings对象
    m_settings = DSettings::fromJsonFile(":assets/data/deepin-compressor.json");

    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const QString confPath = confDir + QDir::separator() + "deepin-compressor.conf";

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
    qDebug() << "slotSettingsChanged:  " << key;
    qDebug() << value;
}

void SettingDialog::slotClickSelectAllButton()
{
    foreach (QString key, m_associtionList) {
        m_settings->setOption(key, true);
    }
}

void SettingDialog::slotClickCancelSelectAllButton()
{
    foreach (QString key, m_associtionList) {
        m_settings->setOption(key, false);
    }
}

void SettingDialog::slotClickRecommendedButton()
{
    foreach (QString key, m_associtionList) {
        if (key == "file_association.file_association_type.x-cd-image"
                || key == "file_association.file_association_type.x-iso9660-appimage"
                || key == "file_association.file_association_type.x-source-rpm") {
            m_settings->setOption(key, false);
        } else {
            m_settings->setOption(key, true);
        }
    }
}
