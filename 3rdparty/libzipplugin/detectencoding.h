#ifndef DETECTENCODING_H
#define DETECTENCODING_H


#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <QLatin1String>
#include <QDebug>

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
