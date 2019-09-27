#ifndef COMPRESSSETTING_H
#define COMPRESSSETTING_H

#include <QWidget>
#include <DPushButton>
#include <DComboBox>
#include <DLabel>
#include <DLineEdit>
#include "utils.h"
#include <DSwitchButton>
#include <dpasswordedit.h>
#include <QVBoxLayout>
#include <DSpinBox>
#include "lib_edit_button.h"
#include "pluginmanager.h"
#include "dfilechooseredit.h"

DWIDGET_USE_NAMESPACE

#define D_COMPRESS_SPLIT_MIX    5000
#define D_COMPRESS_SPLIT_MIN   0

class CompressSetting :public QWidget
{
    Q_OBJECT
public:
    CompressSetting(QWidget* parent = 0);
    ~CompressSetting();

    void InitUI();
    void InitConnection();

    void setTypeImage(QString type);
    void setDefaultPath(QString path);
    void setDefaultName(QString name);

private:
    DPushButton* m_nextbutton;
    QPixmap m_compressicon;
    DComboBox* m_compresstype;
    DLineEdit* m_filename;
    DFileChooserEdit* m_savepath;
    DLabel* m_pixmaplabel;
//    Lib_Edit_Button* m_pathbutton;
    QVBoxLayout *m_fileLayout;

    QHBoxLayout *m_moresetlayout;
    DSwitchButton* m_moresetbutton;
    DPasswordEdit* m_password;
    QHBoxLayout *m_file_secretlayout;
    DSwitchButton* m_file_secret;
    QHBoxLayout *m_splitlayout;
    DDoubleSpinBox* m_splitnumedit;
    DPushButton* m_plusbutton;
    DPushButton* m_minusbutton;
    DLabel* m_encryptedlabel;
    DLabel* m_splitcompress;
    DLabel* m_encryptedfilelistlabel;

    PluginManager m_pluginManger;
    QStringList m_supportedMimeTypes;


signals:
    void sigCompressPressed(QMap<QString, QString> &Args);

public slots:
    void onNextButoonClicked();
    void onAdvanceButtonClicked(bool status);
    void ontypeChanged(int index);

    void onRetrunPressed();
};

#endif // COMPRESSSETTING_H
