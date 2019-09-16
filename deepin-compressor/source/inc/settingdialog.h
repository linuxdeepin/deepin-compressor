#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H


#include <DDialog>
#include <DSettingsDialog>
#include <DStandardPaths>
#include <QDir>
#include <DSettings>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


class SettingDialog: public DSettingsDialog
{
public:
    explicit SettingDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();

public slots:
    void settingsChanged(const QString &key, const QVariant &value);
};

#endif // SETTINGDIALOG_H
