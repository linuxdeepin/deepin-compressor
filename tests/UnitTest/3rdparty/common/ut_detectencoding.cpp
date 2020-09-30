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
#include "detectencoding.h"
#include "common.h"
#include "../ChardetDetector/chardet.h"
#include <iostream>
#include <QTextCodec>
#include <QRegularExpressionMatch>
#include <QMimeType>

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT001)
{
    QString code;
    QString code_mime; QString type;
    unsigned char *buf;
    short a = -12345;
    buf= (unsigned char*)&a;
    file_encoding(buf,0,code,code_mime,type);
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT002)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    file_encoding(buf,0,code,code_mime,type);
}

bool test_looks_utf7(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen)
{
    return true;
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT003)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_utf7), test_looks_utf7);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}

bool test_looks_ascii_false(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen)
{
    return false;
}

bool test_looks_utf8_with_BOM_true(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen)
{
    return true;
}

int test_file_looks_utf8_true(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    return 5;
}

int test_looks_ucs16_true(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    return 5;
}

int test_looks_ucs16_1(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    return 1;
}
int test_looks_latin1_false(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    return 0;
}

int test_looks_extended_true(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    return 1;
}
int test_looks_extended_false(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    return 0;
}
TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT004)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT005)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    stub->set(ADDR(DetectEncoding, looks_utf8_with_BOM), test_looks_utf8_with_BOM_true);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}


TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT006)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    stub->set(ADDR(DetectEncoding, file_looks_utf8), test_file_looks_utf8_true);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT007)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    stub->set(ADDR(DetectEncoding, looks_ucs16), test_looks_ucs16_true);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT008)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    stub->set(ADDR(DetectEncoding, looks_ucs16), test_looks_ucs16_1);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT009)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    stub->set(ADDR(DetectEncoding, looks_latin1), test_looks_latin1_false);
    stub->set(ADDR(DetectEncoding, looks_extended), test_looks_extended_true);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT010)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    stub->set(ADDR(DetectEncoding, looks_latin1), test_looks_latin1_false);
    stub->set(ADDR(DetectEncoding, looks_extended), test_looks_extended_false);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}
static int ilooks_ascii = 0;
int test_looks_ascii_false_true(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    if(ilooks_ascii ==0 )
    {
        ilooks_ascii++;
        return 0;
    }
    return ilooks_ascii;
}

static int ilooks_latin1 = 0;
int test_looks_latin1_false_true(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen){
    if(ilooks_latin1 ==0 )
    {
        ilooks_latin1++;
        return 0;
    }
    return ilooks_latin1;
}
TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT011)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false_true);
    stub->set(ADDR(DetectEncoding, looks_latin1), test_looks_latin1_false);
    stub->set(ADDR(DetectEncoding, looks_extended), test_looks_extended_false);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}

TEST(detectencoding_file_encoding_UT, detectencoding_file_encoding_UT012)
{
    QString code;
    QString code_mime; QString type;

     Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();

    Stub *stub = new Stub;
    stub->set(ADDR(DetectEncoding, looks_ascii), test_looks_ascii_false);
    stub->set(ADDR(DetectEncoding, looks_latin1), test_looks_latin1_false_true);
    stub->set(ADDR(DetectEncoding, looks_extended), test_looks_extended_false);
    file_encoding(buf,0,code,code_mime,type);
    delete stub;
}


TEST(detectencoding_DetectEncoding_UT, detectencoding_file_DetectEncoding_UT001)
{
    DetectEncoding * a = new DetectEncoding;
    delete a;
}

TEST(detectencoding_looks_ascii_UT, detectencoding_looks_ascii_UT001)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_ascii(buf,4,ubuf,i);
    delete i;
}


TEST(detectencoding_looks_utf8_with_BOM_UT, detectencoding_looks_utf8_with_BOM_UT001)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_utf8_with_BOM(buf,4,ubuf,i);
    delete i;
}


TEST(detectencoding_looks_utf7_UT, detectencoding_looks_utf7_UT001)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_utf7(buf,5,ubuf,i);
    delete i;
}


TEST(detectencoding_looks_ucs16_UT, detectencoding_looks_ucs16_UT001)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_ucs16(buf,1,ubuf,i);
    delete i;
}


TEST(detectencoding_looks_ucs16_UT, detectencoding_looks_ucs16_UT002)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_ucs16(buf,5,ubuf,i);
    delete i;
}

TEST(detectencoding_looks_latin1_UT, detectencoding_looks_latin1_UT001)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_latin1(buf,5,ubuf,i);
    delete i;
}

TEST(detectencoding_looks_latin1_UT, detectencoding_looks_latin1_UT002)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_latin1(buf,0,ubuf,i);
    delete i;
}
TEST(detectencoding_looks_extended_UT, detectencoding_looks_extended_UT001)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_extended(buf,5,ubuf,i);
    delete i;
}

TEST(detectencoding_looks_extended_UT, detectencoding_looks_extended_UT002)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("1234g");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unichar *ubuf = (unichar *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::looks_extended(buf,0,ubuf,i);
    delete i;
}



TEST(detectencoding_from_ebcdic_utf8_UT, detectencoding_from_ebcdic_UT002)
{
    QString code;
    QString code_mime; QString type;

    Common common(nullptr);
    QString a = common.trans2uft8("110xxxxx");
    QByteArray ba = a.toLatin1();
    unsigned char *buf= (unsigned char*)ba.data();
    unsigned char *ubuf = (unsigned char *)ba.data();
    unsigned long *i =new unsigned long(10);
    DetectEncoding::from_ebcdic(buf,5,ubuf);
    delete i;
}
