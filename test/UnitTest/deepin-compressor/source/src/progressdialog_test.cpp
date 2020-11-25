#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "progressdialog.h"

TEST(ProgressDialog_ProgressDialog_UT, ProgressDialog_ProgressDialog_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    ASSERT_NE(progressDialog, nullptr);
    delete progressDialog;
}

TEST(Progressdialog_slotextractpress_UT, Progressdialog_slotextractpress_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->slotextractpress(1);
}

TEST(Progressdialog_slotextractpress_UT, Progressdialog_slotextractpress_UT002)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->slotextractpress(0);
}

TEST(Progressdialog_closeEvent_UT, Progressdialog_closeEvent_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->m_circleprogress->setValue(50);
    progressDialog->closeEvent(nullptr);
}

TEST(Progressdialog_setCurrentTask_UT, Progressdialog_setCurrentTask_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->setCurrentTask("/home/chenglu/Desktop/pic.rar");
//    std::cout << progressDialog->m_tasklable->text();
}

TEST(Progressdialog_setCurrentFile_UT, Progressdialog_setCurrentFile_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->setCurrentFile("pic.png");
//    std::cout << progressDialog->m_filelable->text();
}

TEST(Progressdialog_setProcess_UT, Progressdialog_setProcess_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->setProcess(100);
//    std::cout << progressDialog->m_circleprogress->value();
}

TEST(Progressdialog_setProcess_UT, Progressdialog_setProcess_UT002)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->setProcess(50);
//    std::cout << progressDialog->m_circleprogress->value();
}

TEST(Progressdialog_setFinished_UT, Progressdialog_setFinished_UT002)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->setFinished("/home/chenglu/Desktop");
}

TEST(Progressdialog_setMsg_UT, Progressdialog_setMsg_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->setMsg("Skip all files");
}

TEST(Progressdialog_clearprocess_UT, Progressdialog_clearprocess_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->clearprocess();
    ASSERT_EQ(progressDialog->m_circleprogress->value(), 0);
}

TEST(Progressdialog_showdialog_UT, Progressdialog_showdialog_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    progressDialog->showdialog();
}

TEST(Progressdialog_isshown_UT, Progressdialog_isshown_UT001)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    bool isshow = progressDialog->isshown();
    ASSERT_EQ(isshow, true);
}

TEST(Progressdialog_isshown_UT, Progressdialog_isshown_UT002)
{
    ProgressDialog *progressDialog = new ProgressDialog(nullptr);
    bool isshow = progressDialog->isshown();
    ASSERT_EQ(isshow, false);
}
