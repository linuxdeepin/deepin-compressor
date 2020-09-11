#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "extractpausedialog.h"

TEST(ExtractPauseDialog_ExtractPauseDialog_UT, ExtractPauseDialog_ExtractPauseDialog_UT001)
{
    ExtractPauseDialog *extractPauseDialog = new ExtractPauseDialog(nullptr);
    ASSERT_NE(extractPauseDialog, nullptr);
    delete extractPauseDialog;
}

TEST(ExtractPauseDialog_closeEvent_UT, ExtractPauseDialog_closeEvent_UT001)
{
    ExtractPauseDialog *extractPauseDialog = new ExtractPauseDialog(nullptr);
    extractPauseDialog->closeEvent(nullptr);
    ASSERT_NE(extractPauseDialog, nullptr);
    delete extractPauseDialog;
}
