#ifndef COMPRESSSETTING_H
#define COMPRESSSETTING_H

#include <QWidget>
#include <DSuggestButton>
#include <QImage>
#include <QComboBox>
#include <DLabel>
#include <DLineEdit>
#include "utils.h"
#include <dimagebutton.h>
#include <dswitchbutton.h>
#include <dpasswordedit.h>
#include <QVBoxLayout>
#include "lib_edit_button.h"

DWIDGET_USE_NAMESPACE

#define D_COMPRESS_SPLIT_MIX    100
#define D_COMPRESS_SPLIT_MIN   1

class CompressSetting :public QWidget
{
    Q_OBJECT
public:
    CompressSetting(QWidget* parent = 0);
    ~CompressSetting();

    void InitUI();
    void InitConnection();

private:
    DSuggestButton* m_nextbutton;
    QPixmap m_compressicon;
    QComboBox* m_compresstype;
    DLineEdit* m_filename;
    DLineEdit* m_savepath;
    DLabel* m_pixmaplabel;
    Lib_Edit_Button* m_pathbutton;
    QVBoxLayout *m_fileLayout;

    QHBoxLayout *m_moresetlayout;
    DSwitchButton* m_moresetbutton;
    DPasswordEdit* m_password;
    QHBoxLayout *m_file_secretlayout;
    DSwitchButton* m_file_secret;
    QHBoxLayout *m_splitlayout;
    DLineEdit* m_splitnumedit;
    DSuggestButton* m_plusbutton;
    DSuggestButton* m_minusbutton;
    DLabel* m_encryptedlabel;
    DLabel* m_splitcompress;
    DLabel* m_encryptedfilelistlabel;

    uint m_splitnum;

signals:
    void sigCompressPressed();

public slots:
    void onPathButoonClicked();
    void onNextButoonClicked();
    void onAdvanceButtonClicked(bool status);
    void onPlusButoonClicked();
    void onLessButoonClicked();

    void onRetrunPressed();
};

#endif // COMPRESSSETTING_H
