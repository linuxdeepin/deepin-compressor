// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uitools.h"
#include "ut_commonstub.h"
#include "archive_session.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>
#include <QFile>
#include <QSet>
#include <QStandardPaths>
#include <QTemporaryDir>

/*******************************函数打桩************************************/
bool UiTools_isExistMimeType_stub()
{
    return true;
}

QString UiTools_readConf_stub()
{
    return "file_association.file_association_type.x-7z-compressed:true\nfile_association.file_association_type.x-archive:false";
}

QString UiTools_readConf_withDisabledZstd_stub()
{
    return QStringLiteral("file_association.file_association_type.x-zstd-compressed-tar:false\n");
}


/*******************************单元测试************************************/
// 测试CompressPage
class UT_UiTools : public ::testing::Test
{
public:
    UT_UiTools(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UiTools();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UiTools *m_tester;
};

TEST_F(UT_UiTools, initTest)
{

}

TEST_F(UT_UiTools, test_getConfigPath)
{
    Stub stub;
    QDirStub::stub_QDir_filePath(stub, "/a/b/c");
    EXPECT_EQ(m_tester->getConfigPath() == "/a/b/c", true);
}

TEST_F(UT_UiTools, test_renderSVG)
{
    QPixmap pix = m_tester->renderSVG("/a/b/1.png", QSize(32, 32));
    EXPECT_EQ(pix.isNull(), true);
}

TEST_F(UT_UiTools, test_humanReadableSize)
{
    EXPECT_EQ(m_tester->humanReadableSize(0, 1) == QLatin1String("-"), true);
    EXPECT_EQ(m_tester->humanReadableSize(1023, 1) == QLatin1String("1023.0 B"), true);
}

TEST_F(UT_UiTools, test_isArchiveFile)
{
    Stub stub;
    stub.set(ADDR(UiTools, isExistMimeType), UiTools_isExistMimeType_stub);

    EXPECT_EQ(m_tester->isArchiveFile("/a/b/1.zip"), true);
    EXPECT_EQ(m_tester->isArchiveFile("/a/b/1.crx"), true);
    EXPECT_EQ(m_tester->isArchiveFile("/a/b/1.deb"), false);
}

TEST_F(UT_UiTools, test_isExistMimeType)
{
    Stub stub;
    stub.set(ADDR(UiTools, readConf), UiTools_readConf_stub);
    bool bArchive = false;
    EXPECT_EQ(m_tester->isExistMimeType("x-7z-compressed", bArchive), true);
    bArchive = false;
    EXPECT_EQ(m_tester->isExistMimeType("x-7z-compressedabc", bArchive), false);
    bArchive = false;
    EXPECT_EQ(m_tester->isExistMimeType("x-archive", bArchive), false);
}

TEST_F(UT_UiTools, NewTarAssociationsRespectExistingPreferences)
{
    Stub stub;
    stub.set(ADDR(UiTools, readConf), UiTools_readConf_withDisabledZstd_stub);
    bool isArchive = false;
    EXPECT_FALSE(UiTools::isExistMimeType(QStringLiteral("x-zstd-compressed-tar"), isArchive));
    EXPECT_TRUE(isArchive);
    isArchive = false;
    EXPECT_TRUE(UiTools::isExistMimeType(QStringLiteral("x-lz4-compressed-tar"), isArchive));
    EXPECT_TRUE(isArchive);
    isArchive = false;
    const bool hasLrzip = !QStandardPaths::findExecutable(QStringLiteral("lrzip")).isEmpty();
    EXPECT_EQ(UiTools::isExistMimeType(QStringLiteral("x-lrzip-compressed-tar"), isArchive), hasLrzip);
    EXPECT_EQ(isArchive, hasLrzip);
}

TEST_F(UT_UiTools, test_readConf)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileStub::stub_QFile_open(stub, true);
    QFileStub::stub_QFile_close(stub, true);
    QByteArray ba = QString("hello world").toLatin1();
    QFileStub::stub_QFile_readAll(stub, ba);

    EXPECT_EQ(m_tester->readConf() == QLatin1String("hello world"), true);
}

TEST_F(UT_UiTools, test_toShortString)
{
    QString str = "123456789123465789";
    EXPECT_EQ(m_tester->toShortString(str, 4, 4), "12...89");
}

TEST_F(UT_UiTools, test_handleFileName_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1.tar");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "1.tar.lz");
    EXPECT_EQ(m_tester->handleFileName("1.tar.lz"), "1");
}

TEST_F(UT_UiTools, test_handleFileName_002)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1.7z");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "1.7z.001");
    EXPECT_EQ(m_tester->handleFileName("1.7z.001"), "1");
}

TEST_F(UT_UiTools, test_handleFileName_003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1.part01");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "1.part01.rar");
    EXPECT_EQ(m_tester->handleFileName("1.part01.rar"), "1");
}

TEST_F(UT_UiTools, test_handleFileName_004)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1.part1");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "1.part1.rar");
    EXPECT_EQ(m_tester->handleFileName("1.part1.rar"), "1");
}

TEST_F(UT_UiTools, test_handleFileName_005)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1.zip");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "1.zip.001");
    EXPECT_EQ(m_tester->handleFileName("1.zip.001"), "1");
}

TEST_F(UT_UiTools, test_isLocalDeviceFile)
{
    EXPECT_EQ(m_tester->isLocalDeviceFile("1.txt"), false);
}

