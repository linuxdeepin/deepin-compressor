#include "settingdialog.h"
#include <qsettingbackend.h>


SettingDialog::SettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    initUI();
    initConnect();
}

void SettingDialog::initUI()
{
    const QString confDir = DStandardPaths::writableLocation(
                    QStandardPaths::AppConfigLocation);
    const QString confPath = confDir + QDir::separator() + "deepin-compressor.conf";

    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath);

    // 通过json文件创建DSettings对象
    DSettings *settings = DSettings::fromJsonFile(":/data/deepin-compressor.json");
    // 设置DSettings存储后端
    settings->setBackend(backend);

    // 通过DSettings对象构建设置界面
    updateSettings(settings);

    connect(settings, &DSettings::valueChanged,
            this, &SettingDialog::settingsChanged);


}

void SettingDialog::initConnect()
{

}

void SettingDialog::settingsChanged(const QString &key, const QVariant &value)
{

}
