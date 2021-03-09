/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:
*
* Maintainer:
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

#ifndef DETECTENCODING_H
#define DETECTENCODING_H


#include <QLatin1String>
#include <QDebug>

#include <stdlib.h>
#include <string.h>

#include <memory.h>

/* Type for Unicode characters */
typedef unsigned long unichar;

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#ifdef DEBUG_ENCODING
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif



EXTERN_C int file_encoding(
    const unsigned char *buf,
    size_t nbytes,  QString &code,  QString &code_mime,  QString &type
//    const char **code,
//    const char **code_mime,
//    const char **type
);

class DetectEncoding
{
public:
    DetectEncoding();

    static int looks_ascii(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen);
    static int looks_utf8_with_BOM(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen);
    static int looks_utf7(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen);
    static int looks_ucs16(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen);
    static int looks_latin1(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen);
    static int looks_extended(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen);
    static int file_looks_utf8(const unsigned char *buf, size_t nbytes, unichar *ubuf, size_t *ulen);
    static void from_ebcdic(const unsigned char *buf, size_t nbytes, unsigned char *out);
};

#endif // DETECTENCODING_H
