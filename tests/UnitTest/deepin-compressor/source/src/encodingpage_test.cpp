#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "encodingpage.h"

TEST(EncodingPage_EncodingPage_UT, EncodingPage_EncodingPage_UT001)
{
    EncodingPage *encodingPage = new EncodingPage(nullptr);
    ASSERT_NE(encodingPage, nullptr);
    delete encodingPage;
}

TEST(EncodingPage_setFilename_UT, EncodingPage_setFilename_UT001)
{
    EncodingPage *encryptionPage = new EncodingPage(nullptr);
    encryptionPage->setFilename("/home.chenglu/Desktop/so.sh");
}

TEST(EncodingPage_setTypeImage_UT, EncodingPage_setTypeImage_UT001)
{
    EncodingPage *encryptionPage = new EncodingPage(nullptr);
    encryptionPage->setFilename("UTF-8");
}
