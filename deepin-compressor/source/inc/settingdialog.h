#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H


#include <DDialog>
#include <DSettingsDialog>
#include <DStandardPaths>
#include <QDir>
#include <DSettings>
#include <DTableView>
#include <DSettingsOption>
#include "kprocess.h"
#include <DSettingsWidgetFactory>
#include <DPushButton>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


class SettingDialog: public DSettingsDialog
{
public:
    explicit SettingDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();

    int getCurExtractPath();
    bool isAutoCreatDir();
    bool isAutoOpen();

    void startcmd(QString &mimetype, bool state);

public slots:
    void settingsChanged(const QString &key, const QVariant &value);
    void selectpressed();
    void cancelpressed();
    virtual void done(int status) override;

private:
    DSettings *m_settings;
    KProcess *m_process = nullptr;
    QStringList m_associtionlist;
    QList<bool> m_valuelist;
    QList<bool> m_valuelisttemp;
};

#endif // SETTINGDIALOG_H
