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

#include "progresspage.h"
#include "ut_commonstub.h"
#include "customwidget.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

class TestProgressPage : public ::testing::Test
{
public:
    TestProgressPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProgressPage;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ProgressPage *m_tester;
};

TEST_F(TestProgressPage, initTest)
{

}

TEST_F(TestProgressPage, testsetProgressType_Compress)
{
    m_tester->setProgressType(PT_Compress);
    ASSERT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "compress") + ": " + QObject::tr("Calculating...")));
}

TEST_F(TestProgressPage, testsetProgressType_Delete)
{
    m_tester->setProgressType(PT_Delete);
    ASSERT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "delete") + ": " + QObject::tr("Calculating...")));
}

TEST_F(TestProgressPage, testsetProgressType_Convert)
{
    m_tester->setProgressType(PT_Convert);
    ASSERT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "convert") + ": " + QObject::tr("Calculating...")));
}

TEST_F(TestProgressPage, testsetProgressType_Comment)
{
    m_tester->setProgressType(PT_Comment);
    ASSERT_EQ(m_tester->m_pSpeedLbl->text(), "");
}

TEST_F(TestProgressPage, testsetProgressType_Other)
{
    m_tester->setProgressType(PT_UnCompress);
    ASSERT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "uncompress") + ": " + QObject::tr("Calculating...")));
}

TEST_F(TestProgressPage, testsetTotalSize)
{
    m_tester->m_qTotalSize = 0;
    m_tester->setTotalSize(1);
    ASSERT_EQ(m_tester->m_qTotalSize, 1);
}

TEST_F(TestProgressPage, testsetArchiveName)
{
    m_tester->m_strArchiveName.clear();
    m_tester->setArchiveName("123");
    ASSERT_EQ(m_tester->m_strArchiveName, "123");
}

TEST_F(TestProgressPage, testarchiveName)
{
    m_tester->m_strArchiveName = "123";
    ASSERT_EQ(m_tester->archiveName(), "123");
}

TEST_F(TestProgressPage, testsetProgress)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    m_tester->m_iPerent = 50;
    m_tester->setProgress(40);
    ASSERT_EQ(m_tester->m_iPerent, 50);
}

TEST_F(TestProgressPage, testsetProgress_normal)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    m_tester->m_iPerent = 50;
    m_tester->setProgress(60);
    ASSERT_EQ(m_tester->m_iPerent, 60);
}

TEST_F(TestProgressPage, testsetCurrentFileName_Compress)
{
    m_tester->m_eType = PT_Compress;
    m_tester->setCurrentFileName("11");
    ASSERT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Compressing"), true);
}

TEST_F(TestProgressPage, testsetCurrentFileName_Delete)
{
    m_tester->m_eType = PT_Delete;
    m_tester->setCurrentFileName("11");
    ASSERT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Deleting"), true);
}

TEST_F(TestProgressPage, testsetCurrentFileName_Convert)
{
    m_tester->m_eType = PT_Convert;
    m_tester->setCurrentFileName("11");
    ASSERT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Converting"), true);
}

TEST_F(TestProgressPage, testsetCurrentFileName_Comment)
{
    m_tester->m_eType = PT_Comment;
    m_tester->setCurrentFileName("11");
    ASSERT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Updating the comment"), true);
}

TEST_F(TestProgressPage, testsetCurrentFileName_Other)
{
    m_tester->m_eType = PT_None;
    m_tester->setCurrentFileName("11");
    ASSERT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Extracting"), true);
}

TEST_F(TestProgressPage, testresetProgress_Comment)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    m_tester->m_eType = PT_Comment;
    m_tester->resetProgress();
    ASSERT_EQ(m_tester->m_qConsumeTime, 0);
}

TEST_F(TestProgressPage, testresetProgress_Other)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    m_tester->m_eType = PT_None;
    m_tester->resetProgress();
    ASSERT_EQ(m_tester->m_qConsumeTime, 0);
}

TEST_F(TestProgressPage, testresetProgress)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    m_tester->restartTimer();
}

TEST_F(TestProgressPage, testsetPushButtonCheckable)
{
    m_tester->setPushButtonCheckable(true, true);
    ASSERT_EQ(m_tester->m_pPauseContinueButton->isEnabled(), true);
}

TEST_F(TestProgressPage, testcalSpeedAndRemainingTime_return)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 0);

    m_tester->m_qConsumeTime = -1;
    m_tester->m_qTotalSize = 0;
    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    ASSERT_EQ(dSpeed, 0);
}

TEST_F(TestProgressPage, testcalSpeedAndRemainingTime_0ms)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 0);

    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    ASSERT_EQ(dSpeed, 0);
}

TEST_F(TestProgressPage, testcalSpeedAndRemainingTime_Convert)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 10);

    m_tester->m_qConsumeTime = 0;
    m_tester->m_qTotalSize = 1024;
    m_tester->m_iPerent = 100;
    m_tester->m_eType = PT_Convert;
    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    ASSERT_EQ(dSpeed, 200);
}

TEST_F(TestProgressPage, testcalSpeedAndRemainingTime_Other)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 10);

    m_tester->m_qConsumeTime = 0;
    m_tester->m_qTotalSize = 1024;
    m_tester->m_iPerent = 100;
    m_tester->m_eType = PT_Compress;
    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    ASSERT_EQ(dSpeed, 100);
}

//TEST_F(TestProgressPage, testdisplaySpeedAndTime_Compress)
//{
//    double dSpeed = 20;
//    qint64 qRemainingTime = 10;
//    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
//    ASSERT_EQ(dSpeed, 100);
//}

//TEST_F(TestProgressPage, testdisplaySpeedAndTime_Delete)
//{
//    double dSpeed = 20;
//    qint64 qRemainingTime = 10;
//    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
//    ASSERT_EQ(dSpeed, 100);
//}

//TEST_F(TestProgressPage, testdisplaySpeedAndTime_UnCompress)
//{
//    double dSpeed = 20;
//    qint64 qRemainingTime = 10;
//    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
//    ASSERT_EQ(dSpeed, 100);
//}

//TEST_F(TestProgressPage, testdisplaySpeedAndTime_Convert)
//{
//    double dSpeed = 20;
//    qint64 qRemainingTime = 10;
//    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
//    ASSERT_EQ(dSpeed, 100);
//}

//TEST_F(TestProgressPage, testdisplaySpeedAndTime_Comment)
//{
//    double dSpeed = 20;
//    qint64 qRemainingTime = 10;
//    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
//    ASSERT_EQ(dSpeed, 100);
//}
