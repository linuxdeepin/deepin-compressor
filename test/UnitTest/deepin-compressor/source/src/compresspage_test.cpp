#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include <QSettings>
#include "compresspage.h"
#include "fileViewer.h"
#include "customwidget.h"

TEST(CompressPage_CompressPage_UT, CompressPage_CompressPage_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->m_settings->setValue("dir", "");
    EXPECT_NE(compress, nullptr);
    delete compress;
}

TEST(CompressPage_onNextPress_UT, CompressPAge_onNextPress_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->m_filelist.clear();
    compress->onNextPress();
}

TEST(CompressPage_onNextPress_UT, CompressPAge_onNextPress_UT002)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->m_filelist.append("/home/chenglu/Desktop/pic.rar");
    compress->onNextPress();
}

TEST(CompressPage_showDialog_UT, CompressPAge_showDialog_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->showDialog();
}

TEST(CompressPage_showReplaceDialog_UT, CompressPAge_showReplaceDialog_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->showReplaceDialog("/home/chenglu/Desktop/pic.rar");
}

TEST(CompressPage_setRootPathIndex_UT, CompressPAge_setRootPathIndex_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->setRootPathIndex();
}

TEST(CompressPage_getFileViewer_UT, CompressPAge_getFileViewer_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    fileViewer *fileview = compress->getFileViewer();
    EXPECT_NE(fileview, nullptr);
}

TEST(CompressPage_getNextbutton_UT, CompressPAge_getNextbutton_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    CustomPushButton *custom = compress->getNextbutton();
    EXPECT_NE(custom, nullptr);
}

TEST(CompressPage_onAddfileSlot_UT, CompressPAge_onAddfileSlot_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->m_fileviewer->m_pathindex = 1;
    compress->m_settings->setValue("dir", "");
    compress->onAddfileSlot();
}

TEST(CompressPage_onAddfileSlot_UT, CompressPAge_onAddfileSlot_UT002)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->onAddfileSlot();
}

TEST(CompressPage_onSelectedFilesSlot_UT, CompressPAge_onSelectedFilesSlot_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->m_fileviewer->m_pathindex = 1;
    compress->m_filelist.append("/home/chenglu/Desktop/so.sh");
    compress->m_filelist.append("/home/chenglu/Desktop/dev.sh");
    compress->m_filelist.append("/home/chenglu/Desktop/cmake.sh");
    QStringList files;
    files.append("/home/chenglu/Desktop/special");
    files.append("/home/chenglu/Desktop/so.sh");
    compress->onSelectedFilesSlot(files);
}

TEST(CompressPage_onSelectedFilesSlot_UT, CompressPAge_onSelectedFilesSlot_UT002)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->m_filelist.append("/home/chenglu/Desktop/so.sh");
    compress->m_filelist.append("/home/chenglu/Desktop/dev.sh");
    compress->m_filelist.append("/home/chenglu/Desktop/cmake.sh");
    QStringList files;
    files.append("/home/chenglu/Desktop/special");
    files.append("/home/chenglu/Desktop/so.sh");
    compress->onSelectedFilesSlot(files);
}

TEST(CompressPage_onRefreshFilelist_UT, CompressPAge_onRefreshFilelist_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    QStringList fileList;
    fileList.append("/home/chenglu/Desktop/special");
    fileList.append("/home/chenglu/Desktop/so.sh");
    compress->onRefreshFilelist(fileList);
}

TEST(CompressPage_onRefreshFilelist_UT, CompressPAge_onRefreshFilelist_UT002)
{
    CompressPage *compress = new CompressPage(nullptr);
    QStringList fileList;
    fileList.append("/home/chenglu/Desktop/special");
    fileList.append("/home/chenglu/Desktop/so.sh");
    fileList.clear();
    compress->onRefreshFilelist(fileList);
}

TEST(CompressPage_onPathIndexChanged_UT, CompressPAge_onPathIndexChanged_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->onPathIndexChanged();
}

TEST(CompressPage_onPathIndexChanged_UT, CompressPAge_onPathIndexChanged_UT002)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->m_fileviewer->m_pathindex = 1;
    compress->onPathIndexChanged();
}

TEST(CompressPage_clearFiles_UT, CompressPAge_clearFiles_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    compress->clearFiles();
}

TEST(CompressPage_getCompressFilelist_UT, CompressPAge_getCompressFilelist_UT001)
{
    CompressPage *compress = new CompressPage(nullptr);
    QStringList strList = compress->getCompressFilelist();
}
