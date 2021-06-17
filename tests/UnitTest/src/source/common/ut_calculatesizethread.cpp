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
#include "calculatesizethread.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>

/*******************************函数打桩************************************/



/*******************************单元测试************************************/
// 测试CompressPage
class TestCalculateSizeThread : public ::testing::Test
{
public:
    TestCalculateSizeThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QStringList m_files{ "a.txt", "b.txt", "c.txt" }; // 添加的源文件数据（首层）
        QString m_strArchiveFullPath("/a/b/c/");       // 压缩包全路径（首层）

        FileEntry a;
        QList<FileEntry> m_listAddEntry{a};  // 添加的entry数据
        CompressOptions m_stOptions;        // 压缩参数
        QList<FileEntry> m_listAllEntry;    // 所有文件数据

        m_tester = new CalculateSizeThread(m_files, m_strArchiveFullPath, m_listAddEntry, m_stOptions);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CalculateSizeThread *m_tester;
};

TEST_F(TestCalculateSizeThread, initTest)
{

}

TEST_F(TestCalculateSizeThread, testset_thread_stop)
{
    m_tester->set_thread_stop(true);
    ASSERT_EQ(m_tester->m_thread_stop, true);
}

TEST_F(TestCalculateSizeThread, testrun)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);

    m_tester->run();
    ASSERT_EQ(m_tester->m_qTotalSize, 3);

    Stub stub11;
    QFileInfoStub::stub_QFileInfo_isDir(stub11, true);
    m_tester->run();

    Stub stub1;
    QFileInfoStub::stub_QFileInfo_exists(stub1, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub1, true);
    m_tester->run();

    Stub stub2;
    QFileInfoStub::stub_QFileInfo_exists(stub2, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub2, false);
    m_tester->run();

    Stub stub3;
    QFileInfoStub::stub_QFileInfo_exists(stub3, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub3, false);
    m_tester->run();

}

TEST_F(TestCalculateSizeThread, testConstructAddOptionsByThread)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/a/b/1.txt");
    QFileInfo fi1;
    QFileInfoList filist{fi1};
    QDirStub::stub_QDir_exists(stub, true);
    QDirStub::stub_QDir_entryInfoList(stub, filist);

    m_tester->ConstructAddOptionsByThread("/a/b/");
    ASSERT_EQ(m_tester->m_qTotalSize, 1);

    Stub stub11;
    QFileInfoStub::stub_QFileInfo_isDir(stub11, true);
    m_tester->ConstructAddOptionsByThread("/a/b/");

    Stub stub1;
    QFileInfoStub::stub_QFileInfo_exists(stub1, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub1, true);
    m_tester->ConstructAddOptionsByThread("/a/b/");

    Stub stub2;
    QFileInfoStub::stub_QFileInfo_exists(stub2, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub2, false);
    m_tester->ConstructAddOptionsByThread("/a/b/");

    Stub stub3;
    QFileInfoStub::stub_QFileInfo_exists(stub3, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub3, false);
    m_tester->ConstructAddOptionsByThread("/a/b/");
}
