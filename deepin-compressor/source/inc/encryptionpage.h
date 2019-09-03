#ifndef ENCRYPTIONPAGE_H
#define ENCRYPTIONPAGE_H
#include <QWidget>
#include <DPushButton>
#include <DLabel>
#include <dpasswordedit.h>



DWIDGET_USE_NAMESPACE

class EncryptionPage: public QWidget
{
    Q_OBJECT
public:
    EncryptionPage(QWidget *parent = 0);
    void InitUI();
    void InitConnection();


private:
    QPixmap m_encrypticon;
    DLabel* m_pixmaplabel;
    DLabel* m_stringinfolabel;
    DPushButton* m_nextbutton;
    DPasswordEdit* m_password;
public slots:
    void nextbuttonClicked();
    void wrongPassWordSlot();

signals:
    void sigExtractPassword(QString password);
};

#endif // ENCRYPTIONPAGE_H
