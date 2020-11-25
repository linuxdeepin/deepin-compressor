#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include <iostream>
#include "archivesortfiltermodel.h"
#include <QResizeEvent>
#include "archivemodel.h"

#include "uncompresspage.h"

TEST(UnCompressPage_UnCompressPage, UnCompressPage_UnCompressPage_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    EXPECT_NE(unPage, nullptr);
}

int myshowWarningDialog(void *obj, const QString &msg)
{
    std::cout << msg.toStdString() << std::endl;
}

TEST(UnCompressPage_oneCompressPress, UnCompressPage_oneCompressPress_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    Stub *stub = new Stub;
    stub->set(ADDR(UnCompressPage, showWarningDialog), myshowWarningDialog);
    unPage->oneCompressPress();
    delete stub;
}

TEST(UnCompressPage_oneCompressPress, UnCompressPage_oneCompressPress_UT002)
{
    UnCompressPage *unPage = new UnCompressPage;
    Stub *stub = new Stub;
    stub->set(ADDR(UnCompressPage, showWarningDialog), myshowWarningDialog);
    unPage->m_pathstr = "/home/lx777/mydemo/test.cpp";
    unPage->oneCompressPress();
    delete stub;
}

TEST(UnCompressPage_oneCompressPress, UnCompressPage_oneCompressPress_UT003)
{
    UnCompressPage *unPage = new UnCompressPage;
    Stub *stub = new Stub;
    stub->set(ADDR(UnCompressPage, showWarningDialog), myshowWarningDialog);
    unPage->m_pathstr = "/home/lx777/mydemo";
    unPage->oneCompressPress();
    delete stub;
}

void mysetDecompressModel(void *obj, ArchiveSortFilterModel *model)
{

}

TEST(UnCompressPage_setModel, UnCompressPage_setModel_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    ArchiveSortFilterModel *model = new ArchiveSortFilterModel;
    Stub *stub = new Stub;
    stub->set(ADDR(fileViewer, setDecompressModel), mysetDecompressModel);
    unPage->setModel(model);
    EXPECT_NE(unPage->m_model, nullptr);
//   delete  model;
    // delete  unPage;
}

TEST(UnCompressPage_onPathButoonClicked, UnCompressPage_onPathButoonClicked_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    //stub->set(A_foo,myexec);
    unPage->m_pathstr = "/home/lx777/mydemo";
    unPage->onPathButoonClicked();
}
TEST(UnCompressPage_onPathButoonClicked, UnCompressPage_onPathButoonClicked_UT002)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_pathstr = "/home/lx777/mydemo";
    unPage->onPathButoonClicked();
}

TEST(UnCompressPage_setdefaultpath, UnCompressPage_setdefaultpath_UT001)
{
    const QString strPath = "/home/lx777/mydemo/test.zip";
    UnCompressPage *unPage = new UnCompressPage;
    unPage->setdefaultpath(strPath);
}

TEST(UnCompressPage_SetDefaultFile, UnCompressPage_SetDefaultFile_UT001)
{
    QFileInfo info("/home/lx777/mydemo/test.zip");
    UnCompressPage *unPage = new UnCompressPage;
    unPage->SetDefaultFile(info);
}

TEST(UnComparessPage_getFileCount, UnComparessPage_getFileCount_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    int a = unPage->getFileCount();
    EXPECT_EQ(a, 0);
}

TEST(UnComparessPage_getDeFileCount, UnComparessPage_getDeFileCount_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    int a = unPage->getDeFileCount();
    EXPECT_EQ(a, 0);
}

TEST(UnComparessPage_showWarningDialog, UnComparessPage_showWarningDialog_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    int a = unPage->showWarningDialog("Hello");
    EXPECT_EQ(a, 0);
}

TEST(UnComparess_getExtractType, UnComparess_getExtractType_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    auto type = unPage->getExtractType();
    EXPECT_EQ(type, 1);
}

TEST(UnComparess_setRootPathIndex, UnComparess_setRootPathIndex_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->setRootPathIndex();
}

TEST(UnComparess_setWidth, UnComparess_setWidth_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->setWidth(10);
    EXPECT_EQ(unPage->m_iWidth, 10);
}

TEST(UnConparess_resizeEvent, UnConparess_resizeEvent_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    QSize size = unPage->size();
    QResizeEvent *res = new QResizeEvent(QSize(size.width() + 10, size.height() + 5), size);
    unPage->resizeEvent(res);
}

