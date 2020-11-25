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
#include "common.h"
#include "../ChardetDetector/chardet.h"
#include <iostream>
#include <QTextCodec>
#include <QRegularExpressionMatch>
#include <QMimeType>

QString strCaptured = "charset";
QLocale::Script myScript = QLocale::ArabicScript;

TEST(Common_trans2uft8_UT, Common_trans2uft8_UT001)
{
    Common common(nullptr);
    QString str = common.trans2uft8("古风");
    int ret = str.compare("古风");

    ASSERT_EQ(ret, 0);
}

TEST(Common_trans2uft8_UT, Common_trans2uft8_UT002)
{
    QByteArray (*mydetectEncode)(const QByteArray &, const QString &) =
        [](const QByteArray &data, const QString &fileName) -> QByteArray {
        Q_UNUSED(data);
        Q_UNUSED(fileName)
        return "gb18030";
    };

    Stub *stub = new Stub();
    stub->set(ADDR(Common, detectEncode), mydetectEncode);

    Common common(nullptr);
    QString str = common.trans2uft8("你好");
    int ret = str.compare("浣犲ソ");

    stub->reset(ADDR(Common, detectEncode));
    delete stub;
    ASSERT_EQ(ret, 0);
}

TEST(Common_trans2uft8_UT, Common_trans2uft8_UT003)
{
    QByteArray (*mydetectEncode)(const QByteArray &, const QString &) =
        [](const QByteArray &data, const QString &fileName) -> QByteArray {
        Q_UNUSED(data);
        Q_UNUSED(fileName)
        return "windows";
    };

    Stub *stub = new Stub();
    stub->set(ADDR(Common, detectEncode), mydetectEncode);

    Common common(nullptr);
    QString str = common.trans2uft8("你好");
    int ret = str.compare("浣犲ソ");

    stub->reset(ADDR(Common, detectEncode));
    delete stub;
    ASSERT_EQ(ret, 0);
}

TEST(Common_trans2uft8_UT, Common_trans2uft8_UT004)
{
    QByteArray (*mydetectEncode)(const QByteArray &, const QString &) =
        [](const QByteArray &data, const QString &fileName) -> QByteArray {
        Q_UNUSED(data);
        Q_UNUSED(fileName)
        return "KOI8-R";
    };

    Stub *stub = new Stub();
    stub->set(ADDR(Common, detectEncode), mydetectEncode);

    Common common(nullptr);
    QString str = common.trans2uft8("你好");
    int ret = str.compare("Д╫═Е╔╫");

    stub->reset(ADDR(Common, detectEncode));
    delete stub;
    ASSERT_EQ(ret, 0);
}

TEST(Common_trans2uft8_UT, Common_trans2uft8_UT005)
{
    QByteArray (*mydetectEncode)(const QByteArray &, const QString &) =
        [](const QByteArray &data, const QString &fileName) -> QByteArray {
        Q_UNUSED(data);
        Q_UNUSED(fileName)
        return "UTF";
    };

    Stub *stub = new Stub();
    stub->set(ADDR(Common, detectEncode), mydetectEncode);

    Common common(nullptr);
    QString str = common.trans2uft8("你好");
    int ret = str.compare("你好");

    stub->reset(ADDR(Common, detectEncode));
    delete stub;
    ASSERT_EQ(ret, 0);
}

TEST(Common_detectEncode_UT, Common_detectEncode_UT001)
{
    QByteArray (*mydetectEncode)(const QByteArray &, const QString &) =
        [](const QByteArray &data, const QString &fileName) -> QByteArray {
        Q_UNUSED(data);
        Q_UNUSED(fileName)
        return "test";
    };
    QByteArray (*mytextCodecDetect)(const QByteArray &, const QString &) =
        [](const QByteArray &data, const QString &fileName) -> QByteArray {
        Q_UNUSED(data);
        Q_UNUSED(fileName)
        return "test";
    };
    Stub *stub = new Stub();
    stub->set(ADDR(Common, detectEncode), mydetectEncode);
    stub->set(ADDR(Common, textCodecDetect), mytextCodecDetect);

    Common common(nullptr);
    std::string str = common.detectEncode("古风", "").toStdString();

    stub->reset(ADDR(Common, detectEncode));
    stub->reset(ADDR(Common, textCodecDetect));
    delete stub;
    ASSERT_STREQ(str.c_str(), "test");
}

