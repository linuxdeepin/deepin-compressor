#ifndef LIB_EDIT_BUTTON_H
#define LIB_EDIT_BUTTON_H

#include <DLineEdit>
#include <DPushButton>
#include <QWidget>
#include <QBoxLayout>

DWIDGET_USE_NAMESPACE

class Lib_Edit_Button :public DPushButton
{
public:
    Lib_Edit_Button(DLineEdit *edit = nullptr);
    void InitUI(DLineEdit *edit);
    void InitConnection();

public:
    void removebutton();
    void addbutton();

public slots:
    void onPathButoonClicked();

private:
    DLineEdit *parent;
    QHBoxLayout *m_buttonLayout;
};

#endif // LIB_EDIT_BUTTON_H
