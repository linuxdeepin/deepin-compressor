#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "encryptionpage.h"

TEST(EncryptionPage_EncryptionPage_UT, EncryptionPage_EncryptionPage_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    ASSERT_NE(encryptionPage, nullptr);
    delete encryptionPage;
}

TEST(EncryptionPage_setPassowrdFocus_UT, EncryptionPage_setPassowrdFocus_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->setPassowrdFocus();
}

TEST(EncryptionPage_resetPage_UT, EncryptionPage_resetPage_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->resetPage();
}

TEST(EncryptionPage_setInputflag_UT, EncryptionPage_setInputflag_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->setInputflag(true);
}

//TEST(EncryptionPage_getPasswordEdit_UT, EncryptionPage_getPasswordEdit_UT001)
//{
//    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
//    DPasswordEdit *pwdEdit = encryptionPage->getPasswordEdit();
//    ASSERT_EQ(pwdEdit, nullptr);
//}

TEST(EncryptionPage_getPasswordEdit_UT, EncryptionPage_getPasswordEdit_UT002)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    DPasswordEdit *pwdEdit = encryptionPage->getPasswordEdit();
    ASSERT_NE(pwdEdit, nullptr);
}

TEST(EncryptionPage_nextbuttonClicked_UT, EncryptionPage_nextbuttonClicked_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->nextbuttonClicked();
}

TEST(EncryptionPage_wrongPassWordSlot_UT, EncryptionPage_wrongPassWordSlot_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->m_inputflag = true;
    encryptionPage->wrongPassWordSlot();
}

TEST(EncryptionPage_wrongPassWordSlot_UT, EncryptionPage_wrongPassWordSlot_UT002)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->m_inputflag = false;
    encryptionPage->wrongPassWordSlot();
}

TEST(EncryptionPage_onPasswordChanged_UT, EncryptionPage_onPasswordChanged_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->m_password->setText("111");
    encryptionPage->onPasswordChanged();
}

TEST(EncryptionPage_onPasswordChanged_UT, EncryptionPage_onPasswordChanged_UT002)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->m_password->setText("");
    encryptionPage->onPasswordChanged();
}

TEST(EncryptionPage_slotEchoModeChanged_UT, EncryptionPage_slotEchoModeChanged_UT001)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->slotEchoModeChanged(true);
}

TEST(EncryptionPage_slotEchoModeChanged_UT, EncryptionPage_slotEchoModeChanged_UT002)
{
    EncryptionPage *encryptionPage = new EncryptionPage(nullptr);
    encryptionPage->slotEchoModeChanged(false);
}
