/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenglu <chenglu@uniontech.com>
*
* Maintainer: chenglu <chenglu@uniontech.com>
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
#include "popupdialog.h"
#include "customwidget.h"

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

TEST_F(TestProgressPage, setProgressType1)
{
    m_tester->setProgressType(PT_Compress);
}

TEST_F(TestProgressPage, setProgressType2)
{
    m_tester->setProgressType(PT_Delete);
}

TEST_F(TestProgressPage, setProgressType3)
{
    m_tester->setProgressType(PT_Convert);
}

TEST_F(TestProgressPage, setProgressType4)
{
    m_tester->setProgressType(PT_Comment);
}

TEST_F(TestProgressPage, setProgressType5)
{
    m_tester->setProgressType(PT_UnCompress);
}

TEST_F(TestProgressPage, setTotalSize)
{
    m_tester->setTotalSize(10);
    EXPECT_EQ(m_tester->m_qTotalSize, 10);
}

TEST_F(TestProgressPage, setArchiveName)
{
    m_tester->setArchiveName("utTest.zip");
}

TEST_F(TestProgressPage, setProgress1)
{
    m_tester->m_iPerent = 0;
    m_tester->setProgress(0);
}

TEST_F(TestProgressPage, setProgress2)
{
    m_tester->m_iPerent = 10;
    m_tester->setProgress(50);
}

TEST_F(TestProgressPage, setCurrentFileName1)
{
    m_tester->m_eType = PT_Compress;
    m_tester->setCurrentFileName("1.txt");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Compressing"), true);
}

TEST_F(TestProgressPage, setCurrentFileName2)
{
    m_tester->m_eType = PT_Delete;
    m_tester->setCurrentFileName("1.txt");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Deleting"), true);
}

TEST_F(TestProgressPage, setCurrentFileName3)
{
    m_tester->m_eType = PT_Convert;
    m_tester->setCurrentFileName("1.txt");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Converting"), true);
}

TEST_F(TestProgressPage, setCurrentFileName4)
{
    m_tester->m_eType = PT_Comment;
    m_tester->setCurrentFileName("1.txt");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Updating the comment"), true);
}

TEST_F(TestProgressPage, setCurrentFileName5)
{
    m_tester->m_eType = PT_UnCompress;
    m_tester->setCurrentFileName("1.txt");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Extracting"), true);
}

TEST_F(TestProgressPage, resetProgress1)
{
    m_tester->m_eType = PT_Comment;
    m_tester->resetProgress();
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Updating the comment"), true);
}

TEST_F(TestProgressPage, resetProgress2)
{
    m_tester->m_eType = PT_UnCompress;
    m_tester->resetProgress();
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Calculating"), true);
}

TEST_F(TestProgressPage, restartTimer)
{
    m_tester->restartTimer();
}

TEST_F(TestProgressPage, initUI)
{
    m_tester->initUI();
}

TEST_F(TestProgressPage, initConnections)
{
    m_tester->initConnections();
}

TEST_F(TestProgressPage, calSpeedAndRemainingTime1)
{
    double speed = 100;
    qint64 time = 10;
    m_tester->m_timer.start();
    m_tester->m_qConsumeTime = -1;
    m_tester->m_eType = PT_UnCompress;
    m_tester->m_qTotalSize = 10240;
    m_tester->m_iPerent = 10;
    m_tester->calSpeedAndRemainingTime(speed, time);
//    qInfo() << speed; // -1000
//    qInfo() <<  time; // 1
//    EXPECT_EQ(speed, -1000);
//    EXPECT_EQ(time, 1);
    EXPECT_EQ(time, 1);
}

TEST_F(TestProgressPage, calSpeedAndRemainingTime2)
{
    double speed = 100;
    qint64 time = 10;
    m_tester->m_timer.start();
    m_tester->m_qConsumeTime = 0;
    m_tester->m_eType = PT_UnCompress;
    m_tester->m_qTotalSize = 10240;
    m_tester->m_iPerent = 10;
    m_tester->calSpeedAndRemainingTime(speed, time);
//    qInfo() << "tttt " << time; // 10
    EXPECT_EQ(speed, 0.0);
    EXPECT_EQ(time, 10);
}

TEST_F(TestProgressPage, calSpeedAndRemainingTime3)
{
    double speed = 100;
    qint64 time = 10;
    m_tester->m_timer.start();
    m_tester->m_qConsumeTime = 10000;
    m_tester->m_eType = PT_UnCompress;
    m_tester->m_qTotalSize = 10240000;
    m_tester->m_iPerent = 10;
    m_tester->calSpeedAndRemainingTime(speed, time);
    EXPECT_EQ(speed, 100);
    EXPECT_EQ(time, 90);
}

TEST_F(TestProgressPage, calSpeedAndRemainingTime4)
{
    double speed = 100;
    qint64 time = 10;
    m_tester->m_timer.start();
    m_tester->m_qConsumeTime = 10000;
    m_tester->m_eType = PT_Convert;
    m_tester->m_qTotalSize = 10240000;
    m_tester->m_iPerent = 10;
    m_tester->calSpeedAndRemainingTime(speed, time);
    EXPECT_EQ(speed, 200);
    EXPECT_EQ(time, 90);
}

TEST_F(TestProgressPage, displaySpeedAndTime1)
{
    m_tester->m_eType = PT_Compress;
    m_tester->displaySpeedAndTime(1000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("1000.00KB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime2)
{
    m_tester->m_eType = PT_Compress;
    m_tester->displaySpeedAndTime(10240, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("10.00MB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime3)
{
    m_tester->m_eType = PT_Compress;
    m_tester->displaySpeedAndTime(400000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith(">300MB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime4)
{
    m_tester->m_eType = PT_Delete;
    m_tester->displaySpeedAndTime(1000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("1000.00KB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime5)
{
    m_tester->m_eType = PT_Delete;
    m_tester->displaySpeedAndTime(10240, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("10.00MB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime6)
{
    m_tester->m_eType = PT_UnCompress;
    m_tester->displaySpeedAndTime(1000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("1000.00KB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime7)
{
    m_tester->m_eType = PT_UnCompress;
    m_tester->displaySpeedAndTime(10240, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("10.00MB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime8)
{
    m_tester->m_eType = PT_UnCompress;
    m_tester->displaySpeedAndTime(400000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith(">300MB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime9)
{
    m_tester->m_eType = PT_Convert;
    m_tester->displaySpeedAndTime(1000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("1000.00KB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime10)
{
    m_tester->m_eType = PT_Convert;
    m_tester->displaySpeedAndTime(10240, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith("10.00MB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime11)
{
    m_tester->m_eType = PT_Convert;
    m_tester->displaySpeedAndTime(400000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().endsWith("00:01:40"), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().endsWith(">300MB/s"), true);
}

TEST_F(TestProgressPage, displaySpeedAndTime12)
{
    m_tester->m_eType = PT_Comment;
    m_tester->displaySpeedAndTime(1000, 100);
    EXPECT_EQ(m_tester->m_pRemainingTimeLbl->text().isEmpty(), true);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text().isEmpty(), true);
}

TEST_F(TestProgressPage, slotPauseClicked1)
{
    m_tester->slotPauseClicked(true);
    EXPECT_EQ(m_tester->m_pPauseContinueButton->text() == "Continue", true);
}

TEST_F(TestProgressPage, slotPauseClicked2)
{
    m_tester->slotPauseClicked(false);
    EXPECT_EQ(m_tester->m_pPauseContinueButton->text() == "Pause", true);
}

TEST_F(TestProgressPage, slotCancelClicked)
{
//    m_tester->slotCancelClicked();
}
