#ifndef ENCODINGPAGE_H
#define ENCODINGPAGE_H
#include <QWidget>
#include <DPushButton>
#include <DLabel>
#include "utils.h"
#include <DComboBox>
#include <DPalette>
#include <DApplicationHelper>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class EncodingPage: public QWidget
{
    Q_OBJECT
public:
    EncodingPage(QWidget *parent = 0);
    void InitUI();
    void InitConnection();

    void setTypeImage(QString type);
    void setFilename(QString filename);
private:
    QPixmap m_fileicon;
    DLabel* m_pixmaplabel;
    DLabel* m_filenamelabel;
    DComboBox* m_codebox;
    DLabel* m_codelabel;
    DLabel* m_detaillabel;
    DPushButton* m_cancelbutton;
    DPushButton* m_confirmbutton;
};

#endif // ENCODINGPAGE_H
