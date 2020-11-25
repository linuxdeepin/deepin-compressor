#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include <iostream>
#include "mimetypedisplaymanager.h"


TEST(MimeTypeDisplayManager_MimeTypeDisplayManager,MimeTypeDisplayManager_MimeTypeDisplayManager_UT001)
{
     MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
     EXPECT_NE(mimeType,nullptr);
     delete  mimeType;
}

TEST(MimeTypeDisplayManager_initData,MimeTypeDisplayManager_initData_UT001)
{
      MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
      mimeType->initData();
      EXPECT_NE(mimeType->m_defaultIconNames.size(),0);
      delete  mimeType;
}

TEST(MimeTypeDisplayManager_initConnect,MimeTypeDisplayManager_initConnect_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    mimeType->initConnect();
    delete  mimeType;
}

TEST(MimeTypeDisplayManager_displayName,MimeTypeDisplayManager_displayName_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    QString str = mimeType->displayName("application/x-desktop");
    EXPECT_EQ(str.isEmpty(),false);
    delete  mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("application/x-desktop");
    EXPECT_EQ(ft,FileType::DesktopApplication);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT002)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("inode/directory");
    EXPECT_EQ(ft,FileType::Directory);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT003)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("application/x-executable");
    EXPECT_EQ(ft,FileType::Executable);
    delete mimeType;
}
TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT004)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("video/x-executable");
    EXPECT_EQ(ft,FileType::Videos);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT005)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("audio/x-executable");
    EXPECT_EQ(ft,FileType::Audios);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT006)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("image/x-executable");
    EXPECT_EQ(ft,FileType::Images);
    delete mimeType;
    //
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT007)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("text/x-executable");
    EXPECT_EQ(ft,FileType::Documents);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT008)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("application/x-ace");
    EXPECT_EQ(ft,FileType::Archives);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT009)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("application/x-deepinclone-dim");
    EXPECT_EQ(ft,FileType::Backups);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNameToEnum,MimeTypeDisplayManager_displayNameToEnum_UT010)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    FileType ft = mimeType->displayNameToEnum("application/x-deepinclone-dim111");
    EXPECT_EQ(ft,FileType::Unknown);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_defaultIcon,MimeTypeDisplayManager_defaultIcon_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    QString str =  mimeType->defaultIcon("Images (8)");
    EXPECT_EQ(str.isEmpty(),false);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_displayNames,MimeTypeDisplayManager_displayName_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    auto map = mimeType->displayNames();
    EXPECT_EQ(map.size(),10);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_readlines,MimeTypeDisplayManager_readlines_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    QStringList strList = mimeType->readlines("/home/lx777/tess.cpp");

    EXPECT_NE(strList.size(),0);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_readlines,MimeTypeDisplayManager_readlines_UT002)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    QStringList strList = mimeType->readlines("/home/lx777/t.cpp");

    EXPECT_EQ(strList.size(),0);
    delete mimeType;
}


TEST(MimeTypeDisplayManager_loadSupportMimeTypes,MimeTypeDisplayManager_loadSupportMimeTypes_UT002)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    mimeType->loadSupportMimeTypes();
    delete mimeType;
}


TEST(MimeTypeDisplayManager_supportArchiveMimetypes,MimeTypeDisplayManager_supportArchiveMimetypes_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    auto strList = mimeType->supportArchiveMimetypes();
    EXPECT_EQ(strList.size(),0);
    delete mimeType;
}

TEST(MimeTypeDisplayManager_supportVideoMimeTypes,MimeTypeDisplayManager_supportVideoMimeTypes_UT001)
{
    MimeTypeDisplayManager *mimeType = new MimeTypeDisplayManager;
    auto strList = mimeType->supportVideoMimeTypes();
    EXPECT_EQ(strList.size(),0);
    delete mimeType;
}














