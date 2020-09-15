#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "openloadingpage.h"

TEST(OpenLoadingPage_OpenLoadingPage_UT, OpenLoadingPage_OpenLoadingPage_UT001)
{
    OpenLoadingPage *openPage = new OpenLoadingPage(nullptr);
    ASSERT_NE(openPage, nullptr);
    delete openPage;
}

TEST(OpenLoadingPage_start_UT, OpenLoadingPage_start_UT001)
{
    OpenLoadingPage *openPage = new OpenLoadingPage(nullptr);
    openPage->start();
    ASSERT_NE(openPage, nullptr);
    delete openPage;
}

TEST(OpenLoadingPage_stop_UT, OpenLoadingPage_stop_UT001)
{
    OpenLoadingPage *openPage = new OpenLoadingPage(nullptr);
    openPage->stop();
    ASSERT_NE(openPage, nullptr);
    delete openPage;
}