TEST(UnCompressPage_setUpdateFiles, UnCompressPage_setUpdateFiles_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    QStringList  list;
    list << "1" << "2";
    unPage->setUpdateFiles(list);
}

QString str = "";
QStringList myconvertArchiveDialog()
{
    QStringList list;
    list << "true" << str;
    return list;
}

TEST(UnCompressPage_convertArchive, UnCompressPage_convertArchive_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    Stub *stub = new Stub;
    str = "zip";
    stub->set(ADDR(UnCompressPage, convertArchiveDialog), myconvertArchiveDialog);
    unPage->convertArchive();
    delete stub;
}

TEST(UnCompressPage_convertArchive, UnCompressPage_convertArchive_UT002)
{
    UnCompressPage *unPage = new UnCompressPage;
    Stub *stub = new Stub;
    str = "7z";
    stub->set(ADDR(UnCompressPage, convertArchiveDialog), myconvertArchiveDialog);
    unPage->convertArchive();
    delete stub;
}

TEST(UnCompressPage_getAndDisplayPath, UnCompressPage_getAndDisplayPath_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    const QString strPath = "/home/lx777/mydemo/test.zip";
    unPage->m_iWidth = 1;
    QString str = unPage->getAndDisplayPath(strPath);
    EXPECT_EQ(str.toLocal8Bit().data(), "");
}

TEST(UnCompressPage_slotCompressedAddFile, UnCompressPage_slotCompressedAddFile_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->slotCompressedAddFile();
}

TEST(UnCompressPage_slotCompressedAddFile, UnCompressPage_slotCompressedAddFile_UT002)
{
    /* UnCompressPage *unPage = new UnCompressPage;
     ArchiveSortFilterModel *model = new ArchiveSortFilterModel;
     unPage->setModel(model);
     unPage->slotCompressedAddFile();*/
}

TEST(UnCompressPage_getFileViewer, UnCompressPage_getFileViewer_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    auto fv = unPage->getFileViewer();
    EXPECT_NE(fv, nullptr);
}

TEST(UnCompressPage_getNextbutton, UnCompressPage_getNextbutton_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    auto fv = unPage->getNextbutton();
    EXPECT_NE(fv, nullptr);
}

TEST(UnCompressPage_getPathCommandLinkButton, UnCompressPage_getPathCommandLinkButton_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    auto fv = unPage->getPathCommandLinkButton();
    EXPECT_NE(fv, nullptr);
}

TEST(UnCompressPage_getDecompressPath, UnCompressPage_getDecompressPath_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_pathstr = "test";
    QString str = unPage->getDecompressPath();
    EXPECT_STREQ(str.toStdString().c_str(), "test");
}

TEST(UnComparessPage_onextractfilesSlot, UnComparessPage_onextractfilesSlot_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    QVector<Archive::Entry *> fileList;
    unPage->onextractfilesSlot(fileList, EXTRACT_TYPE::EXTRACT_TO, "");
}
TEST(UnComparessPage_onextractfilesSlot, UnComparessPage_onextractfilesSlot_UT002)
{
    UnCompressPage *unPage = new UnCompressPage;
    QVector<Archive::Entry *> fileList;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo/test.zip");
    fileList.append(entry);
    unPage->onextractfilesSlot(fileList, EXTRACT_TYPE::EXTRACT_TO, "");
}
TEST(UnComparessPage_onextractfilesSlot, UnComparessPage_onextractfilesSlot_UT003)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_pathstr = "/home/lx777/mydemo/";
    QVector<Archive::Entry *> fileList;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo/test.zip");
    fileList.append(entry);
    unPage->onextractfilesSlot(fileList, EXTRACT_TYPE::EXTRACT_TO, "");
}

TEST(UnComparessPage_onextractfilesSlot, UnComparessPage_onextractfilesSlot_UT004)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_pathstr = "/home/lx777/mydemo/";
    QVector<Archive::Entry *> fileList;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo/test.zip");
    fileList.append(entry);
    unPage->onextractfilesSlot(fileList, EXTRACT_TYPE::EXTRACT_DRAG, "");
}

TEST(UnComparessPage_onextractfilesSlot, UnComparessPage_onextractfilesSlot_UT005)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_pathstr = "/home/lx777/mydemo/";
    QVector<Archive::Entry *> fileList;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo/test.zip");
    fileList.append(entry);
    unPage->onextractfilesSlot(fileList, EXTRACT_TYPE::EXTRACT_TEMP, "");
}

