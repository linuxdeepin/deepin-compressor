#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include <QModelIndex>
#include "structs.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <jobs.h>
TEST(ProgressAssistant_startTimer_UT, ProgressAssistant_startTimer_UT001)
{
    ProgressAssistant options ;
    options.startTimer();

}

TEST(ProgressAssistant_restartTimer_UT, ProgressAssistant_restartTimer_UT001)
{
    ProgressAssistant options ;
    options.restartTimer();
}

TEST(ProgressAssistant_resetProgress_UT, ProgressAssistant_resetProgress_UT001)
{
    ProgressAssistant options ;
    options.resetProgress();
}

TEST(ProgressAssistant_setTotalSize_UT, ProgressAssistant_setTotalSize_UT001)
{
    ProgressAssistant options ;
    options.setTotalSize(10);
}

TEST(ProgressAssistant_getTotalSize_UT, ProgressAssistant_getTotalSize_UT001)
{
    ProgressAssistant options ;
    options.getTotalSize();
}

TEST(ProgressAssistant_getSpeed_UT, ProgressAssistant_getSpeed_UT001)
{
    ProgressAssistant options ;
    options.resetProgress();
    options.getSpeed(10,true);
}

TEST(ProgressAssistant_getSpeed_UT, ProgressAssistant_getSpeed_UT003)
{
    ProgressAssistant options ;
    options.resetProgress();
    options.consumeTime = -1;
    options.getSpeed(10,true);
}

TEST(ProgressAssistant_getSpeed_UT, ProgressAssistant_getSpeed_UT002)
{
    ProgressAssistant options ;
    options.getSpeed(10,false);
}

TEST(ProgressAssistant_getLeftTime_UT, ProgressAssistant_getLeftTime_UT001)
{
    ProgressAssistant options ;
    options.resetProgress();
    options.getLeftTime(10,true);
}
TEST(ProgressAssistant_getLeftTime_UT, ProgressAssistant_getLeftTime_UT002)
{
    ProgressAssistant options ;
    options.getLeftTime(10,false);
}

