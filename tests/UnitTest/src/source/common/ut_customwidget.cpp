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

#include "customwidget.h"

#include <gtest/gtest.h>
#include <QTest>

// 测试CustomSuggestButton
class TestCustomSuggestButton : public ::testing::Test
{
public:
    TestCustomSuggestButton() {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomSuggestButton;
        m_tester1 = new CustomSuggestButton("hh");
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_tester1;
    }

protected:
    CustomSuggestButton *m_tester;
    CustomSuggestButton *m_tester1;
};

// 测试CustomCombobox
class TestCustomCombobox : public ::testing::Test
{
public:
    TestCustomCombobox() {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCombobox;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomCombobox *m_tester;
};

// 测试CustomPushButton
class TestCustomPushButton : public ::testing::Test
{
public:
    TestCustomPushButton() {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomPushButton;
        m_tester1 = new CustomPushButton("hh");
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_tester1;
    }

protected:
    CustomPushButton *m_tester;
    CustomPushButton *m_tester1;
};

// 测试CustomSwitchButton
class TestCustomSwitchButton : public ::testing::Test
{
public:
    TestCustomSwitchButton() {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomSwitchButton;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomSwitchButton *m_tester;
};

// 测试CustomCheckBox
class TestCustomCheckBox : public ::testing::Test
{
public:
    TestCustomCheckBox() {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCheckBox;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomCheckBox *m_tester;
};

// 测试CustomCheckBox
class TestCustomCommandLinkButton : public ::testing::Test
{
public:
    TestCustomCommandLinkButton() {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCommandLinkButton("hh");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomCommandLinkButton *m_tester;
};

// 测试CustomSuggestButton
TEST_F(TestCustomSuggestButton, initTest)
{

}

TEST_F(TestCustomSuggestButton, testkeyPressEvent1)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomSuggestButton, testkeyPressEvent2)
{
    QTest::keyPress(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomSuggestButton, testkeyReleaseEvent1)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomSuggestButton, testkeyReleaseEvent2)
{
    QTest::keyRelease(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}


// 测试CustomCombobox
TEST_F(TestCustomCombobox, initTest)
{

}

TEST_F(TestCustomCombobox, testkeyPressEvent1)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCombobox, testkeyPressEvent2)
{
    QTest::keyPress(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCombobox, testkeyReleaseEvent1)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCombobox, testkeyReleaseEvent2)
{
    QTest::keyRelease(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}


// 测试CustomPushButton
TEST_F(TestCustomPushButton, initTest)
{

}

TEST_F(TestCustomPushButton, testkeyPressEvent1)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomPushButton, testkeyPressEvent2)
{
    QTest::keyPress(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomPushButton, testkeyReleaseEvent1)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomPushButton, testkeyReleaseEvent2)
{
    QTest::keyRelease(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}


// 测试CustomSwitchButton
TEST_F(TestCustomSwitchButton, initTest)
{

}

TEST_F(TestCustomSwitchButton, testkeyPressEvent1)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomSwitchButton, testkeyPressEvent2)
{
    QTest::keyPress(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}


// 测试CustomCheckBox
TEST_F(TestCustomCheckBox, initTest)
{

}

TEST_F(TestCustomCheckBox, testkeyPressEvent1)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCheckBox, testkeyPressEvent2)
{
    QTest::keyPress(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCheckBox, testkeyReleaseEvent1)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCheckBox, testkeyReleaseEvent2)
{
    QTest::keyRelease(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}


// 测试CustomCommandLinkButton
TEST_F(TestCustomCommandLinkButton, initTest)
{

}

TEST_F(TestCustomCommandLinkButton, testkeyPressEvent1)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCommandLinkButton, testkeyPressEvent2)
{
    QTest::keyPress(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCommandLinkButton, testkeyReleaseEvent1)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    ASSERT_NE(m_tester, nullptr);
}

TEST_F(TestCustomCommandLinkButton, testkeyReleaseEvent2)
{
    QTest::keyRelease(m_tester, Qt::Key_Delete);
    ASSERT_NE(m_tester, nullptr);
}