TEST_F(UT_UiTools, test_removeSameFileName)
{
    QStringList listFiles = QStringList() << "1.txt" << "1.txt";
    QStringList listResult = m_tester->removeSameFileName(listFiles);
    bool bResult = (listResult.count() == 1 && listResult[0] == "1.txt");
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_UiTools, test_transSplitFileName_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);

    QString str = "1.7z.001";
    UnCompressParameter::SplitType type;
    m_tester->transSplitFileName(str, type);
    EXPECT_EQ(type, UnCompressParameter::ST_Other);
}

TEST_F(UT_UiTools, test_transSplitFileName_002)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);

    QString str = "1.part1.rar";
    UnCompressParameter::SplitType type;
    m_tester->transSplitFileName(str, type);
    EXPECT_EQ(type, UnCompressParameter::ST_Other);
}

TEST_F(UT_UiTools, test_transSplitFileName_003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);

    QString str = "1.zip.001";
    UnCompressParameter::SplitType type;
    m_tester->transSplitFileName(str, type);
    EXPECT_EQ(type, UnCompressParameter::ST_Zip);
}

TEST_F(UT_UiTools, test_transSplitFileName_004)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);

    QString str = "1.z01";
    UnCompressParameter::SplitType type;
    m_tester->transSplitFileName(str, type);
    EXPECT_EQ(type, UnCompressParameter::ST_Zip);
}

TEST_F(UT_UiTools, test_transSplitFileName_005)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);

    QString str = "1.part1.rar";
    UnCompressParameter::SplitType type;
    m_tester->transSplitFileName(str, type);
    EXPECT_EQ(type, UnCompressParameter::ST_Other);
}

TEST_F(UT_UiTools, test_createInterface_001)
{
    dcarchive::ArchiveSession *pInterface = m_tester->createInterface("1.zip", true, UiTools::APT_Auto);
    EXPECT_NE(pInterface, nullptr);
    SAFE_DELETE_ELE(pInterface);
}

TEST_F(UT_UiTools, test_createInterface_002)
{
    dcarchive::ArchiveSession *pInterface = m_tester->createInterface("1.zip", true, UiTools::APT_Cli7z);

    if (!QStandardPaths::findExecutable("7z").isEmpty()) {
        EXPECT_NE(pInterface, nullptr);
    } else {
        EXPECT_EQ(pInterface, nullptr);
    }

    SAFE_DELETE_ELE(pInterface);
}

TEST_F(UT_UiTools, test_createInterface_003)
{
    dcarchive::ArchiveSession *pInterface = m_tester->createInterface("1.zip", true, UiTools::APT_Libarchive);
    EXPECT_NE(pInterface, nullptr);
    SAFE_DELETE_ELE(pInterface);
}

TEST_F(UT_UiTools, test_createInterface_004)
{
    dcarchive::ArchiveSession *pInterface = m_tester->createInterface("1.zip", true, UiTools::APT_Libzip);
    EXPECT_NE(pInterface, nullptr);
    SAFE_DELETE_ELE(pInterface);
}

TEST_F(UT_UiTools, test_createInterface_usesOwnBackendsForAllFormats)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString zipPath = workspace.filePath(QStringLiteral("archive.zip"));
    const QString debPath = workspace.filePath(QStringLiteral("package.deb"));
    QFile zip(zipPath);
    ASSERT_TRUE(zip.open(QIODevice::WriteOnly));
    zip.close();
    QFile deb(debPath);
    ASSERT_TRUE(deb.open(QIODevice::WriteOnly));
    deb.close();

    QScopedPointer<dcarchive::ArchiveSession> zipInterface(
            m_tester->createInterface(zipPath, true, UiTools::APT_Auto));
    ASSERT_NE(zipInterface.data(), nullptr);

    QScopedPointer<dcarchive::ArchiveSession> debInterface(
            m_tester->createInterface(debPath, false, UiTools::APT_Auto));
    ASSERT_NE(debInterface.data(), nullptr);
}

TEST_F(UT_UiTools, test_isWritableArchive)
{
    QTemporaryDir workspace;
    ASSERT_TRUE(workspace.isValid());
    const QString tarPath = workspace.filePath(QStringLiteral("archive.tar"));
    const QString debPath = workspace.filePath(QStringLiteral("package.deb"));
    QFile tar(tarPath);
    ASSERT_TRUE(tar.open(QIODevice::WriteOnly));
    tar.close();
    QFile deb(debPath);
    ASSERT_TRUE(deb.open(QIODevice::WriteOnly));
    deb.close();

    EXPECT_TRUE(UiTools::isWritableArchive(tarPath));
    EXPECT_FALSE(UiTools::isWritableArchive(debPath));
}

TEST_F(UT_UiTools, test_supportedWriteMimeTypes)
{
    const QStringList types = UiTools::supportedWriteMimeTypes();
    EXPECT_TRUE(types.contains(QStringLiteral("application/zip")));
    EXPECT_TRUE(types.contains(QStringLiteral("application/x-7z-compressed")));
    EXPECT_TRUE(types.contains(QStringLiteral("application/x-compressed-tar")));
    EXPECT_EQ(types.contains(QStringLiteral("application/x-lrzip-compressed-tar")),
              !QStandardPaths::findExecutable(QStringLiteral("lrzip")).isEmpty());
    QSet<QString> unique;
    for (const QString &type : types)
        unique.insert(type);
    EXPECT_EQ(types.size(), unique.size());
}
