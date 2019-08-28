#ifndef COMPRESSOR_FAIL_H
#define COMPRESSOR_FAIL_H


#include <QWidget>
#include <DPushButton>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class Compressor_Fail: public QWidget
{
    Q_OBJECT
public:
    Compressor_Fail(QWidget *parent = 0);
    void InitUI();
    void InitConnection();

private:
    DPushButton* m_retrybutton;
    QPixmap m_compressicon;
    DLabel* m_pixmaplabel;
    DLabel* m_stringinfolabel;
    QString m_stringinfo;

    DLabel* m_stringdetaillabel;
    QString m_stringdetail;

public slots:
    void retrySlot();
};


#endif // COMPRESSOR_FAIL_H
