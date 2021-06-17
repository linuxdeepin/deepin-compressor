/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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

#include "xzplugin.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>

#define MAX_FILENAME 512

Q_DECLARE_METATYPE(KPluginMetaData)

class TestLibXzInterfaceFactory : public QObject, public ::testing::Test
{
public:
    TestLibXzInterfaceFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibXzInterfaceFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibXzInterfaceFactory *m_tester;
};

class TestLibXzInterface : public QObject, public ::testing::Test
{
public:
    TestLibXzInterface(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("test.xz").absoluteFilePath();
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibXzInterface(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibXzInterface *m_tester;
};


TEST_F(TestLibXzInterfaceFactory, initTest)
{

}

TEST_F(TestLibXzInterface, initTest)
{

}
