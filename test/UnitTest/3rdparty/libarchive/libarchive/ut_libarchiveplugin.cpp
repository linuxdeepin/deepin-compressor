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
#include "libarchiveplugin.h"
//#include "
#include <QFileInfo>
#include <QMimeDatabase>

//Q_DECLARE_METATYPE(KPluginMetaData)

//class TestReadOnlyLibarchivePluginFactory : public QObject, public ::testing::Test
//{
//public:
//    TestReadOnlyLibarchivePluginFactory() {}

//public:
//    virtual void SetUp()
//    {
//        m_tester = new ReadOnlyLibarchivePluginFactory();
//    }

//    virtual void TearDown()
//    {
//        delete m_tester;
//    }

//protected:
//    ReadOnlyLibarchivePluginFactory *m_tester;
//};

//class TestReadOnlyLibarchivePlugin : public QObject, public ::testing::Test
//{
//public:
//    TestReadOnlyLibarchivePlugin() {}

//public:
//    virtual void SetUp()
//    {
//        QString strFile = QFileInfo("../UnitTest/test_sources/zip/extract/test.zip").absoluteFilePath();
//        KPluginMetaData data;
//        QMimeDatabase db;
//        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
//        const QVariantList args = {QVariant(strFile),
//                                   QVariant().fromValue(data),
//                                   QVariant::fromValue(mimeFromContent)
//                                  };

//        m_tester = new ReadOnlyLibarchivePlugin(this, args);
//    }

//    virtual void TearDown()
//    {
//        delete m_tester;
//    }

//protected:
//    ReadOnlyLibarchivePlugin *m_tester;
//};
