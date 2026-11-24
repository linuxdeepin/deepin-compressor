// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encoding_utils.h"

#include "encoding_detector.h"

#include <QByteArray>
#include <QTextCodec>

namespace dcarchive {

QString decodeArchiveName(const char *data, int size)
{
    if (!data || size <= 0)
        return {};

    const QByteArray bytes(data, size);
    const encdet::Result detected = encdet::detect(data, size_t(size));
    if (detected.encoding == "UTF-8" || detected.encoding == "ASCII")
        return QString::fromUtf8(bytes);

    if (QTextCodec *codec = QTextCodec::codecForName(detected.encoding.c_str()))
        return codec->toUnicode(bytes);

    return QString::fromUtf8(bytes);
}

} // namespace dcarchive