TEST(Common_detectEncode_UT, Common_detectEncode_UT002)
{
    int (*myChartDet_DetectingTextCoding)(void *, const char *, QString &, float &) =
        [](void *obj, const char *str, QString &encoding, float &confidence) -> int {
        Q_UNUSED(obj);
        Q_UNUSED(str);
        Q_UNUSED(confidence);
        encoding = QString("ASCII");
        return 0;
    };
    Stub *stub = new Stub();
    stub->set(ADDR(Common, ChartDet_DetectingTextCoding), myChartDet_DetectingTextCoding);

    Common common(nullptr);
    std::string str = common.detectEncode("古风", "").toStdString();

    stub->reset(ADDR(Common, ChartDet_DetectingTextCoding));
    delete stub;
    ASSERT_STREQ(str.c_str(), "UTF-8");
}

TEST(Common_detectEncode_UT, Common_detectEncode_UT003)
{
    int (*myChartDet_DetectingTextCoding)(void *, const char *, QString &, float &) =
        [](void *obj, const char *str, QString &encoding, float &confidence) -> int {
        Q_UNUSED(obj);
        Q_UNUSED(str);
        Q_UNUSED(confidence);
        encoding = QString("windows");
        return 0;
    };
    Stub *stub = new Stub();
    stub->set(ADDR(Common, ChartDet_DetectingTextCoding), myChartDet_DetectingTextCoding);

    Common common(nullptr);
    std::string str = common.detectEncode("古风", "").toStdString();

    stub->reset(ADDR(Common, ChartDet_DetectingTextCoding));
    delete stub;
    ASSERT_STREQ(str.c_str(), "windows");
}

TEST(Common_detectEncode_UT, Common_detectEncode_UT004)
{
    int (*myChartDet_DetectingTextCoding)(void *, const char *, QString &, float &) =
        [](void *obj, const char *str, QString &encoding, float &confidence) -> int {
        Q_UNUSED(obj);
        Q_UNUSED(str);
        Q_UNUSED(confidence);
        encoding = QString("test");
        return 0;
    };
    QByteArray (*mytextCodecDetect)(void *, const QByteArray &, const QString &) =
        [](void *obj, const QByteArray &data, const QString &fileName) -> QByteArray {
        Q_UNUSED(obj);
        Q_UNUSED(data);
        Q_UNUSED(fileName);
        return "test";
    };
    Stub *stub = new Stub();
    stub->set(ADDR(Common, ChartDet_DetectingTextCoding), myChartDet_DetectingTextCoding);
    stub->set(ADDR(Common, textCodecDetect), mytextCodecDetect);

    Common common(nullptr);
    std::string str = common.detectEncode("古风", "").toStdString();

    stub->reset(ADDR(Common, ChartDet_DetectingTextCoding));
    stub->reset(ADDR(Common, textCodecDetect));
    delete stub;
    ASSERT_STREQ(str.c_str(), "test");
}

TEST(Common_ChartDet_DetectingTextCoding_UT, Common_ChartDet_DetectingTextCoding_UT001)
{
    DetectObj *(*mydetect_obj_init)(void) =
        []() -> DetectObj * {
        return nullptr;
    };
    Stub *stub = new Stub();
    stub->set(detect_obj_init, mydetect_obj_init);

    QString encoding("");
    float confidence = 0.0;
    Common common(nullptr);
    int ret = common.ChartDet_DetectingTextCoding("", encoding, confidence);

    stub->reset(detect_obj_init);
    delete stub;
    ASSERT_EQ(ret, CHARDET_MEM_ALLOCATED_FAIL);
}

TEST(Common_ChartDet_DetectingTextCoding_UT, Common_ChartDet_DetectingTextCoding_UT002)
{
    short (*mydetect)(const char *, DetectObj **) =
        [](const char *buf, DetectObj **obj) -> short {
        return CHARDET_OUT_OF_MEMORY;
    };
    Stub *stub = new Stub();
    stub->set(detect, mydetect);

    QString encoding("");
    float confidence = 0.0;
    Common common(nullptr);
    int ret = common.ChartDet_DetectingTextCoding("", encoding, confidence);

    stub->reset(detect);
    delete stub;
    ASSERT_EQ(ret, CHARDET_OUT_OF_MEMORY);
}

