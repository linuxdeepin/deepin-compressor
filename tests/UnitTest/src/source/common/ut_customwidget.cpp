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
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QTest>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CustomSuggestButton
class TestCustomSuggestButton : public ::testing::Test
{
public:
    TestCustomSuggestButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomSuggestButton();
        m_tester1 = new CustomSuggestButton("1");
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

TEST_F(TestCustomSuggestButton, initTest)
{

}

TEST_F(TestCustomSuggestButton, testkeyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(TestCustomSuggestButton, testkeyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomCombobox
class TestCustomCombobox : public ::testing::Test
{
public:
    TestCustomCombobox(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCombobox();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomCombobox *m_tester;
};

TEST_F(TestCustomCombobox, initTest)
{

}

TEST_F(TestCustomCombobox, testkeyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(TestCustomCombobox, testkeyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomPushButton
class TestCustomPushButton : public ::testing::Test
{
public:
    TestCustomPushButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomPushButton();
        m_tester1 = new CustomPushButton("1");
        m_tester2 = new CustomPushButton(QIcon(), "1");
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_tester1;
        delete m_tester2;
    }

protected:
    CustomPushButton *m_tester;
    CustomPushButton *m_tester1;
    CustomPushButton *m_tester2;
};

TEST_F(TestCustomPushButton, initTest)
{

}

TEST_F(TestCustomPushButton, testkeyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(TestCustomPushButton, testkeyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomPushButton
class TestCustomSwitchButton : public ::testing::Test
{
public:
    TestCustomSwitchButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomSwitchButton();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomSwitchButton *m_tester;
};

TEST_F(TestCustomSwitchButton, initTest)
{

}

TEST_F(TestCustomSwitchButton, testkeyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

// 测试CustomCheckBox
class TestCustomCheckBox : public ::testing::Test
{
public:
    TestCustomCheckBox(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCheckBox();
        m_tester1 = new CustomCheckBox("1");
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_tester1;
    }

protected:
    CustomCheckBox *m_tester;
    CustomCheckBox *m_tester1;
};

TEST_F(TestCustomCheckBox, initTest)
{

}

TEST_F(TestCustomCheckBox, testkeyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(TestCustomCheckBox, testkeyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomCommandLinkButton
class TestCustomCommandLinkButton : public ::testing::Test
{
public:
    TestCustomCommandLinkButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCommandLinkButton("1");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomCommandLinkButton *m_tester;
};

TEST_F(TestCustomCommandLinkButton, initTest)
{

}

TEST_F(TestCustomCommandLinkButton, testkeyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(TestCustomCommandLinkButton, testkeyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomFloatingMessage
class TestCustomFloatingMessage : public ::testing::Test
{
public:
    TestCustomFloatingMessage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomFloatingMessage(QIcon(), "1");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomFloatingMessage *m_tester;
};

TEST_F(TestCustomFloatingMessage, initTest)
{

}

