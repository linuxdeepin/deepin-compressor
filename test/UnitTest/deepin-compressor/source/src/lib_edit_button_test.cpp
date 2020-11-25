#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "lib_edit_button.h"

TEST(Lib_Edit_Button_Lib_Edit_Button_UT, Lib_Edit_Button_Lib_Edit_Button_UT001)
{
    DLineEdit *lineEdit = new DLineEdit(nullptr);
    Lib_Edit_Button *libEditButton = new Lib_Edit_Button(lineEdit);
    ASSERT_NE(libEditButton, nullptr);
    delete libEditButton;
}

TEST(Lib_Edit_Button_removebutton_UT, Lib_Edit_Button_removebutton_UT001)
{
    DLineEdit *lineEdit = new DLineEdit(nullptr);
    Lib_Edit_Button *libEditButton = new Lib_Edit_Button(lineEdit);
    libEditButton->removebutton();
    ASSERT_NE(libEditButton, nullptr);
    delete libEditButton;
}

TEST(Lib_Edit_Button_addbutton_UT, Lib_Edit_Button_addbutton_UT001)
{
    DLineEdit *lineEdit = new DLineEdit(nullptr);
    Lib_Edit_Button *libEditButton = new Lib_Edit_Button(lineEdit);
    libEditButton->addbutton();
    ASSERT_NE(libEditButton, nullptr);
    delete libEditButton;
}

TEST(Lib_Edit_Button_onPathButoonClicked_UT, Lib_Edit_Button_onPathButoonClicked_UT001)
{
    DLineEdit *lineEdit = new DLineEdit(nullptr);
    Lib_Edit_Button *libEditButton = new Lib_Edit_Button(lineEdit);
    libEditButton->onPathButoonClicked();
    ASSERT_NE(libEditButton, nullptr);
    delete libEditButton;
}
