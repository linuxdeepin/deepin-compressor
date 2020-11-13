#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include <QModelIndex>
#include "mimetypes.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <jobs.h>
TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT001)
{
   determineMimeType("asdf");
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT002)
{
   determineMimeType("../UnitTest/regress/test.tar.bz2");
}
TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT003)
{
   determineMimeType("../UnitTest/regress/test.tar.lz4");
}
TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT004)
{
   determineMimeType("../UnitTest/regress/test.tar.7z");
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT005)
{
   determineMimeType("../UnitTest/regress/test.zip");
}

TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT006)
{
   determineMimeType("../UnitTest/regress/test.iso");

}

bool isDefault()
{
    return false;
}
//TEST(QMimeType_determineMimeType_UT, QMimeType_determineMimeType_UT007)
//{

//   Stub *stub = new Stub;
//   stub->set(ADDR(QMimeType, isDefault), isDefault);
//   determineMimeType("../UnitTest/regress/test.iso");
//   delete stub;
//   stub = nullptr;
//}


