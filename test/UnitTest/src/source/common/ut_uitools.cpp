/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#include <gtest/src/stub.h>

#include "config.h"
#include "uitools.h"
#include "pluginmanager.h"
#include "kpluginloader.h"

//#include <QVector>
#include <QImageReader>
#include <QPixmap>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

class TestUiTools : public ::testing::Test
{
public:
    TestUiTools() {}

public:
    virtual void SetUp()
    {
        m_tester = new UiTools;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UiTools *m_tester;
};

TEST_F(TestUiTools, initTest)
{

}

QString organizationName_stub()
{
    return "deepin";
}

QString applicationName_stub()
{
    return "deepin-compressor";
}

TEST_F(TestUiTools, testGetConfigPath)
{
    Stub stub;
    stub.set(ADDR(QCoreApplication, organizationName), organizationName_stub);
    stub.set(ADDR(QCoreApplication, applicationName), applicationName_stub);

    ASSERT_EQ(UiTools::getConfigPath(),
              (QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/deepin-compressor"));
}

TEST_F(TestUiTools, testRenderSVG)
{
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    ASSERT_EQ(pixmap.isNull(), false);
}

bool canRead_stub()
{
    return false;
}

TEST_F(TestUiTools, testRenderSVG1)
{
    Stub stub;
    stub.set(ADDR(QImageReader, canRead), canRead_stub);
    QPixmap pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    ASSERT_EQ(pixmap.isNull(), false);
}

TEST_F(TestUiTools, testHumanReadableSize)
{
    ASSERT_EQ(UiTools::humanReadableSize(0, 1), "-");
}

TEST_F(TestUiTools, testHumanReadableSize1)
{
    qint64 size = 9223372036854775807;
//    qDebug() << UiTools::humanReadableSize(size, 1);
    ASSERT_EQ(UiTools::humanReadableSize(size, 1), "8.0 EB");
}

//int size_stub()
//{
//    return 1;
//}

//bool isExistMimeType_stub(const QString &strMimeType, bool &bArchive)
//{
//    return true;
//}
TEST_F(TestUiTools, testIsArchiveFile)
{
    ASSERT_EQ(UiTools::isArchiveFile("test.zip"), true);
}

TEST_F(TestUiTools, testIsArchiveFile1)
{
    ASSERT_EQ(UiTools::isArchiveFile("test.txt"), false);
}

TEST_F(TestUiTools, testIsArchiveFile2)
{
    ASSERT_EQ(UiTools::isArchiveFile("test.crx"), true);
}

TEST_F(TestUiTools, testJudgeFileMime)
{
    ASSERT_EQ(UiTools::judgeFileMime("test.appimage"), "x-iso9660-appimage");
}

TEST_F(TestUiTools, testIsExistMimeType)
{
    const QString strMimeType = "zip";
    bool bArchive = true;
    ASSERT_EQ(UiTools::isExistMimeType(strMimeType, bArchive), true);
    ASSERT_EQ(bArchive, true);
}

QByteArray readAll_stub()
{
    return QByteArray("success");
}

TEST_F(TestUiTools, testReadConf)
{
    Stub stub;
    stub.set(ADDR(QIODevice, readAll), readAll_stub);
    ASSERT_EQ(UiTools::readConf(), "success");
}

TEST_F(TestUiTools, testToShortString)
{
    ASSERT_EQ(UiTools::toShortString("12345abcdeABCDE一二三四五", 10, 5), "12345...一二三四五");
}

TEST_F(TestUiTools, testCreateInterface)
{
    const QString &fileName = "test.zip";
    ASSERT_EQ(QString(UiTools::createInterface(fileName, true, UiTools::APT_Auto)->metaObject()->className()), "Cli7zPlugin"); //Cli7zPlugin

}

TEST_F(TestUiTools, testCreateInterface1)
{
    const QString &fileName = "test.zip";
    ASSERT_EQ(QString(UiTools::createInterface(fileName, false, UiTools::APT_Auto)->metaObject()->className()), "LibzipPlugin"); //LibzipPlugin

}

bool isEmpty_stub()
{
    return true;
}
TEST_F(TestUiTools, testCreateInterface2)
{
    Stub stub;
    stub.set(ADDR(QVector<Plugin *>, isEmpty), isEmpty_stub);

    const QString &fileName = "test.zip";
    ASSERT_EQ(UiTools::createInterface(fileName, false, UiTools::APT_Auto), nullptr); //nullptr

}

TEST_F(TestUiTools, testCreateInterface3)
{
    const QString &fileName = "test.zip";
    ASSERT_EQ(QString(UiTools::createInterface(fileName, false, UiTools::APT_Libarchive)->metaObject()->className()), "LibarchivePlugin"); //LibarchivePlugin

}
