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

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


class SettingDialog: public DSettingsDialog
{
public:
    explicit SettingDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();

<<<<<<< HEAD
=======
    int getCurExtractPath();
    bool isAutoCreatDir();
    bool isAutoOpen();

    void startcmd(QString &mimetype, bool state);

>>>>>>> feat(Compressor):add settingpage
public slots:
    void settingsChanged(const QString &key, const QVariant &value);

private:
    DSettings *m_settings;
    KProcess *m_process = nullptr;
};

#endif // SETTINGDIALOG_H