TEST(Common_ChartDet_DetectingTextCoding_UT, Common_ChartDet_DetectingTextCoding_UT003)
{
    short (*mydetect)(const char *, DetectObj **) =
        [](const char *buf, DetectObj **obj) -> short {
        return CHARDET_NULL_OBJECT;
    };
    Stub *stub = new Stub();
    stub->set(detect, mydetect);

    QString encoding("");
    float confidence = 0.0;
    Common common(nullptr);
    int ret = common.ChartDet_DetectingTextCoding("", encoding, confidence);

    stub->reset(detect);
    delete stub;
    ASSERT_EQ(ret, CHARDET_NULL_OBJECT);
}

TEST(Common_ChartDet_DetectingTextCoding_UT, Common_ChartDet_DetectingTextCoding_UT004)
{
    short (*mydetect)(const char *, DetectObj **) =
        [](const char *buf, DetectObj **obj) -> short {
        return CHARDET_SUCCESS;
    };
    Stub *stub = new Stub();
    stub->set(detect, mydetect);

    QString encoding("");
    float confidence = 0.0;
    Common common(nullptr);
    int ret = common.ChartDet_DetectingTextCoding("", encoding, confidence);

    stub->reset(detect);
    delete stub;
    ASSERT_EQ(ret, CHARDET_SUCCESS);
}

TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT001)
{
    Common common(nullptr);
    std::string str = common.textCodecDetect("", "").toStdString();

    ASSERT_STREQ(str.c_str(), "UTF-8");
}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT002)
//{
//    QTextCodec *(*mycodecForUtfText)(void *, const QByteArray &, QTextCodec *) =
//        [](void *job, const QByteArray &ba, QTextCodec *defaultCodec) -> QTextCodec * {
//        Q_UNUSED(job);
//        Q_UNUSED(ba);
//        Q_UNUSED(defaultCodec);
//        return QTextCodec::codecForName("Big5");
//    };
//    typedef QTextCodec *(*fptr)(const QByteArray &, QTextCodec *);
//    fptr A_foo = (fptr)(&QTextCodec::codecForUtfText);
//    Stub *stub = new Stub;
//    stub->set(A_foo, mycodecForUtfText);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect("古风", "").toStdString();

//    stub->reset(A_foo);
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "Big5");
//}

TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT003)
{
    Common common(nullptr);
    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();
    ASSERT_STREQ(str.c_str(), "UTF-8");
}

QString mycaptured(void *obj, const QString &name)
{
    Q_UNUSED(obj);
    if (strCaptured == "charset")
        return "test";
    else if (strCaptured == "language" && name == "language")
        return "test";
    else if (strCaptured == "coding" && name == "coding")
        return "coding";
    return "";
}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT004)
//{
//    strCaptured = "charset";
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "test");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT005)
//{
//    strCaptured = "language";
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "UTF-8");
//}

QLocale::Script myscript()
{
    return myScript;
}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT006)
//{
//    strCaptured = "language";
//    myScript = QLocale::ArabicScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT007)
//{
//    strCaptured = "language";
//    myScript = QLocale::SimplifiedChineseScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT008)
//{
//    strCaptured = "language";
//    myScript = QLocale::TraditionalChineseScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT009)
//{
//    strCaptured = "language";
//    myScript = QLocale::CyrillicScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT010)
//{
//    strCaptured = "language";
//    myScript = QLocale::GreekScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT011)
//{
//    strCaptured = "language";
//    myScript = QLocale::HebrewScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT012)
//{
//    strCaptured = "language";
//    myScript = QLocale::JapaneseScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT013)
//{
//    strCaptured = "language";
//    myScript = QLocale::KoreanScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT014)
//{
//    strCaptured = "language";
//    myScript = QLocale::ThaiScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    stub->reset(ADDR(QLocale, script));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
//}

QString myname()
{
    return "text/x-python";
}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT015)
//{
//    Stub *stub = new Stub;
//    stub->set(ADDR(QMimeType, name), myname);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset(ADDR(QMimeType, name));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "UTF-8");
//}

//TEST(Common_textCodecDetect_UT, Common_textCodecDetect_UT016)
//{
//    strCaptured = "coding";
//    Stub *stub = new Stub;
//    stub->set(ADDR(QMimeType, name), myname);
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);

//    Common common(nullptr);
//    std::string str = common.textCodecDetect(QString("<html></html>").toLatin1(), "").toStdString();

//    stub->reset(ADDR(QMimeType, name));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "coding");
//}
