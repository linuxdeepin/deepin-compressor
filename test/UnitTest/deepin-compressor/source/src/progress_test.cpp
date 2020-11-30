#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "progress.h"

TEST(Progress_Progress_UT, Progress_Progress_UT001)
{
    Progress *progress = new Progress(nullptr);
    ASSERT_NE(progress, nullptr);
    delete progress;
}

TEST(Progress_setSpeedAndTimeText_UT, Progress_setSpeedAndTimeText_UT001)
{
    Progress *progress = new Progress(nullptr);
    Progress::ENUM_PROGRESS_TYPE type = Progress::ENUM_PROGRESS_TYPE::OP_NONE;
    progress->setSpeedAndTimeText(type);
}

TEST(Progress_setSpeedAndTimeText_UT, Progress_setSpeedAndTimeText_UT002)
{
    Progress *progress = new Progress(nullptr);
    Progress::ENUM_PROGRESS_TYPE type = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
    progress->setSpeedAndTimeText(type);
}

TEST(Progress_setSpeedAndTimeText_UT, Progress_setSpeedAndTimeText_UT003)
{
    Progress *progress = new Progress(nullptr);
    Progress::ENUM_PROGRESS_TYPE type = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD;
    progress->setSpeedAndTimeText(type);
}

TEST(Progress_setSpeedAndTimeText_UT, Progress_setSpeedAndTimeText_UT004)
{
    Progress *progress = new Progress(nullptr);
    Progress::ENUM_PROGRESS_TYPE type = Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING;
    progress->setSpeedAndTimeText(type);
}

TEST(Progress_setSpeedAndTimeText_UT, Progress_setSpeedAndTimeText_UT005)
{
    Progress *progress = new Progress(nullptr);
    Progress::ENUM_PROGRESS_TYPE type = Progress::ENUM_PROGRESS_TYPE::OP_DELETEING;
    progress->setSpeedAndTimeText(type);
}

TEST(Progress_setSpeedAndTimeText_UT, Progress_setSpeedAndTimeText_UT006)
{
    Progress *progress = new Progress(nullptr);
    Progress::ENUM_PROGRESS_TYPE type = Progress::ENUM_PROGRESS_TYPE::OP_CONVERT;
    progress->setSpeedAndTimeText(type);
}

TEST(Progress_refreshSpeedAndTime_UT, Progress_refreshSpeedAndTime_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->refreshSpeedAndTime(50, false);
}

TEST(Progress_pInfo_UT, Progress_pInfo_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->pInfo();
}

TEST(Progress_resetPauseContinueButton_UT, Progress_resetPauseContinueButton_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->resetPauseContinueButton();
}

TEST(Progress_hidePauseContinueButton_UT, Progress_hidePauseContinueButton_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
    progress->hidePauseContinueButton();
}

TEST(Progress_hidePauseContinueButton_UT, Progress_hidePauseContinueButton_UT002)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_NONE;
    progress->hidePauseContinueButton();
}

TEST(Progress_slotChangeTimeLeft_UT, Progress_slotChangeTimeLeft_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->lastTimeLeft = 1;
    progress->slotChangeTimeLeft();
}

TEST(Progress_slotChangeTimeLeft_UT, Progress_slotChangeTimeLeft_UT002)
{
    Progress *progress = new Progress(nullptr);
    progress->lastTimeLeft = 2;
    progress->slotChangeTimeLeft();
}

TEST(Progress_setprogress_UT, Progress_setprogress_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->setprogress(20);
}

TEST(Progress_setSpeedAndTime_UT, Progress_setSpeedAndTime_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->lastTimeLeft = 100;
    progress->setSpeedAndTime(100, 80);
}

TEST(Progress_setSpeedAndTime_UT, Progress_setSpeedAndTime_UT002)
{
    Progress *progress = new Progress(nullptr);
    progress->lastTimeLeft = 100;
    progress->setSpeedAndTime(100, 20);
}

TEST(Progress_setSpeedAndTime_UT, Progress_setSpeedAndTime_UT003)
{
    Progress *progress = new Progress(nullptr);
    progress->lastTimeLeft = 1;
    progress->setSpeedAndTime(100, 20);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
    progress->displaySpeedAndTime(1000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT002)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
    progress->displaySpeedAndTime(3000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT003)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
    progress->displaySpeedAndTime(400000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT004)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DELETEING;
    progress->displaySpeedAndTime(1000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT005)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DELETEING;
    progress->displaySpeedAndTime(3000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT006)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DELETEING;
    progress->displaySpeedAndTime(400000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT007)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD;
    progress->displaySpeedAndTime(1000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT008)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD;
    progress->displaySpeedAndTime(3000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT009)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSDRAGADD;
    progress->displaySpeedAndTime(400000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT010)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING;
    progress->displaySpeedAndTime(1000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT011)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING;
    progress->displaySpeedAndTime(3000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT012)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DECOMPRESSING;
    progress->displaySpeedAndTime(400000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT013)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_CONVERT;
    progress->displaySpeedAndTime(1000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT014)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_CONVERT;
    progress->displaySpeedAndTime(3000, 120);
}

TEST(Progress_displaySpeedAndTime_UT, Progress_displaySpeedAndTime_UT015)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_CONVERT;
    progress->displaySpeedAndTime(400000, 120);
}

