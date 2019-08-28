#include "lib_edit_button.h"

#include <DFileDialog>


Lib_Edit_Button::Lib_Edit_Button(DLineEdit *edit)
    :DPushButton (edit)
{
    parent = edit;
    InitUI(edit);
    InitConnection();
}

void Lib_Edit_Button::InitUI(DLineEdit *edit)
{


    QSize size = QSize(40, edit->sizeHint().height());
    setMinimumSize(size);
    setMaximumSize(size);
    setFocusPolicy(Qt::ClickFocus);
    setText("Path");

    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(this);
    edit->setLayout(m_buttonLayout);

    // 设置输入框中文件输入区，不让输入的文字在被隐藏在按钮下
    edit->setTextMargins(0, 1, size.width(), 1);

}

void Lib_Edit_Button::removebutton()
{

}

void Lib_Edit_Button::addbutton()
{

}

void Lib_Edit_Button::InitConnection()
{
//    connect(m_pathbutton, &DPushButton::clicked, this, &CompressSetting::onPathButoonClicked);
}

void Lib_Edit_Button::onPathButoonClicked()
{

}
