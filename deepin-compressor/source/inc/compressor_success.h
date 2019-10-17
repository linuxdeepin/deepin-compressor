#ifndef COMPRESSOR_SUCCESS_H
#define COMPRESSOR_SUCCESS_H

#include <QWidget>
#include <DPushButton>
#include <DLabel>
#include <DPalette>
#include <DApplicationHelper>
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class Compressor_Success: public QWidget
{
    Q_OBJECT
public:
    Compressor_Success(QWidget *parent = 0);
    void InitUI();
    void InitConnection();
    void setstringinfo(QString str);
    void setCompressPath(QString path);

private:
    DPushButton* m_showfilebutton;
    QPixmap m_compressicon;
    DLabel* m_pixmaplabel;
    DLabel* m_stringinfolabel;
    QString m_stringinfo;
    QString m_path;
public slots:
    void showfiledirSlot();

signals:
    void sigQuitApp();
};

#endif // COMPRESSOR_SUCCESS_H