TEST(Progress_setFilename_UT, Progress_setFilename_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->setFilename("home/chenglu/Desktop/pic.rar");
}

TEST(Progress_setTypeImage_UT, Progress_setTypeImage_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->setTypeImage("rar");
}

TEST(Progress_getCancelbutton_UT, Progress_getCancelbutton_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->getCancelbutton();
}

TEST(Progress_setProgressFilename_UT, Progress_setProgressFilename_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->setProgressFilename("");
}

TEST(Progress_setProgressFilename_UT, Progress_setProgressFilename_UT002)
{
    Progress *progress = new Progress(nullptr);
    DLabel *label = new DLabel("UT测试");
    progress->m_progressfilelabel = label;
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
    progress->setProgressFilename("/home/chenglu/Desktop/so.sh");
    ASSERT_NE(label, nullptr);
    delete label;
}

TEST(Progress_setProgressFilename_UT, Progress_setProgressFilename_UT003)
{
    Progress *progress = new Progress(nullptr);
    DLabel *label = new DLabel("UT测试");
    progress->m_progressfilelabel = label;
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DELETEING;
    progress->setProgressFilename("/home/chenglu/Desktop/so.sh");
    ASSERT_NE(label, nullptr);
    delete label;
}

TEST(Progress_setProgressFilename_UT, Progress_setProgressFilename_UT004)
{
    Progress *progress = new Progress(nullptr);
    DLabel *label = new DLabel("UT测试");
    progress->m_progressfilelabel = label;
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_CONVERT;
    progress->setProgressFilename("/home/chenglu/Desktop/so.sh");
    ASSERT_NE(label, nullptr);
    delete label;
}

TEST(Progress_setProgressFilename_UT, Progress_setProgressFilename_UT005)
{
    Progress *progress = new Progress(nullptr);
    DLabel *label = new DLabel("UT测试");
    progress->m_progressfilelabel = label;
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_NONE;
    progress->m_openType = true;
    progress->setProgressFilename("/home/chenglu/Desktop/so.sh");
    ASSERT_NE(label, nullptr);
    delete label;
}

TEST(Progress_setProgressFilename_UT, Progress_setProgressFilename_UT006)
{
    Progress *progress = new Progress(nullptr);
    DLabel *label = new DLabel("UT测试");
    progress->m_progressfilelabel = label;
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_NONE;
    progress->m_openType = false;
    progress->setProgressFilename("/home/chenglu/Desktop/so.sh");
    ASSERT_NE(label, nullptr);
    delete label;
}

TEST(Progress_settype_UT, Progress_settype_UT001)
{
    Progress *progress = new Progress(nullptr);
    Progress::ENUM_PROGRESS_TYPE type = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
    progress->settype(type);
}

TEST(Progress_getType_UT, Progress_getType_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_NONE;
    Progress::ENUM_PROGRESS_TYPE type = progress->getType();
}

TEST(Progress_setopentype_UT, Progress_setopentype_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->setopentype(true);
}

TEST(Progress_setopentype_UT, Progress_setopentype_UT002)
{
    Progress *progress = new Progress(nullptr);
    progress->setopentype(false);
}

TEST(Progress_getOpenType_UT, Progress_getOpenType_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->m_openType = false;
    bool opentype = progress->getOpenType();
}

//TEST(Progress_showConfirmDialog_UT, Progress_gshowConfirmDialog_UT001)
//{
//    Progress *progress = new Progress(nullptr);
//    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_COMPRESSING;
//    progress->showConfirmDialog();
//}

//TEST(Progress_showConfirmDialog_UT, Progress_gshowConfirmDialog_UT002)
//{
//    Progress *progress = new Progress(nullptr);
//    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_DELETEING;
//    progress->showConfirmDialog();
//}

//TEST(Progress_showConfirmDialog_UT, Progress_gshowConfirmDialog_UT003)
//{
//    Progress *progress = new Progress(nullptr);
//    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_CONVERT;
//    progress->showConfirmDialog();
//}

//TEST(Progress_showConfirmDialog_UT, Progress_gshowConfirmDialog_UT004)
//{
//    Progress *progress = new Progress(nullptr);
//    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_NONE;
//    progress->m_openType = true;
//    progress->showConfirmDialog();
//}

//TEST(Progress_showConfirmDialog_UT, Progress_gshowConfirmDialog_UT005)
//{
//    Progress *progress = new Progress(nullptr);
//    progress->m_ProgressType = Progress::ENUM_PROGRESS_TYPE::OP_NONE;
//    progress->m_openType = false;
//    progress->showConfirmDialog();
//}

TEST(Progress_resetProgress_UT, Progress_resetProgress_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->resetProgress();
}

//TEST(Progress_cancelbuttonPressedSlot_UT, Progress_cancelbuttonPressedSlot_UT001)
//{
//    Progress *progress = new Progress(nullptr);
//    progress->cancelbuttonPressedSlot();
//}

TEST(Progress_pauseContinueButtonPressedSlot_UT, Progress_pauseContinueButtonPressedSlot_UT001)
{
    Progress *progress = new Progress(nullptr);
    progress->pauseContinueButtonPressedSlot(true);
}

TEST(Progress_pauseContinueButtonPressedSlot_UT, Progress_pauseContinueButtonPressedSlot_UT002)
{
    Progress *progress = new Progress(nullptr);
    progress->pauseContinueButtonPressedSlot(false);
}
