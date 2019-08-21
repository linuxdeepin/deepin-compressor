#ifndef COMPRESSOR_SUCCESS_H
#define COMPRESSOR_SUCCESS_H

#include <QWidget>
#include <DSuggestButton>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class Compressor_Success: public QWidget
{
    Q_OBJECT
public:
    Compressor_Success(QWidget *parent = 0);
    void InitUI();
    void InitConnection();

private:
    DSuggestButton* m_showfilebutton;
    QPixmap m_compressicon;
    DLabel* m_pixmaplabel;
    DLabel* m_stringinfolabel;
    QString m_stringinfo;

public slots:
    void showfiledirSlot();

signals:
    void sigQuitApp();
};

#endif // COMPRESSOR_SUCCESS_H