TEST(UnComparessPage_onextractfilesSlot, UnComparessPage_onextractfilesSlot_UT006)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_pathstr = "/home/lx777/mydemo/";
    QVector<Archive::Entry *> fileList;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo/test.zip");
    fileList.append(entry);
    unPage->onextractfilesSlot(fileList, EXTRACT_TYPE::EXTRACT_TEMP_CHOOSE_OPEN, "");
}

TEST(UnComparessPage_onextractfilesSlot, UnComparessPage_onextractfilesSlot_UT007)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_pathstr = "/home/lx777/mydemo/";
    QVector<Archive::Entry *> fileList;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo/test.zip");
    fileList.append(entry);
    unPage->onextractfilesSlot(fileList, EXTRACT_TYPE::EXTRACT_HEAR, "");
}

TEST(UnCompressPage_onRefreshEntryList, UnCompressPage_onRefreshEntryList_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    QVector<Archive::Entry *> fileList;
    unPage->onRefreshEntryList(fileList, true);
}

TEST(UnCompressPage_onextractfilesOpenSlot, UnCompressPage_onextractfilesOpenSlot_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    QVector<Archive::Entry *> fileList;
    unPage->onextractfilesOpenSlot(fileList, "");
}

TEST(UnComparessPage_onAutoCompress, UnComparessPage_onAutoCompress_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo");
    QStringList list;
    list << "/home/lx777/mydemo";
    unPage->onAutoCompress(list, entry);
}

TEST(UnComparessPage_onAutoCompress, UnComparessPage_onAutoCompress_UT002)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_fileviewer->m_bDropAdd = true;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo");
    QStringList list;
    list << "/home/lx777/mydemo";

    ArchiveSortFilterModel *model = new ArchiveSortFilterModel;
    ArchiveModel *sourceModel =  new ArchiveModel;
    model->setSourceModel(sourceModel);
    unPage->setModel(model);

    unPage->onAutoCompress(list, entry);
}

Archive::Entry *myisExists(QString fullPath)
{
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo");
    return entry;
}

TEST(UnComparessPage_onAutoCompress, UnComparessPage_onAutoCompress_UT003)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_fileviewer->m_bDropAdd = true;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo");
    QStringList list;
    list << "/home/lx777/mydemo" << "/home/lx777/mydemo/test.zip";

    ArchiveSortFilterModel *model = new ArchiveSortFilterModel;
    ArchiveModel *sourceModel =  new ArchiveModel;
    model->setSourceModel(sourceModel);
    unPage->setModel(model);
    Stub *stub = new Stub;
    stub->set(ADDR(ArchiveModel, isExists), myisExists);
    unPage->onAutoCompress(list, entry);
    delete stub;
}

TEST(UnComparessPage_onAutoCompress, UnComparessPage_onAutoCompress_UT004)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->m_fileviewer->m_bDropAdd = true;
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo");
    QStringList list;
    list << "/home/lx777/mydemo/test.zip";

    ArchiveSortFilterModel *model = new ArchiveSortFilterModel;
    ArchiveModel *sourceModel =  new ArchiveModel;
    model->setSourceModel(sourceModel);
    unPage->setModel(model);
    QFileInfo info("/home/lx777/mydemo/321.rar");
    unPage->m_info = info;
    unPage->onAutoCompress(list, entry);
}

TEST(UnCompressPage_slotSubWindowTipsPopSig, UnCompressPage_slotSubWindowTipsPopSig_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    QStringList list;
    list << "123";
    //unPage->slotSubWindowTipsPopSig(1,list);
}


TEST(UnCompressPage_slotDeleteJobFinished, UnCompressPage_slotDeleteJobFinished_UT001)
{
    Archive::Entry *entry = new Archive::Entry(nullptr, "/home/lx777/mydemo");
    QStringList list;
    list << "1";
    UnCompressPage *unPage = new UnCompressPage;
    unPage->slotDeleteJobFinished(entry);
}

TEST(UnCompressPage_showReplaceDialog, UnCompressPage_showReplaceDialog_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    int n = 10;
    int nn = unPage->showReplaceDialog("test", n);
}

TEST(UnCompressPage_convertArchiveDialog, UnCompressPage_convertArchiveDialog_UT001)
{
    UnCompressPage *unPage = new UnCompressPage;
    unPage->convertArchiveDialog();
}
